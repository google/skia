/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrTextureProxy.h"
#include "src/gpu/GrBackendTextureImageGenerator.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/SkGpuDevice.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkSurface_Gpu.h"
#include "tests/Test.h"

static constexpr int kSize = 8;

// Test that the correct mip map states are on the GrTextures when wrapping GrBackendTextures in
// SkImages and SkSurfaces
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrWrappedMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->priv().caps()->mipMapSupport()) {
        return;
    }

    for (auto mipMapped : {GrMipMapped::kNo, GrMipMapped::kYes}) {
        for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
            // createBackendTexture currently doesn't support uploading data to mip maps
            // so we don't send any. However, we pretend there is data for the checks below which is
            // fine since we are never actually using these textures for any work on the gpu.
            GrBackendTexture backendTex = context->priv().createBackendTexture(
                    kSize, kSize, kRGBA_8888_SkColorType, mipMapped, renderable);

            sk_sp<GrTextureProxy> proxy;
            sk_sp<SkImage> image;
            if (GrRenderable::kYes == renderable) {
                sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(
                                                                           context,
                                                                           backendTex,
                                                                           kTopLeft_GrSurfaceOrigin,
                                                                           0,
                                                                           kRGBA_8888_SkColorType,
                                                                           nullptr,
                                                                           nullptr);

                SkGpuDevice* device = ((SkSurface_Gpu*)surface.get())->getDevice();
                proxy = device->accessRenderTargetContext()->asTextureProxyRef();
            } else {
                image = SkImage::MakeFromTexture(context, backendTex,
                                                 kTopLeft_GrSurfaceOrigin,
                                                 kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType, nullptr,
                                                 nullptr, nullptr);
                proxy = as_IB(image)->asTextureProxyRef(context);
            }
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            REPORTER_ASSERT(reporter, proxy->isInstantiated());

            GrTexture* texture = proxy->peekTexture();
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            if (GrMipMapped::kYes == mipMapped) {
                REPORTER_ASSERT(reporter, GrMipMapped::kYes == texture->texturePriv().mipMapped());
                if (GrRenderable::kYes == renderable) {
                    REPORTER_ASSERT(reporter, texture->texturePriv().mipMapsAreDirty());
                } else {
                    REPORTER_ASSERT(reporter, !texture->texturePriv().mipMapsAreDirty());
                }
            } else {
                REPORTER_ASSERT(reporter, GrMipMapped::kNo == texture->texturePriv().mipMapped());
            }
            context->priv().deleteBackendTexture(backendTex);
        }
    }
}

