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
#include "src/gpu/GrBackendTextureImageGenerator.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxy.h"
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
            GrBackendTexture backendTex = context->createBackendTexture(
                    kSize, kSize, kRGBA_8888_SkColorType,
                    SkColors::kTransparent, mipMapped, renderable, GrProtected::kNo);

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
                context->deleteBackendTexture(backendTex);
                return;
            }

            REPORTER_ASSERT(reporter, proxy->isInstantiated());

            GrTexture* texture = proxy->peekTexture();
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                context->deleteBackendTexture(backendTex);
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
            context->deleteBackendTexture(backendTex);
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
            GrBackendTexture backendTex = context->createBackendTexture(
                    kSize, kSize, kRGBA_8888_SkColorType,
                    SkColors::kTransparent, mipMapped, GrRenderable::kNo, GrProtected::kNo);

            sk_sp<SkImage> image = SkImage::MakeFromTexture(context, backendTex,
                                                            kTopLeft_GrSurfaceOrigin,
                                                            kRGBA_8888_SkColorType,
                                                            kPremul_SkAlphaType, nullptr,
                                                            nullptr, nullptr);

            GrTextureProxy* proxy = as_IB(image)->peekProxy();
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                context->deleteBackendTexture(backendTex);
                return;
            }

            REPORTER_ASSERT(reporter, proxy->isInstantiated());

            sk_sp<GrTexture> texture = sk_ref_sp(proxy->peekTexture());
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                context->deleteBackendTexture(backendTex);
                return;
            }

            std::unique_ptr<SkImageGenerator> imageGen = GrBackendTextureImageGenerator::Make(
                    texture, kTopLeft_GrSurfaceOrigin, nullptr, kRGBA_8888_SkColorType,
                    kPremul_SkAlphaType, nullptr);
            REPORTER_ASSERT(reporter, imageGen);
            if (!imageGen) {
                context->deleteBackendTexture(backendTex);
                return;
            }

            SkIPoint origin = SkIPoint::Make(0,0);
            SkImageInfo imageInfo = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);
            sk_sp<GrTextureProxy> genProxy = imageGen->generateTexture(context, imageInfo,
                                                                       origin, willUseMips);

            REPORTER_ASSERT(reporter, genProxy);
            if (!genProxy) {
                context->deleteBackendTexture(backendTex);
                return;
            }

            if (GrSurfaceProxy::LazyState::kNot != genProxy->lazyInstantiationState()) {
                genProxy->priv().doLazyInstantiation(context->priv().resourceProvider());
            } else if (!genProxy->isInstantiated()) {
                genProxy->instantiate(context->priv().resourceProvider());
            }

            REPORTER_ASSERT(reporter, genProxy->isInstantiated());
            if (!genProxy->isInstantiated()) {
                context->deleteBackendTexture(backendTex);
                return;
            }

            GrTexture* genTexture = genProxy->peekTexture();
            REPORTER_ASSERT(reporter, genTexture);
            if (!genTexture) {
                context->deleteBackendTexture(backendTex);
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

            context->deleteBackendTexture(backendTex);
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
            GrBackendTexture backendTex = context->createBackendTexture(
                    kSize, kSize, kRGBA_8888_SkColorType,
                    SkColors::kTransparent, mipMapped, GrRenderable::kYes, GrProtected::kNo);
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
                context->deleteBackendTexture(backendTex);
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
                context->deleteBackendTexture(backendTex);
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
            context->deleteBackendTexture(backendTex);
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

// Create a new render target and draw 'mipmapProxy' into it using the provided 'filter'.
static std::unique_ptr<GrRenderTargetContext> draw_mipmap_into_new_render_target(
        GrDrawingManager* drawingManager, GrProxyProvider* proxyProvider, GrColorType colorType,
        sk_sp<GrTextureProxy> mipmapProxy, GrSamplerState::Filter filter) {
    GrSurfaceDesc desc;
    desc.fWidth = 1;
    desc.fHeight = 1;
    desc.fConfig = mipmapProxy->config();
    sk_sp<GrSurfaceProxy> renderTarget = proxyProvider->createProxy(
            mipmapProxy->backendFormat(), desc, GrRenderable::kYes, 1, kTopLeft_GrSurfaceOrigin,
            SkBackingFit::kApprox, SkBudgeted::kYes, GrProtected::kNo);
    auto rtc = drawingManager->makeRenderTargetContext(
            std::move(renderTarget), colorType, nullptr, nullptr, true);
    rtc->drawTexture(GrNoClip(), mipmapProxy, filter, SkBlendMode::kSrcOver, {1,1,1,1},
                     SkRect::MakeWH(4, 4), SkRect::MakeWH(1,1), GrAA::kYes, GrQuadAAFlags::kAll,
                     SkCanvas::kFast_SrcRectConstraint, SkMatrix::I(), nullptr);
    return rtc;
}

// Test that two opsTasks using the same mipmaps both depend on the same GrTextureResolveRenderTask.
DEF_GPUTEST(GrManyDependentsMipMappedTest, reporter, /* options */) {
    using CanClearFullscreen = GrRenderTargetContext::CanClearFullscreen;
    using Enable = GrContextOptions::Enable;
    using Filter = GrSamplerState::Filter;

    for (auto enableSortingAndReduction : {Enable::kYes, Enable::kNo}) {
        GrMockOptions mockOptions;
        mockOptions.fMipMapSupport = true;
        GrContextOptions ctxOptions;
        ctxOptions.fReduceOpListSplitting = enableSortingAndReduction;
        sk_sp<GrContext> context = GrContext::MakeMock(&mockOptions, ctxOptions);
        if (!context) {
            ERRORF(reporter, "could not create mock context with fReduceOpsTaskSplitting %s.",
                   (Enable::kYes == enableSortingAndReduction) ? "enabled" : "disabled");
            continue;
        }

        SkASSERT(context->priv().caps()->mipMapSupport());

        GrBackendFormat format = context->defaultBackendFormat(
                kRGBA_8888_SkColorType, GrRenderable::kYes);
        GrPixelConfig config = kRGBA_8888_GrPixelConfig;
        GrColorType colorType = GrColorType::kRGBA_8888;

        GrDrawingManager* drawingManager = context->priv().drawingManager();
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

        // Create a mipmapped render target.
        GrSurfaceDesc desc;
        desc.fWidth = 4;
        desc.fHeight = 4;
        desc.fConfig = config;
        sk_sp<GrTextureProxy> mipmapProxy = proxyProvider->createMipMapProxy(
                format, desc, GrRenderable::kYes, 1, kTopLeft_GrSurfaceOrigin, SkBudgeted::kYes,
                GrProtected::kNo);

        // Mark the mipmaps clean to ensure things still work properly when they won't be marked
        // dirty again until GrRenderTask::makeClosed().
        mipmapProxy->markMipMapsClean();

        // Render something to dirty the mips.
        auto mipmapRTC = drawingManager->makeRenderTargetContext(
                mipmapProxy, colorType, nullptr, nullptr, true);
        mipmapRTC->clear(nullptr, {.1f,.2f,.3f,.4f}, CanClearFullscreen::kYes);
        REPORTER_ASSERT(reporter, mipmapProxy->getLastRenderTask());
        // mipmapProxy's last render task should now just be the opsTask containing the clear.
        REPORTER_ASSERT(reporter,
                mipmapRTC->testingOnly_PeekLastOpsTask() == mipmapProxy->getLastRenderTask());

        // Mipmaps don't get marked dirty until makeClosed().
        REPORTER_ASSERT(reporter, !mipmapProxy->mipMapsAreDirty());

        // Draw the dirty mipmap texture into a render target.
        auto rtc1 = draw_mipmap_into_new_render_target(
                drawingManager, proxyProvider, colorType, mipmapProxy, Filter::kMipMap);

        // Mipmaps should have gotten marked dirty during makeClosed, then marked clean again as
        // soon as a GrTextureResolveRenderTask was inserted. The way we know they were resolved is
        // if mipmapProxy->getLastRenderTask() has switched from the opsTask that drew to it, to the
        // task that resolved its mips.
        GrRenderTask* initialMipmapRegenTask = mipmapProxy->getLastRenderTask();
        REPORTER_ASSERT(reporter, initialMipmapRegenTask);
        REPORTER_ASSERT(reporter,
                initialMipmapRegenTask != mipmapRTC->testingOnly_PeekLastOpsTask());
        REPORTER_ASSERT(reporter,
                rtc1->testingOnly_PeekLastOpsTask()->dependsOn(initialMipmapRegenTask));
        REPORTER_ASSERT(reporter, !mipmapProxy->mipMapsAreDirty());

        // Draw the now-clean mipmap texture into a second target.
        auto rtc2 = draw_mipmap_into_new_render_target(
                drawingManager, proxyProvider, colorType, mipmapProxy, Filter::kMipMap);

        // Make sure the mipmap texture still has the same regen task.
        REPORTER_ASSERT(reporter, mipmapProxy->getLastRenderTask() == initialMipmapRegenTask);
        REPORTER_ASSERT(reporter,
                rtc2->testingOnly_PeekLastOpsTask()->dependsOn(initialMipmapRegenTask));
        SkASSERT(!mipmapProxy->mipMapsAreDirty());

        // Reset everything so we can go again, this time with the first draw not mipmapped.
        context->flush();

        // Render something to dirty the mips.
        mipmapRTC->clear(nullptr, {.1f,.2f,.3f,.4f}, CanClearFullscreen::kYes);
        REPORTER_ASSERT(reporter, mipmapProxy->getLastRenderTask());
        // mipmapProxy's last render task should now just be the opsTask containing the clear.
        REPORTER_ASSERT(reporter,
                mipmapRTC->testingOnly_PeekLastOpsTask() == mipmapProxy->getLastRenderTask());

        // Mipmaps don't get marked dirty until makeClosed().
        REPORTER_ASSERT(reporter, !mipmapProxy->mipMapsAreDirty());

        // Draw the dirty mipmap texture into a render target, but don't do mipmap filtering.
        rtc1 = draw_mipmap_into_new_render_target(
                drawingManager, proxyProvider, colorType, mipmapProxy, Filter::kBilerp);

        // Mipmaps should have gotten marked dirty during makeClosed() when adding the dependency.
        // Since the last draw did not use mips, they will not have been regenerated and should
        // therefore still be dirty.
        REPORTER_ASSERT(reporter, mipmapProxy->mipMapsAreDirty());

        // Since mips weren't regenerated, the last render task shouldn't have changed.
        REPORTER_ASSERT(reporter,
                mipmapRTC->testingOnly_PeekLastOpsTask() == mipmapProxy->getLastRenderTask());

        // Draw the stil-dirty mipmap texture into a second target with mipmap filtering.
        rtc2 = draw_mipmap_into_new_render_target(
                drawingManager, proxyProvider, colorType, mipmapProxy, Filter::kMipMap);

        // Make sure the mipmap texture now has a new last render task that regenerates the mips,
        // and that the mipmaps are now clean.
        REPORTER_ASSERT(reporter, mipmapProxy->getLastRenderTask());
        REPORTER_ASSERT(reporter,
                mipmapRTC->testingOnly_PeekLastOpsTask() != mipmapProxy->getLastRenderTask());
        REPORTER_ASSERT(reporter,
                rtc2->testingOnly_PeekLastOpsTask()->dependsOn(mipmapProxy->getLastRenderTask()));
        SkASSERT(!mipmapProxy->mipMapsAreDirty());
    }
}
