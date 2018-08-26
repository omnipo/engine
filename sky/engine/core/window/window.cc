// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/core/window/window.h"

#include "sky/engine/core/compositing/Scene.h"
#include "sky/engine/core/events/Event.h"
#include "sky/engine/core/script/dom_dart_state.h"
#include "sky/engine/public/platform/sky_display_metrics.h"
#include "sky/engine/tonic/dart_converter.h"
#include "sky/engine/tonic/dart_invoke.h"
#include "sky/engine/tonic/dart_library_natives.h"

namespace blink {
namespace {

void ScheduleFrame(Dart_NativeArguments args) {
  DOMDartState::Current()->window()->client()->ScheduleFrame();
}

void Render(Dart_NativeArguments args) {
  Dart_Handle exception = nullptr;
  Scene* scene = DartConverter<Scene*>::FromArgumentsWithNullCheck(
      args, 1, exception);
  if (exception) {
    Dart_ThrowException(exception);
    return;
  }
  DOMDartState::Current()->window()->client()->Render(scene);
}

}  // namespace

WindowClient::~WindowClient() {
}

Window::Window(WindowClient* client)
  : client_(client) {
}

Window::~Window() {
}

void Window::DidCreateIsolate() {
  library_.Set(DartState::Current(), Dart_LookupLibrary(ToDart("dart:ui")));
}

void Window::UpdateWindowMetrics(const SkyDisplayMetrics& metrics) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  double device_pixel_ratio = metrics.device_pixel_ratio;
  double width = metrics.physical_size.width / device_pixel_ratio;
  double height = metrics.physical_size.height / device_pixel_ratio;

  DartInvokeField(library_.value(), "_updateWindowMetrics", {
    ToDart(device_pixel_ratio),
    ToDart(width),
    ToDart(height),
    ToDart(metrics.padding_top),
    ToDart(metrics.padding_right),
    ToDart(metrics.padding_bottom),
    ToDart(metrics.padding_left),
  });
}

void Window::DispatchEvent(Event* event) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  DartInvokeField(library_.value(), "_dispatchEvent", {
    ToDart(event),
  });
}

void Window::BeginFrame(base::TimeTicks frameTime) {
  DartState* dart_state = library_.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  int64_t microseconds = (frameTime - base::TimeTicks()).InMicroseconds();

  DartInvokeField(library_.value(), "_beginFrame", {
    Dart_NewInteger(microseconds),
  });
}

void Window::RegisterNatives(DartLibraryNatives* natives) {
  natives->Register({
    { "Window_scheduleFrame", ScheduleFrame, 1, true },
    { "Window_render", Render, 2, true },
  });
}

}  // namespace blink
