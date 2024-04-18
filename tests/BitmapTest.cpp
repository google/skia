/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkRandom.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include <cstddef>
#include <initializer_list>

static void test_peekpixels(skiatest::Reporter* reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);

    SkPixmap pmap;
    SkBitmap bm;

    // empty should return false
    REPORTER_ASSERT(reporter, !bm.peekPixels(nullptr));
    REPORTER_ASSERT(reporter, !bm.peekPixels(&pmap));

    // no pixels should return false
    bm.setInfo(SkImageInfo::MakeN32Premul(10, 10));
    REPORTER_ASSERT(reporter, !bm.peekPixels(nullptr));
    REPORTER_ASSERT(reporter, !bm.peekPixels(&pmap));

    // real pixels should return true
    bm.allocPixels(info);
    REPORTER_ASSERT(reporter, bm.peekPixels(nullptr));
    REPORTER_ASSERT(reporter, bm.peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.info() == bm.info());
    REPORTER_ASSERT(reporter, pmap.addr() == bm.getPixels());
    REPORTER_ASSERT(reporter, pmap.rowBytes() == bm.rowBytes());
}

// https://code.google.com/p/chromium/issues/detail?id=446164
static void test_bigalloc(skiatest::Reporter* reporter) {
    const int width = 0x40000001;
    const int height = 0x00000096;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);

    SkBitmap bm;
    REPORTER_ASSERT(reporter, !bm.tryAllocPixels(info));

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, info.minRowBytes());
    REPORTER_ASSERT(reporter, !pr);
}

static void test_allocpixels(skiatest::Reporter* reporter) {
    const int width = 10;
    const int height = 10;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    const size_t explicitRowBytes = info.minRowBytes() + 24;

    SkBitmap bm;
    bm.setInfo(info);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());
    bm.allocPixels();
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());
    bm.reset();
    bm.allocPixels(info);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());

    bm.setInfo(info, explicitRowBytes);
    REPORTER_ASSERT(reporter, explicitRowBytes == bm.rowBytes());
    bm.allocPixels();
    REPORTER_ASSERT(reporter, explicitRowBytes == bm.rowBytes());
    bm.reset();
    bm.allocPixels(info, explicitRowBytes);
    REPORTER_ASSERT(reporter, explicitRowBytes == bm.rowBytes());

    bm.reset();
    bm.setInfo(info, 0);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());
    bm.reset();
    bm.allocPixels(info, 0);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());

    bm.reset();
    bool success = bm.setInfo(info, info.minRowBytes() - 1);   // invalid for 32bit
    REPORTER_ASSERT(reporter, !success);
    REPORTER_ASSERT(reporter, bm.isNull());

    for (SkColorType ct : {
        kAlpha_8_SkColorType,
        kRGB_565_SkColorType,
        kARGB_4444_SkColorType,
        kRGBA_8888_SkColorType,
        kBGRA_8888_SkColorType,
        kRGB_888x_SkColorType,
        kRGBA_1010102_SkColorType,
        kRGB_101010x_SkColorType,
        kGray_8_SkColorType,
        kRGBA_F16Norm_SkColorType,
        kRGBA_F16_SkColorType,
        kRGBA_F32_SkColorType,
        kR8G8_unorm_SkColorType,
        kA16_unorm_SkColorType,
        kR16G16_unorm_SkColorType,
        kA16_float_SkColorType,
        kR16G16_float_SkColorType,
        kR16G16B16A16_unorm_SkColorType,
    }) {
        SkImageInfo imageInfo = info.makeColorType(ct);
        for (int rowBytesPadding = 1; rowBytesPadding <= 17; rowBytesPadding++) {
            bm.reset();
            success = bm.setInfo(imageInfo, imageInfo.minRowBytes() + rowBytesPadding);
            if (rowBytesPadding % imageInfo.bytesPerPixel() == 0) {
                REPORTER_ASSERT(reporter, success);
                success = bm.tryAllocPixels();
                REPORTER_ASSERT(reporter, success);
            } else {
                // Not pixel aligned.
                REPORTER_ASSERT(reporter, !success);
                REPORTER_ASSERT(reporter, bm.isNull());
            }
        }
    }
}

static void test_bigwidth(skiatest::Reporter* reporter) {
    SkBitmap bm;
    int width = 1 << 29;    // *4 will be the high-bit of 32bit int

    SkImageInfo info = SkImageInfo::MakeA8(width, 1);
    REPORTER_ASSERT(reporter, bm.setInfo(info));
    REPORTER_ASSERT(reporter, bm.setInfo(info.makeColorType(kRGB_565_SkColorType)));

    // for a 4-byte config, this width will compute a rowbytes of 0x80000000,
    // which does not fit in a int32_t. setConfig should detect this, and fail.

    // TODO: perhaps skia can relax this, and only require that rowBytes fit
    //       in a uint32_t (or larger), but for now this is the constraint.

    REPORTER_ASSERT(reporter, !bm.setInfo(info.makeColorType(kN32_SkColorType)));
}

