/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrBackendSurface.h"
#include "GrBackendTextureImageGenerator.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrRenderTargetContext.h"
#include "GrSemaphore.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTest.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkGpuDevice.h"
#include "SkPoint.h"
#include "SkSurface.h"
#include "SkSurface_Gpu.h"
#include "Test.h"

static constexpr int kSize = 8;

// Test that the correct mip map states are on the GrTextures when wrapping GrBackendTextures in
// SkImages and SkSurfaces
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrWrappedMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->caps()->mipMapSupport()) {
        return;
    }
    for (auto mipMapped : {GrMipMapped::kNo, GrMipMapped::kYes}) {
        for (auto isRT : {false, true}) {
            // CreateTestingOnlyBackendTexture currently doesn't support uploading data to mip maps
            // so we don't send any. However, we pretend there is data for the checks below which is
            // fine since we are never actually using these textures for any work on the gpu.
            GrBackendObject backendHandle = context->getGpu()->createTestingOnlyBackendTexture(
                    nullptr, 8, 8, kRGBA_8888_GrPixelConfig, isRT, mipMapped);

            GrBackend backend = context->contextPriv().getBackend();
            GrBackendTexture backendTex = GrTest::CreateBackendTexture(backend,
                                                                       kSize,
                                                                       kSize,
                                                                       kRGBA_8888_GrPixelConfig,
                                                                       mipMapped,
                                                                       backendHandle);

            GrTextureProxy* proxy;
            sk_sp<SkImage> image;
            if (isRT) {
                sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(
                                                                           context,
                                                                           backendTex,
                                                                           kTopLeft_GrSurfaceOrigin,
                                                                           0,
                                                                           nullptr,
                                                                           nullptr);

                SkGpuDevice* device = ((SkSurface_Gpu*)surface.get())->getDevice();
                proxy = device->accessRenderTargetContext()->asTextureProxy();
            } else {
                image = SkImage::MakeFromTexture(context, backendTex,
                                                 kTopLeft_GrSurfaceOrigin,
                                                 kPremul_SkAlphaType, nullptr);
                proxy = as_IB(image)->peekProxy();
            }
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            REPORTER_ASSERT(reporter, proxy->priv().isInstantiated());

            GrTexture* texture = proxy->priv().peekTexture();
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            if (GrMipMapped::kYes == mipMapped) {
                REPORTER_ASSERT(reporter, GrMipMapped::kYes == texture->texturePriv().mipMapped());
                if (isRT) {
                    REPORTER_ASSERT(reporter, texture->texturePriv().mipMapsAreDirty());
                } else {
                    REPORTER_ASSERT(reporter, !texture->texturePriv().mipMapsAreDirty());
                }
            } else {
                REPORTER_ASSERT(reporter, GrMipMapped::kNo == texture->texturePriv().mipMapped());
            }
            context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
        }
    }
}

