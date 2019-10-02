/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

static void init_src(const SkBitmap& bitmap) {
    if (bitmap.getPixels()) {
        bitmap.eraseColor(SK_ColorWHITE);
    }
}

struct Pair {
    SkColorType fColorType;
    const char* fValid;
};

// Utility functions for copyPixelsTo()/copyPixelsFrom() tests.
// getPixel()
// setPixel()
// getSkConfigName()
// struct Coordinates
// reportCopyVerification()
// writeCoordPixels()

// Helper struct to contain pixel locations, while avoiding need for STL.
struct Coordinates {

    const int length;
    SkIPoint* const data;

    explicit Coordinates(int _length): length(_length)
                                     , data(new SkIPoint[length]) { }

    ~Coordinates(){
        delete [] data;
    }

    SkIPoint* operator[](int i) const {
        // Use with care, no bounds checking.
        return data + i;
    }
};

static const Pair gPairs[] = {
    { kUnknown_SkColorType,     "0000000"  },
    { kAlpha_8_SkColorType,     "0100000"  },
    { kRGB_565_SkColorType,     "0101011"  },
    { kARGB_4444_SkColorType,   "0101111"  },
    { kN32_SkColorType,         "0101111"  },
    { kRGBA_F16_SkColorType,    "0101011"  },
};

static const int W = 20;
static const int H = 33;

static void setup_src_bitmaps(SkBitmap* srcOpaque, SkBitmap* srcPremul,
                              SkColorType ct) {
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (kRGBA_F16_SkColorType == ct) {
        colorSpace = SkColorSpace::MakeSRGB();
    }

    srcOpaque->allocPixels(SkImageInfo::Make(W, H, ct, kOpaque_SkAlphaType, colorSpace));
    srcPremul->allocPixels(SkImageInfo::Make(W, H, ct, kPremul_SkAlphaType, colorSpace));
    init_src(*srcOpaque);
    init_src(*srcPremul);
}

DEF_TEST(BitmapCopy_extractSubset, reporter) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        SkBitmap srcOpaque, srcPremul;
        setup_src_bitmaps(&srcOpaque, &srcPremul, gPairs[i].fColorType);

        SkBitmap bitmap(srcOpaque);
        SkBitmap subset;
        SkIRect r;
        // Extract a subset which has the same width as the original. This
        // catches a bug where we cloned the genID incorrectly.
        r.setLTRB(0, 1, W, 3);
        bitmap.setIsVolatile(true);
        // Relies on old behavior of extractSubset failing if colortype is unknown
        if (kUnknown_SkColorType != bitmap.colorType() && bitmap.extractSubset(&subset, r)) {
            REPORTER_ASSERT(reporter, subset.width() == W);
            REPORTER_ASSERT(reporter, subset.height() == 2);
            REPORTER_ASSERT(reporter, subset.alphaType() == bitmap.alphaType());
            REPORTER_ASSERT(reporter, subset.isVolatile() == true);

            // Test copying an extracted subset.
            for (size_t j = 0; j < SK_ARRAY_COUNT(gPairs); j++) {
                SkBitmap copy;
                bool     success = ToolUtils::copy_to(&copy, gPairs[j].fColorType, subset);
                if (!success) {
                    // Skip checking that success matches fValid, which is redundant
                    // with the code below.
                    REPORTER_ASSERT(reporter, gPairs[i].fColorType != gPairs[j].fColorType);
                    continue;
                }

                // When performing a copy of an extracted subset, the gen id should
                // change.
                REPORTER_ASSERT(reporter, copy.getGenerationID() != subset.getGenerationID());

                REPORTER_ASSERT(reporter, copy.width() == W);
                REPORTER_ASSERT(reporter, copy.height() == 2);
            }
        }

        bitmap = srcPremul;
        bitmap.setIsVolatile(false);
        if (bitmap.extractSubset(&subset, r)) {
            REPORTER_ASSERT(reporter, subset.alphaType() == bitmap.alphaType());
            REPORTER_ASSERT(reporter, subset.isVolatile() == false);
        }
    }
}

