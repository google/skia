//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompilerGL.h: Defines the class interface for CompilerGL.

#ifndef LIBANGLE_RENDERER_GL_COMPILERGL_H_
#define LIBANGLE_RENDERER_GL_COMPILERGL_H_

#include "libANGLE/renderer/CompilerImpl.h"

namespace rx
{
class FunctionsGL;

class CompilerGL : public CompilerImpl
{
  public:
    CompilerGL(const FunctionsGL *functions);
    ~CompilerGL() override {}

    gl::Error release() override { return gl::Error(GL_NO_ERROR); }
    ShShaderOutput getTranslatorOutputType() const { return mTranslatorOutputType; }

  private:
    ShShaderOutput mTranslatorOutputType;
};

}

#endif // LIBANGLE_RENDERER_GL_COMPILERGL_H_
