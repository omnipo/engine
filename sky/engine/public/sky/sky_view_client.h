// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_PUBLIC_SKY_SKY_VIEW_CLIENT_H_
#define SKY_ENGINE_PUBLIC_SKY_SKY_VIEW_CLIENT_H_

#include <memory>

#include "sky/compositor/layer_tree.h"

typedef struct _Dart_Isolate* Dart_Isolate;

namespace blink {

class SkyViewClient {
 public:
  virtual void ScheduleFrame() = 0;
  virtual void Render(std::unique_ptr<sky::compositor::LayerTree> layer_tree) = 0;

  virtual void DidCreateIsolate(Dart_Isolate isolate) = 0;

 protected:
  virtual ~SkyViewClient();
};

} // namespace blink

#endif  // SKY_ENGINE_PUBLIC_SKY_SKY_VIEW_CLIENT_H_
