// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/gpu/ganesh_surface.h"

#include "base/logging.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace sky {
namespace shell {

GaneshSurface::GaneshSurface(intptr_t window_fbo,
              GaneshContext* context,
              const gfx::Size& size) {
  GrBackendRenderTargetDesc desc;
  desc.fWidth = size.width();
  desc.fHeight = size.height();
  desc.fConfig = kSkia8888_GrPixelConfig;
  desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
  desc.fRenderTargetHandle = window_fbo;

  skia::RefPtr<GrRenderTarget> target = skia::AdoptRef(
    context->gr()->textureProvider()->wrapBackendRenderTarget(desc));
  DCHECK(target);
  surface_ = skia::AdoptRef(SkSurface::NewRenderTargetDirect(target.get()));
  DCHECK(surface_);
}

GaneshSurface::~GaneshSurface() {
}

}  // namespace shell
}  // namespace sky
