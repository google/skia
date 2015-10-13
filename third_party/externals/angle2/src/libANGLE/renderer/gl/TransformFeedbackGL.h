//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackGL.h: Defines the class interface for TransformFeedbackGL.

#ifndef LIBANGLE_RENDERER_GL_TRANSFORMFEEDBACKGL_H_
#define LIBANGLE_RENDERER_GL_TRANSFORMFEEDBACKGL_H_

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

class TransformFeedbackGL : public TransformFeedbackImpl
{
  public:
    TransformFeedbackGL();
    ~TransformFeedbackGL() override;

    void begin(GLenum primitiveMode) override;
    void end() override;
    void pause() override;
    void resume() override;

    void bindGenericBuffer(const BindingPointer<gl::Buffer> &binding) override;
    void bindIndexedBuffer(size_t index, const OffsetBindingPointer<gl::Buffer> &binding) override;
};

}

#endif // LIBANGLE_RENDERER_GL_TRANSFORMFEEDBACKGL_H_
