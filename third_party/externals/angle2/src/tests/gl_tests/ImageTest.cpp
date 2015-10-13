//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageTest:
//   Tests the correctness of eglImage.
//

#include "test_utils/ANGLETest.h"

namespace angle
{
class ImageTest : public ANGLETest
{
  protected:
    ImageTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const std::string vsSource =
            "precision highp float;\n"
            "attribute vec4 position;\n"
            "varying vec2 texcoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_Position = position;\n"
            "    texcoord = (position.xy * 0.5) + 0.5;\n"
            "    texcoord.y = 1.0 - texcoord.y;\n"
            "}\n";

        const std::string textureFSSource =
            "precision highp float;\n"
            "uniform sampler2D tex;\n"
            "varying vec2 texcoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = texture2D(tex, texcoord);\n"
            "}\n";

        mTextureProgram = CompileProgram(vsSource, textureFSSource);
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        eglCreateImageKHR =
            reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
        eglDestroyImageKHR =
            reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        ANGLETest::TearDown();

        glDeleteProgram(mTextureProgram);
    }

    void createEGLImage2DTextureSource(size_t width,
                                       size_t height,
                                       GLenum format,
                                       GLenum type,
                                       void *data,
                                       GLuint *outSourceTexture,
                                       EGLImageKHR *outSourceImage)
    {
        // Create a source 2D texture
        GLuint source;
        glGenTextures(1, &source);
        glBindTexture(GL_TEXTURE_2D, source);

        glTexImage2D(GL_TEXTURE_2D, 0, format, static_cast<GLsizei>(width),
                     static_cast<GLsizei>(height), 0, format, type, data);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();
        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                              reinterpretHelper<EGLClientBuffer>(source), nullptr);

        ASSERT_EGL_SUCCESS();

