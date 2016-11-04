/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextFactory.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLUtil.h"
#include "gl/GLTestContext.h"

using sk_gpu_test::GLTestContext;

static void cleanup(GLTestContext* glctx0, GrGLuint texID0, GLTestContext* glctx1, GrContext* grctx1,
                    const GrGLTextureInfo* grbackendtex1, GrEGLImage image1) {
    if (glctx1) {
        glctx1->makeCurrent();
        if (grctx1) {
            if (grbackendtex1) {
                GrGLGpu* gpu1 = static_cast<GrGLGpu*>(grctx1->getGpu());
                GrBackendObject handle = reinterpret_cast<GrBackendObject>(grbackendtex1);
                gpu1->deleteTestingOnlyBackendTexture(handle, false);
            }
            grctx1->unref();
        }
        if (GR_EGL_NO_IMAGE != image1) {
            glctx1->destroyEGLImage(image1);
        }
    }

    glctx0->makeCurrent();
    if (texID0) {
        GR_GL_CALL(glctx0->gl(), DeleteTextures(1, &texID0));
    }
}

static void test_read_pixels(skiatest::Reporter* reporter, GrContext* context,
                             GrTexture* externalTexture, uint32_t expectedPixelValues[]) {
    int pixelCnt = externalTexture->width() * externalTexture->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    memset(pixels.get(), 0, sizeof(uint32_t)*pixelCnt);
    bool read = externalTexture->readPixels(0, 0, externalTexture->width(),
                                            externalTexture->height(), kRGBA_8888_GrPixelConfig,
                                            pixels.get());
    if (!read) {
        ERRORF(reporter, "Error reading external texture.");
    }
    for (int i = 0; i < pixelCnt; ++i) {
        if (pixels.get()[i] != expectedPixelValues[i]) {
            ERRORF(reporter, "Error, external texture pixel value %d should be 0x%08x,"
                             " got 0x%08x.", i, expectedPixelValues[i], pixels.get()[i]);
            break;
        }
    }
}

static void test_write_pixels(skiatest::Reporter* reporter, GrContext* context,
                              GrTexture* externalTexture) {
    int pixelCnt = externalTexture->width() * externalTexture->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    memset(pixels.get(), 0, sizeof(uint32_t)*pixelCnt);
    bool write = externalTexture->writePixels(0, 0, 0, 0, kRGBA_8888_GrPixelConfig, pixels.get());
    REPORTER_ASSERT_MESSAGE(reporter, !write, "Should not be able to write to a EXTERNAL"
                                              " texture.");
}

