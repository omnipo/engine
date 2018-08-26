// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/color_filter_layer.h"

namespace sky {
namespace compositor {

ColorFilterLayer::ColorFilterLayer() {
}

ColorFilterLayer::~ColorFilterLayer() {
}

void ColorFilterLayer::Paint(PaintContext::ScopedFrame& frame) {
  RefPtr<SkColorFilter> color_filter =
      adoptRef(SkColorFilter::CreateModeFilter(color_, transfer_mode_));
  SkPaint paint;
  paint.setColorFilter(color_filter.get());
  SkCanvas& canvas = frame.canvas();
  canvas.saveLayer(&paint_bounds(), &paint);
  PaintChildren(frame);
  canvas.restore();
}

}  // namespace compositor
}  // namespace sky
