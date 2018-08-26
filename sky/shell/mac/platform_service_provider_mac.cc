// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/lazy_instance.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"
#include "sky/engine/wtf/Assertions.h"
#include "sky/services/ns_net/network_service_impl.h"
#include "sky/shell/service_provider.h"

#if TARGET_OS_IPHONE
#include "sky/services/keyboard/ios/keyboard_service_impl.h"
#include "sky/services/media/ios/media_service_impl.h"
#include "sky/services/media/ios/media_player_impl.h"
#include "sky/services/vsync/ios/vsync_provider_impl.h"
#endif

#if !TARGET_OS_IPHONE
#include "sky/shell/testing/test_runner.h"
#endif

namespace sky {
namespace shell {

class PlatformServiceProvider : public mojo::ServiceProvider {
 public:
  PlatformServiceProvider(mojo::InterfaceRequest<mojo::ServiceProvider> request)
      : binding_(this, request.Pass()) {}

  void ConnectToService(const mojo::String& service_name,
                        mojo::ScopedMessagePipeHandle client_handle) override {
    if (service_name == mojo::NetworkService::Name_) {
      network_.Create(nullptr, mojo::MakeRequest<mojo::NetworkService>(
                                   client_handle.Pass()));
    }
#if TARGET_OS_IPHONE
    if (service_name == ::keyboard::KeyboardService::Name_) {
      keyboard_.Create(nullptr, mojo::MakeRequest<::keyboard::KeyboardService>(
                                    client_handle.Pass()));
    }
    if (service_name == ::media::MediaPlayer::Name_) {
      media_player_.Create(nullptr, mojo::MakeRequest<::media::MediaPlayer>(
                                        client_handle.Pass()));
    }
    if (service_name == ::media::MediaService::Name_) {
      media_service_.Create(nullptr, mojo::MakeRequest<::media::MediaService>(
                                         client_handle.Pass()));
    }
    if (service_name == ::vsync::VSyncProvider::Name_) {
      vsync_.Create(nullptr, mojo::MakeRequest<::vsync::VSyncProvider>(
                                 client_handle.Pass()));
    }
#endif
  }

 private:
  mojo::StrongBinding<mojo::ServiceProvider> binding_;
  mojo::NetworkServiceFactory network_;
#if TARGET_OS_IPHONE
  sky::services::keyboard::KeyboardServiceFactory keyboard_;
  sky::services::media::MediaPlayerFactory media_player_;
  sky::services::media::MediaServiceFactory media_service_;
  sky::services::vsync::VSyncProviderFactory vsync_;
#endif
};

static void CreatePlatformServiceProvider(
    mojo::InterfaceRequest<mojo::ServiceProvider> request) {
  new PlatformServiceProvider(request.Pass());
}

mojo::ServiceProviderPtr CreateServiceProvider(
    ServiceProviderContext* context) {
  DCHECK(context);
  mojo::MessagePipe pipe;
  auto request = mojo::MakeRequest<mojo::ServiceProvider>(pipe.handle1.Pass());
  context->platform_task_runner->PostTask(
      FROM_HERE,
      base::Bind(CreatePlatformServiceProvider, base::Passed(request.Pass())));
  return mojo::MakeProxy(
      mojo::InterfacePtrInfo<mojo::ServiceProvider>(pipe.handle0.Pass(), 0u));
}

}  // namespace shell
}  // namespace sky
