//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceSyncImpl.h: Defines the rx::FenceSyncImpl class.

#ifndef LIBANGLE_RENDERER_FENCESYNCIMPL_H_
#define LIBANGLE_RENDERER_FENCESYNCIMPL_H_

#include "libANGLE/Error.h"

#include "common/angleutils.h"

#include "angle_gl.h"

namespace rx
{

class FenceSyncImpl : angle::NonCopyable
{
  public:
    FenceSyncImpl() { };
    virtual ~FenceSyncImpl() { };

    virtual gl::Error set(GLenum condition, GLbitfield flags) = 0;
    virtual gl::Error clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult) = 0;
    virtual gl::Error serverWait(GLbitfield flags, GLuint64 timeout) = 0;
    virtual gl::Error getStatus(GLint *outResult) = 0;
};

}

#endif // LIBANGLE_RENDERER_FENCESYNCIMPL_H_