        *outSourceTexture = source;
        *outSourceImage   = image;
    }

    void createEGLImageCubemapTextureSource(size_t width,
                                            size_t height,
                                            GLenum format,
                                            GLenum type,
                                            uint8_t *data,
                                            size_t dataStride,
                                            EGLenum imageTarget,
                                            GLuint *outSourceTexture,
                                            EGLImageKHR *outSourceImage)
    {
        // Create a source cube map texture
        GLuint source;
        glGenTextures(1, &source);
        glBindTexture(GL_TEXTURE_CUBE_MAP, source);

        for (GLenum faceIdx = 0; faceIdx < 6; faceIdx++)
        {
            glTexImage2D(faceIdx + GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format,
                         static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, format, type,
                         data + (faceIdx * dataStride));
        }

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();
        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), imageTarget,
                              reinterpretHelper<EGLClientBuffer>(source), nullptr);

        ASSERT_EGL_SUCCESS();

        *outSourceTexture = source;
        *outSourceImage   = image;
    }

    void createEGLImage3DTextureSource(size_t width,
                                       size_t height,
                                       size_t depth,
                                       GLenum format,
                                       GLenum type,
                                       void *data,
                                       size_t imageLayer,
                                       GLuint *outSourceTexture,
                                       EGLImageKHR *outSourceImage)
    {
        // Create a source 3D texture
        GLuint source;
        glGenTextures(1, &source);
        glBindTexture(GL_TEXTURE_3D, source);

        glTexImage3D(GL_TEXTURE_3D, 0, format, static_cast<GLsizei>(width),
                     static_cast<GLsizei>(height), static_cast<GLsizei>(depth), 0, format, type,
                     data);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source texture
        EGLWindow *window = getEGLWindow();

        EGLint attribs[] = {
            EGL_GL_TEXTURE_ZOFFSET_KHR, static_cast<EGLint>(imageLayer), EGL_NONE,
        };
        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_3D_KHR,
                              reinterpretHelper<EGLClientBuffer>(source), attribs);

        ASSERT_EGL_SUCCESS();

        *outSourceTexture = source;
        *outSourceImage   = image;
    }

    void createEGLImageRenderbufferSource(size_t width,
                                          size_t height,
                                          GLenum internalFormat,
                                          GLubyte data[4],
                                          GLuint *outSourceRenderbuffer,
                                          EGLImageKHR *outSourceImage)
    {
        // Create a source renderbuffer
        GLuint source;
        glGenRenderbuffers(1, &source);
        glBindRenderbuffer(GL_RENDERBUFFER, source);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, static_cast<GLsizei>(width),
                              static_cast<GLsizei>(height));

        // Create a framebuffer and clear it to set the data
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, source);

        glClearColor(data[0] / 255.0f, data[1] / 255.0f, data[2] / 255.0f, data[3] / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDeleteFramebuffers(1, &framebuffer);

        ASSERT_GL_NO_ERROR();

        // Create an image from the source renderbuffer
        EGLWindow *window = getEGLWindow();
        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_RENDERBUFFER_KHR,
                              reinterpretHelper<EGLClientBuffer>(source), nullptr);

        ASSERT_EGL_SUCCESS();

        *outSourceRenderbuffer = source;
        *outSourceImage        = image;
    }

    void createEGLImageTargetTexture2D(EGLImageKHR image, GLuint *outTargetTexture)
    {
        // Create a target texture from the image
        GLuint target;
        glGenTextures(1, &target);
        glBindTexture(GL_TEXTURE_2D, target);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();

        *outTargetTexture = target;
    }

    void createEGLImageTargetRenderbuffer(EGLImageKHR image, GLuint *outTargetRenderbuffer)
    {
        // Create a target texture from the image
        GLuint target;
        glGenRenderbuffers(1, &target);
        glBindRenderbuffer(GL_RENDERBUFFER, target);
        glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image);

        ASSERT_GL_NO_ERROR();

        *outTargetRenderbuffer = target;
    }

    void verifyResults2D(GLuint texture, GLubyte data[4])
    {
        // Draw a quad with the target texture
        glUseProgram(mTextureProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(mTextureUniformLocation, 0);

        drawQuad(mTextureProgram, "position", 0.5f);

        // Expect that the rendered quad has the same color as the source texture
        EXPECT_PIXEL_EQ(0, 0, data[0], data[1], data[2], data[3]);
    }

    void verifyResultsRenderbuffer(GLuint renderbuffer, GLubyte data[4])
    {
        // Bind the renderbuffer to a framebuffer
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer);

        // Expect that the rendered quad has the same color as the source texture
        EXPECT_PIXEL_EQ(0, 0, data[0], data[1], data[2], data[3]);

        glDeleteFramebuffers(1, &framebuffer);
    }

    template <typename destType, typename sourcetype>
    destType reinterpretHelper(sourcetype source)
    {
        static_assert(sizeof(destType) == sizeof(size_t),
                      "destType should be the same size as a size_t");
        size_t sourceSizeT = static_cast<size_t>(source);
        return reinterpret_cast<destType>(sourceSizeT);
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;

    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
};

