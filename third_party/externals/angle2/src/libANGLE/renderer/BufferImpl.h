//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferImpl.h: Defines the abstract rx::BufferImpl class.

#ifndef LIBANGLE_RENDERER_BUFFERIMPL_H_
#define LIBANGLE_RENDERER_BUFFERIMPL_H_

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/Error.h"

#include <stdint.h>

namespace rx
{

class BufferImpl : angle::NonCopyable
{
  public:
    virtual ~BufferImpl() { }

    virtual gl::Error setData(const void* data, size_t size, GLenum usage) = 0;
    virtual gl::Error setSubData(const void* data, size_t size, size_t offset) = 0;
    virtual gl::Error copySubData(BufferImpl* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size) = 0;
    virtual gl::Error map(GLenum access, GLvoid **mapPtr) = 0;
    virtual gl::Error mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr) = 0;
    virtual gl::Error unmap(GLboolean *result) = 0;

    virtual gl::Error getIndexRange(GLenum type,
                                    size_t offset,
                                    size_t count,
                                    bool primitiveRestartEnabled,
                                    gl::IndexRange *outRange) = 0;
};

}

#endif // LIBANGLE_RENDERER_BUFFERIMPL_H_
