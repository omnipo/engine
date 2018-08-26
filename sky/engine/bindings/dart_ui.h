// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS_DART_UI_H_
#define SKY_ENGINE_BINDINGS_DART_UI_H_

#include "base/macros.h"

namespace blink {

class DartUI {
 public:
  static void InitForIsolate();

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(DartUI);
};

}  // namespace blink

#endif  // SKY_ENGINE_BINDINGS_DART_UI_H_
