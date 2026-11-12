/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkColorMatrix.h"
#include "include/private/chromium/SkConvertPixelsWithColorMatrix.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "tests/Test.h"

#include <cmath>

DEF_TEST(ConvertPixels_in_place, r) {
    static constexpr SkISize kTestSize = { 256, 256 };
    static constexpr SkColorType gTestCTs[] = {
        kAlpha_8_SkColorType,
        kRGB_565_SkColorType,
        kARGB_4444_SkColorType,
        kRGBA_8888_SkColorType,
        kBGRA_8888_SkColorType,
        kRGBA_1010102_SkColorType,
        kBGRA_1010102_SkColorType,
        kGray_8_SkColorType,
        kRGBA_F16Norm_SkColorType,
        kRGBA_F16_SkColorType,
        kRGBA_F32_SkColorType,
    };

    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kTestSize));
    surface->getCanvas()->drawColor(SK_ColorGREEN);
    auto image = surface->makeImageSnapshot();

    for (const auto& srcCT :gTestCTs) {
        const auto srcInfo = SkImageInfo::Make(kTestSize, srcCT, kPremul_SkAlphaType);
        SkAutoPixmapStorage pm;
        pm.alloc(srcInfo);
        REPORTER_ASSERT(r, image->readPixels(nullptr, pm, 0, 0));

        for (const auto& dstCT : gTestCTs) {
            const auto dstInfo = SkImageInfo::Make(kTestSize, dstCT, kPremul_SkAlphaType);
            // Expected to succeed iff bpp matches.
            const bool should_succeed = srcInfo.bytesPerPixel() == dstInfo.bytesPerPixel();
            REPORTER_ASSERT(r, SkConvertPixels(dstInfo, pm.writable_addr(), dstInfo.minRowBytes(),
                                               srcInfo, pm.addr()         , srcInfo.minRowBytes())
                                    == should_succeed);
        }
    }
}

