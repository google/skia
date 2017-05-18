/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestUtils.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrContextFactory.h"
#include "GrShaderCaps.h"
#include "GrSurfaceContext.h"
#include "GrTest.h"
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

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(EGLImageTest, reporter, ctxInfo) {
    GrContext* context0 = ctxInfo.grContext();
    sk_gpu_test::GLTestContext* glCtx0 = ctxInfo.glContext();

    // Try to create a second GL context and then check if the contexts have necessary
    // extensions to run this test.

    if (kGLES_GrGLStandard != glCtx0->gl()->fStandard) {
        return;
    }
    GrGLGpu* gpu0 = static_cast<GrGLGpu*>(context0->getGpu());
    if (!gpu0->glCaps().shaderCaps()->externalTextureSupport()) {
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
    GrBackendTexture backendTex(kSize, kSize, kRGBA_8888_GrPixelConfig, externalTexture);

    // TODO: If I make this TopLeft origin to match resolve_origin calls for kDefault, this test
    // fails on the Nexus5. Why?
    sk_sp<GrSurfaceContext> surfaceContext = context0->contextPriv().makeBackendSurfaceContext(
            backendTex, kBottomLeft_GrSurfaceOrigin, kNone_GrBackendTextureFlag, 0, nullptr);

    if (!surfaceContext) {
        ERRORF(reporter, "Error wrapping external texture in GrSurfaceContext.");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
        return;
    }

    // Should not be able to wrap as a RT
    {
        sk_sp<GrSurfaceContext> temp = context0->contextPriv().makeBackendSurfaceContext(
            backendTex, kBottomLeft_GrSurfaceOrigin, kRenderTarget_GrBackendTextureFlag, 0,
            nullptr);
        if (temp) {
            ERRORF(reporter, "Should not be able to wrap an EXTERNAL texture as a RT.");
        }
    }

    // Should not be able to wrap with a sample count
    {
        sk_sp<GrSurfaceContext> temp = context0->contextPriv().makeBackendSurfaceContext(
            backendTex, kBottomLeft_GrSurfaceOrigin, kNone_GrBackendTextureFlag, 4, nullptr);
        if (temp) {
            ERRORF(reporter, "Should not be able to wrap an EXTERNAL texture with MSAA.");
        }
    }

    test_read_pixels(reporter, surfaceContext.get(), pixels.get(), "EGLImageTest-read");

    // We should not be able to write to a EXTERNAL texture
    test_write_pixels(reporter, surfaceContext.get(), false, "EGLImageTest-write");

    // Only test RT-config
    // TODO: why do we always need to draw to copy from an external texture?
    test_copy_from_surface(reporter, context0, surfaceContext->asSurfaceProxy(),
                           pixels.get(), true, "EGLImageTest-copy");

    cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, backendTexture1, image);
}

#endif
