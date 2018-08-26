// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/shell_view.h"

#include "base/bind.h"
#include "base/single_thread_task_runner.h"
#include "sky/shell/gpu/rasterizer.h"
#include "sky/shell/platform_view.h"
#include "sky/shell/shell.h"
#include "sky/shell/ui/engine.h"

namespace sky {
namespace shell {
namespace {

template<typename T>
void Drop(scoped_ptr<T> ptr) { }

}  // namespace

ShellView::ShellView(Shell& shell)
    : shell_(shell) {
  shell_.tracing_controller().RegisterShellView(this);
  rasterizer_.reset(new Rasterizer());
  CreateEngine();
  CreatePlatformView();
}

ShellView::~ShellView() {
  shell_.tracing_controller().UnregisterShellView(this);
  shell_.gpu_task_runner()->PostTask(FROM_HERE,
      base::Bind(&Drop<Rasterizer>, base::Passed(&rasterizer_)));
  shell_.ui_task_runner()->PostTask(FROM_HERE,
      base::Bind(&Drop<Engine>, base::Passed(&engine_)));
}

void ShellView::CreateEngine() {
  Engine::Config config;
  config.service_provider_context = shell_.service_provider_context();
  config.gpu_task_runner = shell_.gpu_task_runner();
  config.gpu_delegate = rasterizer_->GetWeakPtr();
  engine_.reset(new Engine(config));
}

void ShellView::CreatePlatformView() {
  PlatformView::Config config;
  config.ui_task_runner = shell_.ui_task_runner();
  config.ui_delegate = engine_->GetWeakPtr();
  view_.reset(PlatformView::Create(config));
}

void ShellView::StartDartTracing() {
  shell_.ui_task_runner()->PostTask(
      FROM_HERE, base::Bind(&Engine::StartDartTracing, engine_->GetWeakPtr()));
}

void ShellView::StopDartTracing(
    mojo::ScopedDataPipeProducerHandle producer) {
  shell_.ui_task_runner()->PostTask(
      FROM_HERE, base::Bind(&Engine::StopDartTracing, engine_->GetWeakPtr(),
                            base::Passed(&producer)));
}

}  // namespace shell
}  // namespace sky
