//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Indexffer9.cpp: Defines the D3D9 IndexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d9/IndexBuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"

namespace rx
{

IndexBuffer9::IndexBuffer9(Renderer9 *const renderer) : mRenderer(renderer)
{
    mIndexBuffer = NULL;
    mBufferSize = 0;
    mIndexType = 0;
    mDynamic = false;
}

IndexBuffer9::~IndexBuffer9()
{
    SafeRelease(mIndexBuffer);
}

gl::Error IndexBuffer9::initialize(unsigned int bufferSize, GLenum indexType, bool dynamic)
{
    SafeRelease(mIndexBuffer);

    updateSerial();

    if (bufferSize > 0)
    {
        D3DFORMAT format = D3DFMT_UNKNOWN;
        if (indexType == GL_UNSIGNED_SHORT || indexType == GL_UNSIGNED_BYTE)
        {
            format = D3DFMT_INDEX16;
        }
        else if (indexType == GL_UNSIGNED_INT)
        {
            ASSERT(mRenderer->getRendererExtensions().elementIndexUint);
            format = D3DFMT_INDEX32;
        }
        else UNREACHABLE();

        DWORD usageFlags = D3DUSAGE_WRITEONLY;
        if (dynamic)
        {
            usageFlags |= D3DUSAGE_DYNAMIC;
        }

        HRESULT result = mRenderer->createIndexBuffer(bufferSize, usageFlags, format, &mIndexBuffer);
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal index buffer of size, %lu.", bufferSize);
        }
    }

    mBufferSize = bufferSize;
    mIndexType = indexType;
    mDynamic = dynamic;

    return gl::Error(GL_NO_ERROR);
}

gl::Error IndexBuffer9::mapBuffer(unsigned int offset, unsigned int size, void** outMappedMemory)
{
    if (!mIndexBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal index buffer is not initialized.");
    }

    DWORD lockFlags = mDynamic ? D3DLOCK_NOOVERWRITE : 0;

    void *mapPtr = NULL;
    HRESULT result = mIndexBuffer->Lock(offset, size, &mapPtr, lockFlags);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to lock internal index buffer, HRESULT: 0x%08x.", result);
    }

    *outMappedMemory = mapPtr;
    return gl::Error(GL_NO_ERROR);
}

gl::Error IndexBuffer9::unmapBuffer()
{
    if (!mIndexBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal index buffer is not initialized.");
    }

    HRESULT result = mIndexBuffer->Unlock();
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to unlock internal index buffer, HRESULT: 0x%08x.", result);
    }

    return gl::Error(GL_NO_ERROR);
}

GLenum IndexBuffer9::getIndexType() const
{
    return mIndexType;
}

unsigned int IndexBuffer9::getBufferSize() const
{
    return mBufferSize;
}

gl::Error IndexBuffer9::setSize(unsigned int bufferSize, GLenum indexType)
{
    if (bufferSize > mBufferSize || indexType != mIndexType)
    {
        return initialize(bufferSize, indexType, mDynamic);
    }
    else
    {
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error IndexBuffer9::discard()
{
    if (!mIndexBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal index buffer is not initialized.");
    }

    void *dummy;
    HRESULT result;

    result = mIndexBuffer->Lock(0, 1, &dummy, D3DLOCK_DISCARD);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to lock internal index buffer, HRESULT: 0x%08x.", result);
    }

    result = mIndexBuffer->Unlock();
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to unlock internal index buffer, HRESULT: 0x%08x.", result);
    }

    return gl::Error(GL_NO_ERROR);
}

D3DFORMAT IndexBuffer9::getIndexFormat() const
{
    switch (mIndexType)
    {
      case GL_UNSIGNED_BYTE:    return D3DFMT_INDEX16;
      case GL_UNSIGNED_SHORT:   return D3DFMT_INDEX16;
      case GL_UNSIGNED_INT:     return D3DFMT_INDEX32;
      default: UNREACHABLE();   return D3DFMT_UNKNOWN;
    }
}

IDirect3DIndexBuffer9 * IndexBuffer9::getBuffer() const
{
    return mIndexBuffer;
}

}
