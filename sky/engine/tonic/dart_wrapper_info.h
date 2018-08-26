// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_TONIC_DART_WRAPPER_INFO_H_
#define SKY_ENGINE_TONIC_DART_WRAPPER_INFO_H_

#include "base/macros.h"

namespace blink {
class DartWrappable;

typedef void (*DartWrappableAccepter)(DartWrappable*);

struct DartWrapperInfo {
  const char* interface_name;
  const size_t size_in_bytes;
  const DartWrappableAccepter ref_object;
  const DartWrappableAccepter deref_object;

 private:
  DartWrapperInfo(const DartWrapperInfo&) = delete;
  DartWrapperInfo& operator=(const DartWrapperInfo&) = delete;
};

}  // namespace blink

#endif  // SKY_ENGINE_TONIC_DART_WRAPPER_INFO_H_
