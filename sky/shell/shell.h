// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_SHELL_H_
#define SKY_SHELL_SHELL_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "sky/shell/service_provider.h"
#include "sky/shell/tracing_controller.h"

namespace sky {
namespace shell {
class ServiceProviderContext;

class Shell {
 public:
  ~Shell();

  static void Init(
      scoped_ptr<ServiceProviderContext> service_provider_context);
  static Shell& Shared();

  base::SingleThreadTaskRunner* gpu_task_runner() const {
    return gpu_task_runner_.get();
  }

  base::SingleThreadTaskRunner* ui_task_runner() const {
    return ui_task_runner_.get();
  }

  base::SingleThreadTaskRunner* io_task_runner() const {
    return io_task_runner_.get();
  }

  ServiceProviderContext* service_provider_context() const {
    return service_provider_context_.get();
  }

  TracingController& tracing_controller();

 private:
  explicit Shell(scoped_ptr<ServiceProviderContext> service_provider_context);

  void InitGPU(const base::Thread::Options& options);
  void InitUI(const base::Thread::Options& options);

  scoped_ptr<base::Thread> gpu_thread_;
  scoped_ptr<base::Thread> ui_thread_;
  scoped_ptr<base::Thread> io_thread_;

  scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;

  scoped_ptr<ServiceProviderContext> service_provider_context_;
  TracingController tracing_controller_;

  DISALLOW_COPY_AND_ASSIGN(Shell);
};

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_SHELL_H_
