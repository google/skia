/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTextureGenerator.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrBackendTextureImageGenerator.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/ops/OpsTask.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/gpu/ProxyUtils.h"

#include <initializer_list>
#include <memory>
#include <utility>

class GrRenderTask;

#if defined(SK_DIRECT3D)
#include "include/gpu/d3d/GrD3DTypes.h"
#endif

#if defined(SK_GL)
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#endif

#if defined(SK_VULKAN)
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"
#endif

static constexpr int kSize = 8;

// Test that the correct mip map states are on the GrTextures when wrapping GrBackendTextures in
// SkImages and SkSurfaces
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrWrappedMipMappedTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    if (!dContext->priv().caps()->mipmapSupport()) {
        return;
    }

    for (auto mipmapped : {skgpu::Mipmapped::kNo, skgpu::Mipmapped::kYes}) {
        for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
            // createBackendTexture currently doesn't support uploading data to mip maps
            // so we don't send any. However, we pretend there is data for the checks below which is
            // fine since we are never actually using these textures for any work on the gpu.
            auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithData(dContext,
                                                                         kSize,
                                                                         kSize,
                                                                         kRGBA_8888_SkColorType,
                                                                         SkColors::kTransparent,
                                                                         mipmapped,
                                                                         renderable,
                                                                         GrProtected::kNo);
            if (!mbet) {
                ERRORF(reporter, "Could not make texture.");
                return;
            }

            sk_sp<GrTextureProxy> proxy;
            sk_sp<SkImage> image;
            if (renderable == GrRenderable::kYes) {
                sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(
                        dContext,
                        mbet->texture(),
                        kTopLeft_GrSurfaceOrigin,
                        0,
                        kRGBA_8888_SkColorType,
                        /*color space*/ nullptr,
                        /*surface props*/ nullptr,
                        sk_gpu_test::ManagedBackendTexture::ReleaseProc,
                        mbet->releaseContext());

                auto device = ((SkSurface_Ganesh*)surface.get())->getDevice();
                proxy = device->readSurfaceView().asTextureProxyRef();
            } else {
                image = SkImages::BorrowTextureFrom(dContext,
                                                    mbet->texture(),
                                                    kTopLeft_GrSurfaceOrigin,
                                                    kRGBA_8888_SkColorType,
                                                    kPremul_SkAlphaType,
                                                    /* color space */ nullptr,
                                                    sk_gpu_test::ManagedBackendTexture::ReleaseProc,
                                                    mbet->releaseContext());
                REPORTER_ASSERT(reporter,
                                (mipmapped == skgpu::Mipmapped::kYes) == image->hasMipmaps());
                proxy = sk_ref_sp(sk_gpu_test::GetTextureImageProxy(image.get(), dContext));
            }
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                continue;
            }

            REPORTER_ASSERT(reporter, proxy->isInstantiated());

            GrTexture* texture = proxy->peekTexture();
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                continue;
            }

            if (mipmapped == skgpu::Mipmapped::kYes) {
                REPORTER_ASSERT(reporter, skgpu::Mipmapped::kYes == texture->mipmapped());
                if (GrRenderable::kYes == renderable) {
                    REPORTER_ASSERT(reporter, texture->mipmapsAreDirty());
                } else {
                    REPORTER_ASSERT(reporter, !texture->mipmapsAreDirty());
                }
            } else {
                REPORTER_ASSERT(reporter, skgpu::Mipmapped::kNo == texture->mipmapped());
            }
        }
    }
}

