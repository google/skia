//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// QueryGL.cpp: Implements the class methods for QueryGL.

#include "libANGLE/renderer/gl/QueryGL.h"

#include "common/debug.h"

namespace rx
{

QueryGL::QueryGL(GLenum type)
    : QueryImpl(type)
{}

QueryGL::~QueryGL()
{}

gl::Error QueryGL::begin()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error QueryGL::end()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error QueryGL::getResult(GLuint *params)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error QueryGL::isResultAvailable(GLuint *available)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

}
