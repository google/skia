/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h" // Keep this before any #ifdef for skbug.com/3362

#ifdef SK_GL

#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTextureContext.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/gl/GLTestContext.h"

using sk_gpu_test::GLTestContext;

static void cleanup(GLTestContext* glctx0, GrGLuint texID0, GLTestContext* glctx1,
                    sk_sp<GrContext> grctx1, GrBackendTexture* backendTex1,
                    GrEGLImage image1) {
    if (glctx1) {
        glctx1->makeCurrent();
        if (grctx1) {
            if (backendTex1 && backendTex1->isValid()) {
                grctx1->deleteBackendTexture(*backendTex1);
            }
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
    GrGLGpu* gpu0 = static_cast<GrGLGpu*>(context0->priv().getGpu());
    if (!gpu0->glCaps().shaderCaps()->externalTextureSupport()) {
        return;
    }

    std::unique_ptr<GLTestContext> glCtx1 = glCtx0->makeNew();
    if (!glCtx1) {
        return;
    }
    sk_sp<GrContext> context1 = GrContext::MakeGL(sk_ref_sp(glCtx1->gl()));
    GrBackendTexture backendTexture1;
    GrEGLImage image = GR_EGL_NO_IMAGE;
    GrGLTextureInfo externalTexture;
    externalTexture.fID = 0;

    if (!context1) {
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
        return;
    }

    if (!glCtx1->gl()->hasExtension("EGL_KHR_image") ||
        !glCtx1->gl()->hasExtension("EGL_KHR_gl_texture_2D_image")) {
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
        return;
    }

    ///////////////////////////////// CONTEXT 1 ///////////////////////////////////

    // Use GL Context 1 to create a texture unknown to GrContext.
    context1->flush();
    GrGpu* gpu1 = context1->priv().getGpu();
    static const int kSize = 100;
    backendTexture1 =
        context1->createBackendTexture(kSize, kSize, kRGBA_8888_SkColorType,
                                       GrMipMapped::kNo, GrRenderable::kNo);

    if (!backendTexture1.isValid() || !gpu1->isTestingOnlyBackendTexture(backendTexture1)) {
        ERRORF(reporter, "Error creating texture for EGL Image");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
        return;
    }

    GrGLTextureInfo texInfo;
    if (!backendTexture1.getGLTextureInfo(&texInfo)) {
        ERRORF(reporter, "Failed to get GrGLTextureInfo");
        return;
    }

    if (GR_GL_TEXTURE_2D != texInfo.fTarget) {
        ERRORF(reporter, "Expected backend texture to be 2D");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
        return;
    }

    // Wrap the texture in an EGLImage
    image = glCtx1->texture2DToEGLImage(texInfo.fID);
    if (GR_EGL_NO_IMAGE == image) {
        ERRORF(reporter, "Error creating EGL Image from texture");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
        return;
    }

    // Since we are dealing with two different GL contexts here, we need to call finish so that the
    // clearing of the texture that happens in createTextingOnlyBackendTexture occurs before we call
    // TexSubImage below on the other context. Otherwise, it is possible the calls get reordered and
    // the clearing overwrites the TexSubImage writes.
    GR_GL_CALL(glCtx1->gl(), Finish());

    // Populate the texture using GL context 1. Important to use TexSubImage as TexImage orphans
    // the EGL image. Also, this must be done after creating the EGLImage as the texture
    // contents may not be preserved when the image is created.
    SkAutoTMalloc<uint32_t> pixels(kSize * kSize);
    for (int i = 0; i < kSize*kSize; ++i) {
        pixels.get()[i] = 0xDDAABBCC;
    }
    GR_GL_CALL(glCtx1->gl(), ActiveTexture(GR_GL_TEXTURE0));
    GR_GL_CALL(glCtx1->gl(), BindTexture(texInfo.fTarget, texInfo.fID));
    GR_GL_CALL(glCtx1->gl(), TexSubImage2D(texInfo.fTarget, 0, 0, 0, kSize, kSize,
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
    if (0 == externalTexture.fID) {
        ERRORF(reporter, "Error converting EGL Image back to texture");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
        return;
    }

    // Wrap this texture ID in a GrTexture
    GrBackendTexture backendTex(kSize, kSize, GrMipMapped::kNo, externalTexture);
    backendTex.setPixelConfig(kRGBA_8888_GrPixelConfig);

    // TODO: If I make this TopLeft origin to match resolve_origin calls for kDefault, this test
    // fails on the Nexus5. Why?
    sk_sp<GrTextureContext> surfaceContext = context0->priv().makeBackendTextureContext(
            backendTex, kBottomLeft_GrSurfaceOrigin, nullptr);

    if (!surfaceContext) {
        ERRORF(reporter, "Error wrapping external texture in GrSurfaceContext.");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
        return;
    }

    GrTextureProxy* proxy = surfaceContext->asTextureProxy();
    REPORTER_ASSERT(reporter, proxy->mipMapped() == GrMipMapped::kNo);
    REPORTER_ASSERT(reporter, proxy->peekTexture()->texturePriv().mipMapped() == GrMipMapped::kNo);

    REPORTER_ASSERT(reporter, proxy->textureType() == GrTextureType::kExternal);
    REPORTER_ASSERT(reporter,
                    proxy->peekTexture()->texturePriv().textureType() == GrTextureType::kExternal);
    REPORTER_ASSERT(reporter, proxy->hasRestrictedSampling());
    REPORTER_ASSERT(reporter, proxy->peekTexture()->texturePriv().hasRestrictedSampling());

    // Should not be able to wrap as a RT
    {
        sk_sp<GrRenderTargetContext> temp =
                context0->priv().makeBackendTextureRenderTargetContext(
                        backendTex, kBottomLeft_GrSurfaceOrigin, 1, nullptr);
        if (temp) {
            ERRORF(reporter, "Should not be able to wrap an EXTERNAL texture as a RT.");
        }
    }

    test_read_pixels(reporter, surfaceContext.get(), pixels.get(), "EGLImageTest-read");

    // We should not be able to write to a EXTERNAL texture
    test_write_pixels(reporter, surfaceContext.get(), false, "EGLImageTest-write");

    // Only test RT-config
    // TODO: why do we always need to draw to copy from an external texture?
    test_copy_from_surface(reporter, context0, surfaceContext->asSurfaceProxy(),
                           pixels.get(), true, "EGLImageTest-copy");

    cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, &backendTexture1, image);
}

#endif