// Check validation from the EGL_KHR_image_base extension
TEST_P(ImageTest, ValidationImageBase)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLuint glTexture2D;
    glGenTextures(1, &glTexture2D);
    glBindTexture(GL_TEXTURE_2D, glTexture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    EGLDisplay display        = window->getDisplay();
    EGLContext context        = window->getContext();
    EGLConfig config          = window->getConfig();
    EGLImageKHR image         = EGL_NO_IMAGE_KHR;
    EGLClientBuffer texture2D = reinterpretHelper<EGLClientBuffer>(glTexture2D);

    // Test validation of eglCreateImageKHR

    // If <dpy> is not the handle of a valid EGLDisplay object, the error EGL_BAD_DISPLAY is
    // generated.
    image = eglCreateImageKHR(reinterpretHelper<EGLDisplay>(0xBAADF00D), context,
                              EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    // If <ctx> is neither the handle of a valid EGLContext object on <dpy> nor EGL_NO_CONTEXT, the
    // error EGL_BAD_CONTEXT is generated.
    image = eglCreateImageKHR(display, reinterpretHelper<EGLContext>(0xBAADF00D),
                              EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_CONTEXT);

    // Test EGL_NO_CONTEXT with a 2D texture target which does require a context.
    image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_CONTEXT);

    // If an attribute specified in <attrib_list> is not one of the attributes listed in Table bbb,
    // the error EGL_BAD_PARAMETER is generated.
    EGLint badAttributes[] = {
        static_cast<EGLint>(0xDEADBEEF), 0, EGL_NONE,
    };

    image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, texture2D, badAttributes);
    EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // If the resource specified by <dpy>, <ctx>, <target>, <buffer> and <attrib_list> has an off -
    // screen buffer bound to it(e.g., by a
    // previous call to eglBindTexImage), the error EGL_BAD_ACCESS is generated.
    EGLint surfaceType = 0;
    eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);

    EGLint bindToTextureRGBA = 0;
    eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGBA, &bindToTextureRGBA);
    if ((surfaceType & EGL_PBUFFER_BIT) != 0 && bindToTextureRGBA == EGL_TRUE)
    {
        EGLint pbufferAttributes[] = {
            EGL_WIDTH,          1,
            EGL_HEIGHT,         1,
            EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
            EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
            EGL_NONE,           EGL_NONE,
        };
        EGLSurface pbuffer = eglCreatePbufferSurface(display, config, pbufferAttributes);
        ASSERT_NE(pbuffer, EGL_NO_SURFACE);
        EXPECT_EGL_SUCCESS();

        eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);
        EXPECT_EGL_SUCCESS();

        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_ACCESS);

        eglReleaseTexImage(display, pbuffer, EGL_BACK_BUFFER);
        eglDestroySurface(display, pbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_EGL_SUCCESS();
        EXPECT_GL_NO_ERROR();
    }

    // If the resource specified by <dpy>, <ctx>, <target>, <buffer> and
    // <attrib_list> is itself an EGLImage sibling, the error EGL_BAD_ACCESS is generated.
    image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, texture2D, nullptr);
    EXPECT_NE(image, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_SUCCESS();

    /* TODO(geofflang): Enable this validation when it passes.
    EGLImageKHR image2 = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
    reinterpret_cast<EGLClientBuffer>(texture2D), nullptr);
    EXPECT_EQ(image2, EGL_NO_IMAGE_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_ACCESS);
    */

    // Test validation of eglDestroyImageKHR
    // Note: image is now a valid EGL image
    EGLBoolean result = EGL_FALSE;

    // If <dpy> is not the handle of a valid EGLDisplay object, the error EGL_BAD_DISPLAY is
    // generated.
    result = eglDestroyImageKHR(reinterpretHelper<EGLDisplay>(0xBAADF00D), image);
    EXPECT_EQ(result, static_cast<EGLBoolean>(EGL_FALSE));
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    // If <image> is not a valid EGLImageKHR object created with respect to <dpy>, the error
    // EGL_BAD_PARAMETER is generated.
    result = eglDestroyImageKHR(display, reinterpretHelper<EGLImageKHR>(0xBAADF00D));
    EXPECT_EQ(result, static_cast<EGLBoolean>(EGL_FALSE));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // Clean up and validate image is destroyed
    result = eglDestroyImageKHR(display, image);
    EXPECT_EQ(result, static_cast<EGLBoolean>(EGL_TRUE));
    EXPECT_EGL_SUCCESS();

    glDeleteTextures(1, &glTexture2D);
    EXPECT_GL_NO_ERROR();
}

