//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureImpl_mock.h: Defines a mock of the TextureImpl class.

#ifndef LIBANGLE_RENDERER_TEXTUREIMPLMOCK_H_
#define LIBANGLE_RENDERER_TEXTUREIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/TextureImpl.h"

namespace rx
{

class MockTextureImpl : public TextureImpl
{
  public:
    virtual ~MockTextureImpl() { destructor(); }
    MOCK_METHOD1(setUsage, void(GLenum));
    MOCK_METHOD8(setImage, gl::Error(GLenum, size_t, GLenum, const gl::Extents &, GLenum, GLenum, const gl::PixelUnpackState &, const uint8_t *));
    MOCK_METHOD7(setSubImage, gl::Error(GLenum, size_t, const gl::Box &, GLenum, GLenum, const gl::PixelUnpackState &, const uint8_t *));
    MOCK_METHOD7(setCompressedImage, gl::Error(GLenum, size_t, GLenum, const gl::Extents &, const gl::PixelUnpackState &, size_t, const uint8_t *));
    MOCK_METHOD7(setCompressedSubImage, gl::Error(GLenum, size_t, const gl::Box &, GLenum, const gl::PixelUnpackState &, size_t, const uint8_t *));
    MOCK_METHOD5(copyImage, gl::Error(GLenum, size_t, const gl::Rectangle &, GLenum, const gl::Framebuffer *));
    MOCK_METHOD5(copySubImage, gl::Error(GLenum, size_t, const gl::Offset &, const gl::Rectangle &, const gl::Framebuffer *));
    MOCK_METHOD4(setStorage, gl::Error(GLenum, size_t, GLenum, const gl::Extents &));
    MOCK_METHOD2(setEGLImageTarget, gl::Error(GLenum, egl::Image *));
    MOCK_METHOD1(generateMipmaps, gl::Error(const gl::TextureState &));
    MOCK_METHOD1(bindTexImage, void(egl::Surface *));
    MOCK_METHOD0(releaseTexImage, void(void));

    MOCK_METHOD2(getAttachmentRenderTarget, gl::Error(const gl::FramebufferAttachment::Target &, FramebufferAttachmentRenderTarget **));

    MOCK_METHOD0(destructor, void());
};

}

#endif // LIBANGLE_RENDERER_TEXTUREIMPLMOCK_H_