// Test that we correctly copy or don't copy GrBackendTextures in the GrBackendTextureImageGenerator
// based on if we will use mips in the draw and the mip status of the GrBackendTexture.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrBackendTextureImageMipMappedTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    if (!dContext->priv().caps()->mipmapSupport()) {
        return;
    }

    for (auto betMipmapped : {skgpu::Mipmapped::kNo, skgpu::Mipmapped::kYes}) {
        for (auto requestMipmapped : {skgpu::Mipmapped::kNo, skgpu::Mipmapped::kYes}) {
            auto ii =
                    SkImageInfo::Make({kSize, kSize}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            sk_sp<SkImage> image = sk_gpu_test::MakeBackendTextureImage(
                    dContext, ii, SkColors::kTransparent, betMipmapped);
            REPORTER_ASSERT(reporter,
                            (betMipmapped == skgpu::Mipmapped::kYes) == image->hasMipmaps());

            GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(image.get(), dContext);
            REPORTER_ASSERT(reporter, proxy);
            if (!proxy) {
                return;
            }

            REPORTER_ASSERT(reporter, proxy->isInstantiated());

            sk_sp<GrTexture> texture = sk_ref_sp(proxy->peekTexture());
            REPORTER_ASSERT(reporter, texture);
            if (!texture) {
                return;
            }

            std::unique_ptr<GrTextureGenerator> textureGen = GrBackendTextureImageGenerator::Make(
                    texture, kTopLeft_GrSurfaceOrigin, nullptr, kRGBA_8888_SkColorType,
                    kPremul_SkAlphaType, nullptr);
            REPORTER_ASSERT(reporter, textureGen);
            if (!textureGen) {
                return;
            }

            SkImageInfo imageInfo = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);
            GrSurfaceProxyView genView = textureGen->generateTexture(
                    dContext, imageInfo, requestMipmapped, GrImageTexGenPolicy::kDraw);
            GrSurfaceProxy* genProxy = genView.proxy();

            REPORTER_ASSERT(reporter, genProxy);
            if (!genProxy) {
                return;
            }

            if (genProxy->isLazy()) {
                genProxy->priv().doLazyInstantiation(dContext->priv().resourceProvider());
            } else if (!genProxy->isInstantiated()) {
                genProxy->instantiate(dContext->priv().resourceProvider());
            }

            REPORTER_ASSERT(reporter, genProxy->isInstantiated());
            if (!genProxy->isInstantiated()) {
                return;
            }

            GrTexture* genTexture = genProxy->peekTexture();
            REPORTER_ASSERT(reporter, genTexture);
            if (!genTexture) {
                return;
            }

            GrBackendTexture backendTex = texture->getBackendTexture();
            GrBackendTexture genBackendTex = genTexture->getBackendTexture();

            if (GrBackendApi::kOpenGL == genBackendTex.backend()) {
#ifdef SK_GL
                GrGLTextureInfo genTexInfo;
                GrGLTextureInfo origTexInfo;
                if (GrBackendTextures::GetGLTextureInfo(genBackendTex, &genTexInfo) &&
                    GrBackendTextures::GetGLTextureInfo(backendTex, &origTexInfo)) {
                    if (requestMipmapped == skgpu::Mipmapped::kYes &&
                        betMipmapped == skgpu::Mipmapped::kNo) {
                        // We did a copy so the texture IDs should be different
                        REPORTER_ASSERT(reporter, origTexInfo.fID != genTexInfo.fID);
                    } else {
                        REPORTER_ASSERT(reporter, origTexInfo.fID == genTexInfo.fID);
                    }
                } else {
                    ERRORF(reporter, "Failed to get GrGLTextureInfo");
                }
#endif
#ifdef SK_VULKAN
            } else if (GrBackendApi::kVulkan == genBackendTex.backend()) {
                GrVkImageInfo genImageInfo;
                GrVkImageInfo origImageInfo;
                if (GrBackendTextures::GetVkImageInfo(genBackendTex, &genImageInfo) &&
                    GrBackendTextures::GetVkImageInfo(backendTex, &origImageInfo)) {
                    if (requestMipmapped == skgpu::Mipmapped::kYes &&
                        betMipmapped == skgpu::Mipmapped::kNo) {
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
                    if (requestMipmapped == skgpu::Mipmapped::kYes &&
                        betMipmapped == skgpu::Mipmapped::kNo) {
                        // We did a copy so the texture IDs should be different
                        REPORTER_ASSERT(reporter, origImageInfo.fTexture != genImageInfo.fTexture);
                    } else {
                        REPORTER_ASSERT(reporter, origImageInfo.fTexture == genImageInfo.fTexture);
                    }
                } else {
                    ERRORF(reporter, "Failed to get GrMtlTextureInfo");
                }
#endif
#ifdef SK_DIRECT3D
            } else if (GrBackendApi::kDirect3D == genBackendTex.backend()) {
                GrD3DTextureResourceInfo genImageInfo;
                GrD3DTextureResourceInfo origImageInfo;
                if (genBackendTex.getD3DTextureResourceInfo(&genImageInfo) &&
                    backendTex.getD3DTextureResourceInfo(&origImageInfo)) {
                    if (requestMipmapped == skgpu::Mipmapped::kYes &&
                        betMipmapped == skgpu::Mipmapped::kNo) {
                        // We did a copy so the texture resources should be different
                        REPORTER_ASSERT(reporter,
                                        origImageInfo.fResource != genImageInfo.fResource);
                    } else {
                        REPORTER_ASSERT(reporter,
                                        origImageInfo.fResource == genImageInfo.fResource);
                    }
                } else {
                    ERRORF(reporter, "Failed to get GrMtlTextureInfo");
                }
#endif
            } else {
                REPORTER_ASSERT(reporter, false);
            }
        }
    }
}

