/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ProxyUtils.h"

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <utility>

struct GrContextOptions;

// Tests that MIP maps are created and invalidated as expected when drawing to and from GrTextures.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrTextureMipMapInvalidationTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    if (!context->priv().caps()->mipmapSupport()) {
        return;
    }

    auto isMipped = [reporter](SkSurface* surf) {
        sk_sp<SkImage> image = surf->makeImageSnapshot();
        GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(image.get(),
                                                                  surf->recordingContext());
        bool proxyIsMipmapped = proxy->mipmapped() == skgpu::Mipmapped::kYes;
        REPORTER_ASSERT(reporter, proxyIsMipmapped == image->hasMipmaps());
        return image->hasMipmaps();
    };

    auto mipsAreDirty = [](SkSurface* surf) {
        sk_sp<SkImage> image = surf->makeImageSnapshot();
        GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(image.get(),
                                                                  surf->recordingContext());
        return proxy->peekTexture()->mipmapsAreDirty();
    };

    auto info = SkImageInfo::MakeN32Premul(256, 256);
    for (auto allocateMips : {false, true}) {
        auto surf1 = SkSurfaces::RenderTarget(context,
                                              skgpu::Budgeted::kYes,
                                              info,
                                              0,
                                              kBottomLeft_GrSurfaceOrigin,
                                              nullptr,
                                              allocateMips);
        auto surf2 = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info);
        // Draw something just in case we ever had a solid color optimization
        surf1->getCanvas()->drawCircle(128, 128, 50, SkPaint());
        context->flushAndSubmit(surf1.get(), GrSyncCpu::kNo);

        // No mipmaps initially
        REPORTER_ASSERT(reporter, isMipped(surf1.get()) == allocateMips);

        // Painting with downscale and medium filter quality should result in mipmap creation
        // Flush the context rather than the canvas as flushing the canvas triggers MIP level
        // generation.
        SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kLinear);

        surf2->getCanvas()->scale(0.2f, 0.2f);
        surf2->getCanvas()->drawImage(surf1->makeImageSnapshot(), 0, 0, sampling);
        context->flushAndSubmit();
        REPORTER_ASSERT(reporter, isMipped(surf1.get()) == allocateMips);
        REPORTER_ASSERT(reporter, !allocateMips || !mipsAreDirty(surf1.get()));

        // Changing the contents of the surface should invalidate the mipmap, but not de-allocate
        surf1->getCanvas()->drawCircle(128, 128, 100, SkPaint());
        context->flushAndSubmit();
        REPORTER_ASSERT(reporter, isMipped(surf1.get()) == allocateMips);
        REPORTER_ASSERT(reporter, mipsAreDirty(surf1.get()));
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ReimportImageTextureWithMipLevels,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    if (!dContext->priv().caps()->mipmapSupport()) {
        return;
    }
    static constexpr auto kCreateWithMipMaps = true;
    auto surf = SkSurfaces::RenderTarget(
            dContext,
            skgpu::Budgeted::kYes,
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
            1,
            kTopLeft_GrSurfaceOrigin,
            nullptr,
            kCreateWithMipMaps);
    if (!surf) {
        return;
    }
    surf->getCanvas()->drawColor(SK_ColorDKGRAY);
    auto img = surf->makeImageSnapshot();
    if (!img) {
        return;
    }
    surf.reset();
    GrBackendTexture btex;
    SkImages::BackendTextureReleaseProc texRelease;
    if (!SkImages::MakeBackendTextureFromImage(dContext, std::move(img), &btex, &texRelease)) {
        // Not all backends support stealing textures yet.
        // ERRORF(reporter, "Could not turn image into texture");
        return;
    }
    REPORTER_ASSERT(reporter, btex.hasMipmaps());
    // Reimport the texture as an image and perform a downsampling draw with medium quality which
    // should use the upper MIP levels.
    img = SkImages::BorrowTextureFrom(dContext,
                                      btex,
                                      kTopLeft_GrSurfaceOrigin,
                                      kRGBA_8888_SkColorType,
                                      kPremul_SkAlphaType,
                                      nullptr);
    const auto singlePixelInfo =
            SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    surf = SkSurfaces::RenderTarget(
            dContext, skgpu::Budgeted::kYes, singlePixelInfo, 1, kTopLeft_GrSurfaceOrigin, nullptr);

    surf->getCanvas()->drawImageRect(img, SkRect::MakeWH(1, 1),
                                     SkSamplingOptions(SkFilterMode::kLinear,
                                                       SkMipmapMode::kLinear));
    uint32_t pixel;
    surf->readPixels(singlePixelInfo, &pixel, sizeof(uint32_t), 0, 0);
    REPORTER_ASSERT(reporter, pixel == SkPreMultiplyColor(SK_ColorDKGRAY));
    img.reset();
    texRelease(btex);
}
