// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_SWITCHES_H_
#define SKY_SHELL_SWITCHES_H_

#include <string>

namespace sky {
namespace shell {
namespace switches {

extern const char kHelp[];
extern const char kPackageRoot[];
extern const char kNonInteractive[];
extern const char kSnapshot[];
extern const char kEnableCheckedMode[];

void PrintUsage(const std::string& executable_name);

}  // namespace switches
}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_SWITCHES_H_
