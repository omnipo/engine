// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/clip_path_layer.h"

namespace sky {
namespace compositor {

ClipPathLayer::ClipPathLayer() {
}

ClipPathLayer::~ClipPathLayer() {
}

void ClipPathLayer::Paint(PaintContext::ScopedFrame& frame) {
  SkCanvas& canvas = frame.canvas();
  canvas.saveLayer(&clip_path_.getBounds(), nullptr);
  canvas.clipPath(clip_path_);
  PaintChildren(frame);
  canvas.restore();
}

}  // namespace compositor
}  // namespace sky
