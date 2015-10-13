//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceNVGL.h: Defines the class interface for FenceNVGL.

#ifndef LIBANGLE_RENDERER_GL_FENCENVGL_H_
#define LIBANGLE_RENDERER_GL_FENCENVGL_H_

#include "libANGLE/renderer/FenceNVImpl.h"

namespace rx
{
class FunctionsGL;

class FenceNVGL : public FenceNVImpl
{
  public:
    explicit FenceNVGL(const FunctionsGL *functions);
    ~FenceNVGL() override;

    gl::Error set(GLenum condition) override;
    gl::Error test(GLboolean *outFinished) override;
    gl::Error finish() override;

  private:
    GLuint mFence;

    const FunctionsGL *mFunctions;
};

}

#endif // LIBANGLE_RENDERER_GL_FENCENVGL_H_
