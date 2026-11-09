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
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSurface.h"
#include "include/private/SkTo.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkMask.h"

#include <cstdint>
#include <memory>
#include <vector>

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

namespace {
struct RecordedAntiSpan {
    int x;
    int y;
    int width;
    SkAlpha alpha;
};

class RecordingBlitter final : public SkBlitter {
public:
    void blitH(int x, int y, int width) override { fSpans.push_back({x, y, width, 0xFF}); }

    void blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) override {
        int curX = x;
        for (;;) {
            int count = runs[0];
            if (count <= 0) {
                if (count < 0) {
                    fSawNegativeRun = true;
                }
                break;
            }
            if (aa[0] != 0) {
                fSpans.push_back({curX, y, count, aa[0]});
            }
            curX += count;
            runs += count;
            aa += count;
        }
    }

    std::vector<RecordedAntiSpan> fSpans;
    bool fSawNegativeRun = false;
};
}  // namespace

DEF_TEST(ClipBlitter_BlitAntiH_LargeOffsetsAndGaps, r) {
    auto seedTwoRuns = [](std::vector<int16_t>* runs,
                          std::vector<SkAlpha>* aa,
                          int run0,
                          int run1,
                          SkAlpha alpha) {
        int total = run0 + run1;
        runs->assign(total + 1, 0);
        aa->assign(total, 0);
        (*runs)[0] = SkToS16(run0);
        (*aa)[0] = alpha;
        (*runs)[run0] = SkToS16(run1);
        (*aa)[run0] = alpha;
        (*runs)[total] = 0;
    };

    // 1. Span starting > 32767 px left of region bounds.
    {
        SkRegion rgn;
        rgn.op(SkIRect::MakeLTRB(10, 0, 15, 10), SkRegion::kUnion_Op);
        rgn.op(SkIRect::MakeLTRB(20, 0, 25, 10), SkRegion::kUnion_Op);

        RecordingBlitter rec;
        SkRgnClipBlitter clipper;
        clipper.init(&rec, &rgn);

        std::vector<int16_t> runs;
        std::vector<SkAlpha> aa;
        seedTwoRuns(&runs, &aa, 25000, 25000, 0x80);
        clipper.blitAntiH(-40000, 0, aa.data(), runs.data());

        REPORTER_ASSERT(r, !rec.fSawNegativeRun);
        REPORTER_ASSERT(r, rec.fSpans.size() == 2);
        if (rec.fSpans.size() == 2) {
            REPORTER_ASSERT(r,
                            rec.fSpans[0].x == 10 && rec.fSpans[0].width == 5 &&
                                    rec.fSpans[0].alpha == 0x80);
            REPORTER_ASSERT(r,
                            rec.fSpans[1].x == 20 && rec.fSpans[1].width == 5 &&
                                    rec.fSpans[1].alpha == 0x80);
        }
    }

    // 2. First active clip span on row y is > 32767 px right of bounds.fLeft.
    {
        SkRegion rgn;
        rgn.op(SkIRect::MakeLTRB(0, 10, 5, 20), SkRegion::kUnion_Op);
        rgn.op(SkIRect::MakeLTRB(40000, 0, 40006, 10), SkRegion::kUnion_Op);

        RecordingBlitter rec;
        SkRgnClipBlitter clipper;
        clipper.init(&rec, &rgn);

        std::vector<int16_t> runs;
        std::vector<SkAlpha> aa;
        seedTwoRuns(&runs, &aa, 25000, 15010, 0x99);
        clipper.blitAntiH(0, 0, aa.data(), runs.data());

        REPORTER_ASSERT(r, !rec.fSawNegativeRun);
        REPORTER_ASSERT(r, rec.fSpans.size() == 1);
        if (rec.fSpans.size() == 1) {
            REPORTER_ASSERT(r,
                            rec.fSpans[0].x == 40000 && rec.fSpans[0].width == 6 &&
                                    rec.fSpans[0].alpha == 0x99);
        }
    }

    // 3. Gap > 32767 px between two active clip spans on the same scanline.
    {
        SkRegion rgn;
        rgn.op(SkIRect::MakeLTRB(2, 0, 5, 10), SkRegion::kUnion_Op);
        rgn.op(SkIRect::MakeLTRB(40000, 0, 40004, 10), SkRegion::kUnion_Op);

        RecordingBlitter rec;
        SkRgnClipBlitter clipper;
        clipper.init(&rec, &rgn);

        std::vector<int16_t> runs;
        std::vector<SkAlpha> aa;
        seedTwoRuns(&runs, &aa, 25000, 15010, 0x77);
        clipper.blitAntiH(0, 0, aa.data(), runs.data());

        REPORTER_ASSERT(r, !rec.fSawNegativeRun);
        REPORTER_ASSERT(r, rec.fSpans.size() == 2);
        if (rec.fSpans.size() == 2) {
            REPORTER_ASSERT(r,
                            rec.fSpans[0].x == 2 && rec.fSpans[0].width == 3 &&
                                    rec.fSpans[0].alpha == 0x77);
            REPORTER_ASSERT(r,
                            rec.fSpans[1].x == 40000 && rec.fSpans[1].width == 4 &&
                                    rec.fSpans[1].alpha == 0x77);
        }
    }

    // 4. SkRectClipBlitter left/right trimming and extreme coordinates (INT32_MIN / INT32_MAX).
    {
        RecordingBlitter rec;
        SkRectClipBlitter rectClipper;
        rectClipper.init(&rec, SkIRect::MakeLTRB(5, 0, 25, 10));

        int16_t runs[31] = {};
        SkAlpha aa[30] = {};
        runs[0] = 10;
        aa[0] = 0x20;
        runs[10] = 10;
        aa[10] = 0x40;
        runs[20] = 10;
        aa[20] = 0x60;
        runs[30] = 0;

        // Out-of-Y and extreme X rejects should not emit spans or mutate `runs`.
        rectClipper.blitAntiH(INT32_MIN, 0, aa, runs);
        rectClipper.blitAntiH(INT32_MAX, 0, aa, runs);
        rectClipper.blitAntiH(0, INT32_MIN, aa, runs);
        REPORTER_ASSERT(r, rec.fSpans.empty());
        REPORTER_ASSERT(r, runs[0] == 10 && runs[10] == 10 && runs[20] == 10);

        rectClipper.blitAntiH(0, 0, aa, runs);
        REPORTER_ASSERT(r, rec.fSpans.size() == 3);
        if (rec.fSpans.size() == 3) {
            REPORTER_ASSERT(r,
                            rec.fSpans[0].x == 5 && rec.fSpans[0].width == 5 &&
                                    rec.fSpans[0].alpha == 0x20);
            REPORTER_ASSERT(r,
                            rec.fSpans[1].x == 10 && rec.fSpans[1].width == 10 &&
                                    rec.fSpans[1].alpha == 0x40);
            REPORTER_ASSERT(r,
                            rec.fSpans[2].x == 20 && rec.fSpans[2].width == 5 &&
                                    rec.fSpans[2].alpha == 0x60);
        }
    }
}
