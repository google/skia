//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.cpp: Implements the class methods for VertexArrayGL.

#include "libANGLE/renderer/gl/VertexArrayGL.h"

#include "common/BitSetIterator.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

using namespace gl;

namespace rx
{
namespace
{
bool AttributeNeedsStreaming(const VertexAttribute &attribute)
{
    return (attribute.enabled && attribute.buffer.get() == nullptr);
}

}  // anonymous namespace

VertexArrayGL::VertexArrayGL(const VertexArray::Data &data,
                             const FunctionsGL *functions,
                             StateManagerGL *stateManager)
    : VertexArrayImpl(data),
      mFunctions(functions),
      mStateManager(stateManager),
      mVertexArrayID(0),
      mAppliedElementArrayBuffer(),
      mStreamingElementArrayBufferSize(0),
      mStreamingElementArrayBuffer(0),
      mStreamingArrayBufferSize(0),
      mStreamingArrayBuffer(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);
    mFunctions->genVertexArrays(1, &mVertexArrayID);

    // Set the cached vertex attribute array size
    GLint maxVertexAttribs;
    mFunctions->getIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    mAppliedAttributes.resize(maxVertexAttribs);
}

VertexArrayGL::~VertexArrayGL()
{
    mStateManager->deleteVertexArray(mVertexArrayID);
    mVertexArrayID = 0;

    mStateManager->deleteBuffer(mStreamingElementArrayBuffer);
    mStreamingElementArrayBufferSize = 0;
    mStreamingElementArrayBuffer = 0;

    mStateManager->deleteBuffer(mStreamingArrayBuffer);
    mStreamingArrayBufferSize = 0;
    mStreamingArrayBuffer = 0;

    mAppliedElementArrayBuffer.set(nullptr);
    for (size_t idx = 0; idx < mAppliedAttributes.size(); idx++)
    {
        mAppliedAttributes[idx].buffer.set(nullptr);
    }
}

gl::Error VertexArrayGL::syncDrawArraysState(const gl::AttributesMask &activeAttributesMask,
                                             GLint first,
                                             GLsizei count,
                                             GLsizei instanceCount) const
{
    return syncDrawState(activeAttributesMask, first, count, GL_NONE, nullptr, instanceCount, false,
                         nullptr);
}

gl::Error VertexArrayGL::syncDrawElementsState(const gl::AttributesMask &activeAttributesMask,
                                               GLsizei count,
                                               GLenum type,
                                               const GLvoid *indices,
                                               GLsizei instanceCount,
                                               bool primitiveRestartEnabled,
                                               const GLvoid **outIndices) const
{
    return syncDrawState(activeAttributesMask, 0, count, type, indices, instanceCount,
                         primitiveRestartEnabled, outIndices);
}

gl::Error VertexArrayGL::syncDrawState(const gl::AttributesMask &activeAttributesMask,
                                       GLint first,
                                       GLsizei count,
                                       GLenum type,
                                       const GLvoid *indices,
                                       GLsizei instanceCount,
                                       bool primitiveRestartEnabled,
                                       const GLvoid **outIndices) const
{
    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    // Check if any attributes need to be streamed, determines if the index range needs to be computed
    bool attributesNeedStreaming = mAttributesNeedStreaming.any();

    // Determine if an index buffer needs to be streamed and the range of vertices that need to be copied
    IndexRange indexRange;
    if (type != GL_NONE)
    {
        Error error = syncIndexData(count, type, indices, primitiveRestartEnabled,
                                    attributesNeedStreaming, &indexRange, outIndices);
        if (error.isError())
        {
            return error;
        }
    }
    else
    {
        // Not an indexed call, set the range to [first, first + count - 1]
        indexRange.start = first;
        indexRange.end = first + count - 1;
    }

    if (attributesNeedStreaming)
    {
        Error error = streamAttributes(activeAttributesMask, instanceCount, indexRange);
        if (error.isError())
        {
            return error;
        }
    }

    return Error(GL_NO_ERROR);
}

