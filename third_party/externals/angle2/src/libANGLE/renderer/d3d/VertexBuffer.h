//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer.h: Defines the abstract VertexBuffer class and VertexBufferInterface
// class with derivations, classes that perform graphics API agnostic vertex buffer operations.

#ifndef LIBANGLE_RENDERER_D3D_VERTEXBUFFER_H_
#define LIBANGLE_RENDERER_D3D_VERTEXBUFFER_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"

#include <GLES2/gl2.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace gl
{
struct VertexAttribute;
struct VertexAttribCurrentValueData;
}

namespace rx
{
class BufferFactoryD3D;

class VertexBuffer : angle::NonCopyable
{
  public:
    VertexBuffer();
    virtual ~VertexBuffer();

    virtual gl::Error initialize(unsigned int size, bool dynamicUsage) = 0;

    virtual gl::Error storeVertexAttributes(const gl::VertexAttribute &attrib,
                                            GLenum currentValueType,
                                            GLint start,
                                            GLsizei count,
                                            GLsizei instances,
                                            unsigned int offset,
                                            const uint8_t *sourceData) = 0;
    virtual gl::Error getSpaceRequired(const gl::VertexAttribute &attrib, GLsizei count, GLsizei instances,
                                       unsigned int *outSpaceRequired) const = 0;

    virtual unsigned int getBufferSize() const = 0;
    virtual gl::Error setBufferSize(unsigned int size) = 0;
    virtual gl::Error discard() = 0;

    unsigned int getSerial() const;

    // This may be overridden (e.g. by VertexBuffer11) if necessary.
    virtual void hintUnmapResource() { };

  protected:
    void updateSerial();

  private:
    unsigned int mSerial;
    static unsigned int mNextSerial;
};

class VertexBufferInterface : angle::NonCopyable
{
  public:
    VertexBufferInterface(BufferFactoryD3D *factory, bool dynamic);
    virtual ~VertexBufferInterface();

    gl::Error reserveVertexSpace(const gl::VertexAttribute &attribute, GLsizei count, GLsizei instances);

    unsigned int getBufferSize() const;

    unsigned int getSerial() const;

    virtual gl::Error storeVertexAttributes(const gl::VertexAttribute &attrib,
                                            GLenum currentValueType,
                                            GLint start,
                                            GLsizei count,
                                            GLsizei instances,
                                            unsigned int *outStreamOffset,
                                            const uint8_t *sourceData);

    bool directStoragePossible(const gl::VertexAttribute &attrib,
                               GLenum currentValueType) const;

    VertexBuffer* getVertexBuffer() const;

  protected:
    virtual gl::Error reserveSpace(unsigned int size) = 0;

    unsigned int getWritePosition() const;
    void setWritePosition(unsigned int writePosition);

    gl::Error discard();

    gl::Error setBufferSize(unsigned int size);

  private:
    BufferFactoryD3D *const mFactory;

    VertexBuffer* mVertexBuffer;

    unsigned int mWritePosition;
    unsigned int mReservedSpace;
    bool mDynamic;
};

class StreamingVertexBufferInterface : public VertexBufferInterface
{
  public:
    StreamingVertexBufferInterface(BufferFactoryD3D *factory, std::size_t initialSize);
    ~StreamingVertexBufferInterface();

  protected:
    gl::Error reserveSpace(unsigned int size);
};

class StaticVertexBufferInterface : public VertexBufferInterface
{
  public:
    explicit StaticVertexBufferInterface(BufferFactoryD3D *factory);
    ~StaticVertexBufferInterface();

    gl::Error storeVertexAttributes(const gl::VertexAttribute &attrib,
                                    GLenum currentValueType,
                                    GLint start,
                                    GLsizei count,
                                    GLsizei instances,
                                    unsigned int *outStreamOffset,
                                    const uint8_t *sourceData) override;

    bool lookupAttribute(const gl::VertexAttribute &attribute, unsigned int* outStreamFffset);

  protected:
    gl::Error reserveSpace(unsigned int size);

  private:
    struct VertexElement
    {
        GLenum type;
        GLuint size;
        GLuint stride;
        bool normalized;
        bool pureInteger;
        size_t attributeOffset;

        unsigned int streamOffset;
    };

    std::vector<VertexElement> mCache;
};

}

#endif // LIBANGLE_RENDERER_D3D_VERTEXBUFFER_H_
