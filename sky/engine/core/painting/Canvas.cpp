// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "math.h"

#include "sky/engine/core/painting/Canvas.h"

#include "sky/engine/core/painting/CanvasImage.h"
#include "sky/engine/core/painting/Matrix.h"
#include "sky/engine/platform/geometry/IntRect.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace blink {

Canvas::Canvas(SkCanvas* skCanvas)
    : m_canvas(skCanvas)
{
}

Canvas::~Canvas()
{
}

void Canvas::save()
{
    if (!m_canvas)
        return;
    m_canvas->save();
}

void Canvas::saveLayer(const Rect& bounds, const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->saveLayer(!bounds.is_null ? &bounds.sk_rect : nullptr,
                        paint.paint());
}

void Canvas::restore()
{
    if (!m_canvas)
        return;
    m_canvas->restore();
}

int Canvas::getSaveCount()
{
    if (!m_canvas)
        return 0;
    return m_canvas->getSaveCount();
}

void Canvas::translate(float dx, float dy)
{
    if (!m_canvas)
        return;
    m_canvas->translate(dx, dy);
}

void Canvas::scale(float sx, float sy)
{
    if (!m_canvas)
        return;
    m_canvas->scale(sx, sy);
}

void Canvas::rotate(float radians)
{
    if (!m_canvas)
        return;
    m_canvas->rotate(radians * 180.0/M_PI);
}

void Canvas::skew(float sx, float sy)
{
    if (!m_canvas)
        return;
    m_canvas->skew(sx, sy);
}

void Canvas::concat(const Float64List& matrix4, ExceptionState& es)
{
    if (!m_canvas)
        return es.ThrowTypeError("No canvas");

    SkMatrix sk_matrix = toSkMatrix(matrix4, es);
    if (es.had_exception())
        return;
    m_canvas->concat(sk_matrix);
}

void Canvas::setMatrix(const Float64List& matrix4, ExceptionState& es)
{
    if (!m_canvas)
        return es.ThrowTypeError("No canvas");

    SkMatrix sk_matrix = toSkMatrix(matrix4, es);
    if (es.had_exception())
        return;
    m_canvas->setMatrix(sk_matrix);
}

Float64List Canvas::getTotalMatrix() const
{
    // Maybe we should throw an exception instead of returning an empty matrix?
    SkMatrix sk_matrix;
    if (m_canvas)
        sk_matrix = m_canvas->getTotalMatrix();
    return toMatrix4(sk_matrix);
}

void Canvas::clipRect(const Rect& rect)
{
    if (!m_canvas)
        return;
    m_canvas->clipRect(rect.sk_rect);
}

void Canvas::clipRRect(const RRect& rrect)
{
    if (!m_canvas)
        return;
    m_canvas->clipRRect(rrect.sk_rrect, SkRegion::kIntersect_Op);
}

void Canvas::clipPath(const CanvasPath* path)
{
    if (!m_canvas)
        return;
    m_canvas->clipPath(path->path(), SkRegion::kIntersect_Op);
}

void Canvas::drawColor(SkColor color, SkXfermode::Mode transferMode)
{
    if (!m_canvas)
        return;
    m_canvas->drawColor(color, transferMode);
}

void Canvas::drawLine(const Point& p1, const Point& p2, const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->drawLine(p1.sk_point.x(), p1.sk_point.y(), p2.sk_point.x(), p2.sk_point.y(), paint.sk_paint);
}

void Canvas::drawPaint(const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->drawPaint(paint.sk_paint);
}

void Canvas::drawRect(const Rect& rect, const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->drawRect(rect.sk_rect, paint.sk_paint);
}

void Canvas::drawRRect(const RRect& rrect, const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->drawRRect(rrect.sk_rrect, paint.sk_paint);
}

void Canvas::drawDRRect(const RRect& outer, const RRect& inner, const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->drawDRRect(outer.sk_rrect, inner.sk_rrect, paint.sk_paint);
}

void Canvas::drawOval(const Rect& rect, const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->drawOval(rect.sk_rect, paint.sk_paint);
}

void Canvas::drawCircle(const Point& c, float radius, const Paint& paint)
{
    if (!m_canvas)
        return;
    m_canvas->drawCircle(c.sk_point.x(), c.sk_point.y(), radius, paint.sk_paint);
}

void Canvas::drawPath(const CanvasPath* path, const Paint& paint)
{
    if (!m_canvas)
        return;
    ASSERT(path);
    m_canvas->drawPath(path->path(), paint.sk_paint);
}

void Canvas::drawImage(const CanvasImage* image, const Point& p, const Paint& paint) {
    if (!m_canvas)
        return;
    ASSERT(image);
    m_canvas->drawImage(image->image(), p.sk_point.x(), p.sk_point.y(), paint.paint());
}