Error VertexArrayGL::syncIndexData(GLsizei count,
                                   GLenum type,
                                   const GLvoid *indices,
                                   bool primitiveRestartEnabled,
                                   bool attributesNeedStreaming,
                                   IndexRange *outIndexRange,
                                   const GLvoid **outIndices) const
{
    ASSERT(outIndices);

    gl::Buffer *elementArrayBuffer = mData.getElementArrayBuffer().get();

    // Need to check the range of indices if attributes need to be streamed
    if (elementArrayBuffer != nullptr)
    {
        if (elementArrayBuffer != mAppliedElementArrayBuffer.get())
        {
            const BufferGL *bufferGL = GetImplAs<BufferGL>(elementArrayBuffer);
            mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferGL->getBufferID());
            mAppliedElementArrayBuffer.set(elementArrayBuffer);
        }

        // Only compute the index range if the attributes also need to be streamed
        if (attributesNeedStreaming)
        {
            ptrdiff_t elementArrayBufferOffset = reinterpret_cast<ptrdiff_t>(indices);
            Error error = mData.getElementArrayBuffer()->getIndexRange(
                type, elementArrayBufferOffset, count, primitiveRestartEnabled, outIndexRange);
            if (error.isError())
            {
                return error;
            }
        }

        // Indices serves as an offset into the index buffer in this case, use the same value for the draw call
        *outIndices = indices;
    }
    else
    {
        // Need to stream the index buffer
        // TODO: if GLES, nothing needs to be streamed

        // Only compute the index range if the attributes also need to be streamed
        if (attributesNeedStreaming)
        {
            *outIndexRange = ComputeIndexRange(type, indices, count, primitiveRestartEnabled);
        }

        // Allocate the streaming element array buffer
        if (mStreamingElementArrayBuffer == 0)
        {
            mFunctions->genBuffers(1, &mStreamingElementArrayBuffer);
            mStreamingElementArrayBufferSize = 0;
        }

        mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, mStreamingElementArrayBuffer);
        mAppliedElementArrayBuffer.set(nullptr);

        // Make sure the element array buffer is large enough
        const Type &indexTypeInfo          = GetTypeInfo(type);
        size_t requiredStreamingBufferSize = indexTypeInfo.bytes * count;
        if (requiredStreamingBufferSize > mStreamingElementArrayBufferSize)
        {
            // Copy the indices in while resizing the buffer
            mFunctions->bufferData(GL_ELEMENT_ARRAY_BUFFER, requiredStreamingBufferSize, indices, GL_DYNAMIC_DRAW);
            mStreamingElementArrayBufferSize = requiredStreamingBufferSize;
        }
        else
        {
            // Put the indices at the beginning of the buffer
            mFunctions->bufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, requiredStreamingBufferSize, indices);
        }

        // Set the index offset for the draw call to zero since the supplied index pointer is to client data
        *outIndices = nullptr;
    }

    return Error(GL_NO_ERROR);
}

void VertexArrayGL::computeStreamingAttributeSizes(const gl::AttributesMask &activeAttributesMask,
                                                   GLsizei instanceCount,
                                                   const gl::IndexRange &indexRange,
                                                   size_t *outStreamingDataSize,
                                                   size_t *outMaxAttributeDataSize) const
{
    *outStreamingDataSize    = 0;
    *outMaxAttributeDataSize = 0;

    ASSERT(mAttributesNeedStreaming.any());

    const auto &attribs = mData.getVertexAttributes();
    for (unsigned int idx : angle::IterateBitSet(mAttributesNeedStreaming & activeAttributesMask))
    {
        const auto &attrib = attribs[idx];
        ASSERT(AttributeNeedsStreaming(attrib));

        // If streaming is going to be required, compute the size of the required buffer
        // and how much slack space at the beginning of the buffer will be required by determining
        // the attribute with the largest data size.
        size_t typeSize = ComputeVertexAttributeTypeSize(attrib);
        *outStreamingDataSize += typeSize * ComputeVertexAttributeElementCount(
                                                attrib, indexRange.vertexCount(), instanceCount);
        *outMaxAttributeDataSize = std::max(*outMaxAttributeDataSize, typeSize);
    }
}

