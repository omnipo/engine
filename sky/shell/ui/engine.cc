// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/ui/engine.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/threading/worker_pool.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "mojo/data_pipe_utils/data_pipe_utils.h"
#include "mojo/public/cpp/application/connect.h"
#include "services/asset_bundle/asset_unpacker_job.h"
#include "sky/engine/public/platform/WebInputEvent.h"
#include "sky/engine/public/platform/sky_display_metrics.h"
#include "sky/engine/public/platform/sky_display_metrics.h"
#include "sky/engine/public/web/Sky.h"
#include "sky/engine/public/web/WebRuntimeFeatures.h"
#include "sky/shell/dart/dart_library_provider_files.h"
#include "sky/shell/dart/dart_library_provider_network.h"
#include "sky/shell/service_provider.h"
#include "sky/shell/switches.h"
#include "sky/shell/ui/animator.h"
#include "sky/shell/ui/input_event_converter.h"
#include "sky/shell/ui/internals.h"
#include "sky/shell/ui/platform_impl.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"

namespace sky {
namespace shell {
namespace {

const char kSnapshotKey[] = "snapshot_blob.bin";

void Ignored(bool) {
}

mojo::ScopedDataPipeConsumerHandle Fetch(const base::FilePath& path) {
  mojo::DataPipe pipe;
  auto runner = base::WorkerPool::GetTaskRunner(true);
  mojo::common::CopyFromFile(base::FilePath(path), pipe.producer_handle.Pass(),
                             0, runner.get(), base::Bind(&Ignored));
  return pipe.consumer_handle.Pass();
}

PlatformImpl* g_platform_impl = nullptr;

}  // namespace

using mojo::asset_bundle::AssetUnpackerJob;

Engine::Config::Config() {
}

Engine::Config::~Config() {
}

Engine::Engine(const Config& config)
    : config_(config),
      animator_(new Animator(config, this)),
      binding_(this),
      activity_running_(false),
      have_surface_(false),
      weak_factory_(this) {
  mojo::ServiceProviderPtr service_provider =
      CreateServiceProvider(config.service_provider_context);
  mojo::ConnectToService(service_provider.get(), &network_service_);

#if defined(OS_ANDROID) || defined(OS_IOS)
  // TODO(abarth): Implement VSyncProvider on other platforms.
  vsync::VSyncProviderPtr vsync_provider;
  mojo::ConnectToService(service_provider.get(), &vsync_provider);
  animator_->set_vsync_provider(vsync_provider.Pass());
#endif
}

Engine::~Engine() {
}

base::WeakPtr<Engine> Engine::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void Engine::Init() {
  TRACE_EVENT0("sky", "Engine::Init");

  base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  blink::WebRuntimeFeatures::enableDartCheckedMode(
      command_line.HasSwitch(switches::kEnableCheckedMode));

  DCHECK(!g_platform_impl);
  g_platform_impl = new PlatformImpl();
  blink::initialize(g_platform_impl);
}

std::unique_ptr<compositor::LayerTree> Engine::BeginFrame(
    base::TimeTicks frame_time) {
  TRACE_EVENT0("sky", "Engine::BeginFrame");

  if (!sky_view_)
    return nullptr;

  auto begin_time = base::TimeTicks::Now();
  std::unique_ptr<compositor::LayerTree> layer_tree =
      sky_view_->BeginFrame(frame_time);
  if (layer_tree) {
    layer_tree->set_frame_size(
        SkISize::Make(physical_size_.width(), physical_size_.height()));
    layer_tree->set_construction_time(base::TimeTicks::Now() - begin_time);
  }
  return layer_tree;
}

void Engine::ConnectToEngine(mojo::InterfaceRequest<SkyEngine> request) {
  binding_.Bind(request.Pass());
}

void Engine::OnAcceleratedWidgetAvailable(gfx::AcceleratedWidget widget) {
  config_.gpu_task_runner->PostTask(
      FROM_HERE, base::Bind(&GPUDelegate::OnAcceleratedWidgetAvailable,
                            config_.gpu_delegate, widget));
  have_surface_ = true;
  StartAnimatorIfPossible();
  if (sky_view_)
    ScheduleFrame();
}

void Engine::OnOutputSurfaceDestroyed() {
  have_surface_ = false;
  StopAnimator();
  config_.gpu_task_runner->PostTask(
      FROM_HERE,
      base::Bind(&GPUDelegate::OnOutputSurfaceDestroyed, config_.gpu_delegate));
}

void Engine::OnViewportMetricsChanged(ViewportMetricsPtr metrics) {
  physical_size_.SetSize(metrics->physical_width, metrics->physical_height);

  display_metrics_.physical_size = physical_size_;
  display_metrics_.device_pixel_ratio = metrics->device_pixel_ratio;
  display_metrics_.padding_top = metrics->padding_top;
  display_metrics_.padding_right = metrics->padding_right;
  display_metrics_.padding_bottom = metrics->padding_bottom;
  display_metrics_.padding_left = metrics->padding_left;

  if (sky_view_)
    sky_view_->SetDisplayMetrics(display_metrics_);
}

void Engine::OnInputEvent(InputEventPtr event) {
  TRACE_EVENT0("sky", "Engine::OnInputEvent");
  scoped_ptr<blink::WebInputEvent> web_event =
      ConvertEvent(event, display_metrics_.device_pixel_ratio);
  if (!web_event)
    return;
  if (sky_view_)
    sky_view_->HandleInputEvent(*web_event);
}

void Engine::OnPointerPacket(pointer::PointerPacketPtr packet) {
  // TODO(abarth): Process pointer events in packets.
}

void Engine::RunFromLibrary(const std::string& name) {
  sky_view_ = blink::SkyView::Create(this);
  sky_view_->CreateView(blink::WebString::fromUTF8(name));
  sky_view_->RunFromLibrary(blink::WebString::fromUTF8(name),
                            dart_library_provider_.get());
  sky_view_->SetDisplayMetrics(display_metrics_);
}

void Engine::RunFromSnapshotStream(
    const std::string& name,
    mojo::ScopedDataPipeConsumerHandle snapshot) {
  sky_view_ = blink::SkyView::Create(this);
  sky_view_->CreateView(blink::WebString::fromUTF8(name));
  sky_view_->RunFromSnapshot(blink::WebString::fromUTF8(name), snapshot.Pass());
  sky_view_->SetDisplayMetrics(display_metrics_);
}

void Engine::RunFromPrecompiledSnapshot(const mojo::String& bundle_path) {
  AssetUnpackerJob* unpacker = new AssetUnpackerJob(
      mojo::GetProxy(&root_bundle_), base::WorkerPool::GetTaskRunner(true));
  std::string path_str = bundle_path;
  unpacker->Unpack(Fetch(base::FilePath(path_str)));

  sky_view_ = blink::SkyView::Create(this);
  sky_view_->CreateView("http://localhost");
  sky_view_->RunFromPrecompiledSnapshot();
  sky_view_->SetDisplayMetrics(display_metrics_);
}

void Engine::RunFromNetwork(const mojo::String& url) {
  dart_library_provider_.reset(
      new DartLibraryProviderNetwork(network_service_.get()));
  RunFromLibrary(url);
}

void Engine::RunFromFile(const mojo::String& main,
                         const mojo::String& package_root) {
  std::string package_root_str = package_root;
  dart_library_provider_.reset(
      new DartLibraryProviderFiles(base::FilePath(package_root_str)));
  RunFromLibrary(main);
}

void Engine::RunFromSnapshot(const mojo::String& path) {
  std::string path_str = path;
  RunFromSnapshotStream(path_str, Fetch(base::FilePath(path_str)));
}

void Engine::RunFromBundle(const mojo::String& path) {
  AssetUnpackerJob* unpacker = new AssetUnpackerJob(
      mojo::GetProxy(&root_bundle_), base::WorkerPool::GetTaskRunner(true));
  std::string path_str = path;
  unpacker->Unpack(Fetch(base::FilePath(path_str)));
  root_bundle_->GetAsStream(kSnapshotKey,
                            base::Bind(&Engine::RunFromSnapshotStream,
                                       weak_factory_.GetWeakPtr(), path_str));
}

void Engine::OnActivityPaused() {
  activity_running_ = false;
  StopAnimator();
}

void Engine::OnActivityResumed() {
  activity_running_ = true;
  StartAnimatorIfPossible();
}

void Engine::DidCreateIsolate(Dart_Isolate isolate) {
  Internals::Create(isolate,
                    CreateServiceProvider(config_.service_provider_context),
                    root_bundle_.Pass());
}

void Engine::StopAnimator() {
  animator_->Stop();
}

void Engine::StartAnimatorIfPossible() {
  if (activity_running_ && have_surface_)
    animator_->Start();
}

void Engine::ScheduleFrame() {
  animator_->RequestFrame();
}

void Engine::Render(std::unique_ptr<compositor::LayerTree> layer_tree) {
}

mojo::NavigatorHost* Engine::NavigatorHost() {
  return this;
}

void Engine::RequestNavigate(mojo::Target target,
                             mojo::URLRequestPtr request) {
  // Ignoring target for now.
  base::MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&Engine::RunFromNetwork, GetWeakPtr(), request->url));
}

void Engine::DidNavigateLocally(const mojo::String& url) {
}

void Engine::RequestNavigateHistory(int32_t delta) {
  NOTIMPLEMENTED();
}

void Engine::StartDartTracing() {
  sky_view_->StartDartTracing();
}

void Engine::StopDartTracing(mojo::ScopedDataPipeProducerHandle producer) {
  sky_view_->StopDartTracing(producer.Pass());
}

}  // namespace shell
}  // namespace sky
