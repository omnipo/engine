// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_CORE_PAINTING_CANVAS_H_
#define SKY_ENGINE_CORE_PAINTING_CANVAS_H_

#include "sky/engine/bindings/exception_state.h"
#include "sky/engine/core/painting/CanvasPath.h"
#include "sky/engine/core/painting/Drawable.h"
#include "sky/engine/core/painting/Offset.h"
#include "sky/engine/core/painting/Paint.h"
#include "sky/engine/core/painting/Picture.h"
#include "sky/engine/core/painting/PictureRecorder.h"
#include "sky/engine/core/painting/Point.h"
#include "sky/engine/core/painting/RRect.h"
#include "sky/engine/core/painting/Rect.h"
#include "sky/engine/core/painting/Size.h"
#include "sky/engine/core/painting/RSTransform.h"
#include "sky/engine/core/painting/VertexMode.h"
#include "sky/engine/tonic/dart_wrappable.h"
#include "sky/engine/tonic/float64_list.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefCounted.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace blink {
class CanvasImage;

class Canvas : public RefCounted<Canvas>, public DartWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtr<Canvas> create(SkCanvas* skCanvas) {
        ASSERT(skCanvas);
        return adoptRef(new Canvas(skCanvas));
    }

    // TODO(ianh): fix crashes here https://github.com/domokit/mojo/issues/326
    static PassRefPtr<Canvas> create(PictureRecorder* recorder,
                                     Rect& bounds,
                                     ExceptionState& es) {
        ASSERT(recorder);
        if (recorder->isRecording()) {
            es.ThrowTypeError(
                "You must call PictureRecorder.endRecording() before reusing a"
                " PictureRecorder to create a new Canvas object.");
            // TODO(iansf): We should return a nullptr here, I think, but doing
            //              so will require modifying the dart template code to
            //              to correctly handle constructors that throw
            //              exceptions.  For now, just let it return a dart
            //              object that may cause a crash later on -- if the
            //              dart code catches the error, it will leak a canvas
            //              but it won't crash.
        }
        PassRefPtr<Canvas> canvas = create(recorder->beginRecording(bounds));
        recorder->set_canvas(canvas.get());
        return canvas;
    }

    ~Canvas() override;

    void save();
    void saveLayer(const Rect& bounds, const Paint& paint);
    void restore();
    int getSaveCount();

    void translate(float dx, float dy);
    void scale(float sx, float sy);
    void rotate(float radians);
    void skew(float sx, float sy);
    void concat(const Float64List& matrix4, ExceptionState&);

    void setMatrix(const Float64List& matrix4, ExceptionState&);
    Float64List getTotalMatrix() const;

    void clipRect(const Rect& rect);
    void clipRRect(const RRect& rrect);
    void clipPath(const CanvasPath* path);

    void drawColor(SkColor color, SkXfermode::Mode transferMode);
    void drawLine(const Point& p1, const Point& p2, const Paint& paint);
    void drawPaint(const Paint& paint);
    void drawRect(const Rect& rect, const Paint& paint);
    void drawRRect(const RRect& rrect, const Paint& paint);
    void drawDRRect(const RRect& outer, const RRect& inner, const Paint& paint);
    void drawOval(const Rect& rect, const Paint& paint);
    void drawCircle(const Point& c, float radius, const Paint& paint);
    void drawPath(const CanvasPath* path, const Paint& paint);
    void drawImage(const CanvasImage* image, const Point& p, const Paint& paint);
    void drawImageRect(const CanvasImage* image, Rect& src, Rect& dst, const Paint& paint);
    void drawImageNine(const CanvasImage* image, Rect& center, Rect& dst, const Paint& paint);
    void drawPicture(Picture* picture);
    void drawDrawable(Drawable* drawable);

    void drawVertices(SkCanvas::VertexMode vertexMode,
        const Vector<Point>& vertices,
        const Vector<Point>& textureCoordinates,
        const Vector<SkColor>& colors,
        SkXfermode::Mode transferMode,
        const Vector<int>& indices,
        const Paint& paint,
        ExceptionState& es);

    void drawAtlas(CanvasImage* atlas,
        const Vector<RSTransform>& transforms, const Vector<Rect>& rects,
        const Vector<SkColor>& colors, SkXfermode::Mode mode,
        const Rect& cullRect, const Paint& paint, ExceptionState&);

    SkCanvas* skCanvas() { return m_canvas; }
    void clearSkCanvas() { m_canvas = nullptr; }
    bool isRecording() const { return !!m_canvas; }

protected:
    explicit Canvas(SkCanvas* skCanvas);

private:
    // The SkCanvas is supplied by a call to SkPictureRecorder::beginRecording,
    // which does not transfer ownership.  For this reason, we hold a raw
    // pointer and manually set the SkCanvas to null in clearSkCanvas.
    SkCanvas* m_canvas;
};

} // namespace blink

#endif  // SKY_ENGINE_CORE_PAINTING_CANVAS_H_