// Test that when we call makeImageSnapshot on an SkSurface we retains the same mip status as the
// resource we took the snapshot of.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrImageSnapshotMipMappedTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    if (!dContext->priv().caps()->mipmapSupport()) {
        return;
    }

    auto resourceProvider = dContext->priv().resourceProvider();

    for (auto willUseMips : {false, true}) {
        for (auto isWrapped : {false, true}) {
            skgpu::Mipmapped mipmapped =
                    willUseMips ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
            sk_sp<SkSurface> surface;
            SkImageInfo info =
                    SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            if (isWrapped) {
                surface = sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                                 info,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 /* sample count */ 1,
                                                                 mipmapped);
            } else {
                surface = SkSurfaces::RenderTarget(dContext,
                                                   skgpu::Budgeted::kYes,
                                                   info,
                                                   /* sample count */ 1,
                                                   kTopLeft_GrSurfaceOrigin,
                                                   nullptr,
                                                   willUseMips);
            }
            REPORTER_ASSERT(reporter, surface);
            auto device = ((SkSurface_Ganesh*)surface.get())->getDevice();
            GrTextureProxy* texProxy = device->readSurfaceView().asTextureProxy();
            REPORTER_ASSERT(reporter, mipmapped == texProxy->mipmapped());

            texProxy->instantiate(resourceProvider);
            GrTexture* texture = texProxy->peekTexture();
            REPORTER_ASSERT(reporter, mipmapped == texture->mipmapped());

            sk_sp<SkImage> image = surface->makeImageSnapshot();
            REPORTER_ASSERT(reporter, willUseMips == image->hasMipmaps());
            REPORTER_ASSERT(reporter, image);
            texProxy = sk_gpu_test::GetTextureImageProxy(image.get(), dContext);
            REPORTER_ASSERT(reporter, mipmapped == texProxy->mipmapped());

            texProxy->instantiate(resourceProvider);
            texture = texProxy->peekTexture();
            REPORTER_ASSERT(reporter, mipmapped == texture->mipmapped());
        }
    }
}

// Test that we don't create a mip mapped texture if the size is 1x1 even if the filter mode is set
// to use mips. This test passes by not crashing or hitting asserts in code.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Gr1x1TextureMipMappedTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    if (!dContext->priv().caps()->mipmapSupport()) {
        return;
    }

    // Make surface to draw into
    SkImageInfo info = SkImageInfo::MakeN32(16, 16, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);

    // Make 1x1 raster bitmap
    SkBitmap bmp;
    bmp.allocN32Pixels(1, 1);
    SkPMColor* pixel = reinterpret_cast<SkPMColor*>(bmp.getPixels());
    *pixel = 0;

    sk_sp<SkImage> bmpImage = bmp.asImage();

    // Make sure we scale so we don't optimize out the use of mips.
    surface->getCanvas()->scale(0.5f, 0.5f);

    // This should upload the image to a non mipped GrTextureProxy.
    surface->getCanvas()->drawImage(bmpImage, 0, 0);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);

    // Now set the filter quality to high so we use mip maps. We should find the non mipped texture
    // in the cache for the SkImage. Since the texture is 1x1 we should just use that texture
    // instead of trying to do a copy to a mipped texture.
    surface->getCanvas()->drawImage(bmpImage, 0, 0, SkSamplingOptions({1.0f/3, 1.0f/3}));
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
}

