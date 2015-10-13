//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferD3D.cpp Defines common functionality between the Buffer9 and Buffer11 classes.

#include "libANGLE/renderer/d3d/BufferD3D.h"

#include "common/utilities.h"
#include "libANGLE/renderer/d3d/IndexBuffer.h"
#include "libANGLE/renderer/d3d/VertexBuffer.h"

namespace rx
{

unsigned int BufferD3D::mNextSerial = 1;

BufferD3D::BufferD3D(BufferFactoryD3D *factory)
    : BufferImpl(),
      mFactory(factory),
      mStaticVertexBuffer(nullptr),
      mStaticIndexBuffer(nullptr),
      mUnmodifiedDataUse(0),
      mUsage(D3D_BUFFER_USAGE_STATIC)
{
    updateSerial();
}

BufferD3D::~BufferD3D()
{
    SafeDelete(mStaticVertexBuffer);
    SafeDelete(mStaticIndexBuffer);
}

void BufferD3D::updateSerial()
{
    mSerial = mNextSerial++;
}

void BufferD3D::updateD3DBufferUsage(GLenum usage)
{
    switch (usage)
    {
        case GL_STATIC_DRAW:
        case GL_STATIC_READ:
        case GL_STATIC_COPY:
            mUsage = D3D_BUFFER_USAGE_STATIC;
            initializeStaticData();
            break;

        case GL_STREAM_DRAW:
        case GL_STREAM_READ:
        case GL_STREAM_COPY:
        case GL_DYNAMIC_READ:
        case GL_DYNAMIC_COPY:
        case GL_DYNAMIC_DRAW:
            mUsage = D3D_BUFFER_USAGE_DYNAMIC;
            break;
        default:
            UNREACHABLE();
    }
}

void BufferD3D::initializeStaticData()
{
    if (!mStaticVertexBuffer)
    {
        mStaticVertexBuffer = new StaticVertexBufferInterface(mFactory);
    }
    if (!mStaticIndexBuffer)
    {
        mStaticIndexBuffer = new StaticIndexBufferInterface(mFactory);
    }
}

void BufferD3D::invalidateStaticData()
{
    if ((mStaticVertexBuffer && mStaticVertexBuffer->getBufferSize() != 0) || (mStaticIndexBuffer && mStaticIndexBuffer->getBufferSize() != 0))
    {
        SafeDelete(mStaticVertexBuffer);
        SafeDelete(mStaticIndexBuffer);

        // If the buffer was created with a static usage then we recreate the static
        // buffers so that they are populated the next time we use this buffer.
        if (mUsage == D3D_BUFFER_USAGE_STATIC)
        {
            initializeStaticData();
        }
    }

    mUnmodifiedDataUse = 0;
}

// Creates static buffers if sufficient used data has been left unmodified
void BufferD3D::promoteStaticUsage(int dataSize)
{
    if (!mStaticVertexBuffer && !mStaticIndexBuffer)
    {
        mUnmodifiedDataUse += dataSize;

        if (mUnmodifiedDataUse > 3 * getSize())
        {
            initializeStaticData();
        }
    }
}

gl::Error BufferD3D::getIndexRange(GLenum type,
                                   size_t offset,
                                   size_t count,
                                   bool primitiveRestartEnabled,
                                   gl::IndexRange *outRange)
{
    const uint8_t *data = nullptr;
    gl::Error error = getData(&data);
    if (error.isError())
    {
        return error;
    }

    *outRange = gl::ComputeIndexRange(type, data + offset, count, primitiveRestartEnabled);
    return gl::Error(GL_NO_ERROR);
}

}
