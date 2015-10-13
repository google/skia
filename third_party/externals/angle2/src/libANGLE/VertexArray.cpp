//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the state class for mananging GLES 3 Vertex Array Objects.
//

#include "libANGLE/VertexArray.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/renderer/ImplFactory.h"
#include "libANGLE/renderer/VertexArrayImpl.h"

namespace gl
{

VertexArray::Data::Data(size_t maxAttribs)
    : mVertexAttributes(maxAttribs),
      mMaxEnabledAttribute(0)
{
}

VertexArray::Data::~Data()
{
    for (size_t i = 0; i < getMaxAttribs(); i++)
    {
        mVertexAttributes[i].buffer.set(nullptr);
    }
    mElementArrayBuffer.set(nullptr);
}

VertexArray::VertexArray(rx::ImplFactory *factory, GLuint id, size_t maxAttribs)
    : mId(id),
      mVertexArray(factory->createVertexArray(mData)),
      mData(maxAttribs)
{
    ASSERT(mVertexArray != nullptr);
}

VertexArray::~VertexArray()
{
    SafeDelete(mVertexArray);
}

GLuint VertexArray::id() const
{
    return mId;
}

void VertexArray::detachBuffer(GLuint bufferName)
{
    for (size_t attribute = 0; attribute < getMaxAttribs(); attribute++)
    {
        if (mData.mVertexAttributes[attribute].buffer.id() == bufferName)
        {
            mData.mVertexAttributes[attribute].buffer.set(nullptr);
        }
    }

    if (mData.mElementArrayBuffer.id() == bufferName)
    {
        mData.mElementArrayBuffer.set(nullptr);
    }
}

const VertexAttribute &VertexArray::getVertexAttribute(size_t attributeIndex) const
{
    ASSERT(attributeIndex < getMaxAttribs());
    return mData.mVertexAttributes[attributeIndex];
}

void VertexArray::setVertexAttribDivisor(size_t index, GLuint divisor)
{
    ASSERT(index < getMaxAttribs());
    mData.mVertexAttributes[index].divisor = divisor;
    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_DIVISOR + index);
}

void VertexArray::enableAttribute(size_t attributeIndex, bool enabledState)
{
    ASSERT(attributeIndex < getMaxAttribs());
    mData.mVertexAttributes[attributeIndex].enabled = enabledState;
    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_ENABLED + attributeIndex);

    // Update state cache
    if (enabledState)
    {
        mData.mMaxEnabledAttribute = std::max(attributeIndex + 1, mData.mMaxEnabledAttribute);
    }
    else if (mData.mMaxEnabledAttribute == attributeIndex + 1)
    {
        while (mData.mMaxEnabledAttribute > 0 &&
               !mData.mVertexAttributes[mData.mMaxEnabledAttribute - 1].enabled)
        {
            --mData.mMaxEnabledAttribute;
        }
    }
}

void VertexArray::setAttributeState(size_t attributeIndex, gl::Buffer *boundBuffer, GLint size, GLenum type,
                                    bool normalized, bool pureInteger, GLsizei stride, const void *pointer)
{
    ASSERT(attributeIndex < getMaxAttribs());

    VertexAttribute *attrib = &mData.mVertexAttributes[attributeIndex];

    attrib->buffer.set(boundBuffer);
    attrib->size = size;
    attrib->type = type;
    attrib->normalized = normalized;
    attrib->pureInteger = pureInteger;
    attrib->stride = stride;
    attrib->pointer = pointer;
    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_POINTER + attributeIndex);
}

void VertexArray::setElementArrayBuffer(Buffer *buffer)
{
    mData.mElementArrayBuffer.set(buffer);
    mDirtyBits.set(DIRTY_BIT_ELEMENT_ARRAY_BUFFER);
}

void VertexArray::syncImplState()
{
    if (mDirtyBits.any())
    {
        mVertexArray->syncState(mDirtyBits);
        mDirtyBits.reset();
    }
}

}