// Create a new render target and draw 'mipmapView' into it using the provided 'filter'.
static std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> draw_mipmap_into_new_render_target(
        GrRecordingContext* rContext,
        GrColorType colorType,
        SkAlphaType alphaType,
        GrSurfaceProxyView mipmapView,
        GrSamplerState::MipmapMode mm) {
    auto proxyProvider = rContext->priv().proxyProvider();
    sk_sp<GrSurfaceProxy> renderTarget =
            proxyProvider->createProxy(mipmapView.proxy()->backendFormat(),
                                       {1, 1},
                                       GrRenderable::kYes,
                                       1,
                                       skgpu::Mipmapped::kNo,
                                       SkBackingFit::kApprox,
                                       skgpu::Budgeted::kYes,
                                       GrProtected::kNo,
                                       /*label=*/"DrawMipMapViewTest");

    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(rContext,
                                                       colorType,
                                                       std::move(renderTarget),
                                                       nullptr,
                                                       kTopLeft_GrSurfaceOrigin,
                                                       SkSurfaceProps());

    sdc->drawTexture(nullptr,
                     std::move(mipmapView),
                     alphaType,
                     GrSamplerState::Filter::kLinear,
                     mm,
                     SkBlendMode::kSrcOver,
                     {1, 1, 1, 1},
                     SkRect::MakeWH(4, 4),
                     SkRect::MakeWH(1, 1),
                     GrQuadAAFlags::kAll,
                     SkCanvas::kFast_SrcRectConstraint,
                     SkMatrix::I(),
                     nullptr);
    return sdc;
}

