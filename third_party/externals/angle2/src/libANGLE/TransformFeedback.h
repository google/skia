//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_TRANSFORM_FEEDBACK_H_
#define LIBANGLE_TRANSFORM_FEEDBACK_H_

#include "libANGLE/RefCountObject.h"

#include "common/angleutils.h"

#include "angle_gl.h"

namespace rx
{
class TransformFeedbackImpl;
}

namespace gl
{
class Buffer;
struct Caps;

class TransformFeedback : public RefCountObject
{
  public:
    TransformFeedback(rx::TransformFeedbackImpl* impl, GLuint id, const Caps &caps);
    virtual ~TransformFeedback();

    void begin(GLenum primitiveMode);
    void end();
    void pause();
    void resume();

    bool isActive() const;
    bool isPaused() const;
    GLenum getPrimitiveMode() const;

    void bindGenericBuffer(Buffer *buffer);
    const BindingPointer<Buffer> &getGenericBuffer() const;

    void bindIndexedBuffer(size_t index, Buffer *buffer, size_t offset, size_t size);
    const OffsetBindingPointer<Buffer> &getIndexedBuffer(size_t index) const;
    size_t getIndexedBufferCount() const;

    rx::TransformFeedbackImpl *getImplementation();
    const rx::TransformFeedbackImpl *getImplementation() const;

  private:
    rx::TransformFeedbackImpl* mImplementation;

    bool mActive;
    GLenum mPrimitiveMode;
    bool mPaused;

    BindingPointer<Buffer> mGenericBuffer;
    std::vector<OffsetBindingPointer<Buffer>> mIndexedBuffers;
};

}

#endif // LIBANGLE_TRANSFORM_FEEDBACK_H_