// Test that we correctly copy or don't copy GrBackendTextures in the GrBackendTextureImageGenerator
// based on if we will use mips in the draw and the mip status of the GrBackendTexture.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrBackendTextureImageMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->priv().caps()->mipMapSupport()) {
        return;
    }

    for (auto mipMapped : {GrMipMapped::kNo, GrMipMapped::kYes}) {
        for (auto willUseMips : {false, true}) {
            GrBackendTexture backendTex = context->priv().createBackendTexture(
                    kSize, kSize, kRGBA_8888_SkColorType, mipMapped, GrRenderable::kNo);

            sk_sp<SkImage> image = SkImage::MakeFromTexture(context, backendTex,
                                                            kTopLeft_GrSurfaceOrigin,
                                                            kRGBA_8888_SkColorType,
                                                            kPremul_SkAlphaType, nullptr,
                                                            nullptr, nullptr);

            GrTextureProxy* proxy = as_IB(image)->peekProxy();
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            REPORTER_ASSERT(reporter, proxy->isInstantiated());

            sk_sp<GrTexture> texture = sk_ref_sp(proxy->peekTexture());
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            std::unique_ptr<SkImageGenerator> imageGen = GrBackendTextureImageGenerator::Make(
                    texture, kTopLeft_GrSurfaceOrigin, nullptr, kRGBA_8888_SkColorType,
                    kPremul_SkAlphaType, nullptr);
            REPORTER_ASSERT(reporter, imageGen);
            if (!imageGen) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            SkIPoint origin = SkIPoint::Make(0,0);
            SkImageInfo imageInfo = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);
            sk_sp<GrTextureProxy> genProxy = imageGen->generateTexture(context, imageInfo,
                                                                       origin, willUseMips);

            REPORTER_ASSERT(reporter, genProxy);
            if (!genProxy) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            if (GrSurfaceProxy::LazyState::kNot != genProxy->lazyInstantiationState()) {
                genProxy->priv().doLazyInstantiation(context->priv().resourceProvider());
            } else if (!genProxy->isInstantiated()) {
                genProxy->instantiate(context->priv().resourceProvider());
            }

            REPORTER_ASSERT(reporter, genProxy->isInstantiated());
            if (!genProxy->isInstantiated()) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            GrTexture* genTexture = genProxy->peekTexture();
            REPORTER_ASSERT(reporter, genTexture);
            if (!genTexture) {
                context->priv().deleteBackendTexture(backendTex);
                return;
            }

            GrBackendTexture genBackendTex = genTexture->getBackendTexture();

            if (GrBackendApi::kOpenGL == genBackendTex.backend()) {
                GrGLTextureInfo genTexInfo;
                GrGLTextureInfo origTexInfo;
                if (genBackendTex.getGLTextureInfo(&genTexInfo) &&
                    backendTex.getGLTextureInfo(&origTexInfo)) {
                    if (willUseMips && GrMipMapped::kNo == mipMapped) {
                        // We did a copy so the texture IDs should be different
                        REPORTER_ASSERT(reporter, origTexInfo.fID != genTexInfo.fID);
                    } else {
                        REPORTER_ASSERT(reporter, origTexInfo.fID == genTexInfo.fID);
                    }
                } else {
                    ERRORF(reporter, "Failed to get GrGLTextureInfo");
                }
#ifdef SK_VULKAN
            } else if (GrBackendApi::kVulkan == genBackendTex.backend()) {
                GrVkImageInfo genImageInfo;
                GrVkImageInfo origImageInfo;
                if (genBackendTex.getVkImageInfo(&genImageInfo) &&
                    backendTex.getVkImageInfo(&origImageInfo)) {
                    if (willUseMips && GrMipMapped::kNo == mipMapped) {
                        // We did a copy so the texture IDs should be different
                        REPORTER_ASSERT(reporter, origImageInfo.fImage != genImageInfo.fImage);
                    } else {
                        REPORTER_ASSERT(reporter, origImageInfo.fImage == genImageInfo.fImage);
                    }
                } else {
                    ERRORF(reporter, "Failed to get GrVkImageInfo");
                }
#endif
#ifdef SK_METAL
            } else if (GrBackendApi::kMetal == genBackendTex.backend()) {
                GrMtlTextureInfo genImageInfo;
                GrMtlTextureInfo origImageInfo;
                if (genBackendTex.getMtlTextureInfo(&genImageInfo) &&
                    backendTex.getMtlTextureInfo(&origImageInfo)) {
                    if (willUseMips && GrMipMapped::kNo == mipMapped) {
                        // We did a copy so the texture IDs should be different
                        REPORTER_ASSERT(reporter, origImageInfo.fTexture != genImageInfo.fTexture);
                    } else {
                        REPORTER_ASSERT(reporter, origImageInfo.fTexture == genImageInfo.fTexture);
                    }
                } else {
                    ERRORF(reporter, "Failed to get GrMtlTextureInfo");
                }
#endif
            } else {
                REPORTER_ASSERT(reporter, false);
            }

            // Must make sure the uses of the backend texture have finished (we possibly have a
            // queued up copy) before we delete the backend texture.
            context->flush();

            context->priv().getGpu()->testingOnly_flushGpuAndSync();

            context->priv().deleteBackendTexture(backendTex);
        }
    }
}

