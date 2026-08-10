/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "tests/Test.h"

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
