//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer.cpp: Implements the gl::Buffer class, representing storage of vertex and/or
// index data. Implements GL buffer objects and related functionality.
// [OpenGL ES 2.0.24] section 2.9 page 21.

#include "libANGLE/Buffer.h"
#include "libANGLE/renderer/BufferImpl.h"
#include "libANGLE/renderer/Renderer.h"

namespace gl
{

Buffer::Buffer(rx::BufferImpl *impl, GLuint id)
    : RefCountObject(id),
      mBuffer(impl),
      mUsage(GL_STATIC_DRAW),
      mSize(0),
      mAccessFlags(0),
      mAccess(GL_WRITE_ONLY_OES),
      mMapped(GL_FALSE),
      mMapPointer(NULL),
      mMapOffset(0),
      mMapLength(0)
{
}

Buffer::~Buffer()
{
    SafeDelete(mBuffer);
}

Error Buffer::bufferData(const void *data, GLsizeiptr size, GLenum usage)
{
    gl::Error error = mBuffer->setData(data, size, usage);
    if (error.isError())
    {
        return error;
    }

    mIndexRangeCache.clear();
    mUsage = usage;
    mSize = size;

    return error;
}

Error Buffer::bufferSubData(const void *data, GLsizeiptr size, GLintptr offset)
{
    gl::Error error = mBuffer->setSubData(data, size, offset);
    if (error.isError())
    {
        return error;
    }

    mIndexRangeCache.invalidateRange(static_cast<unsigned int>(offset), static_cast<unsigned int>(size));

    return error;
}

Error Buffer::copyBufferSubData(Buffer* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size)
{
    gl::Error error = mBuffer->copySubData(source->getImplementation(), sourceOffset, destOffset, size);
    if (error.isError())
    {
        return error;
    }

    mIndexRangeCache.invalidateRange(static_cast<unsigned int>(destOffset), static_cast<unsigned int>(size));

    return error;
}

Error Buffer::map(GLenum access)
{
    ASSERT(!mMapped);

    Error error = mBuffer->map(access, &mMapPointer);
    if (error.isError())
    {
        mMapPointer = NULL;
        return error;
    }

    ASSERT(access == GL_WRITE_ONLY_OES);

    mMapped = GL_TRUE;
    mMapOffset = 0;
    mMapLength = mSize;
    mAccess = access;
    mAccessFlags = GL_MAP_WRITE_BIT;
    mIndexRangeCache.clear();

    return error;
}

Error Buffer::mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    ASSERT(!mMapped);
    ASSERT(offset + length <= mSize);

    Error error = mBuffer->mapRange(offset, length, access, &mMapPointer);
    if (error.isError())
    {
        mMapPointer = NULL;
        return error;
    }

    mMapped = GL_TRUE;
    mMapOffset = static_cast<GLint64>(offset);
    mMapLength = static_cast<GLint64>(length);
    mAccess = GL_WRITE_ONLY_OES;
    mAccessFlags = access;

    // The OES_mapbuffer extension states that GL_WRITE_ONLY_OES is the only valid
    // value for GL_BUFFER_ACCESS_OES because it was written against ES2.  Since there is
    // no update for ES3 and the GL_READ_ONLY and GL_READ_WRITE enums don't exist for ES,
    // we cannot properly set GL_BUFFER_ACCESS_OES when glMapBufferRange is called.

    if ((access & GL_MAP_WRITE_BIT) > 0)
    {
        mIndexRangeCache.invalidateRange(static_cast<unsigned int>(offset), static_cast<unsigned int>(length));
    }

    return error;
}

Error Buffer::unmap(GLboolean *result)
{
    ASSERT(mMapped);

    Error error = mBuffer->unmap(result);
    if (error.isError())
    {
        *result = GL_FALSE;
        return error;
    }

    mMapped = GL_FALSE;
    mMapPointer = NULL;
    mMapOffset = 0;
    mMapLength = 0;
    mAccess = GL_WRITE_ONLY_OES;
    mAccessFlags = 0;

    return error;
}

void Buffer::onTransformFeedback()
{
    mIndexRangeCache.clear();
}

void Buffer::onPixelUnpack()
{
    mIndexRangeCache.clear();
}

Error Buffer::getIndexRange(GLenum type,
                            size_t offset,
                            size_t count,
                            bool primitiveRestartEnabled,
                            IndexRange *outRange) const
{
    if (mIndexRangeCache.findRange(type, offset, count, primitiveRestartEnabled, outRange))
    {
        return gl::Error(GL_NO_ERROR);
    }

    Error error = mBuffer->getIndexRange(type, offset, count, primitiveRestartEnabled, outRange);
    if (error.isError())
    {
        return error;
    }

    mIndexRangeCache.addRange(type, offset, count, primitiveRestartEnabled, *outRange);

    return Error(GL_NO_ERROR);
}

}