DEF_TEST(Bitmap, reporter) {
    // Zero-sized bitmaps are allowed
    for (int width = 0; width < 2; ++width) {
        for (int height = 0; height < 2; ++height) {
            SkBitmap bm;
            bool setConf = bm.setInfo(SkImageInfo::MakeN32Premul(width, height));
            REPORTER_ASSERT(reporter, setConf);
            if (setConf) {
                bm.allocPixels();
            }
            REPORTER_ASSERT(reporter, SkToBool(width & height) != bm.empty());
        }
    }

    test_bigwidth(reporter);
    test_allocpixels(reporter);
    test_bigalloc(reporter);
    test_peekpixels(reporter);
}

DEF_TEST(Bitmap_setColorSpace, r) {
    // Make a 1x1 bitmap holding 50% gray in default colorspace.
    SkBitmap source;
    source.allocN32Pixels(1,1);
    source.eraseColor(SkColorSetARGB(0xFF, 0x80, 0x80, 0x80));

    // Readback should use the normal sRGB colorspace.
    const SkImageInfo kReadbackInfo = SkImageInfo::Make(/*width=*/1,
                                                        /*height=*/1,
                                                        kN32_SkColorType,
                                                        kOpaque_SkAlphaType,
                                                        SkColorSpace::MakeSRGB());
    // Do readback and verify that the color is gray.
    uint8_t pixelData[4];
    REPORTER_ASSERT(r, source.readPixels(kReadbackInfo,
                                         pixelData,
                                         /*dstRowBytes=*/4,
                                         /*srcX=*/0,
                                         /*srcY=*/0));
    REPORTER_ASSERT(r, pixelData[0] == 0x80);
    REPORTER_ASSERT(r, pixelData[1] == 0x80);
    REPORTER_ASSERT(r, pixelData[2] == 0x80);

    // Also check the color with getColor4f, which does not honor colorspaces.
    uint32_t colorRGBA = source.getColor4f(0, 0).toBytes_RGBA();
    REPORTER_ASSERT(r, colorRGBA == 0xFF808080, "RGBA=%08X", colorRGBA);

    // Convert the SkBitmap's colorspace to linear.
    source.setColorSpace(SkColorSpace::MakeSRGBLinear());

    // Readback again and verify that the color is interpreted differently.
    REPORTER_ASSERT(r, source.readPixels(kReadbackInfo,
                                         pixelData,
                                         /*dstRowBytes=*/4,
                                         /*srcX=*/0,
                                         /*srcY=*/0));
    REPORTER_ASSERT(r, pixelData[0] == 0xBC, "R:%02X", pixelData[0]);
    REPORTER_ASSERT(r, pixelData[1] == 0xBC, "G:%02X", pixelData[1]);
    REPORTER_ASSERT(r, pixelData[2] == 0xBC, "B:%02X", pixelData[2]);

    // Since getColor4f does not honor colorspaces, this should still contain 50% gray.
    colorRGBA = source.getColor4f(0, 0).toBytes_RGBA();
    REPORTER_ASSERT(r, colorRGBA == 0xFF808080, "RGBA=%08X", colorRGBA);
}

/**
 *  This test checks that getColor works for both swizzles.
 */
DEF_TEST(Bitmap_getColor_Swizzle, r) {
    SkBitmap source;
    source.allocN32Pixels(1,1);
    source.eraseColor(SK_ColorRED);
    SkColorType colorTypes[] = {
        kRGBA_8888_SkColorType,
        kBGRA_8888_SkColorType,
    };
    for (SkColorType ct : colorTypes) {
        SkBitmap copy;
        if (!ToolUtils::copy_to(&copy, ct, source)) {
            ERRORF(r, "SkBitmap::copy failed %d", (int)ct);
            continue;
        }
        REPORTER_ASSERT(r, source.getColor(0, 0) == copy.getColor(0, 0));
    }
}

static void test_erasecolor_premul(skiatest::Reporter* reporter, SkColorType ct, SkColor input,
                                   SkColor expected) {
  SkBitmap bm;
  bm.allocPixels(SkImageInfo::Make(1, 1, ct, kPremul_SkAlphaType));
  bm.eraseColor(input);
  INFOF(reporter, "expected: %x actual: %x\n", expected, bm.getColor(0, 0));
  REPORTER_ASSERT(reporter, bm.getColor(0, 0) == expected);
}

/**
 *  This test checks that eraseColor premultiplies the color correctly.
 */
