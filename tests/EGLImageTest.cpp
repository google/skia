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
#include "gl/SkGLContext.h"

static void cleanup(SkGLContext* glctx0, GrGLuint texID0, SkGLContext* glctx1, GrContext* grctx1,
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
        glctx1->unref();
    }

    glctx0->makeCurrent();
    if (texID0) {
        GR_GL_CALL(glctx0->gl(), DeleteTextures(1, &texID0));
    }
}

DEF_GPUTEST(EGLImageTest, reporter, factory) {
    for (int glCtxType = 0; glCtxType < GrContextFactory::kGLContextTypeCnt; ++glCtxType) {
        GrContextFactory::GLContextType type = (GrContextFactory::GLContextType)glCtxType;
        if (!GrContextFactory::IsRenderingGLContext(type)) {
            continue;
        }
        
        // Try to create a second GL context and then check if the contexts have necessary
        // extensions to run this test.

        GrContext* context0 = factory->get(type);
        if (!context0) {
            continue;
        }
        SkGLContext* glCtx0 = factory->getGLContext(type);
        SkASSERT(glCtx0);
        if (kGLES_GrGLStandard != glCtx0->gl()->fStandard) {
            continue;
        }
        GrGLGpu* gpu0 = static_cast<GrGLGpu*>(context0->getGpu());
        if (!gpu0->glCaps().externalTextureSupport()) {
            continue;
        }

        SkGLContext* glCtx1 = glCtx0->createNew();
        if (!glCtx1) {
            continue;
        }
        GrContext* context1 = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)glCtx1->gl());
        const GrGLTextureInfo* backendTexture1 = nullptr;
        GrEGLImage image = GR_EGL_NO_IMAGE;
        GrGLTextureInfo externalTexture;
        externalTexture.fID = 0;

        if (!context1) {
            cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
            continue;
        }

        if (!glCtx1->gl()->hasExtension("EGL_KHR_image") ||
            !glCtx1->gl()->hasExtension("EGL_KHR_gl_texture_2D_image")) {
            cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
            continue;
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
            cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
            continue;
        }
        if (GR_GL_TEXTURE_2D != backendTexture1->fTarget) {
            ERRORF(reporter, "Expected backend texture to be 2D");
            cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
            continue;
        }

        // Wrap the texture in an EGLImage
        image = glCtx1->texture2DToEGLImage(backendTexture1->fID);
        if (GR_EGL_NO_IMAGE == image) {
            ERRORF(reporter, "Error creating EGL Image from texture");
            cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
            continue;
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
        SkAutoTUnref<GrTexture> externalTextureObj(
            context0->textureProvider()->wrapBackendTexture(externalDesc));
        if (!externalTextureObj) {
            ERRORF(reporter, "Error wrapping external texture in GrTexture.");
            cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
            continue;
        }

        // Read the pixels and see if we get the values set in GL context 1
        memset(pixels.get(), 0, sizeof(uint32_t)*kSize*kSize);
        bool read = externalTextureObj->readPixels(0, 0, kSize, kSize, kRGBA_8888_GrPixelConfig,
                                                   pixels.get());
        if (!read) {
            ERRORF(reporter, "Error reading external texture.");
            cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
            continue;
        }
        for (int i = 0; i < kSize*kSize; ++i) {
            if (pixels.get()[i] != 0xDDAABBCC) {
                ERRORF(reporter, "Error, external texture pixel value %d should be 0xDDAABBCC,"
                                 " got 0x%08x.", pixels.get()[i]);
                break;
            }
        }
        cleanup(glCtx0, externalTexture.fID, glCtx1, context1, backendTexture1, image);
    }
}

#endif