// Check validation from the EGL_KHR_gl_texture_2D_image extension
TEST_P(ImageTest, ValidationImagePixmap)
{
    // This extension is not implemented anywhere yet.  This makes sure that it is tested once it is
    // added.
    EGLWindow *window = getEGLWindow();
    EXPECT_FALSE(eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_pixmap"));
}

// Check validation from the EGL_KHR_gl_texture_2D_image, EGL_KHR_gl_texture_cubemap_image,
// EGL_KHR_gl_texture_3D_image and EGL_KHR_gl_renderbuffer_image extensions
TEST_P(ImageTest, ValidationGLImage)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base"))
    {
        std::cout << "Test skipped because OES_EGL_image or EGL_KHR_image_base is not available."
                  << std::endl;
        return;
    }

    EGLDisplay display = window->getDisplay();
    EGLContext context = window->getContext();
    EGLImageKHR image  = EGL_NO_IMAGE_KHR;

    if (eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        // If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBE_MAP_*_KHR or
        // EGL_GL_TEXTURE_3D_KHR and <buffer> is not the name of a texture object of type <target>,
        // the error EGL_BAD_PARAMETER is generated.
        GLuint textureCube;
        glGenTextures(1, &textureCube);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube);
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glTexImage2D(face, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(textureCube), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If EGL_GL_TEXTURE_LEVEL_KHR is 0, <target> is EGL_GL_TEXTURE_2D_KHR,
        // EGL_GL_TEXTURE_CUBE_MAP_*_KHR or EGL_GL_TEXTURE_3D_KHR, <buffer> is the name of an
        // incomplete GL texture object, and any mipmap levels other than mipmap level 0 are
        // specified, the error EGL_BAD_PARAMETER is generated.
        GLuint incompleteTexture;
        glGenTextures(1, &incompleteTexture);
        glBindTexture(GL_TEXTURE_2D, incompleteTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        EGLint level0Attribute[] = {
            EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTexture),
                                  level0Attribute);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If EGL_GL_TEXTURE_LEVEL_KHR is 0, <target> is EGL_GL_TEXTURE_2D_KHR or
        // EGL_GL_TEXTURE_3D_KHR, <buffer> is not the name of a complete GL texture object, and
        // mipmap level 0 is not specified, the error EGL_BAD_PARAMETER is generated.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTexture), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBE_MAP_*_KHR,
        // EGL_GL_RENDERBUFFER_KHR or EGL_GL_TEXTURE_3D_KHR and <buffer> refers to the default GL
        // texture object(0) for the corresponding GL target, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR, 0, nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        // If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBE_MAP_*_KHR, or
        // EGL_GL_TEXTURE_3D_KHR, and the value specified in <attr_list> for
        // EGL_GL_TEXTURE_LEVEL_KHR is not a valid mipmap level for the specified GL texture object
        // <buffer>, the error EGL_BAD_MATCH is generated.
        EGLint level2Attribute[] = {
            EGL_GL_TEXTURE_LEVEL_KHR, 2, EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTexture),
                                  level2Attribute);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
    else
    {
        GLuint texture2D;
        glGenTextures(1, &texture2D);
        glBindTexture(GL_TEXTURE_2D, texture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // From EGL_KHR_image_base:
        // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture2D), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    if (eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_cubemap_image"))
    {
        // If EGL_GL_TEXTURE_LEVEL_KHR is 0, <target> is EGL_GL_TEXTURE_CUBE_MAP_*_KHR, <buffer> is
        // not the name of a complete GL texture object, and one or more faces do not have mipmap
        // level 0 specified, the error EGL_BAD_PARAMETER is generated.
        GLuint incompleteTextureCube;
        glGenTextures(1, &incompleteTextureCube);
        glBindTexture(GL_TEXTURE_CUBE_MAP, incompleteTextureCube);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        EGLint level0Attribute[] = {
            EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR,
                                  reinterpretHelper<EGLClientBuffer>(incompleteTextureCube),
                                  level0Attribute);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
    else
    {
        GLuint textureCube;
        glGenTextures(1, &textureCube);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube);
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glTexImage2D(face, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        // From EGL_KHR_image_base:
        // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR,
                                  reinterpretHelper<EGLClientBuffer>(textureCube), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }

    if (eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_3D_image") &&
        getClientVersion() >= 3)
    {
        // If <target> is EGL_GL_TEXTURE_3D_KHR, and the value specified in <attr_list> for
        // EGL_GL_TEXTURE_ZOFFSET_KHR exceeds the depth of the specified mipmap level - of - detail
        // in <buffer>, the error EGL_BAD_PARAMETER is generated.
        GLuint texture3D;
        glGenTextures(1, &texture3D);
        glBindTexture(GL_TEXTURE_3D, texture3D);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        EGLint zOffset3Parameter[] = {
            EGL_GL_TEXTURE_ZOFFSET_KHR, 3, EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_3D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture3D), zOffset3Parameter);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        EGLint zOffsetNegative1Parameter[] = {
            EGL_GL_TEXTURE_ZOFFSET_KHR, -1, EGL_NONE,
        };
        image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_3D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture3D),
                                  zOffsetNegative1Parameter);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
    else
    {
        if (eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
        {
            GLuint texture2D;
            glGenTextures(1, &texture2D);
            glBindTexture(GL_TEXTURE_2D, texture2D);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            // Verify EGL_GL_TEXTURE_ZOFFSET_KHR is not a valid parameter
            EGLint zOffset0Parameter[] = {
                EGL_GL_TEXTURE_ZOFFSET_KHR, 0, EGL_NONE,
            };

            image =
                eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                  reinterpretHelper<EGLClientBuffer>(texture2D), zOffset0Parameter);
            EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
            EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
        }

        if (getClientVersion() >= 3)
        {
            GLuint texture3D;
            glGenTextures(1, &texture3D);
            glBindTexture(GL_TEXTURE_3D, texture3D);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            // From EGL_KHR_image_base:
            // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
            // generated.
            image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_3D_KHR,
                                      reinterpretHelper<EGLClientBuffer>(texture3D), nullptr);
            EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
            EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
        }
    }

    if (eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_renderbuffer_image"))
    {
        // If <target> is EGL_GL_RENDERBUFFER_KHR and <buffer> is not the name of a renderbuffer
        // object, or if <buffer> is the name of a multisampled renderbuffer object, the error
        // EGL_BAD_PARAMETER is generated.
        image = eglCreateImageKHR(display, context, EGL_GL_RENDERBUFFER_KHR,
                                  reinterpret_cast<EGLClientBuffer>(0), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

        if (extensionEnabled("GL_ANGLE_framebuffer_multisample"))
        {
            GLuint renderbuffer;
            glGenRenderbuffers(1, &renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
            glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 1, GL_RGBA8, 1, 1);
            EXPECT_GL_NO_ERROR();

            image = eglCreateImageKHR(display, context, EGL_GL_RENDERBUFFER_KHR,
                                      reinterpret_cast<EGLClientBuffer>(0), nullptr);
            EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
            EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
        }
    }
    else
    {
        GLuint renderbuffer;
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 1, 1);

        // From EGL_KHR_image_base:
        // If <target> is not one of the values in Table aaa, the error EGL_BAD_PARAMETER is
        // generated.
        image = eglCreateImageKHR(display, context, EGL_GL_RENDERBUFFER_KHR,
                                  reinterpretHelper<EGLClientBuffer>(renderbuffer), nullptr);
        EXPECT_EQ(image, EGL_NO_IMAGE_KHR);
        EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    }
}

// Check validation from the GL_OES_EGL_image extension
TEST_P(ImageTest, ValidationGLEGLImage)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLubyte data[4] = {255, 0, 255, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data, &source, &image);

    // If <target> is not TEXTURE_2D, the error INVALID_ENUM is generated.
    glEGLImageTargetTexture2DOES(GL_TEXTURE_CUBE_MAP_POSITIVE_X, image);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // If <image> does not refer to a valid eglImageOES object, the error INVALID_VALUE is
    // generated.
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, reinterpretHelper<GLeglImageOES>(0xBAADF00D));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // <target> must be RENDERBUFFER_OES, and <image> must be the handle of a valid EGLImage
    // resource, cast into the type
    // eglImageOES.
    glEGLImageTargetRenderbufferStorageOES(GL_TEXTURE_2D, image);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // If the GL is unable to create a renderbuffer using the specified eglImageOES, the error
    // INVALID_OPERATION is generated.If <image>
    // does not refer to a valid eglImageOES object, the error INVALID_VALUE is generated.
    GLuint renderbuffer;
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER,
                                           reinterpretHelper<GLeglImageOES>(0xBAADF00D));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &renderbuffer);
}

