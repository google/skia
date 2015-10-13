//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer.h: Defines the gl::Buffer class, representing storage of vertex and/or
// index data. Implements GL buffer objects and related functionality.
// [OpenGL ES 2.0.24] section 2.9 page 21.

#ifndef LIBANGLE_BUFFER_H_
#define LIBANGLE_BUFFER_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/IndexRangeCache.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class BufferImpl;
};

namespace gl
{

class Buffer : public RefCountObject
{
  public:
    Buffer(rx::BufferImpl *impl, GLuint id);

    virtual ~Buffer();

    Error bufferData(const void *data, GLsizeiptr size, GLenum usage);
    Error bufferSubData(const void *data, GLsizeiptr size, GLintptr offset);
    Error copyBufferSubData(Buffer* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size);
    Error map(GLenum access);
    Error mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access);
    Error unmap(GLboolean *result);

    void onTransformFeedback();
    void onPixelUnpack();

    Error getIndexRange(GLenum type,
                        size_t offset,
                        size_t count,
                        bool primitiveRestartEnabled,
                        IndexRange *outRange) const;

    GLenum getUsage() const { return mUsage; }
    GLbitfield getAccessFlags() const { return mAccessFlags; }
    GLenum getAccess() const { return mAccess; }
    GLboolean isMapped() const { return mMapped; }
    GLvoid *getMapPointer() const { return mMapPointer; }
    GLint64 getMapOffset() const { return mMapOffset; }
    GLint64 getMapLength() const { return mMapLength; }
    GLint64 getSize() const { return mSize; }

    rx::BufferImpl *getImplementation() const { return mBuffer; }

  private:
    rx::BufferImpl *mBuffer;

    GLenum mUsage;
    GLint64 mSize;
    GLbitfield mAccessFlags;
    GLenum mAccess;
    GLboolean mMapped;
    GLvoid *mMapPointer;
    GLint64 mMapOffset;
    GLint64 mMapLength;

    mutable IndexRangeCache mIndexRangeCache;
};

}

#endif   // LIBANGLE_BUFFER_H_
