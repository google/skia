//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Compiler.h: Defines the gl::Compiler class, abstracting the ESSL compiler
// that a GL context holds.

#ifndef LIBANGLE_COMPILER_H_
#define LIBANGLE_COMPILER_H_

#include "libANGLE/Error.h"
#include "GLSLANG/ShaderLang.h"

namespace rx
{
class CompilerImpl;
class ImplFactory;
}

namespace gl
{
struct Data;

class Compiler final : angle::NonCopyable
{
  public:
    Compiler(rx::ImplFactory *implFactory, const Data &data);
    ~Compiler();

    Error release();

    ShHandle getCompilerHandle(GLenum type);
    ShShaderOutput getShaderOutputType() const { return mOutputType; }

  private:
    rx::CompilerImpl *mImplementation;
    ShShaderSpec mSpec;
    ShShaderOutput mOutputType;
    ShBuiltInResources mResources;

    ShHandle mFragmentCompiler;
    ShHandle mVertexCompiler;
};

}  // namespace gl

#endif // LIBANGLE_COMPILER_H_
