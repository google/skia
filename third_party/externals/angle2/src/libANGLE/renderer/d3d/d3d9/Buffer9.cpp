//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer9.cpp Defines the Buffer9 class.

#include "libANGLE/renderer/d3d/d3d9/Buffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"

namespace rx
{

Buffer9::Buffer9(Renderer9 *renderer)
    : BufferD3D(renderer),
      mSize(0)
{}

Buffer9::~Buffer9()
{
    mSize = 0;
}

gl::Error Buffer9::setData(const void* data, size_t size, GLenum usage)
{
    if (size > mMemory.size())
    {
        if (!mMemory.resize(size))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to resize internal buffer.");
        }
    }

    mSize = size;
    if (data && size > 0)
    {
        memcpy(mMemory.data(), data, size);
    }

    invalidateStaticData();

    updateD3DBufferUsage(usage);
    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer9::getData(const uint8_t **outData)
{
    *outData = mMemory.data();
    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer9::setSubData(const void* data, size_t size, size_t offset)
{
    if (offset + size > mMemory.size())
    {
        if (!mMemory.resize(offset + size))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to resize internal buffer.");
        }
    }

    mSize = std::max(mSize, offset + size);
    if (data && size > 0)
    {
        memcpy(mMemory.data() + offset, data, size);
    }

    invalidateStaticData();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Buffer9::copySubData(BufferImpl* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size)
{
    // Note: this method is currently unreachable
    Buffer9* sourceBuffer = GetAs<Buffer9>(source);
    ASSERT(sourceBuffer);

    memcpy(mMemory.data() + destOffset, sourceBuffer->mMemory.data() + sourceOffset, size);

    invalidateStaticData();

    return gl::Error(GL_NO_ERROR);
}

// We do not support buffer mapping in D3D9
gl::Error Buffer9::map(GLenum access, GLvoid **mapPtr)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error Buffer9::mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error Buffer9::unmap(GLboolean *result)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

void Buffer9::markTransformFeedbackUsage()
{
    UNREACHABLE();
}

}
