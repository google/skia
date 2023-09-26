/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "tests/Test.h"


#ifdef SK_GL
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLFunctions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h" // IWYU pragma: keep
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tests/CtsEnforcement.h"
#include "tests/TestUtils.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/gpu/gl/GLTestContext.h"

#include <cstdint>
#include <memory>
#include <utility>

using namespace skia_private;

struct GrContextOptions;

using sk_gpu_test::GLTestContext;

static void cleanup(GLTestContext* glctx0,
                    GrGLuint texID0,
                    GLTestContext* glctx1,
                    sk_sp<GrDirectContext> dContext,
                    GrEGLImage image1) {
    if (glctx1) {
        glctx1->makeCurrent();
        if (GR_EGL_NO_IMAGE != image1) {
            glctx1->destroyEGLImage(image1);
        }
    }

    glctx0->makeCurrent();
    if (texID0) {
        GR_GL_CALL(glctx0->gl(), DeleteTextures(1, &texID0));
    }
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(EGLImageTest, reporter, ctxInfo, CtsEnforcement::kApiLevel_T) {
    auto context0 = ctxInfo.directContext();
    sk_gpu_test::GLTestContext* glCtx0 = ctxInfo.glContext();

    // Try to create a second GL context and then check if the contexts have necessary
    // extensions to run this test.

    if (kGLES_GrGLStandard != glCtx0->gl()->fStandard) {
        return;
    }
    GrGLGpu* gpu0 = static_cast<GrGLGpu*>(context0->priv().getGpu());
    if (!gpu0->glCaps().shaderCaps()->fExternalTextureSupport) {
        return;
    }

    std::unique_ptr<GLTestContext> glCtx1 = glCtx0->makeNew();
    if (!glCtx1) {
        return;
    }
    sk_sp<GrDirectContext> context1 = GrDirectContexts::MakeGL(sk_ref_sp(glCtx1->gl()));
    GrEGLImage image = GR_EGL_NO_IMAGE;
    GrGLTextureInfo externalTexture;
    externalTexture.fID = 0;

    if (!context1) {
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
        return;
    }

    if (!glCtx1->gl()->hasExtension("EGL_KHR_image") ||
        !glCtx1->gl()->hasExtension("EGL_KHR_gl_texture_2D_image")) {
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
        return;
    }

    ///////////////////////////////// CONTEXT 1 ///////////////////////////////////

    // Use GL Context 1 to create a texture unknown to context 0.
    context1->flushAndSubmit();
    static const int kSize = 100;

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(context1.get(),
                                                                    kSize,
                                                                    kSize,
                                                                    kRGBA_8888_SkColorType,
                                                                    skgpu::Mipmapped::kNo,
                                                                    GrRenderable::kNo,
                                                                    GrProtected::kNo);

    if (!mbet) {
        ERRORF(reporter, "Error creating texture for EGL Image");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
        return;
    }

    GrGLTextureInfo texInfo;
    if (!GrBackendTextures::GetGLTextureInfo(mbet->texture(), &texInfo)) {
        ERRORF(reporter, "Failed to get GrGLTextureInfo");
        return;
    }

    if (GR_GL_TEXTURE_2D != texInfo.fTarget) {
        ERRORF(reporter, "Expected backend texture to be 2D");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
        return;
    }

    // Wrap the texture in an EGLImage
    image = glCtx1->texture2DToEGLImage(texInfo.fID);
    if (GR_EGL_NO_IMAGE == image) {
        ERRORF(reporter, "Error creating EGL Image from texture");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
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
    AutoTMalloc<uint32_t> pixels(kSize * kSize);
    for (int i = 0; i < kSize*kSize; ++i) {
        pixels.get()[i] = 0xDDAABBCC;
    }
    GR_GL_CALL(glCtx1->gl(), ActiveTexture(GR_GL_TEXTURE0));
    GR_GL_CALL(glCtx1->gl(), BindTexture(texInfo.fTarget, texInfo.fID));
    GR_GL_CALL(glCtx1->gl(), TexSubImage2D(texInfo.fTarget, 0, 0, 0, kSize, kSize,
                                           GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, pixels.get()));
    GR_GL_CALL(glCtx1->gl(), Finish());
    // We've been making direct GL calls in GL context 1, let GrDirectContext 1 know its internal
    // state is invalid.
    context1->resetContext();

    ///////////////////////////////// CONTEXT 0 ///////////////////////////////////

    // Make a new texture ID in GL Context 0 from the EGL Image
    glCtx0->makeCurrent();
    externalTexture.fTarget = GR_GL_TEXTURE_EXTERNAL;
    externalTexture.fID = glCtx0->eglImageToExternalTexture(image);
    externalTexture.fFormat = GR_GL_RGBA8;
    if (0 == externalTexture.fID) {
        ERRORF(reporter, "Error converting EGL Image back to texture");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
        return;
    }

    // Wrap this texture ID in a GrTexture
    GrBackendTexture backendTex =
            GrBackendTextures::MakeGL(kSize, kSize, skgpu::Mipmapped::kNo, externalTexture);

    GrColorInfo colorInfo(GrColorType::kRGBA_8888, kPremul_SkAlphaType, nullptr);
    // TODO: If I make this TopLeft origin to match resolve_origin calls for kDefault, this test
    // fails on the Nexus5. Why?
    GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin;
    sk_sp<GrSurfaceProxy> texProxy = context0->priv().proxyProvider()->wrapBackendTexture(
            backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);
    if (!texProxy) {
        ERRORF(reporter, "Error wrapping external texture in GrTextureProxy.");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
        return;
    }
    skgpu::Swizzle swizzle = context0->priv().caps()->getReadSwizzle(texProxy->backendFormat(),
                                                                     colorInfo.colorType());
    GrSurfaceProxyView view(std::move(texProxy), origin, swizzle);
    auto surfaceContext = context0->priv().makeSC(std::move(view), colorInfo);

    if (!surfaceContext) {
        ERRORF(reporter, "Error wrapping external texture in SurfaceContext.");
        cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
        return;
    }

    GrTextureProxy* proxy = surfaceContext->asTextureProxy();
    REPORTER_ASSERT(reporter, proxy->mipmapped() == skgpu::Mipmapped::kNo);
    REPORTER_ASSERT(reporter, proxy->peekTexture()->mipmapped() == skgpu::Mipmapped::kNo);

    REPORTER_ASSERT(reporter, proxy->textureType() == GrTextureType::kExternal);
    REPORTER_ASSERT(reporter, proxy->peekTexture()->textureType() == GrTextureType::kExternal);
    REPORTER_ASSERT(reporter, proxy->hasRestrictedSampling());
    REPORTER_ASSERT(reporter, proxy->peekTexture()->hasRestrictedSampling());

    // Should not be able to wrap as a RT
    {
        auto temp = context0->priv().makeSFCFromBackendTexture(colorInfo,
                                                               backendTex,
                                                               1,
                                                               origin,
                                                               /*release helper*/ nullptr);
        if (temp) {
            ERRORF(reporter, "Should not be able to wrap an EXTERNAL texture as a RT.");
        }
    }

    //TestReadPixels(reporter, context0, surfaceContext.get(), pixels.get(), "EGLImageTest-read");

    SkDebugf("type: %d\n", (int)surfaceContext->asTextureProxy()->textureType());
    // We should not be able to write to an EXTERNAL texture
    TestWritePixels(reporter, context0, surfaceContext.get(), false, "EGLImageTest-write");

    // Only test RT-config
    // TODO: why do we always need to draw to copy from an external texture?
    TestCopyFromSurface(reporter,
                        context0,
                        surfaceContext->asSurfaceProxyRef(),
                        surfaceContext->origin(),
                        colorInfo.colorType(),
                        pixels.get(),
                        "EGLImageTest-copy");

    cleanup(glCtx0, externalTexture.fID, glCtx1.get(), context1, image);
}

#endif  // SK_GL