// Test that we correctly copy or don't copy GrBackendTextures in the GrBackendTextureImageGenerator
// based on if we will use mips in the draw and the mip status of the GrBackendTexture.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrBackendTextureImageMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->caps()->mipMapSupport()) {
        return;
    }
    for (auto mipMapped : {GrMipMapped::kNo, GrMipMapped::kYes}) {
        for (auto willUseMips : {false, true}) {
            GrBackendObject backendHandle = context->getGpu()->createTestingOnlyBackendTexture(
                    nullptr, kSize, kSize, kRGBA_8888_GrPixelConfig, false, mipMapped);

            GrBackend backend = context->contextPriv().getBackend();
            GrBackendTexture backendTex = GrTest::CreateBackendTexture(backend,
                                                                       kSize,
                                                                       kSize,
                                                                       kRGBA_8888_GrPixelConfig,
                                                                       mipMapped,
                                                                       backendHandle);

            sk_sp<SkImage> image = SkImage::MakeFromTexture(context, backendTex,
                                                            kTopLeft_GrSurfaceOrigin,
                                                            kPremul_SkAlphaType, nullptr);

            GrTextureProxy* proxy = as_IB(image)->peekProxy();
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            REPORTER_ASSERT(reporter, proxy->priv().isInstantiated());

            sk_sp<GrTexture> texture = sk_ref_sp(proxy->priv().peekTexture());
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            std::unique_ptr<SkImageGenerator> imageGen = GrBackendTextureImageGenerator::Make(
                    texture, kTopLeft_GrSurfaceOrigin, nullptr, kPremul_SkAlphaType, nullptr);
            REPORTER_ASSERT(reporter, imageGen);
            if (!imageGen) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            SkIPoint origin = SkIPoint::Make(0,0);
            // The transfer function behavior isn't used in the generator so set we set it
            // arbitrarily here.
            SkTransferFunctionBehavior behavior = SkTransferFunctionBehavior::kIgnore;
            SkImageInfo imageInfo = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);
            sk_sp<GrTextureProxy> genProxy = imageGen->generateTexture(context, imageInfo,
                                                                       origin, behavior,
                                                                       willUseMips);

            REPORTER_ASSERT(reporter, genProxy);
            if (!genProxy) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            REPORTER_ASSERT(reporter, genProxy->priv().isInstantiated());

            GrTexture* genTexture = genProxy->priv().peekTexture();
            REPORTER_ASSERT(reporter, genTexture);
            if (!genTexture) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
                return;
            }

            GrBackendObject genBackendObject = genTexture->getTextureHandle();

            if (kOpenGL_GrBackend == backend) {
                const GrGLTextureInfo* origTexInfo = backendTex.getGLTextureInfo();
                GrGLTextureInfo* genTexInfo = (GrGLTextureInfo*)genBackendObject;
                if (willUseMips && GrMipMapped::kNo == mipMapped) {
                    // We did a copy so the texture IDs should be different
                    REPORTER_ASSERT(reporter, origTexInfo->fID != genTexInfo->fID);
                } else {
                    REPORTER_ASSERT(reporter, origTexInfo->fID == genTexInfo->fID);
                }
            } else if (kVulkan_GrBackend == backend) {
#ifdef SK_VULKAN
                const GrVkImageInfo* origImageInfo = backendTex.getVkImageInfo();
                GrVkImageInfo* genImageInfo = (GrVkImageInfo*)genBackendObject;
                if (willUseMips && GrMipMapped::kNo == mipMapped) {
                    // We did a copy so the texture IDs should be different
                    REPORTER_ASSERT(reporter, origImageInfo->fImage != genImageInfo->fImage);
                } else {
                    REPORTER_ASSERT(reporter, origImageInfo->fImage == genImageInfo->fImage);
                }
#endif
            } else if (kMetal_GrBackend == backend) {
                REPORTER_ASSERT(reporter, false);
            } else {
                REPORTER_ASSERT(reporter, false);
            }

            // Must make sure the uses of the backend texture have finished (we possibly have a
            // queued up copy) before we delete the backend texture. Thus we use readPixels here
            // just to force the synchronization.
            sk_sp<GrSurfaceContext> surfContext =
                    context->contextPriv().makeWrappedSurfaceContext(genProxy, nullptr);

            SkBitmap bitmap;
            bitmap.allocPixels(imageInfo);
            surfContext->readPixels(imageInfo, bitmap.getPixels(), 0, 0, 0, 0);

            context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
        }
    }
}

// Test that when we call makeImageSnapshot on an SkSurface we retains the same mip status as the
// resource we took the snapshot of.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrImageSnapshotMipMappedTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->caps()->mipMapSupport()) {
        return;
    }

    for (auto willUseMips : {false, true}) {
        for (auto isWrapped : {false, true}) {
            GrMipMapped mipMapped = willUseMips ? GrMipMapped::kYes : GrMipMapped::kNo;
            sk_sp<SkSurface> surface;
            GrBackendObject backendHandle = context->getGpu()->createTestingOnlyBackendTexture(
                    nullptr, 8, 8, kRGBA_8888_GrPixelConfig, true, mipMapped);
            if (isWrapped) {
                GrBackend backend = context->contextPriv().getBackend();
                GrBackendTexture backendTex = GrTest::CreateBackendTexture(backend,
                                                                           kSize,
                                                                           kSize,
                                                                           kRGBA_8888_GrPixelConfig,
                                                                           mipMapped,
                                                                           backendHandle);

                surface = SkSurface::MakeFromBackendTexture(context,
                                                            backendTex,
                                                            kTopLeft_GrSurfaceOrigin,
                                                            0,
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
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
            }
            SkGpuDevice* device = ((SkSurface_Gpu*)surface.get())->getDevice();
            GrTextureProxy* texProxy = device->accessRenderTargetContext()->asTextureProxy();
            REPORTER_ASSERT(reporter, mipMapped == texProxy->mipMapped());

            texProxy->instantiate(context->resourceProvider());
            GrTexture* texture = texProxy->priv().peekTexture();
            REPORTER_ASSERT(reporter, mipMapped == texture->texturePriv().mipMapped());

            sk_sp<SkImage> image = surface->makeImageSnapshot();
            REPORTER_ASSERT(reporter, image);
            if (!image) {
                context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
            }
            texProxy = as_IB(image)->peekProxy();
            REPORTER_ASSERT(reporter, mipMapped == texProxy->mipMapped());

            texProxy->instantiate(context->resourceProvider());
            texture = texProxy->priv().peekTexture();
            REPORTER_ASSERT(reporter, mipMapped == texture->texturePriv().mipMapped());

            // Must flush the context to make sure all the cmds (copies, etc.) from above are sent
            // to the gpu before we delete the backendHandle.
            context->flush();
            context->getGpu()->deleteTestingOnlyBackendTexture(backendHandle);
        }
    }
}

#endif