// Check validation from the GL_OES_EGL_image_external extension
TEST_P(ImageTest, ValidationGLEGLImageExternal)
{
    // This extension is not implemented anywhere yet.  This makes sure that it is tested once it is
    // added.
    EXPECT_FALSE(extensionEnabled("GL_OES_EGL_image_external"));
}

// Check validation from the GL_OES_EGL_image_external_essl3 extension
TEST_P(ImageTest, ValidationGLEGLImageExternalESSL3)
{
    // This extension is not implemented anywhere yet.  This makes sure that it is tested once it is
    // added.
    EXPECT_FALSE(extensionEnabled("GL_OES_EGL_image_external_essl3"));
}

TEST_P(ImageTest, Source2DTarget2D)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLubyte data[4] = {255, 0, 255, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetTexture2D(image, &target);

    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, data);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &target);
}

TEST_P(ImageTest, Source2DTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLubyte data[4] = {255, 0, 255, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetRenderbuffer(image, &target);

    // Expect that the target renderbuffer has the same color as the source texture
    verifyResultsRenderbuffer(target, data);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteRenderbuffers(1, &target);
}

TEST_P(ImageTest, SourceCubeTarget2D)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_cubemap_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_cubemap_image is not available."
                  << std::endl;
        return;
    }

    GLubyte data[24] = {
        255, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 255,
        0,   0, 255, 255, 0,   255, 0,   255, 0,   0, 0, 255,
    };

    for (EGLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImageCubemapTextureSource(
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<uint8_t *>(data), sizeof(GLubyte) * 4,
            EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, &source, &image);

        // Create the target
        GLuint target;
        createEGLImageTargetTexture2D(image, &target);

        // Expect that the target texture has the same color as the source texture
        verifyResults2D(target, &data[faceIdx * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteTextures(1, &target);
    }
}

TEST_P(ImageTest, SourceCubeTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_cubemap_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_cubemap_image is not available."
                  << std::endl;
        return;
    }

    GLubyte data[24] = {
        255, 0, 255, 255, 255, 255, 255, 255, 255, 0, 0, 255,
        0,   0, 255, 255, 0,   255, 0,   255, 0,   0, 0, 255,
    };

    for (EGLenum faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImageCubemapTextureSource(
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<uint8_t *>(data), sizeof(GLubyte) * 4,
            EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR + faceIdx, &source, &image);

        // Create the target
        GLuint target;
        createEGLImageTargetRenderbuffer(image, &target);

        // Expect that the target texture has the same color as the source texture
        verifyResultsRenderbuffer(target, &data[faceIdx * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteRenderbuffers(1, &target);
    }
}

TEST_P(ImageTest, Source3DTargetTexture)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_3D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_3D_image is not available."
                  << std::endl;
        return;
    }

    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_texture_3D"))
    {
        std::cout << "Test skipped because 3D textures are not available." << std::endl;
        return;
    }

    const size_t depth      = 2;
    GLubyte data[4 * depth] = {
        255, 0, 255, 255, 255, 255, 0, 255,
    };

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, data, layer, &source,
                                      &image);

        // Create the target
        GLuint target;
        createEGLImageTargetTexture2D(image, &target);

        // Expect that the target renderbuffer has the same color as the source texture
        verifyResults2D(target, &data[layer * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteTextures(1, &target);
    }
}

