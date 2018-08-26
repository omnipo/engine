// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/clip_rect_layer.h"

namespace sky {
namespace compositor {

ClipRectLayer::ClipRectLayer() {
}

ClipRectLayer::~ClipRectLayer() {
}

void ClipRectLayer::Paint(PaintContext::ScopedFrame& frame) {
  SkCanvas& canvas = frame.canvas();
  canvas.save();
  canvas.clipRect(clip_rect_);
  PaintChildren(frame);
  canvas.restore();
}

}  // namespace compositor
}  // namespace sky
