// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_GPU_RASTERIZER_H_
#define SKY_SHELL_GPU_RASTERIZER_H_

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "skia/ext/refptr.h"
#include "sky/shell/gpu_delegate.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"
#include "sky/compositor/paint_context.h"

class SkPicture;

namespace gfx {
class GLContext;
class GLShareGroup;
class GLSurface;
}

namespace sky {
namespace shell {
class GaneshContext;
class GaneshSurface;

class Rasterizer : public GPUDelegate {
 public:
  explicit Rasterizer();
  ~Rasterizer() override;

  base::WeakPtr<Rasterizer> GetWeakPtr();

  void OnAcceleratedWidgetAvailable(gfx::AcceleratedWidget widget) override;
  void OnOutputSurfaceDestroyed() override;
  void Draw(scoped_ptr<compositor::LayerTree> layer_tree) override;

 private:
  void EnsureGLContext();
  void EnsureGaneshSurface(intptr_t window_fbo, const gfx::Size& size);

  scoped_refptr<gfx::GLShareGroup> share_group_;
  scoped_refptr<gfx::GLSurface> surface_;
  scoped_refptr<gfx::GLContext> context_;

  scoped_ptr<GaneshContext> ganesh_context_;
  scoped_ptr<GaneshSurface> ganesh_surface_;

  compositor::PaintContext paint_context_;

  base::WeakPtrFactory<Rasterizer> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Rasterizer);
};

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_GPU_RASTERIZER_H_
