// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_SERVICE_PROVIDER_H_
#define SKY_SHELL_SERVICE_PROVIDER_H_

#include "base/memory/ref_counted.h"
#include "mojo/public/cpp/system/core.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace sky {
namespace shell {

class ServiceProviderContext {
 public:
  ServiceProviderContext(scoped_refptr<base::SingleThreadTaskRunner> runner);
  ~ServiceProviderContext();

  scoped_refptr<base::SingleThreadTaskRunner> platform_task_runner;
};

// Implemented in platform_service_provider.cc for each platform.
mojo::ServiceProviderPtr CreateServiceProvider(ServiceProviderContext* context);

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_SERVICE_PROVIDER_H_
