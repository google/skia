//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderGL.h: Defines the class interface for ShaderGL.

#ifndef LIBANGLE_RENDERER_GL_SHADERGL_H_
#define LIBANGLE_RENDERER_GL_SHADERGL_H_

#include "libANGLE/renderer/ShaderImpl.h"

namespace rx
{
class FunctionsGL;

class ShaderGL : public ShaderImpl
{
  public:
    ShaderGL(const gl::Shader::Data &data, const FunctionsGL *functions);
    ~ShaderGL() override;

    // ShaderImpl implementation
    int prepareSourceAndReturnOptions(std::stringstream *sourceStream) override;
    bool postTranslateCompile(gl::Compiler *compiler, std::string *infoLog) override;
    std::string getDebugInfo() const override;

    GLuint getShaderID() const;

  private:
    const FunctionsGL *mFunctions;

    GLuint mShaderID;
};

}

#endif // LIBANGLE_RENDERER_GL_SHADERGL_H_