gl::Error VertexArrayGL::streamAttributes(const gl::AttributesMask &activeAttributesMask,
                                          GLsizei instanceCount,
                                          const gl::IndexRange &indexRange) const
{
    // Sync the vertex attribute state and track what data needs to be streamed
    size_t streamingDataSize    = 0;
    size_t maxAttributeDataSize = 0;

    computeStreamingAttributeSizes(activeAttributesMask, instanceCount, indexRange,
                                   &streamingDataSize, &maxAttributeDataSize);

    if (streamingDataSize == 0)
    {
        return gl::Error(GL_NO_ERROR);
    }

    if (mStreamingArrayBuffer == 0)
    {
        mFunctions->genBuffers(1, &mStreamingArrayBuffer);
        mStreamingArrayBufferSize = 0;
    }

    // If first is greater than zero, a slack space needs to be left at the beginning of the buffer so that
    // the same 'first' argument can be passed into the draw call.
    const size_t bufferEmptySpace = maxAttributeDataSize * indexRange.start;
    const size_t requiredBufferSize = streamingDataSize + bufferEmptySpace;

    mStateManager->bindBuffer(GL_ARRAY_BUFFER, mStreamingArrayBuffer);
    if (requiredBufferSize > mStreamingArrayBufferSize)
    {
        mFunctions->bufferData(GL_ARRAY_BUFFER, requiredBufferSize, nullptr, GL_DYNAMIC_DRAW);
        mStreamingArrayBufferSize = requiredBufferSize;
    }

    // Unmapping a buffer can return GL_FALSE to indicate that the system has corrupted the data
    // somehow (such as by a screen change), retry writing the data a few times and return OUT_OF_MEMORY
    // if that fails.
    GLboolean unmapResult = GL_FALSE;
    size_t unmapRetryAttempts = 5;
    while (unmapResult != GL_TRUE && --unmapRetryAttempts > 0)
    {
        uint8_t *bufferPointer = reinterpret_cast<uint8_t*>(mFunctions->mapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        size_t curBufferOffset = bufferEmptySpace;

        const auto &attribs = mData.getVertexAttributes();
        for (unsigned int idx :
             angle::IterateBitSet(mAttributesNeedStreaming & activeAttributesMask))
        {
            const auto &attrib = attribs[idx];
            ASSERT(AttributeNeedsStreaming(attrib));

            const size_t streamedVertexCount =
                ComputeVertexAttributeElementCount(attrib, indexRange.vertexCount(), instanceCount);

            const size_t sourceStride = ComputeVertexAttributeStride(attrib);
            const size_t destStride   = ComputeVertexAttributeTypeSize(attrib);

            const uint8_t *inputPointer = reinterpret_cast<const uint8_t *>(attrib.pointer);

            // Pack the data when copying it, user could have supplied a very large stride that
            // would cause the buffer to be much larger than needed.
            if (destStride == sourceStride)
            {
                // Can copy in one go, the data is packed
                memcpy(bufferPointer + curBufferOffset,
                       inputPointer + (sourceStride * indexRange.start),
                       destStride * streamedVertexCount);
            }
            else
            {
                // Copy each vertex individually
                for (size_t vertexIdx = 0; vertexIdx < streamedVertexCount; vertexIdx++)
                {
                    uint8_t *out = bufferPointer + curBufferOffset + (destStride * vertexIdx);
                    const uint8_t *in =
                        inputPointer + sourceStride * (vertexIdx + indexRange.start);
                    memcpy(out, in, destStride);
                }
            }

            // Compute where the 0-index vertex would be.
            const size_t vertexStartOffset = curBufferOffset - (indexRange.start * destStride);

            if (attrib.pureInteger)
            {
                ASSERT(!attrib.normalized);
                mFunctions->vertexAttribIPointer(
                    idx, attrib.size, attrib.type, static_cast<GLsizei>(destStride),
                    reinterpret_cast<const GLvoid *>(vertexStartOffset));
            }
            else
            {
                mFunctions->vertexAttribPointer(
                    idx, attrib.size, attrib.type, attrib.normalized,
                    static_cast<GLsizei>(destStride),
                    reinterpret_cast<const GLvoid *>(vertexStartOffset));
            }

            curBufferOffset += destStride * streamedVertexCount;

            // Mark the applied attribute as dirty by setting an invalid size so that if it doesn't
            // need to be streamed later, there is no chance that the caching will skip it.
            mAppliedAttributes[idx].size = static_cast<GLuint>(-1);
        }

        unmapResult = mFunctions->unmapBuffer(GL_ARRAY_BUFFER);
    }

    if (unmapResult != GL_TRUE)
    {
        return Error(GL_OUT_OF_MEMORY, "Failed to unmap the client data streaming buffer.");
    }

    return Error(GL_NO_ERROR);
}

GLuint VertexArrayGL::getVertexArrayID() const
{
    return mVertexArrayID;
}

GLuint VertexArrayGL::getAppliedElementArrayBufferID() const
{
    if (mAppliedElementArrayBuffer.get() == nullptr)
    {
        return mStreamingElementArrayBuffer;
    }

    return GetImplAs<BufferGL>(mAppliedElementArrayBuffer.get())->getBufferID();
}

void VertexArrayGL::updateNeedsStreaming(size_t attribIndex)
{
    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    mAttributesNeedStreaming.set(attribIndex, AttributeNeedsStreaming(attrib));
}

void VertexArrayGL::updateAttribEnabled(size_t attribIndex)
{
    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    if (mAppliedAttributes[attribIndex].enabled == attrib.enabled)
    {
        return;
    }

    updateNeedsStreaming(attribIndex);

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());
    if (attrib.enabled)
    {
        mFunctions->enableVertexAttribArray(static_cast<GLuint>(attribIndex));
    }
    else
    {
        mFunctions->disableVertexAttribArray(static_cast<GLuint>(attribIndex));
    }
    mAppliedAttributes[attribIndex].enabled = attrib.enabled;
}