// Test that two opsTasks using the same mipmaps both depend on the same GrTextureResolveRenderTask.
DEF_GANESH_TEST(GrManyDependentsMipMappedTest,
                reporter,
                /* options */,
                CtsEnforcement::kApiLevel_T) {
    using Enable = GrContextOptions::Enable;
    using MipmapMode = GrSamplerState::MipmapMode;

    for (auto enableSortingAndReduction : {Enable::kYes, Enable::kNo}) {
        GrMockOptions mockOptions;
        mockOptions.fMipmapSupport = true;
        GrContextOptions ctxOptions;
        ctxOptions.fReduceOpsTaskSplitting = enableSortingAndReduction;
        sk_sp<GrDirectContext> dContext = GrDirectContext::MakeMock(&mockOptions, ctxOptions);
        GrDrawingManager* drawingManager = dContext->priv().drawingManager();
        if (!dContext) {
            ERRORF(reporter, "could not create mock dContext with fReduceOpsTaskSplitting %s.",
                   (Enable::kYes == enableSortingAndReduction) ? "enabled" : "disabled");
            continue;
        }

        SkASSERT(dContext->priv().caps()->mipmapSupport());

        GrBackendFormat format = dContext->defaultBackendFormat(
                kRGBA_8888_SkColorType, GrRenderable::kYes);
        GrColorType colorType = GrColorType::kRGBA_8888;
        SkAlphaType alphaType = kPremul_SkAlphaType;

        GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

        // Create a mipmapped render target.

        sk_sp<GrTextureProxy> mipmapProxy =
                proxyProvider->createProxy(format,
                                           {4, 4},
                                           GrRenderable::kYes,
                                           1,
                                           skgpu::Mipmapped::kYes,
                                           SkBackingFit::kExact,
                                           skgpu::Budgeted::kYes,
                                           GrProtected::kNo,
                                           /*label=*/"ManyDependentsMipMappedTest");

        // Mark the mipmaps clean to ensure things still work properly when they won't be marked
        // dirty again until GrRenderTask::makeClosed().
        mipmapProxy->markMipmapsClean();

        auto mipmapSDC = skgpu::ganesh::SurfaceDrawContext::Make(dContext.get(),
                                                                 colorType,
                                                                 mipmapProxy,
                                                                 nullptr,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 SkSurfaceProps());

        mipmapSDC->clear(SkPMColor4f{.1f, .2f, .3f, .4f});
        REPORTER_ASSERT(reporter, drawingManager->getLastRenderTask(mipmapProxy.get()));
        // mipmapProxy's last render task should now just be the opsTask containing the clear.
        REPORTER_ASSERT(reporter,
                mipmapSDC->testingOnly_PeekLastOpsTask() ==
                        drawingManager->getLastRenderTask(mipmapProxy.get()));

        // Mipmaps don't get marked dirty until makeClosed().
        REPORTER_ASSERT(reporter, !mipmapProxy->mipmapsAreDirty());

        skgpu::Swizzle swizzle = dContext->priv().caps()->getReadSwizzle(format, colorType);
        GrSurfaceProxyView mipmapView(mipmapProxy, kTopLeft_GrSurfaceOrigin, swizzle);

        // Draw the dirty mipmap texture into a render target.
        auto sdc1 = draw_mipmap_into_new_render_target(dContext.get(), colorType, alphaType,
                                                       mipmapView, MipmapMode::kLinear);
        auto sdc1Task = sk_ref_sp(sdc1->testingOnly_PeekLastOpsTask());

        // Mipmaps should have gotten marked dirty during makeClosed, then marked clean again as
        // soon as a GrTextureResolveRenderTask was inserted. The way we know they were resolved is
        // if mipmapProxy->getLastRenderTask() has switched from the opsTask that drew to it, to the
        // task that resolved its mips.
        GrRenderTask* initialMipmapRegenTask = drawingManager->getLastRenderTask(mipmapProxy.get());
        REPORTER_ASSERT(reporter, initialMipmapRegenTask);
        REPORTER_ASSERT(reporter,
                initialMipmapRegenTask != mipmapSDC->testingOnly_PeekLastOpsTask());
        REPORTER_ASSERT(reporter, !mipmapProxy->mipmapsAreDirty());

        // Draw the now-clean mipmap texture into a second target.
        auto sdc2 = draw_mipmap_into_new_render_target(dContext.get(), colorType, alphaType,
                                                       mipmapView, MipmapMode::kLinear);
        auto sdc2Task = sk_ref_sp(sdc2->testingOnly_PeekLastOpsTask());

        // Make sure the mipmap texture still has the same regen task.
        REPORTER_ASSERT(reporter,
                    drawingManager->getLastRenderTask(mipmapProxy.get()) == initialMipmapRegenTask);
        SkASSERT(!mipmapProxy->mipmapsAreDirty());

        // Reset everything so we can go again, this time with the first draw not mipmapped.
        dContext->flushAndSubmit();

        // Mip regen tasks don't get added as dependencies until makeClosed().
        REPORTER_ASSERT(reporter, sdc1Task->dependsOn(initialMipmapRegenTask));
        REPORTER_ASSERT(reporter, sdc2Task->dependsOn(initialMipmapRegenTask));

        // Render something to dirty the mips.
        mipmapSDC->clear(SkPMColor4f{.1f, .2f, .3f, .4f});
        auto mipmapRTCTask = sk_ref_sp(mipmapSDC->testingOnly_PeekLastOpsTask());
        REPORTER_ASSERT(reporter, mipmapRTCTask);

        // mipmapProxy's last render task should now just be the opsTask containing the clear.
        REPORTER_ASSERT(reporter,
                    mipmapRTCTask.get() == drawingManager->getLastRenderTask(mipmapProxy.get()));

        // Mipmaps don't get marked dirty until makeClosed().
        REPORTER_ASSERT(reporter, !mipmapProxy->mipmapsAreDirty());

        // Draw the dirty mipmap texture into a render target, but don't do mipmap filtering.
        sdc1 = draw_mipmap_into_new_render_target(dContext.get(), colorType, alphaType,
                                                  mipmapView, MipmapMode::kNone);

        // Mipmaps should have gotten marked dirty during makeClosed() when adding the dependency.
        // Since the last draw did not use mips, they will not have been regenerated and should
        // therefore still be dirty.
        REPORTER_ASSERT(reporter, mipmapProxy->mipmapsAreDirty());

        // Since mips weren't regenerated, the last render task shouldn't have changed.
        REPORTER_ASSERT(reporter,
                    mipmapRTCTask.get() == drawingManager->getLastRenderTask(mipmapProxy.get()));

        // Draw the stil-dirty mipmap texture into a second target with mipmap filtering.
        sdc2 = draw_mipmap_into_new_render_target(dContext.get(), colorType, alphaType,
                                                  std::move(mipmapView), MipmapMode::kLinear);
        sdc2Task = sk_ref_sp(sdc2->testingOnly_PeekLastOpsTask());

        // Make sure the mipmap texture now has a new last render task that regenerates the mips,
        // and that the mipmaps are now clean.
        auto mipRegenTask2 = drawingManager->getLastRenderTask(mipmapProxy.get());
        REPORTER_ASSERT(reporter, mipRegenTask2);
        REPORTER_ASSERT(reporter, mipmapRTCTask.get() != mipRegenTask2);
        SkASSERT(!mipmapProxy->mipmapsAreDirty());

        // Mip regen tasks don't get added as dependencies until makeClosed().
        dContext->flushAndSubmit();
        REPORTER_ASSERT(reporter, sdc2Task->dependsOn(mipRegenTask2));
    }
}
