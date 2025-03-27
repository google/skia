/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkMask.h"

#include <memory>

static bool all_pixels_same_color(uint32_t* buffer, size_t len) {
    for (size_t i = 1; i < len; ++i) {
        if (buffer[0] != buffer[i]) {
            return false;
        }
    }
    return true;
}

using BlitterFactory = std::unique_ptr<SkBlitter> (*)(const SkPixmap&, const SkPaint&);

static void compare_mask_and_antiH(skiatest::Reporter* reporter,
                                   SkColor backgroundColor,
                                   SkColor paintColor,
                                   BlitterFactory makeBlitter) {
    // 19 is big enough to exercise any multi-lane code (e.g. SIMD/NEON)
    // and have some remainder to exercise single-pixel code.
    constexpr size_t kPixelsToBlit = 19;
    static_assert(kPixelsToBlit % 4 != 0);
    static_assert(kPixelsToBlit % 8 != 0);
    static_assert(kPixelsToBlit % 16 != 0);

    // Space for a kPixelsToBlit by 1 pixel image of 8888 color
    SkColor buffer1[kPixelsToBlit * 1];
    SkColor buffer2[kPixelsToBlit * 1];
    auto ii = SkImageInfo::Make(
            {kPixelsToBlit, 1},
            SkColorInfo(kN32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()));

    SkPixmap device1(ii, buffer1, ii.minRowBytes());
    auto surface1 = SkSurfaces::WrapPixels(device1);
    SkASSERT(surface1);

    SkPixmap device2(ii, buffer2, ii.minRowBytes());
    auto surface2 = SkSurfaces::WrapPixels(device2);
    SkASSERT(surface2);

    SkPaint paint;
    paint.setColor(paintColor);

    auto blitter1 = makeBlitter(device1, paint);
    SkASSERT(blitter1);
    auto blitter2 = makeBlitter(device2, paint);
    SkASSERT(blitter2);

    SkAlpha antiAlias[1];
    // The blitAntiH needs a buffer where the first value is the number of pixels
    // to blit and then at least that many 0s so we don't read off the end of the
    // buffer to figure out we need to stop.
    int16_t runs[kPixelsToBlit+1] = {kPixelsToBlit};

    uint8_t maskImage[kPixelsToBlit];
    auto maskBounds = SkIRect::MakeXYWH(0, 0, kPixelsToBlit, 1);
    for (int alpha = 0; alpha <= 255; ++alpha) {
        antiAlias[0] = static_cast<SkAlpha>(alpha);
        std::fill_n(maskImage, kPixelsToBlit, static_cast<SkAlpha>(alpha));

        SkMask mask(maskImage, maskBounds, kPixelsToBlit, SkMask::kA8_Format);

        surface1->getCanvas()->clear(backgroundColor);
        blitter1->blitAntiH(0, 0, antiAlias, runs);

        surface2->getCanvas()->clear(backgroundColor);
        blitter2->blitMask(mask, mask.fBounds);

        if (!all_pixels_same_color(buffer1, std::size(buffer1))) {
            REPORT_FAILURE(reporter,
                           "blitAntiH was not the same for all pixels",
                           SkStringPrintf("background=%08x, paint=%08x, alpha=%d",
                                          unsigned(backgroundColor),
                                          unsigned(paintColor),
                                          alpha));
            return;
        }
        if (!all_pixels_same_color(buffer2, std::size(buffer2))) {
            REPORT_FAILURE(reporter,
                           "blitMask was not the same for all pixels",
                           SkStringPrintf("background=%08x, paint=%08x, alpha=%d",
                                          unsigned(backgroundColor),
                                          unsigned(paintColor),
                                          alpha));
            return;
        }

        SkColor antiHColor = buffer1[0];
        SkColor maskColor = buffer2[0];

        REPORTER_ASSERT(reporter,
                        antiHColor == maskColor,
                        "background=%08x, paint=%08x, alpha=%d, "
                        "blitAntiH=%08x, blitMask=%08x, "
                        "diff=%02x %02x %02x %02x",
                        unsigned(backgroundColor), unsigned(paintColor), alpha,
                        unsigned(antiHColor), unsigned(maskColor),
                        unsigned(abs((int)(SkColorGetA(antiHColor) - SkColorGetA(maskColor)))),
                        unsigned(abs((int)(SkColorGetR(antiHColor) - SkColorGetR(maskColor)))),
                        unsigned(abs((int)(SkColorGetG(antiHColor) - SkColorGetG(maskColor)))),
                        unsigned(abs((int)(SkColorGetB(antiHColor) - SkColorGetB(maskColor)))));
    }
}

DEF_TEST(SkARGB32OpaqueBlitter_MaskAndAntiHDrawTheSame, r) {
    SkColor backgroundColors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            // arbitrary opaque color with uneven channels
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
    };
    SkColor paintColors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
    };

    for (SkColor backgroundColor : backgroundColors) {
        for (SkColor paintColor : paintColors) {
            compare_mask_and_antiH(r,
                                   backgroundColor,
                                   paintColor,
                                   [](const SkPixmap& device, const SkPaint& paint) -> std::unique_ptr<SkBlitter> {
                                       return std::make_unique<SkARGB32_Opaque_Blitter>(device, paint);
                                   });
        }
    }
}

DEF_TEST(SkARGB32BlackBlitter_MaskAndAntiHDrawTheSame, r) {
    SkColor backgroundColors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            // arbitrary opaque color with uneven channels
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
    };

    for (SkColor backgroundColor : backgroundColors) {
        compare_mask_and_antiH(r,
                               backgroundColor,
                               SK_ColorBLACK,
                               [](const SkPixmap& device, const SkPaint& paint) -> std::unique_ptr<SkBlitter> {
                                   return std::make_unique<SkARGB32_Black_Blitter>(device, paint);
                               });
    }
}

DEF_TEST(SkARGB32Blitter_MaskAndAntiHDrawTheSame, r) {
    SkColor backgroundColors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            // arbitrary opaque color with uneven channels
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
    };
    SkColor paintColors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
    };
    SkAlpha alphaValues[] = {
            0, 10, 100, 200, 245 /*SkARGB32_Opaque_Blitter is used when alpha is 255*/
    };

    for (SkColor backgroundColor : backgroundColors) {
        for (SkColor paintColor : paintColors) {
            for (SkAlpha alpha : alphaValues) {
                SkColor newColor = SkColorSetA(paintColor, alpha);
                compare_mask_and_antiH(r,
                                       backgroundColor,
                                       newColor,
                                       [](const SkPixmap& device, const SkPaint& paint) -> std::unique_ptr<SkBlitter> {
                                           return std::make_unique<SkARGB32_Blitter>(device, paint);
                                       });
            }
        }
    }
}
