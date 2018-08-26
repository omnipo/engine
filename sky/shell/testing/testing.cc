// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/testing/testing.h"

#include "sky/engine/public/web/WebRuntimeFeatures.h"
#include "base/command_line.h"
#include "sky/shell/switches.h"
#include "sky/shell/testing/test_runner.h"

namespace sky {
namespace shell {

bool InitForTesting() {
  base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  blink::WebRuntimeFeatures::enableObservatory(
      !command_line.HasSwitch(switches::kNonInteractive));

  TestRunner::TestDescriptor test;
  test.package_root = command_line.GetSwitchValueASCII(switches::kPackageRoot);

  if (command_line.HasSwitch(switches::kSnapshot)) {
    test.path = command_line.GetSwitchValueASCII(switches::kSnapshot);
    test.is_snapshot = true;
  } else {
    auto args = command_line.GetArgs();
    if (args.empty())
      return false;
    test.path = args[0];
  }

  TestRunner::Shared().Run(test);
  return true;
}

}  // namespace shell
}  // namespace sky
