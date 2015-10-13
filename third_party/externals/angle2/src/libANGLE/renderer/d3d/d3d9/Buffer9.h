//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer9.h: Defines the rx::Buffer9 class which implements rx::BufferImpl via rx::BufferD3D.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_BUFFER9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_BUFFER9_H_

#include "common/MemoryBuffer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"

namespace rx
{
class Renderer9;

class Buffer9 : public BufferD3D
{
  public:
    Buffer9(Renderer9 *renderer);
    virtual ~Buffer9();

    // BufferD3D implementation
    virtual size_t getSize() const { return mSize; }
    virtual bool supportsDirectBinding() const { return false; }
    gl::Error getData(const uint8_t **outData) override;

    // BufferImpl implementation
    virtual gl::Error setData(const void* data, size_t size, GLenum usage);
    virtual gl::Error setSubData(const void* data, size_t size, size_t offset);
    virtual gl::Error copySubData(BufferImpl* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size);
    virtual gl::Error map(GLenum access, GLvoid **mapPtr);
    virtual gl::Error mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr);
    virtual gl::Error unmap(GLboolean *result);
    virtual void markTransformFeedbackUsage();

  private:
    MemoryBuffer mMemory;
    size_t mSize;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D9_BUFFER9_H_
