//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image_unittest.cpp : Unittets of the Image and ImageSibling classes.

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "libANGLE/Image.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/renderer/ImageImpl_mock.h"
#include "libANGLE/renderer/TextureImpl_mock.h"
#include "libANGLE/renderer/RenderbufferImpl_mock.h"

using ::testing::_;
using ::testing::Return;

namespace angle
{
// Verify ref counts are maintained between images and their siblings when objects are deleted
TEST(ImageTest, RefCounting)
{
    // Create a texture and an EGL image that uses the texture as its source
    rx::MockTextureImpl *textureImpl = new rx::MockTextureImpl();
    gl::Texture *texture = new gl::Texture(textureImpl, 1, GL_TEXTURE_2D);
    texture->addRef();

    rx::MockImageImpl *imageImpl = new rx::MockImageImpl();
    egl::Image *image = new egl::Image(imageImpl, EGL_GL_TEXTURE_2D, texture, egl::AttributeMap());
    image->addRef();

    // Verify that the image added a ref to the texture and the texture has not added a ref to the
    // image
    EXPECT_EQ(texture->getRefCount(), 2u);
    EXPECT_EQ(image->getRefCount(), 1u);

    // Create a renderbuffer and set it as a target of the EGL image
    rx::MockRenderbufferImpl *renderbufferImpl = new rx::MockRenderbufferImpl();
    gl::Renderbuffer *renderbuffer = new gl::Renderbuffer(renderbufferImpl, 1);
    renderbuffer->addRef();

    EXPECT_CALL(*renderbufferImpl, setStorageEGLImageTarget(_))
        .WillOnce(Return(gl::Error(GL_NO_ERROR)))
        .RetiresOnSaturation();
    renderbuffer->setStorageEGLImageTarget(image);

    // Verify that the renderbuffer added a ref to the image and the image did not add a ref to
    // the renderbuffer
    EXPECT_EQ(texture->getRefCount(), 2u);
    EXPECT_EQ(image->getRefCount(), 2u);
    EXPECT_EQ(renderbuffer->getRefCount(), 1u);

    // Simulate deletion of the texture and verify that it still exists because the image holds a
    // ref
    texture->release();
    EXPECT_EQ(texture->getRefCount(), 1u);
    EXPECT_EQ(image->getRefCount(), 2u);
    EXPECT_EQ(renderbuffer->getRefCount(), 1u);

    // Simulate deletion of the image and verify that it still exists because the renderbuffer holds
    // a ref
    image->release();
    EXPECT_EQ(texture->getRefCount(), 1u);
    EXPECT_EQ(image->getRefCount(), 1u);
    EXPECT_EQ(renderbuffer->getRefCount(), 1u);

    // Simulate deletion of the renderbuffer and verify that the deletion cascades to all objects
    EXPECT_CALL(*imageImpl, destructor()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*imageImpl, orphan(_))
        .WillOnce(Return(gl::Error(GL_NO_ERROR)))
        .RetiresOnSaturation();

    EXPECT_CALL(*textureImpl, destructor()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*renderbufferImpl, destructor()).Times(1).RetiresOnSaturation();

    renderbuffer->release();
}

// Verify that respecifiying textures releases references to the Image.
TEST(ImageTest, RespecificationReleasesReferences)
{
    // Create a texture and an EGL image that uses the texture as its source
    rx::MockTextureImpl *textureImpl = new rx::MockTextureImpl();
    gl::Texture *texture = new gl::Texture(textureImpl, 1, GL_TEXTURE_2D);
    texture->addRef();

    EXPECT_CALL(*textureImpl, setImage(_, _, _, _, _, _, _, _))
        .WillOnce(Return(gl::Error(GL_NO_ERROR)))
        .RetiresOnSaturation();
    texture->setImage(nullptr, GL_TEXTURE_2D, 0, GL_RGBA8, gl::Extents(1, 1, 1), GL_RGBA,
                      GL_UNSIGNED_BYTE, nullptr);

    rx::MockImageImpl *imageImpl = new rx::MockImageImpl();
    egl::Image *image = new egl::Image(imageImpl, EGL_GL_TEXTURE_2D, texture, egl::AttributeMap());
    image->addRef();

    // Verify that the image added a ref to the texture and the texture has not added a ref to the
    // image
    EXPECT_EQ(texture->getRefCount(), 2u);
    EXPECT_EQ(image->getRefCount(), 1u);

    // Respecify the texture and verify that the image releases its reference
    EXPECT_CALL(*imageImpl, orphan(_))
        .WillOnce(Return(gl::Error(GL_NO_ERROR)))
        .RetiresOnSaturation();
    EXPECT_CALL(*textureImpl, setImage(_, _, _, _, _, _, _, _))
        .WillOnce(Return(gl::Error(GL_NO_ERROR)))
        .RetiresOnSaturation();

    texture->setImage(nullptr, GL_TEXTURE_2D, 0, GL_RGBA8, gl::Extents(1, 1, 1), GL_RGBA,
                      GL_UNSIGNED_BYTE, nullptr);

    EXPECT_EQ(texture->getRefCount(), 1u);
    EXPECT_EQ(image->getRefCount(), 1u);

    // Delete the texture and verify that the image still exists
    EXPECT_CALL(*textureImpl, destructor()).Times(1).RetiresOnSaturation();
    texture->release();

    EXPECT_EQ(image->getRefCount(), 1u);

    // Delete the image
    EXPECT_CALL(*imageImpl, destructor()).Times(1).RetiresOnSaturation();
    image->release();
}
}
