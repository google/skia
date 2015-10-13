//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBuffer.h: Defines the abstract IndexBuffer class and IndexBufferInterface
// class with derivations, classes that perform graphics API agnostic index buffer operations.

#ifndef LIBANGLE_RENDERER_D3D_INDEXBUFFER_H_
#define LIBANGLE_RENDERER_D3D_INDEXBUFFER_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"

namespace rx
{
class BufferFactoryD3D;

class IndexBuffer : angle::NonCopyable
{
  public:
    IndexBuffer();
    virtual ~IndexBuffer();

    virtual gl::Error initialize(unsigned int bufferSize, GLenum indexType, bool dynamic) = 0;

    virtual gl::Error mapBuffer(unsigned int offset, unsigned int size, void** outMappedMemory) = 0;
    virtual gl::Error unmapBuffer() = 0;

    virtual gl::Error discard() = 0;

    virtual GLenum getIndexType() const = 0;
    virtual unsigned int getBufferSize() const = 0;
    virtual gl::Error setSize(unsigned int bufferSize, GLenum indexType) = 0;

    unsigned int getSerial() const;

  protected:
    void updateSerial();

  private:
    unsigned int mSerial;
    static unsigned int mNextSerial;
};

class IndexBufferInterface : angle::NonCopyable
{
  public:
    IndexBufferInterface(BufferFactoryD3D *factory, bool dynamic);
    virtual ~IndexBufferInterface();

    virtual gl::Error reserveBufferSpace(unsigned int size, GLenum indexType) = 0;

    GLenum getIndexType() const;
    unsigned int getBufferSize() const;

    unsigned int getSerial() const;

    gl::Error mapBuffer(unsigned int size, void** outMappedMemory, unsigned int *streamOffset);
    gl::Error unmapBuffer();

    IndexBuffer *getIndexBuffer() const;

  protected:
    unsigned int getWritePosition() const;
    void setWritePosition(unsigned int writePosition);

    gl::Error discard();

    gl::Error setBufferSize(unsigned int bufferSize, GLenum indexType);

  private:
    IndexBuffer *mIndexBuffer;

    unsigned int mWritePosition;
    bool mDynamic;
};

class StreamingIndexBufferInterface : public IndexBufferInterface
{
  public:
    explicit StreamingIndexBufferInterface(BufferFactoryD3D *factory);
    ~StreamingIndexBufferInterface();

    gl::Error reserveBufferSpace(unsigned int size, GLenum indexType) override;
};

class StaticIndexBufferInterface : public IndexBufferInterface
{
  public:
    explicit StaticIndexBufferInterface(BufferFactoryD3D *factory);
    ~StaticIndexBufferInterface();

    gl::Error reserveBufferSpace(unsigned int size, GLenum indexType) override;
};

}

#endif // LIBANGLE_RENDERER_D3D_INDEXBUFFER_H_
