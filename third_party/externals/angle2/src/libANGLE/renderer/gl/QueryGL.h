//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// QueryGL.h: Defines the class interface for QueryGL.

#ifndef LIBANGLE_RENDERER_GL_QUERYGL_H_
#define LIBANGLE_RENDERER_GL_QUERYGL_H_

#include "libANGLE/renderer/QueryImpl.h"

namespace rx
{

class QueryGL : public QueryImpl
{
  public:
    QueryGL(GLenum type);
    ~QueryGL() override;

    gl::Error begin() override;
    gl::Error end() override;
    gl::Error getResult(GLuint *params) override;
    gl::Error isResultAvailable(GLuint *available) override;
};

}

#endif // LIBANGLE_RENDERER_GL_QUERYGL_H_
