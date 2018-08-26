// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/common/tracing_impl.h"

#include "base/trace_event/trace_event_impl.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_impl.h"

namespace mojo {

TracingImpl::TracingImpl() {}

TracingImpl::~TracingImpl() {}

void TracingImpl::Initialize(ApplicationImpl* app) {
  ApplicationConnection* connection = app->ConnectToApplication("mojo:tracing");
  connection->AddService(this);

#ifdef NDEBUG
  if (app->HasArg("--early-tracing")) {
    provider_impl_.ForceEnableTracing();
  }
#else
  provider_impl_.ForceEnableTracing();
#endif
}

void TracingImpl::Create(ApplicationConnection* connection,
                         InterfaceRequest<tracing::TraceProvider> request) {
  provider_impl_.Bind(request.Pass());
}

}  // namespace mojo
