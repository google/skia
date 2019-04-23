/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/image/SkImage_Base.h"
#include "tests/Test.h"

// Tests that MIP maps are created and invalidated as expected when drawing to and from GrTextures.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTextureMipMapInvalidationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->priv().caps()->mipMapSupport()) {
        return;
    }

    auto isMipped = [] (SkSurface* surf) {
        const GrTexture* texture = surf->makeImageSnapshot()->getTexture();
        return GrMipMapped::kYes == texture->texturePriv().mipMapped();
    };

    auto mipsAreDirty = [] (SkSurface* surf) {
        return surf->makeImageSnapshot()->getTexture()->texturePriv().mipMapsAreDirty();
    };

    auto info = SkImageInfo::MakeN32Premul(256, 256);
    for (auto allocateMips : {false, true}) {
        auto surf1 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info, 0,
                                                 kBottomLeft_GrSurfaceOrigin, nullptr,
                                                 allocateMips);
        auto surf2 = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, info);
        // Draw something just in case we ever had a solid color optimization
        surf1->getCanvas()->drawCircle(128, 128, 50, SkPaint());
        surf1->flush();

        // No mipmaps initially
        REPORTER_ASSERT(reporter, isMipped(surf1.get()) == allocateMips);

        // Painting with downscale and medium filter quality should result in mipmap creation
        // Flush the context rather than the canvas as flushing the canvas triggers MIP level
        // generation.
        SkPaint paint;
        paint.setFilterQuality(kMedium_SkFilterQuality);
        surf2->getCanvas()->scale(0.2f, 0.2f);
        surf2->getCanvas()->drawImage(surf1->makeImageSnapshot(), 0, 0, &paint);
        context->flush();
        REPORTER_ASSERT(reporter, isMipped(surf1.get()) == allocateMips);
        REPORTER_ASSERT(reporter, !allocateMips || !mipsAreDirty(surf1.get()));

        // Changing the contents of the surface should invalidate the mipmap, but not de-allocate
        surf1->getCanvas()->drawCircle(128, 128, 100, SkPaint());
        context->flush();
        REPORTER_ASSERT(reporter, isMipped(surf1.get()) == allocateMips);
        REPORTER_ASSERT(reporter, mipsAreDirty(surf1.get()));
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReimportImageTextureWithMipLevels, reporter, ctxInfo) {
    auto* ctx = ctxInfo.grContext();
    if (!ctx->priv().caps()->mipMapSupport()) {
        return;
    }
    static constexpr auto kCreateWithMipMaps = true;
    auto surf = SkSurface::MakeRenderTarget(
            ctx, SkBudgeted::kYes,
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType), 1,
            kTopLeft_GrSurfaceOrigin, nullptr, kCreateWithMipMaps);
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
    SkImage::BackendTextureReleaseProc texRelease;
    if (!SkImage::MakeBackendTextureFromSkImage(ctx, std::move(img), &btex, &texRelease)) {
        // Not all backends support stealing textures yet.
        // ERRORF(reporter, "Could not turn image into texture");
        return;
    }
    REPORTER_ASSERT(reporter, btex.hasMipMaps());
    // Reimport the texture as an image and perform a downsampling draw with medium quality which
    // should use the upper MIP levels.
    img = SkImage::MakeFromTexture(ctx, btex, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                   kPremul_SkAlphaType, nullptr);
    const auto singlePixelInfo =
            SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    surf = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kYes, singlePixelInfo, 1,
                                       kTopLeft_GrSurfaceOrigin, nullptr);
    SkPaint paint;
    paint.setFilterQuality(kMedium_SkFilterQuality);
    surf->getCanvas()->drawImageRect(img, SkRect::MakeWH(1, 1), &paint);
    uint32_t pixel;
    surf->readPixels(singlePixelInfo, &pixel, sizeof(uint32_t), 0, 0);
    REPORTER_ASSERT(reporter, pixel == SkPreMultiplyColor(SK_ColorDKGRAY));
    img.reset();
    texRelease(btex);
}