TEST_P(ImageTest, Source3DTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_3D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_3D_image is not available."
                  << std::endl;
        return;
    }

    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_texture_3D"))
    {
        std::cout << "Test skipped because 3D textures are not available." << std::endl;
        return;
    }

    const size_t depth      = 2;
    GLubyte data[4 * depth] = {
        255, 0, 255, 255, 255, 255, 0, 255,
    };

    for (size_t layer = 0; layer < depth; layer++)
    {
        // Create the Image
        GLuint source;
        EGLImageKHR image;
        createEGLImage3DTextureSource(1, 1, depth, GL_RGBA, GL_UNSIGNED_BYTE, data, layer, &source,
                                      &image);

        // Create the target
        GLuint target;
        createEGLImageTargetRenderbuffer(image, &target);

        // Expect that the target renderbuffer has the same color as the source texture
        verifyResultsRenderbuffer(target, &data[layer * 4]);

        // Clean up
        glDeleteTextures(1, &source);
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteTextures(1, &target);
    }
}

TEST_P(ImageTest, SourceRenderbufferTargetTexture)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_renderbuffer_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_renderbuffer_image is not available."
                  << std::endl;
        return;
    }

    GLubyte data[4] = {255, 0, 255, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetTexture2D(image, &target);

    // Expect that the target texture has the same color as the source texture
    verifyResults2D(target, data);

    // Clean up
    glDeleteRenderbuffers(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &target);
}