DEF_TEST(Bitmap_eraseColor_Premul, r) {
    SkColor color = 0x80FF0080;
    test_erasecolor_premul(r, kAlpha_8_SkColorType, color, 0x80000000);
    test_erasecolor_premul(r, kRGB_565_SkColorType, color, 0xFF840042);
    test_erasecolor_premul(r, kARGB_4444_SkColorType, color, 0x88FF0080);
    test_erasecolor_premul(r, kRGBA_8888_SkColorType, color, color);
    test_erasecolor_premul(r, kBGRA_8888_SkColorType, color, color);
}

// Test that SkBitmap::ComputeOpaque() is correct for various colortypes.
DEF_TEST(Bitmap_compute_is_opaque, r) {

    for (int i = 1; i <= kLastEnum_SkColorType; ++i) {
        SkColorType ct = (SkColorType) i;
        SkBitmap bm;
        SkAlphaType at = SkColorTypeIsAlwaysOpaque(ct) ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
        bm.allocPixels(SkImageInfo::Make(13, 17, ct, at));
        bm.eraseColor(SkColorSetARGB(255, 10, 20, 30));
        REPORTER_ASSERT(r, SkBitmap::ComputeIsOpaque(bm));

        bm.eraseColor(SkColorSetARGB(128, 255, 255, 255));
        bool isOpaque = SkBitmap::ComputeIsOpaque(bm);
        bool shouldBeOpaque = (at == kOpaque_SkAlphaType);
        REPORTER_ASSERT(r, isOpaque == shouldBeOpaque);
    }
}

// Test that erase+getColor round trips with RGBA_F16 pixels.
DEF_TEST(Bitmap_erase_f16_erase_getColor, r) {
    SkRandom random;
    SkPixmap pm;
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(1, 1, kRGBA_F16_SkColorType, kPremul_SkAlphaType));
    REPORTER_ASSERT(r, bm.peekPixels(&pm));
    for (unsigned i = 0; i < 0x100; ++i) {
        // Test all possible values of blue component.
        SkColor color1 = (SkColor)((random.nextU() & 0xFFFFFF00) | i);
        // Test all possible values of alpha component.
        SkColor color2 = (SkColor)((random.nextU() & 0x00FFFFFF) | (i << 24));
        for (SkColor color : {color1, color2}) {
            pm.erase(color);
            if (SkColorGetA(color) != 0) {
                REPORTER_ASSERT(r, color == pm.getColor(0, 0));
            } else {
                REPORTER_ASSERT(r, 0 == SkColorGetA(pm.getColor(0, 0)));
            }
        }
    }
}

// Verify that SkBitmap::erase erases in SRGB, regardless of the SkColorSpace of the
// SkBitmap.
DEF_TEST(Bitmap_erase_srgb, r) {
    SkBitmap bm;
    // Use a color spin from SRGB.
    bm.allocPixels(SkImageInfo::Make(1, 1, kN32_SkColorType, kPremul_SkAlphaType,
                                     SkColorSpace::MakeSRGB()->makeColorSpin()));
    // RED will be converted into the spun color space.
    bm.eraseColor(SK_ColorRED);
    // getColor doesn't take the color space into account, so the returned color
    // is different due to the color spin.
    REPORTER_ASSERT(r, bm.getColor(0, 0) == SK_ColorBLUE);
}

// Make sure that the bitmap remains valid when pixelref is removed.
DEF_TEST(Bitmap_clear_pixelref_keep_info, r) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeN32Premul(100,100));
    bm.setPixelRef(nullptr, 0, 0);
    SkDEBUGCODE(bm.validate();)
}

// At the time of writing, SkBitmap::erase() works when the color is zero for all formats,
// but some formats failed when the color is non-zero!
DEF_TEST(Bitmap_erase, r) {
    SkColorType colorTypes[] = {
        kRGB_565_SkColorType,
        kARGB_4444_SkColorType,
        kRGB_888x_SkColorType,
        kRGBA_8888_SkColorType,
        kBGRA_8888_SkColorType,
        kRGB_101010x_SkColorType,
        kRGBA_1010102_SkColorType,
    };

    for (SkColorType ct : colorTypes) {
        SkImageInfo info = SkImageInfo::Make(1,1, (SkColorType)ct, kPremul_SkAlphaType);

        SkBitmap bm;
        bm.allocPixels(info);

        bm.eraseColor(0x00000000);
        if (SkColorTypeIsAlwaysOpaque(ct)) {
            REPORTER_ASSERT(r, bm.getColor(0,0) == 0xff000000);
        } else {
            REPORTER_ASSERT(r, bm.getColor(0,0) == 0x00000000);
        }

        bm.eraseColor(0xaabbccdd);
        REPORTER_ASSERT(r, bm.getColor(0,0) != 0xff000000);
        REPORTER_ASSERT(r, bm.getColor(0,0) != 0x00000000);
    }
}

