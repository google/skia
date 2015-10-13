//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl_mock.h: Defines a mock of the RenderbufferImpl class.

#ifndef LIBANGLE_RENDERER_RENDERBUFFERIMPLMOCK_H_
#define LIBANGLE_RENDERER_RENDERBUFFERIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/Image.h"
#include "libANGLE/renderer/RenderbufferImpl.h"

namespace rx
{

class MockRenderbufferImpl : public RenderbufferImpl
{
  public:
    virtual ~MockRenderbufferImpl() { destructor(); }
    MOCK_METHOD3(setStorage, gl::Error(GLenum, size_t, size_t));
    MOCK_METHOD4(setStorageMultisample, gl::Error(size_t, GLenum, size_t, size_t));
    MOCK_METHOD1(setStorageEGLImageTarget, gl::Error(egl::Image *));

    MOCK_METHOD2(getAttachmentRenderTarget, gl::Error(const gl::FramebufferAttachment::Target &, FramebufferAttachmentRenderTarget **));

    MOCK_METHOD0(destructor, void());
};

}

#endif // LIBANGLE_RENDERER_RENDERBUFFERIMPLMOCK_H_
