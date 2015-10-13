//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Config.h"
#include "libANGLE/Data.h"
#include "libANGLE/State.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/FramebufferImpl.h"
#include "libANGLE/renderer/SurfaceImpl.h"

namespace
{

class MockSurfaceImpl : public rx::SurfaceImpl
{
  public:
    virtual ~MockSurfaceImpl() { destroy(); }

    MOCK_METHOD0(initialize, egl::Error());
    MOCK_METHOD1(createDefaultFramebuffer,
                 rx::FramebufferImpl *(const gl::Framebuffer::Data &data));
    MOCK_METHOD0(swap, egl::Error());
    MOCK_METHOD4(postSubBuffer, egl::Error(EGLint, EGLint, EGLint, EGLint));
    MOCK_METHOD2(querySurfacePointerANGLE, egl::Error(EGLint, void**));
    MOCK_METHOD1(bindTexImage, egl::Error(EGLint));
    MOCK_METHOD1(releaseTexImage, egl::Error(EGLint));
    MOCK_METHOD1(setSwapInterval, void(EGLint));
    MOCK_CONST_METHOD0(getWidth, EGLint());
    MOCK_CONST_METHOD0(getHeight, EGLint());
    MOCK_CONST_METHOD0(isPostSubBufferSupported, EGLint(void));
    MOCK_CONST_METHOD0(getSwapBehavior, EGLint(void));
    MOCK_METHOD2(getAttachmentRenderTarget, gl::Error(const gl::FramebufferAttachment::Target &, rx::FramebufferAttachmentRenderTarget **));

    MOCK_METHOD0(destroy, void());
};

class MockFramebufferImpl : public rx::FramebufferImpl
{
  public:
    MockFramebufferImpl() : rx::FramebufferImpl(gl::Framebuffer::Data()) {}
    virtual ~MockFramebufferImpl() { destroy(); }

    MOCK_METHOD1(onUpdateColorAttachment, void(size_t));
    MOCK_METHOD0(onUpdateDepthAttachment, void());
    MOCK_METHOD0(onUpdateStencilAttachment, void());
    MOCK_METHOD0(onUpdateDepthStencilAttachment, void());

    MOCK_METHOD2(setDrawBuffers, void(size_t, const GLenum *));
    MOCK_METHOD1(setReadBuffer, void(GLenum));

    MOCK_METHOD2(discard, gl::Error(size_t, const GLenum *));
    MOCK_METHOD2(invalidate, gl::Error(size_t, const GLenum *));
    MOCK_METHOD3(invalidateSub, gl::Error(size_t, const GLenum *, const gl::Rectangle &));

    MOCK_METHOD2(clear, gl::Error(const gl::Data &, GLbitfield));
    MOCK_METHOD4(clearBufferfv, gl::Error(const gl::State &, GLenum, GLint, const GLfloat *));
    MOCK_METHOD4(clearBufferuiv, gl::Error(const gl::State &, GLenum, GLint, const GLuint *));
    MOCK_METHOD4(clearBufferiv, gl::Error(const gl::State &, GLenum, GLint, const GLint *));
    MOCK_METHOD5(clearBufferfi, gl::Error(const gl::State &, GLenum, GLint, GLfloat, GLint));

    MOCK_CONST_METHOD0(getImplementationColorReadFormat, GLenum());
    MOCK_CONST_METHOD0(getImplementationColorReadType, GLenum());
    MOCK_CONST_METHOD5(
        readPixels,
        gl::Error(const gl::State &, const gl::Rectangle &, GLenum, GLenum, GLvoid *));

    MOCK_METHOD6(blit,
                 gl::Error(const gl::State &,
                           const gl::Rectangle &,
                           const gl::Rectangle &,
                           GLbitfield,
                           GLenum,
                           const gl::Framebuffer *));

    MOCK_CONST_METHOD0(checkStatus, GLenum());

    MOCK_METHOD0(destroy, void());
};

TEST(SurfaceTest, DestructionDeletesImpl)
{
    MockFramebufferImpl *framebuffer = new MockFramebufferImpl;

    MockSurfaceImpl *impl = new MockSurfaceImpl;
    EXPECT_CALL(*impl, getSwapBehavior());
    EXPECT_CALL(*impl, createDefaultFramebuffer(testing::_)).WillOnce(testing::Return(framebuffer));

    EXPECT_CALL(*framebuffer, setDrawBuffers(1, testing::_));
    EXPECT_CALL(*framebuffer, setReadBuffer(GL_BACK));
    EXPECT_CALL(*framebuffer, onUpdateColorAttachment(0));

    egl::Config config;
    egl::Surface *surface = new egl::Surface(impl, EGL_WINDOW_BIT, &config, egl::AttributeMap());

    EXPECT_CALL(*framebuffer, destroy()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*impl, destroy()).Times(1).RetiresOnSaturation();

    surface->onDestroy();

    // Only needed because the mock is leaked if bugs are present,
    // which logs an error, but does not cause the test to fail.
    // Ordinarily mocks are verified when destroyed.
    testing::Mock::VerifyAndClear(impl);
}

} // namespace