#include "include/core/SkColorPriv.h"
#include "src/core/SkUtils.h"

/**
 *  Construct 4x4 pixels where we can look at a color and determine where it should be in the grid.
 *  alpha = 0xFF, blue = 0x80, red = x, green = y
 */
static void fill_4x4_pixels(SkPMColor colors[16]) {
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            colors[y*4+x] = SkPackARGB32(0xFF, x, y, 0x80);
        }
    }
}

static bool check_4x4_pixel(SkPMColor color, unsigned x, unsigned y) {
    SkASSERT(x < 4 && y < 4);
    return  0xFF == SkGetPackedA32(color) &&
            x    == SkGetPackedR32(color) &&
            y    == SkGetPackedG32(color) &&
            0x80 == SkGetPackedB32(color);
}

/**
 *  Fill with all zeros, which will never match any value from fill_4x4_pixels
 */
static void clear_4x4_pixels(SkPMColor colors[16]) {
    sk_memset32(colors, 0, 16);
}

// Much of readPixels is exercised by copyTo testing, since readPixels is the backend for that
// method. Here we explicitly test subset copies.
//
DEF_TEST(BitmapReadPixels, reporter) {
    const int W = 4;
    const int H = 4;
    const size_t rowBytes = W * sizeof(SkPMColor);
    const SkImageInfo srcInfo = SkImageInfo::MakeN32Premul(W, H);
    SkPMColor srcPixels[16];
    fill_4x4_pixels(srcPixels);
    SkBitmap srcBM;
    srcBM.installPixels(srcInfo, srcPixels, rowBytes);

    SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(W, H);
    SkPMColor dstPixels[16];

    const struct {
        bool     fExpectedSuccess;
        SkIPoint fRequestedSrcLoc;
        SkISize  fRequestedDstSize;
        // If fExpectedSuccess, check these, otherwise ignore
        SkIPoint fExpectedDstLoc;
        SkIRect  fExpectedSrcR;
    } gRec[] = {
        { true,  { 0, 0 }, { 4, 4 }, { 0, 0 }, { 0, 0, 4, 4 } },
        { true,  { 1, 1 }, { 2, 2 }, { 0, 0 }, { 1, 1, 3, 3 } },
        { true,  { 2, 2 }, { 4, 4 }, { 0, 0 }, { 2, 2, 4, 4 } },
        { true,  {-1,-1 }, { 2, 2 }, { 1, 1 }, { 0, 0, 1, 1 } },
        { false, {-1,-1 }, { 1, 1 }, { 0, 0 }, { 0, 0, 0, 0 } },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        clear_4x4_pixels(dstPixels);

        dstInfo = dstInfo.makeDimensions(gRec[i].fRequestedDstSize);
        bool success = srcBM.readPixels(dstInfo, dstPixels, rowBytes,
                                        gRec[i].fRequestedSrcLoc.x(), gRec[i].fRequestedSrcLoc.y());

        REPORTER_ASSERT(reporter, gRec[i].fExpectedSuccess == success);
        if (success) {
            const SkIRect srcR = gRec[i].fExpectedSrcR;
            const int dstX = gRec[i].fExpectedDstLoc.x();
            const int dstY = gRec[i].fExpectedDstLoc.y();
            // Walk the dst pixels, and check if we got what we expected
            for (int y = 0; y < H; ++y) {
                for (int x = 0; x < W; ++x) {
                    SkPMColor dstC = dstPixels[y*4+x];
                    // get into src coordinates
                    int sx = x - dstX + srcR.x();
                    int sy = y - dstY + srcR.y();
                    if (srcR.contains(sx, sy)) {
                        REPORTER_ASSERT(reporter, check_4x4_pixel(dstC, sx, sy));
                    } else {
                        REPORTER_ASSERT(reporter, 0 == dstC);
                    }
                }
            }
        }
    }
}
