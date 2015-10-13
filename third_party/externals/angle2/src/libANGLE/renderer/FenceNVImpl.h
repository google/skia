//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceNVImpl.h: Defines the rx::FenceNVImpl class.

#ifndef LIBANGLE_RENDERER_FENCENVIMPL_H_
#define LIBANGLE_RENDERER_FENCENVIMPL_H_

#include "libANGLE/Error.h"

#include "common/angleutils.h"

#include "angle_gl.h"

namespace rx
{

class FenceNVImpl : angle::NonCopyable
{
  public:
    FenceNVImpl() { };
    virtual ~FenceNVImpl() { };

    virtual gl::Error set(GLenum condition) = 0;
    virtual gl::Error test(GLboolean *outFinished) = 0;
    virtual gl::Error finish() = 0;
};

}

#endif // LIBANGLE_RENDERER_FENCENVIMPL_H_