DEF_TEST(ConvertPixelsWithColorMatrix, r) {
    auto nearly_equal = [](float a, float b, float tol = 1e-4f) {
        return std::fabs(a - b) <= tol;
    };

    // 1. Test applying srcMatrix and dstMatrix to unpremul pixels when src and dst are premul.
    {
        const auto infoPremul =
                SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType);
        const auto infoUnpremul =
                SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType);

        // Source pixel has unpremul (0.2, 0.4, 0.6, 0.5) -> premul (0.1, 0.2, 0.3, 0.5).
        SkColor4f srcPixel = {0.1f, 0.2f, 0.3f, 0.5f};
        SkColor4f dstPixel = {0.f, 0.f, 0.f, 0.f};
        SkPixmap srcPm(infoPremul, &srcPixel, sizeof(SkColor4f));
        SkPixmap dstPm(infoPremul, &dstPixel, sizeof(SkColor4f));

        // Matrix adds (0.1, 0.2, 0.3, 0.0) to unpremul channels.
        // If applied to unpremul (0.2, 0.4, 0.6, 0.5), result is unpremul (0.3, 0.6, 0.9, 0.5),
        // which premultiplies to (0.15, 0.30, 0.45, 0.5).
        SkColorMatrix translate;
        translate.postTranslate(0.1f, 0.2f, 0.3f, 0.0f);

        REPORTER_ASSERT(r, SkConvertPixelsWithColorMatrix(srcPm, &translate, dstPm, nullptr));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, 0.15f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, 0.30f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.45f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 0.50f));

        // Same with dstMatrix only.
        dstPixel = {0.f, 0.f, 0.f, 0.f};
        REPORTER_ASSERT(r, SkConvertPixelsWithColorMatrix(srcPm, nullptr, dstPm, &translate));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, 0.15f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, 0.30f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.45f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 0.50f));

        // Output to unpremul dst should not premultiply.
        SkPixmap dstUnpremulPm(infoUnpremul, &dstPixel, sizeof(SkColor4f));
        REPORTER_ASSERT(r,
                        SkConvertPixelsWithColorMatrix(srcPm, &translate, dstUnpremulPm, nullptr));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, 0.30f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, 0.60f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.90f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 0.50f));
    }

    // 2. Test order of operations: unpremul -> srcMatrix -> CS xform -> dstMatrix -> premul,
    // and verify unclamped intermediate/output values.
    {
        auto srcCS = SkColorSpace::MakeSRGBLinear();
        auto dstCS = SkColorSpace::MakeRGB({2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                                           SkNamedGamut::kSRGB);
        const auto srcInfo =
                SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType, srcCS);
        const auto dstInfo =
                SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType, dstCS);

        // Start with premul (0.25, 0.0, 0.0, 0.5) -> unpremul (0.5, 0.0, 0.0, 0.5).
        SkColor4f srcPixel = {0.25f, 0.0f, 0.0f, 0.5f};
        SkColor4f dstPixel = {0.f, 0.f, 0.f, 0.f};
        SkPixmap srcPm(srcInfo, &srcPixel, sizeof(SkColor4f));
        SkPixmap dstPm(dstInfo, &dstPixel, sizeof(SkColor4f));

        // srcMatrix adds 0.5 to R -> unpremul linear R becomes 1.0, G becomes 4.0 (unclamped > 1).
        SkColorMatrix srcMatrix;
        srcMatrix.postTranslate(0.5f, 4.0f, 0.0f, 0.0f);

        // CS xform applies invTF (gamma 1/2 = sqrt) -> unpremul dst R = 1.0, G = sqrt(4.0) = 2.0.
        // dstMatrix scales R by 2.0 and G by 0.25 -> unpremul dst R = 2.0, G = 0.5.
        SkColorMatrix dstMatrix;
        dstMatrix.setScale(2.0f, 0.25f, 1.0f, 1.0f);

        // Finally premul by A = 0.5 -> premul dst R = 1.0, G = 0.25, B = 0.0, A = 0.5.
        REPORTER_ASSERT(r,
                        SkConvertPixelsWithColorMatrix(srcPm, &srcMatrix, dstPm, &dstMatrix));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, 1.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, 0.25f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 0.5f));

        // Also test srcMatrix only with CS xform (ensuring no double unpremul/premul):
        // unpremul (0.5, 0, 0, 0.5) -> srcMatrix (1.0, 4.0, 0, 0.5) -> invTF (1.0, 2.0, 0, 0.5)
        // -> premul (0.5, 1.0, 0, 0.5).
        dstPixel = {0.f, 0.f, 0.f, 0.f};
        REPORTER_ASSERT(r, SkConvertPixelsWithColorMatrix(srcPm, &srcMatrix, dstPm, nullptr));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, 0.5f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, 1.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 0.5f));

        // And dstMatrix only with CS xform (ensuring no double unpremul/premul):
        // unpremul (0.5, 0, 0, 0.5) -> invTF (sqrt(0.5), 0, 0, 0.5)
        // -> dstMatrix (2 * sqrt(0.5), 0, 0, 0.5) -> premul (sqrt(0.5), 0, 0, 0.5).
        dstPixel = {0.f, 0.f, 0.f, 0.f};
        REPORTER_ASSERT(r, SkConvertPixelsWithColorMatrix(srcPm, nullptr, dstPm, &dstMatrix));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, std::sqrt(0.5f)));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, 0.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 0.5f));
    }

    // 3. Test that alpha is clamped to [0, 1] after each matrix while RGB remains unclamped.
    {
        const auto infoPremul =
                SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kPremul_SkAlphaType);
        SkColor4f srcPixel = {0.5f, 0.5f, 0.5f, 1.0f};
        SkColor4f dstPixel = {0.f, 0.f, 0.f, 0.f};
        SkPixmap srcPm(infoPremul, &srcPixel, sizeof(SkColor4f));
        SkPixmap dstPm(infoPremul, &dstPixel, sizeof(SkColor4f));

        // Push RGB above 1 / below 0, and push alpha above 1.
        SkColorMatrix overAlpha;
        overAlpha.postTranslate(1.5f, -1.0f, 0.0f, 2.0f);
        REPORTER_ASSERT(r, SkConvertPixelsWithColorMatrix(srcPm, &overAlpha, dstPm, nullptr));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, 2.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, -0.5f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.5f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 1.0f));

        // Push alpha below 0 in dstMatrix; premul with clamped alpha (0) should zero RGB.
        SkColorMatrix underAlpha;
        underAlpha.postTranslate(1.5f, -1.0f, 0.0f, -2.0f);
        REPORTER_ASSERT(r, SkConvertPixelsWithColorMatrix(srcPm, nullptr, dstPm, &underAlpha));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fR, 0.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fG, 0.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fB, 0.0f));
        REPORTER_ASSERT(r, nearly_equal(dstPixel.fA, 0.0f));
    }

    // 3. Test validation failure cases.
    {
        SkColor4f pixel = {0.f, 0.f, 0.f, 1.f};
        SkPixmap validPm(SkImageInfo::Make(2, 2, kRGBA_F32_SkColorType, kPremul_SkAlphaType),
                         &pixel,
                         2 * sizeof(SkColor4f));
        SkPixmap diffDimPm(SkImageInfo::Make(1, 2, kRGBA_F32_SkColorType, kPremul_SkAlphaType),
                           &pixel,
                           sizeof(SkColor4f));
        SkPixmap nullPm(SkImageInfo::Make(2, 2, kRGBA_F32_SkColorType, kPremul_SkAlphaType),
                        nullptr,
                        2 * sizeof(SkColor4f));
        SkColorMatrix m;
        REPORTER_ASSERT(r, !SkConvertPixelsWithColorMatrix(validPm, &m, diffDimPm, nullptr));
        REPORTER_ASSERT(r, !SkConvertPixelsWithColorMatrix(nullPm, &m, validPm, nullptr));
        REPORTER_ASSERT(r, !SkConvertPixelsWithColorMatrix(validPm, &m, nullPm, nullptr));
    }
}
