// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session_connection.h"

#include "lib/fidl/cpp/optional.h"
#include "lib/ui/scenic/fidl_helpers.h"

namespace flutter {

SessionConnection::SessionConnection(
    fidl::InterfaceHandle<ui::Scenic> scenic_handle,
    std::string debug_label,
    zx::eventpair import_token,
    OnMetricsUpdate session_metrics_did_change_callback,
    fxl::Closure session_error_callback)
    : debug_label_(std::move(debug_label)),
      scenic_(scenic_handle.Bind()),
      session_(scenic_.get()),
      root_node_(&session_),
      surface_producer_(std::make_unique<VulkanSurfaceProducer>(&session_)),
      scene_update_context_(&session_, surface_producer_.get()),
      metrics_changed_callback_(
          std::move(session_metrics_did_change_callback)) {
  session_.set_error_handler(std::move(session_error_callback));
  session_.set_event_handler(std::bind(&SessionConnection::OnSessionEvents,
                                       this, std::placeholders::_1));

  root_node_.Bind(std::move(import_token));
  root_node_.SetEventMask(gfx::kMetricsEventMask);
  session_.Present(0, [](auto) {});
}

SessionConnection::~SessionConnection() = default;

void SessionConnection::OnSessionEvents(fidl::VectorPtr<ui::Event> events) {
  using Type = gfx::Event::Tag;

  for (auto& raw_event : *events) {
    if (!raw_event.is_gfx()) {
      continue;
    }

    auto& event = raw_event.gfx();

    switch (event.Which()) {
      case Type::kMetrics: {
        if (event.metrics().node_id == root_node_.id()) {
          auto& metrics = event.metrics().metrics;
          double device_pixel_ratio = metrics.scale_x;
          scene_update_context_.set_metrics(
              fidl::MakeOptional(std::move(metrics)));
          if (metrics_changed_callback_) {
            metrics_changed_callback_(device_pixel_ratio);
          }
        }
      } break;
      default:
        break;
    }
  }
}

void SessionConnection::Present(flow::CompositorContext::ScopedFrame& frame) {
  // Flush all session ops. Paint tasks have not yet executed but those are
  // fenced. The compositor can start processing ops while we finalize paint
  // tasks.
  session_.Present(0,           // presentation_time. (placeholder).
                   [](auto) {}  // callback
  );

  // Execute paint tasks and signal fences.
  auto surfaces_to_submit = scene_update_context_.ExecutePaintTasks(frame);

  // Tell the surface producer that a present has occurred so it can perform
  // book-keeping on buffer caches.
  surface_producer_->OnSurfacesPresented(std::move(surfaces_to_submit));

  // Prepare for the next frame. These ops won't be processed till the next
  // present.
  EnqueueClearOps();
}

void SessionConnection::EnqueueClearOps() {
  // We are going to be sending down a fresh node hierarchy every frame. So just
  // enqueue a detach op on the imported root node.
  session_.Enqueue(scenic_lib::NewDetachChildrenCommand(root_node_.id()));
}

}  // namespace flutter
