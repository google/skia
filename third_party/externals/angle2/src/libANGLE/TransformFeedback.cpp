//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/TransformFeedback.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace gl
{

TransformFeedback::TransformFeedback(rx::TransformFeedbackImpl* impl, GLuint id, const Caps &caps)
    : RefCountObject(id),
      mImplementation(impl),
      mActive(false),
      mPrimitiveMode(GL_NONE),
      mPaused(false),
      mGenericBuffer(),
      mIndexedBuffers(caps.maxTransformFeedbackSeparateAttributes)
{
    ASSERT(impl != NULL);
}

TransformFeedback::~TransformFeedback()
{
    mGenericBuffer.set(nullptr);
    for (size_t i = 0; i < mIndexedBuffers.size(); i++)
    {
        mIndexedBuffers[i].set(nullptr);
    }

    SafeDelete(mImplementation);
}

void TransformFeedback::begin(GLenum primitiveMode)
{
    mActive = true;
    mPrimitiveMode = primitiveMode;
    mPaused = false;
    mImplementation->begin(primitiveMode);
}

void TransformFeedback::end()
{
    mActive = false;
    mPrimitiveMode = GL_NONE;
    mPaused = false;
    mImplementation->end();
}

void TransformFeedback::pause()
{
    mPaused = true;
    mImplementation->pause();
}

void TransformFeedback::resume()
{
    mPaused = false;
    mImplementation->resume();
}

bool TransformFeedback::isActive() const
{
    return mActive;
}

bool TransformFeedback::isPaused() const
{
    return mPaused;
}

GLenum TransformFeedback::getPrimitiveMode() const
{
    return mPrimitiveMode;
}

void TransformFeedback::bindGenericBuffer(Buffer *buffer)
{
    mGenericBuffer.set(buffer);
    mImplementation->bindGenericBuffer(mGenericBuffer);
}

const BindingPointer<Buffer> &TransformFeedback::getGenericBuffer() const
{
    return mGenericBuffer;
}

void TransformFeedback::bindIndexedBuffer(size_t index, Buffer *buffer, size_t offset, size_t size)
{
    ASSERT(index < mIndexedBuffers.size());
    mIndexedBuffers[index].set(buffer, offset, size);
    mImplementation->bindIndexedBuffer(index, mIndexedBuffers[index]);
}

const OffsetBindingPointer<Buffer> &TransformFeedback::getIndexedBuffer(size_t index) const
{
    ASSERT(index < mIndexedBuffers.size());
    return mIndexedBuffers[index];
}

size_t TransformFeedback::getIndexedBufferCount() const
{
    return mIndexedBuffers.size();
}

rx::TransformFeedbackImpl *TransformFeedback::getImplementation()
{
    return mImplementation;
}

const rx::TransformFeedbackImpl *TransformFeedback::getImplementation() const
{
    return mImplementation;
}

}
