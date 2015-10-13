//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer9.cpp: Defines the D3D9 VertexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d9/VertexBuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/vertexconversion.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/Buffer.h"

namespace rx
{

VertexBuffer9::VertexBuffer9(Renderer9 *renderer) : mRenderer(renderer)
{
    mVertexBuffer = NULL;
    mBufferSize = 0;
    mDynamicUsage = false;
}

VertexBuffer9::~VertexBuffer9()
{
    SafeRelease(mVertexBuffer);
}

gl::Error VertexBuffer9::initialize(unsigned int size, bool dynamicUsage)
{
    SafeRelease(mVertexBuffer);

    updateSerial();

    if (size > 0)
    {
        DWORD flags = D3DUSAGE_WRITEONLY;
        if (dynamicUsage)
        {
            flags |= D3DUSAGE_DYNAMIC;
        }

        HRESULT result = mRenderer->createVertexBuffer(size, flags, &mVertexBuffer);

        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal vertex buffer of size, %lu.", size);
        }
    }

    mBufferSize = size;
    mDynamicUsage = dynamicUsage;
    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexBuffer9::storeVertexAttributes(const gl::VertexAttribute &attrib,
                                               GLenum currentValueType,
                                               GLint start,
                                               GLsizei count,
                                               GLsizei instances,
                                               unsigned int offset,
                                               const uint8_t *sourceData)
{
    if (!mVertexBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal vertex buffer is not initialized.");
    }

    int inputStride = static_cast<int>(gl::ComputeVertexAttributeStride(attrib));
    int elementSize = static_cast<int>(gl::ComputeVertexAttributeTypeSize(attrib));

    DWORD lockFlags = mDynamicUsage ? D3DLOCK_NOOVERWRITE : 0;

    uint8_t *mapPtr = NULL;

    unsigned int mapSize;
    gl::Error error = spaceRequired(attrib, count, instances, &mapSize);
    if (error.isError())
    {
        return error;
    }

    HRESULT result = mVertexBuffer->Lock(offset, mapSize, reinterpret_cast<void**>(&mapPtr), lockFlags);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to lock internal vertex buffer, HRESULT: 0x%08x.", result);
    }

    const uint8_t *input = sourceData;

    if (instances == 0 || attrib.divisor == 0)
    {
        input += inputStride * start;
    }

    gl::VertexFormatType vertexFormatType = gl::GetVertexFormatType(attrib, currentValueType);
    const d3d9::VertexFormat &d3dVertexInfo = d3d9::GetVertexFormatInfo(mRenderer->getCapsDeclTypes(), vertexFormatType);
    bool needsConversion = (d3dVertexInfo.conversionType & VERTEX_CONVERT_CPU) > 0;

    if (!needsConversion && inputStride == elementSize)
    {
        size_t copySize = static_cast<size_t>(count) * static_cast<size_t>(inputStride);
        memcpy(mapPtr, input, copySize);
    }
    else
    {
        d3dVertexInfo.copyFunction(input, inputStride, count, mapPtr);
    }

    mVertexBuffer->Unlock();

    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexBuffer9::getSpaceRequired(const gl::VertexAttribute &attrib, GLsizei count, GLsizei instances,
                                          unsigned int *outSpaceRequired) const
{
    return spaceRequired(attrib, count, instances, outSpaceRequired);
}

unsigned int VertexBuffer9::getBufferSize() const
{
    return mBufferSize;
}

gl::Error VertexBuffer9::setBufferSize(unsigned int size)
{
    if (size > mBufferSize)
    {
        return initialize(size, mDynamicUsage);
    }
    else
    {
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error VertexBuffer9::discard()
{
    if (!mVertexBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal vertex buffer is not initialized.");
    }

    void *dummy;
    HRESULT result;

    result = mVertexBuffer->Lock(0, 1, &dummy, D3DLOCK_DISCARD);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to lock internal buffer for discarding, HRESULT: 0x%08x", result);
    }

    result = mVertexBuffer->Unlock();
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to unlock internal buffer for discarding, HRESULT: 0x%08x", result);
    }

    return gl::Error(GL_NO_ERROR);
}

IDirect3DVertexBuffer9 * VertexBuffer9::getBuffer() const
{
    return mVertexBuffer;
}

gl::Error VertexBuffer9::spaceRequired(const gl::VertexAttribute &attrib, std::size_t count, GLsizei instances,
                                       unsigned int *outSpaceRequired) const
{
    gl::VertexFormatType vertexFormatType = gl::GetVertexFormatType(attrib, GL_FLOAT);
    const d3d9::VertexFormat &d3d9VertexInfo = d3d9::GetVertexFormatInfo(mRenderer->getCapsDeclTypes(), vertexFormatType);

    if (attrib.enabled)
    {
        unsigned int elementCount = 0;
        if (instances == 0 || attrib.divisor == 0)
        {
            elementCount = static_cast<unsigned int>(count);
        }
        else
        {
            // Round up to divisor, if possible
            elementCount = UnsignedCeilDivide(static_cast<unsigned int>(instances), attrib.divisor);
        }

        if (d3d9VertexInfo.outputElementSize <= std::numeric_limits<unsigned int>::max() / elementCount)
        {
            if (outSpaceRequired)
            {
                *outSpaceRequired =
                    static_cast<unsigned int>(d3d9VertexInfo.outputElementSize) * elementCount;
            }
            return gl::Error(GL_NO_ERROR);
        }
        else
        {
            return gl::Error(GL_OUT_OF_MEMORY, "New vertex buffer size would result in an overflow.");
        }
    }
    else
    {
        const unsigned int elementSize = 4;
        if (outSpaceRequired)
        {
            *outSpaceRequired = elementSize * 4;
        }
        return gl::Error(GL_NO_ERROR);
    }
}

}
