//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexDataManager.h: Defines the VertexDataManager, a class that
// runs the Buffer translation process.

#include "libANGLE/renderer/d3d/VertexDataManager.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Program.h"
#include "libANGLE/State.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/VertexBuffer.h"

namespace
{
    enum { INITIAL_STREAM_BUFFER_SIZE = 1024*1024 };
    // This has to be at least 4k or else it fails on ATI cards.
    enum { CONSTANT_VERTEX_BUFFER_SIZE = 4096 };
}

namespace rx
{

static int ElementsInBuffer(const gl::VertexAttribute &attrib, unsigned int size)
{
    // Size cannot be larger than a GLsizei
    if (size > static_cast<unsigned int>(std::numeric_limits<int>::max()))
    {
        size = static_cast<unsigned int>(std::numeric_limits<int>::max());
    }

    GLsizei stride = static_cast<GLsizei>(ComputeVertexAttributeStride(attrib));
    return (size - attrib.offset % stride +
            (stride - static_cast<GLsizei>(ComputeVertexAttributeTypeSize(attrib)))) /
           stride;
}

VertexDataManager::CurrentValueState::CurrentValueState()
    : buffer(nullptr),
      offset(0)
{
    data.FloatValues[0] = std::numeric_limits<float>::quiet_NaN();
    data.FloatValues[1] = std::numeric_limits<float>::quiet_NaN();
    data.FloatValues[2] = std::numeric_limits<float>::quiet_NaN();
    data.FloatValues[3] = std::numeric_limits<float>::quiet_NaN();
    data.Type = GL_FLOAT;
}

VertexDataManager::CurrentValueState::~CurrentValueState()
{
    SafeDelete(buffer);
}

VertexDataManager::VertexDataManager(BufferFactoryD3D *factory)
    : mFactory(factory),
      mStreamingBuffer(nullptr),
      // TODO(jmadill): use context caps
      mCurrentValueCache(gl::MAX_VERTEX_ATTRIBS)
{
    mStreamingBuffer = new StreamingVertexBufferInterface(factory, INITIAL_STREAM_BUFFER_SIZE);

    if (!mStreamingBuffer)
    {
        ERR("Failed to allocate the streaming vertex buffer.");
    }

    // TODO(jmadill): use context caps
    mActiveEnabledAttributes.reserve(gl::MAX_VERTEX_ATTRIBS);
    mActiveDisabledAttributes.reserve(gl::MAX_VERTEX_ATTRIBS);
}

VertexDataManager::~VertexDataManager()
{
    SafeDelete(mStreamingBuffer);
}

void VertexDataManager::hintUnmapAllResources(const std::vector<gl::VertexAttribute> &vertexAttributes)
{
    mStreamingBuffer->getVertexBuffer()->hintUnmapResource();

    for (const TranslatedAttribute *translated : mActiveEnabledAttributes)
    {
        gl::Buffer *buffer = translated->attribute->buffer.get();
        BufferD3D *storage = buffer ? GetImplAs<BufferD3D>(buffer) : nullptr;
        StaticVertexBufferInterface *staticBuffer = storage ? storage->getStaticVertexBuffer() : nullptr;

        if (staticBuffer)
        {
            staticBuffer->getVertexBuffer()->hintUnmapResource();
        }
    }

    for (auto &currentValue : mCurrentValueCache)
    {
        if (currentValue.buffer != nullptr)
        {
            currentValue.buffer->getVertexBuffer()->hintUnmapResource();
        }
    }
}

gl::Error VertexDataManager::prepareVertexData(const gl::State &state,
                                               GLint start,
                                               GLsizei count,
                                               std::vector<TranslatedAttribute> *translatedAttribs,
                                               GLsizei instances)
{
    if (!mStreamingBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal streaming vertex buffer is unexpectedly NULL.");
    }

    // Compute active enabled and active disable attributes, for speed.
    // TODO(jmadill): don't recompute if there was no state change
    const gl::VertexArray *vertexArray = state.getVertexArray();
    const gl::Program *program         = state.getProgram();
    const auto &vertexAttributes       = vertexArray->getVertexAttributes();

    mActiveEnabledAttributes.clear();
    mActiveDisabledAttributes.clear();
    translatedAttribs->clear();

    for (size_t attribIndex = 0; attribIndex < vertexAttributes.size(); ++attribIndex)
    {
        if (program->isAttribLocationActive(attribIndex))
        {
            // Resize automatically puts in empty attribs
            translatedAttribs->resize(attribIndex + 1);

            TranslatedAttribute *translated = &(*translatedAttribs)[attribIndex];

            // Record the attribute now
            translated->active = true;
            translated->attribute = &vertexAttributes[attribIndex];
            translated->currentValueType =
                state.getVertexAttribCurrentValue(static_cast<unsigned int>(attribIndex)).Type;
            translated->divisor = vertexAttributes[attribIndex].divisor;

            if (vertexAttributes[attribIndex].enabled)
            {
                mActiveEnabledAttributes.push_back(translated);

                // Also invalidate static buffers that don't contain matching attributes
                invalidateMatchingStaticData(
                    vertexAttributes[attribIndex],
                    state.getVertexAttribCurrentValue(static_cast<unsigned int>(attribIndex)));
            }
            else
            {
                mActiveDisabledAttributes.push_back(attribIndex);
            }
        }
    }

    // Reserve the required space in the buffers
    for (const TranslatedAttribute *activeAttrib : mActiveEnabledAttributes)
    {
        gl::Error error = reserveSpaceForAttrib(*activeAttrib, count, instances);
        if (error.isError())
        {
            return error;
        }
    }

    // Perform the vertex data translations
    for (TranslatedAttribute *activeAttrib : mActiveEnabledAttributes)
    {
        gl::Error error = storeAttribute(activeAttrib, start, count, instances);

        if (error.isError())
        {
            hintUnmapAllResources(vertexAttributes);
            return error;
        }
    }

    for (size_t attribIndex : mActiveDisabledAttributes)
    {
        if (mCurrentValueCache[attribIndex].buffer == nullptr)
        {
            mCurrentValueCache[attribIndex].buffer = new StreamingVertexBufferInterface(mFactory, CONSTANT_VERTEX_BUFFER_SIZE);
        }

        gl::Error error = storeCurrentValue(
            state.getVertexAttribCurrentValue(static_cast<unsigned int>(attribIndex)),
            &(*translatedAttribs)[attribIndex], &mCurrentValueCache[attribIndex]);
        if (error.isError())
        {
            hintUnmapAllResources(vertexAttributes);
            return error;
        }
    }

    // Hint to unmap all the resources
    hintUnmapAllResources(vertexAttributes);

    for (const TranslatedAttribute *activeAttrib : mActiveEnabledAttributes)
    {
        gl::Buffer *buffer = activeAttrib->attribute->buffer.get();

        if (buffer)
        {
            BufferD3D *bufferD3D = GetImplAs<BufferD3D>(buffer);
            size_t typeSize = ComputeVertexAttributeTypeSize(*activeAttrib->attribute);
            bufferD3D->promoteStaticUsage(count * static_cast<int>(typeSize));
        }
    }

    return gl::Error(GL_NO_ERROR);
}

void VertexDataManager::invalidateMatchingStaticData(const gl::VertexAttribute &attrib,
                                                     const gl::VertexAttribCurrentValueData &currentValue) const
{
    gl::Buffer *buffer = attrib.buffer.get();

    if (buffer)
    {
        BufferD3D *bufferImpl = GetImplAs<BufferD3D>(buffer);
        StaticVertexBufferInterface *staticBuffer = bufferImpl->getStaticVertexBuffer();

        if (staticBuffer &&
            staticBuffer->getBufferSize() > 0 &&
            !staticBuffer->lookupAttribute(attrib, NULL) &&
            !staticBuffer->directStoragePossible(attrib, currentValue.Type))
        {
            bufferImpl->invalidateStaticData();
        }
    }
}

gl::Error VertexDataManager::reserveSpaceForAttrib(const TranslatedAttribute &translatedAttrib,
                                                   GLsizei count,
                                                   GLsizei instances) const
{
    const gl::VertexAttribute &attrib = *translatedAttrib.attribute;
    gl::Buffer *buffer = attrib.buffer.get();
    BufferD3D *bufferImpl = buffer ? GetImplAs<BufferD3D>(buffer) : NULL;
    StaticVertexBufferInterface *staticBuffer = bufferImpl ? bufferImpl->getStaticVertexBuffer() : NULL;
    VertexBufferInterface *vertexBuffer = staticBuffer ? staticBuffer : static_cast<VertexBufferInterface*>(mStreamingBuffer);

    if (!vertexBuffer->directStoragePossible(attrib, translatedAttrib.currentValueType))
    {
        if (staticBuffer)
        {
            if (staticBuffer->getBufferSize() == 0)
            {
                int totalCount =
                    ElementsInBuffer(attrib, static_cast<unsigned int>(bufferImpl->getSize()));
                gl::Error error = staticBuffer->reserveVertexSpace(attrib, totalCount, 0);
                if (error.isError())
                {
                    return error;
                }
            }
        }
        else
        {
            size_t totalCount = ComputeVertexAttributeElementCount(attrib, count, instances);
            ASSERT(!bufferImpl ||
                   ElementsInBuffer(attrib, static_cast<unsigned int>(bufferImpl->getSize())) >=
                       static_cast<int>(totalCount));

            gl::Error error = mStreamingBuffer->reserveVertexSpace(
                attrib, static_cast<GLsizei>(totalCount), instances);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexDataManager::storeAttribute(TranslatedAttribute *translated,
                                            GLint start,
                                            GLsizei count,
                                            GLsizei instances)
{
    const gl::VertexAttribute &attrib = *translated->attribute;

    gl::Buffer *buffer = attrib.buffer.get();
    ASSERT(buffer || attrib.pointer);
    ASSERT(attrib.enabled);

    BufferD3D *storage = buffer ? GetImplAs<BufferD3D>(buffer) : NULL;
    StaticVertexBufferInterface *staticBuffer = storage ? storage->getStaticVertexBuffer() : NULL;
    VertexBufferInterface *vertexBuffer = staticBuffer ? staticBuffer : static_cast<VertexBufferInterface*>(mStreamingBuffer);
    bool directStorage = vertexBuffer->directStoragePossible(attrib, translated->currentValueType);

    // Instanced vertices do not apply the 'start' offset
    GLint firstVertexIndex = (instances > 0 && attrib.divisor > 0 ? 0 : start);

    translated->vertexBuffer = vertexBuffer->getVertexBuffer();

    if (directStorage)
    {
        translated->storage = storage;
        translated->serial = storage->getSerial();
        translated->stride  = static_cast<unsigned int>(ComputeVertexAttributeStride(attrib));
        translated->offset = static_cast<unsigned int>(attrib.offset + translated->stride * firstVertexIndex);

        return gl::Error(GL_NO_ERROR);
    }

    // Compute source data pointer
    const uint8_t *sourceData = nullptr;

    if (buffer)
    {
        gl::Error error = storage->getData(&sourceData);
        if (error.isError())
        {
            return error;
        }
        sourceData += static_cast<int>(attrib.offset);
    }
    else
    {
        sourceData = static_cast<const uint8_t*>(attrib.pointer);
    }

    unsigned int streamOffset = 0;
    unsigned int outputElementSize = 0;

    if (staticBuffer)
    {
        gl::Error error = staticBuffer->getVertexBuffer()->getSpaceRequired(attrib, 1, 0, &outputElementSize);
        if (error.isError())
        {
            return error;
        }

        if (!staticBuffer->lookupAttribute(attrib, &streamOffset))
        {
            // Convert the entire buffer
            int totalCount =
                ElementsInBuffer(attrib, static_cast<unsigned int>(storage->getSize()));
            int startIndex = static_cast<int>(attrib.offset) /
                             static_cast<int>(ComputeVertexAttributeStride(attrib));

            error = staticBuffer->storeVertexAttributes(attrib,
                                                        translated->currentValueType,
                                                        -startIndex,
                                                        totalCount,
                                                        0,
                                                        &streamOffset,
                                                        sourceData);
            if (error.isError())
            {
                return error;
            }
        }

        unsigned int firstElementOffset =
            (static_cast<unsigned int>(attrib.offset) /
             static_cast<unsigned int>(ComputeVertexAttributeStride(attrib))) *
            outputElementSize;
        unsigned int startOffset = (instances == 0 || attrib.divisor == 0) ? firstVertexIndex * outputElementSize : 0;
        if (streamOffset + firstElementOffset + startOffset < streamOffset)
        {
            return gl::Error(GL_OUT_OF_MEMORY);
        }

        streamOffset += firstElementOffset + startOffset;
    }
    else
    {
        size_t totalCount = ComputeVertexAttributeElementCount(attrib, count, instances);
        gl::Error error = mStreamingBuffer->getVertexBuffer()->getSpaceRequired(attrib, 1, 0, &outputElementSize);
        if (error.isError())
        {
            return error;
        }

        error = mStreamingBuffer->storeVertexAttributes(
            attrib, translated->currentValueType, firstVertexIndex,
            static_cast<GLsizei>(totalCount), instances, &streamOffset, sourceData);
        if (error.isError())
        {
            return error;
        }
    }

    translated->storage = nullptr;
    translated->serial = vertexBuffer->getSerial();
    translated->stride = outputElementSize;
    translated->offset = streamOffset;

    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexDataManager::storeCurrentValue(const gl::VertexAttribCurrentValueData &currentValue,
                                               TranslatedAttribute *translated,
                                               CurrentValueState *cachedState)
{
    if (cachedState->data != currentValue)
    {
        const gl::VertexAttribute &attrib = *translated->attribute;

        gl::Error error = cachedState->buffer->reserveVertexSpace(attrib, 1, 0);
        if (error.isError())
        {
            return error;
        }

        const uint8_t *sourceData = reinterpret_cast<const uint8_t*>(currentValue.FloatValues);
        unsigned int streamOffset;
        error = cachedState->buffer->storeVertexAttributes(attrib, currentValue.Type, 0, 1, 0, &streamOffset, sourceData);
        if (error.isError())
        {
            return error;
        }

        cachedState->data = currentValue;
        cachedState->offset = streamOffset;
    }

    translated->storage = NULL;
    translated->vertexBuffer = cachedState->buffer->getVertexBuffer();
    translated->serial = cachedState->buffer->getSerial();
    translated->divisor = 0;

    translated->stride = 0;
    translated->offset = static_cast<unsigned int>(cachedState->offset);

    return gl::Error(GL_NO_ERROR);
}

}