static void check_alphas(skiatest::Reporter* reporter, const SkBitmap& bm,
                         bool (*pred)(float expected, float actual), SkColorType ct) {
    SkASSERT(bm.width() == 16);
    SkASSERT(bm.height() == 16);

    int alpha = 0;
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            float expected = alpha / 255.0f;
            float actual = bm.getAlphaf(x, y);
            if (!pred(expected, actual)) {
                ERRORF(reporter, "%s: got %g, want %g\n",
                       ToolUtils::colortype_name(ct), actual, expected);
            }
            alpha += 1;
        }
    }
}

static bool unit_compare(float expected, float actual, float tol = 1.0f/(1<<12)) {
    SkASSERT(expected >= 0 && expected <= 1);
    SkASSERT(  actual >= 0 &&   actual <= 1);
    if (expected == 0 || expected == 1) {
        return actual == expected;
    } else {
        return SkScalarNearlyEqual(expected, actual, tol);
    }
}

static float unit_discretize(float value, float scale) {
    SkASSERT(value >= 0 && value <= 1);
    if (value == 1) {
        return 1;
    } else {
        return std::floor(value * scale + 0.5f) / scale;
    }
}

DEF_TEST(getalphaf, reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(16, 16);
    SkBitmap bm;
    bm.allocPixels(info);

    int alpha = 0;
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            *bm.getAddr32(x, y) = alpha++ << 24;
        }
    }

    auto nearly = [](float expected, float actual) -> bool {
        return unit_compare(expected, actual);
    };
    auto nearly4bit = [](float expected, float actual) -> bool {
        expected = unit_discretize(expected, 15);
        return unit_compare(expected, actual);
    };
    auto nearly2bit = [](float expected, float actual) -> bool {
        expected = unit_discretize(expected, 3);
        return unit_compare(expected, actual);
    };
    auto opaque = [](float expected, float actual) -> bool {
        return actual == 1.0f;
    };

    auto nearly_half = [](float expected, float actual) -> bool {
        return unit_compare(expected, actual, 1.0f/(1<<10));
    };

    const struct {
        SkColorType fColorType;
        bool (*fPred)(float, float);
    } recs[] = {
        { kRGB_565_SkColorType,            opaque },
        { kGray_8_SkColorType,             opaque },
        { kR8G8_unorm_SkColorType,         opaque },
        { kR16G16_unorm_SkColorType,       opaque },
        { kR16G16_float_SkColorType,       opaque },
        { kRGB_888x_SkColorType,           opaque },
        { kRGB_101010x_SkColorType,        opaque },

        { kAlpha_8_SkColorType,            nearly },
        { kA16_unorm_SkColorType,          nearly },
        { kA16_float_SkColorType,          nearly_half },
        { kRGBA_8888_SkColorType,          nearly },
        { kBGRA_8888_SkColorType,          nearly },
        { kR16G16B16A16_unorm_SkColorType, nearly },
        { kRGBA_F16_SkColorType,           nearly_half },
        { kRGBA_F32_SkColorType,           nearly },

        { kRGBA_1010102_SkColorType,       nearly2bit },

        { kARGB_4444_SkColorType,          nearly4bit },
    };

    for (const auto& rec : recs) {
        SkBitmap tmp;
        tmp.allocPixels(bm.info().makeColorType(rec.fColorType));
        if (bm.readPixels(tmp.pixmap())) {
            check_alphas(reporter, tmp, rec.fPred, rec.fColorType);
        } else {
            SkDebugf("can't readpixels\n");
        }
    }
}

/*  computeByteSize() is documented to return 0 if height is zero, but does not
 *  special-case width==0, so computeByteSize() can return non-zero for that
 *  (since it is defined to return (height-1)*rb + ...
 *
 *  Test that allocPixels() respects this, and allocates a buffer as large as
 *  computeByteSize()... even though the bitmap is logicallly empty.
 */
DEF_TEST(bitmap_zerowidth_crbug_1103827, reporter) {
    const size_t big_rb = 1 << 16;

    struct {
        int width, height;
        size_t rowbytes, expected_size;
    } rec[] = {
        { 2, 0,     big_rb,         0 },    // zero-height means zero-size
        { 0, 2,     big_rb,    big_rb },    // zero-width is computed normally
    };

    for (const auto& r : rec) {
        auto info = SkImageInfo::Make(r.width, r.height,
                                      kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        size_t size = info.computeByteSize(r.rowbytes);
        REPORTER_ASSERT(reporter, size == r.expected_size);

        SkBitmap bm;
        bm.setInfo(info, r.rowbytes);
        REPORTER_ASSERT(reporter, size == bm.computeByteSize());

        // Be sure we can actually write to that much memory. If the bitmap underallocated
        // the buffer, this should trash memory and crash (we hope).
        bm.allocPixels();
        sk_bzero(bm.getPixels(), size);
    }
}
