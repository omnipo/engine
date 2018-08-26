// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_TOOLS_SKY_SNAPSHOT_VM_H_
#define SKY_TOOLS_SKY_SNAPSHOT_VM_H_

#include "dart/runtime/include/dart_api.h"

void InitDartVM();
Dart_Isolate CreateDartIsolate();

#endif  // SKY_TOOLS_SKY_SNAPSHOT_VM_H_
