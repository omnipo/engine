/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SKY_ENGINE_PLATFORM_GRAPHICS_IMAGEBUFFERSURFACE_H_
#define SKY_ENGINE_PLATFORM_GRAPHICS_IMAGEBUFFERSURFACE_H_

#include "sky/engine/platform/PlatformExport.h"
#include "sky/engine/platform/geometry/IntSize.h"
#include "sky/engine/wtf/FastAllocBase.h"
#include "sky/engine/wtf/Noncopyable.h"
#include "sky/engine/wtf/PassRefPtr.h"

class SkBitmap;
class SkCanvas;
class SkPicture;

namespace blink {

class ImageBuffer;
class WebLayer;
class FloatRect;

enum OpacityMode {
    NonOpaque,
    Opaque,
};

class PLATFORM_EXPORT ImageBufferSurface {
    WTF_MAKE_NONCOPYABLE(ImageBufferSurface); WTF_MAKE_FAST_ALLOCATED;
public:
    virtual ~ImageBufferSurface() { }

    virtual SkCanvas* canvas() const = 0;
    virtual const SkBitmap& bitmap();
    virtual void willAccessPixels() { }
    virtual bool isValid() const = 0;
    virtual bool restore() { return false; };
    virtual WebLayer* layer() const { return 0; };
    virtual bool isAccelerated() const { return false; }
    virtual bool cachedBitmapEnabled() const { return false; }
    virtual const SkBitmap& cachedBitmap() const;
    virtual void invalidateCachedBitmap() { }
    virtual void updateCachedBitmapIfNeeded() { }
    virtual void setIsHidden(bool) { }
    virtual void setImageBuffer(ImageBuffer*) { }
    virtual PassRefPtr<SkPicture> getPicture();
    virtual void didClearCanvas() { }
    virtual void finalizeFrame() { }

    OpacityMode opacityMode() const { return m_opacityMode; }
    const IntSize& size() const { return m_size; }
    void notifyIsValidChanged(bool isValid) const;

protected:
    ImageBufferSurface(const IntSize&, OpacityMode);
    void clear();

private:
    OpacityMode m_opacityMode;
    IntSize m_size;
};

} // namespace blink

#endif  // SKY_ENGINE_PLATFORM_GRAPHICS_IMAGEBUFFERSURFACE_H_