void Canvas::drawImageRect(const CanvasImage* image, Rect& src, Rect& dst, const Paint& paint) {
    if (!m_canvas)
        return;
    ASSERT(image);
    m_canvas->drawImageRect(image->image(), src.sk_rect, dst.sk_rect, paint.paint());
}

void Canvas::drawImageNine(const CanvasImage* image, Rect& center, Rect& dst, const Paint& paint) {
    if (!m_canvas)
        return;
    ASSERT(image);
    SkIRect icenter;
    center.sk_rect.round(&icenter);
    m_canvas->drawImageNine(image->image(), icenter, dst.sk_rect, paint.paint());
}

void Canvas::drawPicture(Picture* picture)
{
    if (!m_canvas)
        return;
    ASSERT(picture);
    m_canvas->drawPicture(picture->toSkia());
}

void Canvas::drawDrawable(Drawable* drawable)
{
    if (!m_canvas)
        return;
    ASSERT(drawable);
    m_canvas->drawDrawable(drawable->toSkia());
}

void Canvas::drawVertices(SkCanvas::VertexMode vertexMode,
        const Vector<Point>& vertices,
        const Vector<Point>& textureCoordinates,
        const Vector<SkColor>& colors,
        SkXfermode::Mode transferMode,
        const Vector<int>& indices,
        const Paint& paint,
        ExceptionState& es)
{
    if (!m_canvas)
        return;
    size_t vertexCount = vertices.size();

    if (textureCoordinates.size() && textureCoordinates.size() != vertexCount)
        return es.ThrowRangeError("vertices and textureCoordinates lengths must match");
    if (colors.size() && colors.size() != vertexCount)
        return es.ThrowRangeError("vertices and colors lengths must match");

    Vector<SkPoint> skVertices;
    for (size_t x = 0; x < vertices.size(); x++) {
        const Point& point = vertices[x];
        if (point.is_null)
            return es.ThrowRangeError("vertices contained a null");
        skVertices.append(point.sk_point);
    }

    Vector<SkPoint> skTextureCoordinates;
    for (size_t x = 0; x < textureCoordinates.size(); x++) {
        const Point& point = textureCoordinates[x];
        if (point.is_null)
            return es.ThrowRangeError("textureCoordinates contained a null");
        skTextureCoordinates.append(point.sk_point);
    }

    Vector<uint16_t> skIndices;
    for (size_t x = 0; x < indices.size(); x++) {
        uint16_t i = indices[x];
        skIndices.append(i);
    }

    RefPtr<SkXfermode> transferModePtr = adoptRef(SkXfermode::Create(transferMode));

    m_canvas->drawVertices(
        vertexMode,
        skVertices.size(),
        skVertices.data(),
        skTextureCoordinates.isEmpty() ? nullptr : skTextureCoordinates.data(),
        colors.isEmpty() ? nullptr : colors.data(),
        transferModePtr.get(),
        skIndices.isEmpty() ? nullptr : skIndices.data(),
        skIndices.size(),
        *paint.paint()
    );
}

void Canvas::drawAtlas(CanvasImage* atlas,
    const Vector<RSTransform>& transforms, const Vector<Rect>& rects,
    const Vector<SkColor>& colors, SkXfermode::Mode mode,
    const Rect& cullRect, const Paint& paint, ExceptionState& es)
{
    if (!m_canvas)
        return;
    RefPtr<SkImage> skImage = atlas->image();
    if (transforms.size() != rects.size())
        return es.ThrowRangeError("transforms and rects lengths must match");
    if (colors.size() && colors.size() != rects.size())
        return es.ThrowRangeError("if supplied, colors length must match that of transforms and rects");

    Vector<SkRSXform> skXForms;
    for (size_t x = 0; x < transforms.size(); x++) {
        const RSTransform& transform = transforms[x];
        if (transform.is_null)
            return es.ThrowRangeError("transforms contained a null");
        skXForms.append(transform.sk_xform);
    }

    Vector<SkRect> skRects;
    for (size_t x = 0; x < rects.size(); x++) {
        const Rect& rect = rects[x];
        if (rect.is_null)
            return es.ThrowRangeError("rects contained a null");
        skRects.append(rect.sk_rect);
    }

    m_canvas->drawAtlas(
        skImage.get(),
        skXForms.data(),
        skRects.data(),
        colors.isEmpty() ? nullptr : colors.data(),
        skXForms.size(),
        mode,
        cullRect.is_null ? nullptr : &cullRect.sk_rect,
        paint.paint()
    );
}


} // namespace blink