// Test that when we call makeImageSnapshot on an SkSurface we retains the same mip status as the
// resource we took the snapshot of.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrImageSnapshotMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->priv().caps()->mipMapSupport()) {
        return;
    }

    auto resourceProvider = context->priv().resourceProvider();

    for (auto willUseMips : {false, true}) {
        for (auto isWrapped : {false, true}) {
            GrMipMapped mipMapped = willUseMips ? GrMipMapped::kYes : GrMipMapped::kNo;
            sk_sp<SkSurface> surface;
            GrBackendTexture backendTex = context->priv().createBackendTexture(
                    kSize, kSize, kRGBA_8888_SkColorType, mipMapped, GrRenderable::kYes);
            if (isWrapped) {
                surface = SkSurface::MakeFromBackendTexture(context,
                                                            backendTex,
                                                            kTopLeft_GrSurfaceOrigin,
                                                            0,
                                                            kRGBA_8888_SkColorType,
                                                            nullptr,
                                                            nullptr);
            } else {
                SkImageInfo info = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                                     kPremul_SkAlphaType);
                surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info, 0,
                                                      kTopLeft_GrSurfaceOrigin, nullptr,
                                                      willUseMips);
            }
            REPORTER_ASSERT(reporter, surface);
            if (!surface) {
                context->priv().deleteBackendTexture(backendTex);
            }
            SkGpuDevice* device = ((SkSurface_Gpu*)surface.get())->getDevice();
            GrTextureProxy* texProxy = device->accessRenderTargetContext()->asTextureProxy();
            REPORTER_ASSERT(reporter, mipMapped == texProxy->mipMapped());

            texProxy->instantiate(resourceProvider);
            GrTexture* texture = texProxy->peekTexture();
            REPORTER_ASSERT(reporter, mipMapped == texture->texturePriv().mipMapped());

            sk_sp<SkImage> image = surface->makeImageSnapshot();
            REPORTER_ASSERT(reporter, image);
            if (!image) {
                context->priv().deleteBackendTexture(backendTex);
            }
            texProxy = as_IB(image)->peekProxy();
            REPORTER_ASSERT(reporter, mipMapped == texProxy->mipMapped());

            texProxy->instantiate(resourceProvider);
            texture = texProxy->peekTexture();
            REPORTER_ASSERT(reporter, mipMapped == texture->texturePriv().mipMapped());

            // Must flush the context to make sure all the cmds (copies, etc.) from above are sent
            // to the gpu before we delete the backendHandle.
            context->flush();
            context->priv().getGpu()->testingOnly_flushGpuAndSync();
            context->priv().deleteBackendTexture(backendTex);
        }
    }
}

// Test that we don't create a mip mapped texture if the size is 1x1 even if the filter mode is set
// to use mips. This test passes by not crashing or hitting asserts in code.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(Gr1x1TextureMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->priv().caps()->mipMapSupport()) {
        return;
    }

    // Make surface to draw into
    SkImageInfo info = SkImageInfo::MakeN32(16, 16, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);

    // Make 1x1 raster bitmap
    SkBitmap bmp;
    bmp.allocN32Pixels(1, 1);
    SkPMColor* pixel = reinterpret_cast<SkPMColor*>(bmp.getPixels());
    *pixel = 0;

    sk_sp<SkImage> bmpImage = SkImage::MakeFromBitmap(bmp);

    // Make sure we scale so we don't optimize out the use of mips.
    surface->getCanvas()->scale(0.5f, 0.5f);

    SkPaint paint;
    // This should upload the image to a non mipped GrTextureProxy.
    surface->getCanvas()->drawImage(bmpImage, 0, 0, &paint);
    surface->flush();

    // Now set the filter quality to high so we use mip maps. We should find the non mipped texture
    // in the cache for the SkImage. Since the texture is 1x1 we should just use that texture
    // instead of trying to do a copy to a mipped texture.
    paint.setFilterQuality(kHigh_SkFilterQuality);
    surface->getCanvas()->drawImage(bmpImage, 0, 0, &paint);
    surface->flush();
}

