// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_CORE_PAINTING_RRECT_H_
#define SKY_ENGINE_CORE_PAINTING_RRECT_H_

#include "dart/runtime/include/dart_api.h"
#include "sky/engine/tonic/dart_converter.h"
#include "third_party/skia/include/core/SkRRect.h"

namespace blink {

class RRect {
 public:
  SkRRect sk_rrect;
  bool is_null;
};

template <>
struct DartConverter<RRect> {
  static RRect FromDart(Dart_Handle handle);
  static RRect FromArgumentsWithNullCheck(Dart_NativeArguments args,
                                          int index,
                                          Dart_Handle& exception);
};

} // namespace blink

#endif  // SKY_ENGINE_CORE_PAINTING_RRECT_H_