static void test_copy_surface(skiatest::Reporter* reporter, GrContext* context,
                              GrTexture* externalTexture, uint32_t expectedPixelValues[]) {
    GrSurfaceDesc copyDesc;
    copyDesc.fConfig = kRGBA_8888_GrPixelConfig;
    copyDesc.fWidth = externalTexture->width();
    copyDesc.fHeight = externalTexture->height();
    copyDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    sk_sp<GrTexture> copy(context->textureProvider()->createTexture(copyDesc, SkBudgeted::kYes));
    context->copySurface(copy.get(), externalTexture);
    test_read_pixels(reporter, context, copy.get(), expectedPixelValues);
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(EGLImageTest, reporter, ctxInfo) {
    GrContext* context0 = ctxInfo.grContext();
    sk_gpu_test::GLTestContext* glCtx0 = ctxInfo.glContext();

    // Try to create a second GL context and then check if the contexts have necessary
    // extensions to run this test.

    if (kGLES_GrGLStandard != glCtx0->gl()->fStandard) {
        return;
    }
    GrGLGpu* gpu0 = static_cast<GrGLGpu*>(context0->getGpu());
    if (!gpu0->glCaps().glslCaps()->externalTextureSupport()) {
        return;
    }

    std::unique_ptr<GLTestContext> glCtx1 = glCtx0->makeNew();
    if (!glCtx1) {
        return;
    }
    GrContext* context1 = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)glCtx1->gl());
    const GrGLTextureInfo* backendTexture1 = nullptr;
    GrEGLImage image = GR_EGL_NO_IMAGE;
    GrGLTextureInfo externalTexture;
    externalTexture.fID = 0;

    if (!context1) {
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
        return;
    }

    if (!glCtx1->gl()->hasExtension("EGL_KHR_image") ||
        !glCtx1->gl()->hasExtension("EGL_KHR_gl_texture_2D_image")) {
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
        return;
    }

    ///////////////////////////////// CONTEXT 1 ///////////////////////////////////

    // Use GL Context 1 to create a texture unknown to GrContext.
    context1->flush();
    GrGpu* gpu1 = context1->getGpu();
    static const int kSize = 100;
    backendTexture1 = reinterpret_cast<const GrGLTextureInfo*>(
        gpu1->createTestingOnlyBackendTexture(nullptr, kSize, kSize, kRGBA_8888_GrPixelConfig));
    if (!backendTexture1 || !backendTexture1->fID) {
        ERRORF(reporter, "Error creating texture for EGL Image");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
        return;
    }
    if (GR_GL_TEXTURE_2D != backendTexture1->fTarget) {
        ERRORF(reporter, "Expected backend texture to be 2D");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
        return;
    }

    // Wrap the texture in an EGLImage
    image = glCtx1->texture2DToEGLImage(backendTexture1->fID);
    if (GR_EGL_NO_IMAGE == image) {
        ERRORF(reporter, "Error creating EGL Image from texture");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
        return;
    }

    // Populate the texture using GL context 1. Important to use TexSubImage as TexImage orphans
    // the EGL image. Also, this must be done after creating the EGLImage as the texture
    // contents may not be preserved when the image is created.
    SkAutoTMalloc<uint32_t> pixels(kSize * kSize);
    for (int i = 0; i < kSize*kSize; ++i) {
        pixels.get()[i] = 0xDDAABBCC;
    }
    GR_GL_CALL(glCtx1->gl(), ActiveTexture(GR_GL_TEXTURE0));
    GR_GL_CALL(glCtx1->gl(), BindTexture(backendTexture1->fTarget, backendTexture1->fID));
    GR_GL_CALL(glCtx1->gl(), TexSubImage2D(backendTexture1->fTarget, 0, 0, 0, kSize, kSize,
                                           GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, pixels.get()));
    GR_GL_CALL(glCtx1->gl(), Finish());
    // We've been making direct GL calls in GL context 1, let GrContext 1 know its internal
    // state is invalid.
    context1->resetContext();

    ///////////////////////////////// CONTEXT 0 ///////////////////////////////////

    // Make a new texture ID in GL Context 0 from the EGL Image
    glCtx0->makeCurrent();
    externalTexture.fTarget = GR_GL_TEXTURE_EXTERNAL;
    externalTexture.fID = glCtx0->eglImageToExternalTexture(image);

    // Wrap this texture ID in a GrTexture
    GrBackendTextureDesc externalDesc;
    externalDesc.fConfig = kRGBA_8888_GrPixelConfig;
    externalDesc.fWidth = kSize;
    externalDesc.fHeight = kSize;
    externalDesc.fTextureHandle = reinterpret_cast<GrBackendObject>(&externalTexture);
    sk_sp<GrTexture> externalTextureObj(
        context0->textureProvider()->wrapBackendTexture(externalDesc));
    if (!externalTextureObj) {
        ERRORF(reporter, "Error wrapping external texture in GrTexture.");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
        return;
    }

    // Should not be able to wrap as a RT
    externalDesc.fFlags = kRenderTarget_GrBackendTextureFlag;
    sk_sp<GrTexture> externalTextureRTObj(
        context0->textureProvider()->wrapBackendTexture(externalDesc));
    if (externalTextureRTObj) {
        ERRORF(reporter, "Should not be able to wrap an EXTERNAL texture as a RT.");
    }
    externalDesc.fFlags = kNone_GrBackendTextureFlag;

    // Should not be able to wrap with a sample count
    externalDesc.fSampleCnt = 4;
    sk_sp<GrTexture> externalTextureMSAAObj(
        context0->textureProvider()->wrapBackendTexture(externalDesc));
    if (externalTextureMSAAObj) {
        ERRORF(reporter, "Should not be able to wrap an EXTERNAL texture with MSAA.");
    }
    externalDesc.fSampleCnt = 0;

    test_read_pixels(reporter, context0, externalTextureObj.get(), pixels.get());

    test_write_pixels(reporter, context0, externalTextureObj.get());

    test_copy_surface(reporter, context0, externalTextureObj.get(), pixels.get());

    cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
}

#endif
