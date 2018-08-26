// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/core/painting/CanvasImage.h"

namespace blink {

CanvasImage::CanvasImage() {
}

CanvasImage::~CanvasImage() {
}

int CanvasImage::width() const {
  return image_->width();
}

int CanvasImage::height() const {
  return image_->height();
}

}  // namespace blink
