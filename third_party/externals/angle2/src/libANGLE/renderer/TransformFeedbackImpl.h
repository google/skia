//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackImpl.h: Defines the abstract rx::TransformFeedbackImpl class.

#ifndef LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPL_H_
#define LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/TransformFeedback.h"

namespace rx
{

class TransformFeedbackImpl : angle::NonCopyable
{
  public:
    virtual ~TransformFeedbackImpl() { }

    virtual void begin(GLenum primitiveMode) = 0;
    virtual void end() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual void bindGenericBuffer(const BindingPointer<gl::Buffer> &binding) = 0;
    virtual void bindIndexedBuffer(size_t index, const OffsetBindingPointer<gl::Buffer> &binding) = 0;
};

}

#endif // LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPL_H_