TEST_P(ImageTest, SourceRenderbufferTargetRenderbuffer)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_renderbuffer_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_renderbuffer_image is not available."
                  << std::endl;
        return;
    }

    GLubyte data[4] = {255, 0, 255, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImageRenderbufferSource(1, 1, GL_RGBA8_OES, data, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetRenderbuffer(image, &target);

    // Expect that the target renderbuffer has the same color as the source texture
    verifyResultsRenderbuffer(target, data);

    // Clean up
    glDeleteRenderbuffers(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteRenderbuffers(1, &target);
}

// Delete the source texture and EGL image.  The image targets should still have the same data
// because
// they hold refs to the image.
TEST_P(ImageTest, Deletion)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, originalData, &source, &image);

    // Create multiple targets
    GLuint targetTexture;
    createEGLImageTargetTexture2D(image, &targetTexture);

    GLuint targetRenderbuffer;
    createEGLImageTargetRenderbuffer(image, &targetRenderbuffer);

    // Delete the source texture
    glDeleteTextures(1, &source);
    source = 0;

    // Expect that both the targets have the original data
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);

    // Update the data of the target
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that both targets have the updated data
    verifyResults2D(targetTexture, updateData);
    verifyResultsRenderbuffer(targetRenderbuffer, updateData);

    // Delete the EGL image
    eglDestroyImageKHR(window->getDisplay(), image);
    image = EGL_NO_IMAGE_KHR;

    // Update the data of the target back to the original data
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, originalData);

    // Expect that both targets have the original data again
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);

    // Clean up
    glDeleteTextures(1, &targetTexture);
    glDeleteRenderbuffers(1, &targetRenderbuffer);
}

TEST_P(ImageTest, MipLevels)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    const size_t mipLevels   = 3;
    const size_t textureSize = 4;
    std::vector<GLuint> mip0Data(textureSize * textureSize, 0xFFFF0000);
    std::vector<GLuint> mip1Data(mip0Data.size() << 1, 0xFF00FF00);
    std::vector<GLuint> mip2Data(mip0Data.size() << 2, 0xFF0000FF);
    GLubyte *data[mipLevels] = {
        reinterpret_cast<GLubyte *>(&mip0Data[0]), reinterpret_cast<GLubyte *>(&mip1Data[0]),
        reinterpret_cast<GLubyte *>(&mip2Data[0]),
    };

    GLuint source;
    glGenTextures(1, &source);
    glBindTexture(GL_TEXTURE_2D, source);

    for (size_t level = 0; level < mipLevels; level++)
    {
        glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), GL_RGBA, textureSize >> level,
                     textureSize >> level, 0, GL_RGBA, GL_UNSIGNED_BYTE, data[level]);
    }

    ASSERT_GL_NO_ERROR();

    for (size_t level = 0; level < mipLevels; level++)
    {
        // Create the Image
        EGLint attribs[] = {
            EGL_GL_TEXTURE_LEVEL_KHR, static_cast<EGLint>(level), EGL_NONE,
        };
        EGLImageKHR image =
            eglCreateImageKHR(window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
                              reinterpretHelper<EGLClientBuffer>(source), attribs);
        ASSERT_EGL_SUCCESS();

        // Create a texture and renderbuffer target
        GLuint textureTarget;
        createEGLImageTargetTexture2D(image, &textureTarget);

        // Disable mipmapping
        glBindTexture(GL_TEXTURE_2D, textureTarget);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        GLuint renderbufferTarget;
        createEGLImageTargetRenderbuffer(image, &renderbufferTarget);

        // Expect that the targets have the same color as the source texture
        verifyResults2D(textureTarget, data[level]);
        verifyResultsRenderbuffer(renderbufferTarget, data[level]);

        // Clean up
        eglDestroyImageKHR(window->getDisplay(), image);
        glDeleteTextures(1, &textureTarget);
        glDeleteRenderbuffers(1, &renderbufferTarget);
    }

    // Clean up
    glDeleteTextures(1, &source);
}

