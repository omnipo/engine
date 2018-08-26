// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_CORE_PAINTING_CANVASGRADIENT_H_
#define SKY_ENGINE_CORE_PAINTING_CANVASGRADIENT_H_

#include "sky/engine/core/painting/CanvasColor.h"
#include "sky/engine/core/painting/Point.h"
#include "sky/engine/core/painting/Shader.h"
#include "sky/engine/tonic/dart_wrappable.h"
#include "third_party/skia/include/effects/SkGradientShader.h"

namespace blink {

class TileMode {};

template <>
struct DartConverter<TileMode> : public DartConverterEnum<SkShader::TileMode> {};

COMPILE_ASSERT(SkShader::kTileModeCount == 3, Need_to_update_Gradient_dart);

class CanvasGradient : public Shader {
    DEFINE_WRAPPERTYPEINFO();
 public:
  ~CanvasGradient() override;
  static PassRefPtr<CanvasGradient> create();

  void initLinear(const Vector<Point>& end_points,
                  const Vector<SkColor>& colors,
                  const Vector<float>& color_stops,
                  SkShader::TileMode tile_mode);
  void initRadial(const Point& center,
                  double radius,
                  const Vector<SkColor>& colors,
                  const Vector<float>& color_stops,
                  SkShader::TileMode tile_mode);

 private:
  CanvasGradient();
};

} // namespace blink

#endif  // SKY_ENGINE_CORE_PAINTING_CANVASGRADIENT_H_
