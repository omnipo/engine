// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_CORE_PAINTING_CANVASIMAGE_H_
#define SKY_ENGINE_CORE_PAINTING_CANVASIMAGE_H_

#include "sky/engine/tonic/dart_wrappable.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "third_party/skia/include/core/SkImage.h"

namespace blink {

class CanvasImage final : public RefCounted<CanvasImage>,
                          public DartWrappable {
  DEFINE_WRAPPERTYPEINFO();
 public:
  ~CanvasImage() override;
  static PassRefPtr<CanvasImage> create() { return adoptRef(new CanvasImage); }

  int width() const;
  int height() const;

  SkImage* image() const { return image_.get(); }
  void setImage(PassRefPtr<SkImage> image) { image_ = image; }

 private:
  CanvasImage();

  RefPtr<SkImage> image_;
};

}  // namespace blink

#endif  // SKY_ENGINE_CORE_PAINTING_CANVASIMAGE_H_