// Respecify the source texture, orphaning it.  The target texture should not have updated data.
TEST_P(ImageTest, Respecification)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, originalData, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetTexture2D(image, &target);

    // Respecify source
    glBindTexture(GL_TEXTURE_2D, source);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that the target texture has the original data
    verifyResults2D(target, originalData);

    // Expect that the source texture has the updated data
    verifyResults2D(source, updateData);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &target);
}

// Test that respecifying a level of the target texture orphans it and keeps a copy of the EGLimage
// data
TEST_P(ImageTest, RespecificationOfOtherLevel)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLubyte originalData[2 * 2 * 4] = {
        255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255,
    };

    GLubyte updateData[2 * 2 * 4] = {
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
    };

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(2, 2, GL_RGBA, GL_UNSIGNED_BYTE, originalData, &source, &image);

    // Create the target
    GLuint target;
    createEGLImageTargetTexture2D(image, &target);

    // Expect that the target and source textures have the original data
    verifyResults2D(source, originalData);
    verifyResults2D(target, originalData);

    // Add a new mipLevel to the target, orphaning it
    glBindTexture(GL_TEXTURE_2D, target);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, originalData);
    EXPECT_GL_NO_ERROR();

    // Expect that the target and source textures still have the original data
    verifyResults2D(source, originalData);
    verifyResults2D(target, originalData);

    // Update the source's data
    glBindTexture(GL_TEXTURE_2D, source);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that the target still has the original data and source has the updated data
    verifyResults2D(source, updateData);
    verifyResults2D(target, originalData);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &target);
}

// Update the data of the source and target textures.  All image siblings should have the new data.
TEST_P(ImageTest, UpdatedData)
{
    EGLWindow *window = getEGLWindow();
    if (!extensionEnabled("OES_EGL_image") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_image_base") ||
        !eglDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_texture_2D_image"))
    {
        std::cout << "Test skipped because OES_EGL_image, EGL_KHR_image_base or "
                     "EGL_KHR_gl_texture_2D_image is not available."
                  << std::endl;
        return;
    }

    GLubyte originalData[4] = {255, 0, 255, 255};
    GLubyte updateData[4]   = {0, 255, 0, 255};

    // Create the Image
    GLuint source;
    EGLImageKHR image;
    createEGLImage2DTextureSource(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, originalData, &source, &image);

    // Create multiple targets
    GLuint targetTexture;
    createEGLImageTargetTexture2D(image, &targetTexture);

    GLuint targetRenderbuffer;
    createEGLImageTargetRenderbuffer(image, &targetRenderbuffer);

    // Expect that both the source and targets have the original data
    verifyResults2D(source, originalData);
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);

    // Update the data of the source
    glBindTexture(GL_TEXTURE_2D, source);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, updateData);

    // Expect that both the source and targets have the updated data
    verifyResults2D(source, updateData);
    verifyResults2D(targetTexture, updateData);
    verifyResultsRenderbuffer(targetRenderbuffer, updateData);

    // Update the data of the target back to the original data
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, originalData);

    // Expect that both the source and targets have the original data again
    verifyResults2D(source, originalData);
    verifyResults2D(targetTexture, originalData);
    verifyResultsRenderbuffer(targetRenderbuffer, originalData);

    // Clean up
    glDeleteTextures(1, &source);
    eglDestroyImageKHR(window->getDisplay(), image);
    glDeleteTextures(1, &targetTexture);
    glDeleteRenderbuffers(1, &targetRenderbuffer);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(ImageTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
}
