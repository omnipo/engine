// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SKIA_GANESH_CONTEXT_H_
#define MOJO_SKIA_GANESH_CONTEXT_H_

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "mojo/gpu/gl_context.h"
#include "skia/ext/refptr.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace mojo {

class GaneshContext : public GLContext::Observer {
 public:
  class Scope {
   public:
    explicit Scope(GaneshContext* context);
    ~Scope();

   private:
    MGLContext previous_;
  };

  explicit GaneshContext(base::WeakPtr<GLContext> gl_context);
  ~GaneshContext() override;

  void MakeCurrent();
  GrContext* gr() const {
    DCHECK(InScope());
    return context_.get();
  }

 private:
  bool InScope() const;
  void OnContextLost() override;

  base::WeakPtr<GLContext> gl_context_;
  skia::RefPtr<GrContext> context_;

  DISALLOW_COPY_AND_ASSIGN(GaneshContext);
};

}  // namespace mojo

#endif  // MOJO_SKIA_GANESH_CONTEXT_H_