void VertexArrayGL::updateAttribPointer(size_t attribIndex)
{
    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    if (mAppliedAttributes[attribIndex] == attrib)
    {
        return;
    }

    updateNeedsStreaming(attribIndex);

    // If we need to stream, defer the attribPointer to the draw call.
    if (mAttributesNeedStreaming[attribIndex])
    {
        return;
    }

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());
    const Buffer *arrayBuffer = attrib.buffer.get();
    if (arrayBuffer != nullptr)
    {
        const BufferGL *arrayBufferGL = GetImplAs<BufferGL>(arrayBuffer);
        mStateManager->bindBuffer(GL_ARRAY_BUFFER, arrayBufferGL->getBufferID());
    }
    else
    {
        mStateManager->bindBuffer(GL_ARRAY_BUFFER, 0);
    }
    mAppliedAttributes[attribIndex].buffer = attrib.buffer;

    if (attrib.pureInteger)
    {
        mFunctions->vertexAttribIPointer(static_cast<GLuint>(attribIndex), attrib.size, attrib.type,
                                         attrib.stride, attrib.pointer);
    }
    else
    {
        mFunctions->vertexAttribPointer(static_cast<GLuint>(attribIndex), attrib.size, attrib.type,
                                        attrib.normalized, attrib.stride, attrib.pointer);
    }
    mAppliedAttributes[attribIndex].size        = attrib.size;
    mAppliedAttributes[attribIndex].type        = attrib.type;
    mAppliedAttributes[attribIndex].normalized  = attrib.normalized;
    mAppliedAttributes[attribIndex].pureInteger = attrib.pureInteger;
    mAppliedAttributes[attribIndex].stride      = attrib.stride;
    mAppliedAttributes[attribIndex].pointer     = attrib.pointer;
}

void VertexArrayGL::syncState(const VertexArray::DirtyBits &dirtyBits)
{
    for (unsigned long dirtyBit : angle::IterateBitSet(dirtyBits))
    {
        if (dirtyBit == VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER)
        {
            // TODO(jmadill): Element array buffer bindings
        }
        else if (dirtyBit >= VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED &&
                 dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_ENABLED)
        {
            size_t attribIndex =
                static_cast<size_t>(dirtyBit) - VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED;
            updateAttribEnabled(attribIndex);
        }
        else if (dirtyBit >= VertexArray::DIRTY_BIT_ATTRIB_0_POINTER &&
                 dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_POINTER)
        {
            size_t attribIndex =
                static_cast<size_t>(dirtyBit) - VertexArray::DIRTY_BIT_ATTRIB_0_POINTER;
            updateAttribPointer(attribIndex);
        }
        else if (dirtyBit >= VertexArray::DIRTY_BIT_ATTRIB_0_DIVISOR &&
                 dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_DIVISOR)
        {
            size_t attribIndex =
                static_cast<size_t>(dirtyBit) - VertexArray::DIRTY_BIT_ATTRIB_0_DIVISOR;
            const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);

            if (mAppliedAttributes[attribIndex].divisor != attrib.divisor)
            {
                mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());
                mFunctions->vertexAttribDivisor(static_cast<GLuint>(attribIndex), attrib.divisor);
                mAppliedAttributes[attribIndex].divisor = attrib.divisor;
            }
        }
        else
            UNREACHABLE();
    }
}

}  // rx
