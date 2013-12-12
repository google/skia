
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkRect.h"

static const char* boolStr(bool value) {
    return value ? "true" : "false";
}

// these are in the same order as the SkBitmap::Config enum
static const char* gConfigName[] = {
    "None", "A8", "Index8", "565", "4444", "8888"
};

static void report_opaqueness(skiatest::Reporter* reporter, const SkBitmap& src,
                              const SkBitmap& dst) {
    SkString str;
    str.printf("src %s opaque:%d, dst %s opaque:%d",
               gConfigName[src.config()], src.isOpaque(),
               gConfigName[dst.config()], dst.isOpaque());
    reporter->reportFailed(str);
}

static bool canHaveAlpha(SkBitmap::Config config) {
    return config != SkBitmap::kRGB_565_Config;
}

// copyTo() should preserve isOpaque when it makes sense
static void test_isOpaque(skiatest::Reporter* reporter,
                          const SkBitmap& srcOpaque, const SkBitmap& srcPremul,
                          SkBitmap::Config dstConfig) {
    SkBitmap dst;

    if (canHaveAlpha(srcPremul.config()) && canHaveAlpha(dstConfig)) {
        REPORTER_ASSERT(reporter, srcPremul.copyTo(&dst, dstConfig));
        REPORTER_ASSERT(reporter, dst.config() == dstConfig);
        if (srcPremul.isOpaque() != dst.isOpaque()) {
            report_opaqueness(reporter, srcPremul, dst);
        }
    }

    REPORTER_ASSERT(reporter, srcOpaque.copyTo(&dst, dstConfig));
    REPORTER_ASSERT(reporter, dst.config() == dstConfig);
    if (srcOpaque.isOpaque() != dst.isOpaque()) {
        report_opaqueness(reporter, srcOpaque, dst);
    }
}

static void init_src(const SkBitmap& bitmap) {
    SkAutoLockPixels lock(bitmap);
    if (bitmap.getPixels()) {
        if (bitmap.getColorTable()) {
            sk_bzero(bitmap.getPixels(), bitmap.getSize());
        } else {
            bitmap.eraseColor(SK_ColorWHITE);
        }
    }
}

static SkColorTable* init_ctable(SkAlphaType alphaType) {
    static const SkColor colors[] = {
        SK_ColorBLACK, SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE
    };
    return new SkColorTable(colors, SK_ARRAY_COUNT(colors), alphaType);
}

struct Pair {
    SkBitmap::Config    fConfig;
    const char*         fValid;
};

// Utility functions for copyPixelsTo()/copyPixelsFrom() tests.
// getPixel()
// setPixel()
// getSkConfigName()
// struct Coordinates
// reportCopyVerification()
// writeCoordPixels()

// Utility function to read the value of a given pixel in bm. All
// values converted to uint32_t for simplification of comparisons.
static uint32_t getPixel(int x, int y, const SkBitmap& bm) {
    uint32_t val = 0;
    uint16_t val16;
    uint8_t val8;
    SkAutoLockPixels lock(bm);
    const void* rawAddr = bm.getAddr(x,y);

    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            memcpy(&val, rawAddr, sizeof(uint32_t));
            break;
        case SkBitmap::kARGB_4444_Config:
        case SkBitmap::kRGB_565_Config:
            memcpy(&val16, rawAddr, sizeof(uint16_t));
            val = val16;
            break;
        case SkBitmap::kA8_Config:
        case SkBitmap::kIndex8_Config:
            memcpy(&val8, rawAddr, sizeof(uint8_t));
            val = val8;
            break;
        default:
            break;
    }
    return val;
}

// Utility function to set value of any pixel in bm.
// bm.getConfig() specifies what format 'val' must be
// converted to, but at present uint32_t can handle all formats.
static void setPixel(int x, int y, uint32_t val, SkBitmap& bm) {
    uint16_t val16;
    uint8_t val8;
    SkAutoLockPixels lock(bm);
    void* rawAddr = bm.getAddr(x,y);

    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            memcpy(rawAddr, &val, sizeof(uint32_t));
            break;
        case SkBitmap::kARGB_4444_Config:
        case SkBitmap::kRGB_565_Config:
            val16 = val & 0xFFFF;
            memcpy(rawAddr, &val16, sizeof(uint16_t));
            break;
        case SkBitmap::kA8_Config:
        case SkBitmap::kIndex8_Config:
            val8 = val & 0xFF;
            memcpy(rawAddr, &val8, sizeof(uint8_t));
            break;
        default:
            // Ignore.
            break;
    }
}

// Utility to return string containing name of each format, to
// simplify diagnostic output.
static const char* getSkConfigName(const SkBitmap& bm) {
    switch (bm.config()) {
        case SkBitmap::kNo_Config: return "SkBitmap::kNo_Config";
        case SkBitmap::kA8_Config: return "SkBitmap::kA8_Config";
        case SkBitmap::kIndex8_Config: return "SkBitmap::kIndex8_Config";
        case SkBitmap::kRGB_565_Config: return "SkBitmap::kRGB_565_Config";
        case SkBitmap::kARGB_4444_Config: return "SkBitmap::kARGB_4444_Config";
        case SkBitmap::kARGB_8888_Config: return "SkBitmap::kARGB_8888_Config";
        default: return "Unknown SkBitmap configuration.";
    }
}

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

// A function to verify that two bitmaps contain the same pixel values
// at all coordinates indicated by coords. Simplifies verification of
// copied bitmaps.
static void reportCopyVerification(const SkBitmap& bm1, const SkBitmap& bm2,
                            Coordinates& coords,
                            const char* msg,
                            skiatest::Reporter* reporter){
    bool success = true;

    // Confirm all pixels in the list match.
    for (int i = 0; i < coords.length; ++i) {
        success = success &&
                  (getPixel(coords[i]->fX, coords[i]->fY, bm1) ==
                   getPixel(coords[i]->fX, coords[i]->fY, bm2));
    }

    if (!success) {
        SkString str;
        str.printf("%s [config = %s]",
                   msg, getSkConfigName(bm1));
        reporter->reportFailed(str);
    }
}

// Writes unique pixel values at locations specified by coords.
static void writeCoordPixels(SkBitmap& bm, const Coordinates& coords) {
    for (int i = 0; i < coords.length; ++i)
        setPixel(coords[i]->fX, coords[i]->fY, i, bm);
}

DEF_TEST(BitmapCopy, reporter) {
    static const Pair gPairs[] = {
        { SkBitmap::kNo_Config,         "0000000"  },
        { SkBitmap::kA8_Config,         "0101010"  },
        { SkBitmap::kIndex8_Config,     "0111010"  },
        { SkBitmap::kRGB_565_Config,    "0101010"  },
        { SkBitmap::kARGB_4444_Config,  "0101110"  },
        { SkBitmap::kARGB_8888_Config,  "0101110"  },
    };

    static const bool isExtracted[] = {
        false, true
    };

    const int W = 20;
    const int H = 33;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        for (size_t j = 0; j < SK_ARRAY_COUNT(gPairs); j++) {
            SkBitmap srcOpaque, srcPremul, dst;

            {
                SkColorTable* ctOpaque = NULL;
                SkColorTable* ctPremul = NULL;

                srcOpaque.setConfig(gPairs[i].fConfig, W, H, 0, kOpaque_SkAlphaType);
                srcPremul.setConfig(gPairs[i].fConfig, W, H, 0, kPremul_SkAlphaType);
                if (SkBitmap::kIndex8_Config == gPairs[i].fConfig) {
                    ctOpaque = init_ctable(kOpaque_SkAlphaType);
                    ctPremul = init_ctable(kPremul_SkAlphaType);
                }
                srcOpaque.allocPixels(ctOpaque);
                srcPremul.allocPixels(ctPremul);
                SkSafeUnref(ctOpaque);
                SkSafeUnref(ctPremul);
            }
            init_src(srcOpaque);
            init_src(srcPremul);

            bool success = srcPremul.copyTo(&dst, gPairs[j].fConfig);
            bool expected = gPairs[i].fValid[j] != '0';
            if (success != expected) {
                SkString str;
                str.printf("SkBitmap::copyTo from %s to %s. expected %s returned %s",
                           gConfigName[i], gConfigName[j], boolStr(expected),
                           boolStr(success));
                reporter->reportFailed(str);
            }

            bool canSucceed = srcPremul.canCopyTo(gPairs[j].fConfig);
            if (success != canSucceed) {
                SkString str;
                str.printf("SkBitmap::copyTo from %s to %s. returned %s canCopyTo %s",
                           gConfigName[i], gConfigName[j], boolStr(success),
                           boolStr(canSucceed));
                reporter->reportFailed(str);
            }

            if (success) {
                REPORTER_ASSERT(reporter, srcPremul.width() == dst.width());
                REPORTER_ASSERT(reporter, srcPremul.height() == dst.height());
                REPORTER_ASSERT(reporter, dst.config() == gPairs[j].fConfig);
                test_isOpaque(reporter, srcOpaque, srcPremul, dst.config());
                if (srcPremul.config() == dst.config()) {
                    SkAutoLockPixels srcLock(srcPremul);
                    SkAutoLockPixels dstLock(dst);
                    REPORTER_ASSERT(reporter, srcPremul.readyToDraw());
                    REPORTER_ASSERT(reporter, dst.readyToDraw());
                    const char* srcP = (const char*)srcPremul.getAddr(0, 0);
                    const char* dstP = (const char*)dst.getAddr(0, 0);
                    REPORTER_ASSERT(reporter, srcP != dstP);
                    REPORTER_ASSERT(reporter, !memcmp(srcP, dstP,
                                                      srcPremul.getSize()));
                    REPORTER_ASSERT(reporter, srcPremul.getGenerationID() == dst.getGenerationID());
                } else {
                    REPORTER_ASSERT(reporter, srcPremul.getGenerationID() != dst.getGenerationID());
                }
                // test extractSubset
                {
                    SkBitmap bitmap(srcOpaque);
                    SkBitmap subset;
                    SkIRect r;
                    r.set(1, 1, 2, 2);
                    bitmap.setIsVolatile(true);
                    if (bitmap.extractSubset(&subset, r)) {
                        REPORTER_ASSERT(reporter, subset.width() == 1);
                        REPORTER_ASSERT(reporter, subset.height() == 1);
                        REPORTER_ASSERT(reporter,
                                        subset.alphaType() == bitmap.alphaType());
                        REPORTER_ASSERT(reporter,
                                        subset.isVolatile() == true);

                        SkBitmap copy;
                        REPORTER_ASSERT(reporter,
                                        subset.copyTo(&copy, subset.config()));
                        REPORTER_ASSERT(reporter, copy.width() == 1);
                        REPORTER_ASSERT(reporter, copy.height() == 1);
                        REPORTER_ASSERT(reporter, copy.rowBytes() <= 4);

                        SkAutoLockPixels alp0(subset);
                        SkAutoLockPixels alp1(copy);
                        // they should both have, or both not-have, a colortable
                        bool hasCT = subset.getColorTable() != NULL;
                        REPORTER_ASSERT(reporter,
                                    (copy.getColorTable() != NULL) == hasCT);
                    }
                    bitmap = srcPremul;
                    bitmap.setIsVolatile(false);
                    if (bitmap.extractSubset(&subset, r)) {
                        REPORTER_ASSERT(reporter,
                                        subset.alphaType() == bitmap.alphaType());
                        REPORTER_ASSERT(reporter,
                                        subset.isVolatile() == false);
                    }
                }
            } else {
                // dst should be unchanged from its initial state
                REPORTER_ASSERT(reporter, dst.config() == SkBitmap::kNo_Config);
                REPORTER_ASSERT(reporter, dst.width() == 0);
                REPORTER_ASSERT(reporter, dst.height() == 0);
            }
        } // for (size_t j = ...

        // Tests for getSafeSize(), getSafeSize64(), copyPixelsTo(),
        // copyPixelsFrom().
        //
        for (size_t copyCase = 0; copyCase < SK_ARRAY_COUNT(isExtracted);
             ++copyCase) {
            // Test copying to/from external buffer.
            // Note: the tests below have hard-coded values ---
            //       Please take care if modifying.

            // Tests for getSafeSize64().
            // Test with a very large configuration without pixel buffer
            // attached.
            SkBitmap tstSafeSize;
            tstSafeSize.setConfig(gPairs[i].fConfig, 100000000U,
                                  100000000U);
            Sk64 safeSize = tstSafeSize.getSafeSize64();
            if (safeSize.isNeg()) {
                SkString str;
                str.printf("getSafeSize64() negative: %s",
                    getSkConfigName(tstSafeSize));
                reporter->reportFailed(str);
            }
            bool sizeFail = false;
            // Compare against hand-computed values.
            switch (gPairs[i].fConfig) {
                case SkBitmap::kNo_Config:
                    break;

                case SkBitmap::kA8_Config:
                case SkBitmap::kIndex8_Config:
                    if (safeSize.fHi != 0x2386F2 ||
                        safeSize.fLo != 0x6FC10000)
                        sizeFail = true;
                    break;

                case SkBitmap::kRGB_565_Config:
                case SkBitmap::kARGB_4444_Config:
                    if (safeSize.fHi != 0x470DE4 ||
                        safeSize.fLo != 0xDF820000)
                        sizeFail = true;
                    break;

                case SkBitmap::kARGB_8888_Config:
                    if (safeSize.fHi != 0x8E1BC9 ||
                        safeSize.fLo != 0xBF040000)
                        sizeFail = true;
                    break;

                default:
                    break;
            }
            if (sizeFail) {
                SkString str;
                str.printf("getSafeSize64() wrong size: %s",
                    getSkConfigName(tstSafeSize));
                reporter->reportFailed(str);
            }

            int subW = 2;
            int subH = 2;

            // Create bitmap to act as source for copies and subsets.
            SkBitmap src, subset;
            SkColorTable* ct = NULL;
            if (isExtracted[copyCase]) { // A larger image to extract from.
                src.setConfig(gPairs[i].fConfig, 2 * subW + 1, subH);
            } else { // Tests expect a 2x2 bitmap, so make smaller.
                src.setConfig(gPairs[i].fConfig, subW, subH);
            }
            if (SkBitmap::kIndex8_Config == src.config()) {
                ct = init_ctable(kPremul_SkAlphaType);
            }

            src.allocPixels(ct);
            SkSafeUnref(ct);

            // Either copy src or extract into 'subset', which is used
            // for subsequent calls to copyPixelsTo/From.
            bool srcReady = false;
            if (isExtracted[copyCase]) {
                // The extractedSubset() test case allows us to test copy-
                // ing when src and dst mave possibly different strides.
                SkIRect r;
                r.set(1, 0, 1 + subW, subH); // 2x2 extracted bitmap

                srcReady = src.extractSubset(&subset, r);
            } else {
                srcReady = src.copyTo(&subset, src.config());
            }

            // Not all configurations will generate a valid 'subset'.
            if (srcReady) {

                // Allocate our target buffer 'buf' for all copies.
                // To simplify verifying correctness of copies attach
                // buf to a SkBitmap, but copies are done using the
                // raw buffer pointer.
                const size_t bufSize = subH *
                    SkBitmap::ComputeRowBytes(src.config(), subW) * 2;
                SkAutoMalloc autoBuf (bufSize);
                uint8_t* buf = static_cast<uint8_t*>(autoBuf.get());

                SkBitmap bufBm; // Attach buf to this bitmap.
                bool successExpected;

                // Set up values for each pixel being copied.
                Coordinates coords(subW * subH);
                for (int x = 0; x < subW; ++x)
                    for (int y = 0; y < subH; ++y)
                    {
                        int index = y * subW + x;
                        SkASSERT(index < coords.length);
                        coords[index]->fX = x;
                        coords[index]->fY = y;
                    }

                writeCoordPixels(subset, coords);

                // Test #1 ////////////////////////////////////////////

                // Before/after comparisons easier if we attach buf
                // to an appropriately configured SkBitmap.
                memset(buf, 0xFF, bufSize);
                // Config with stride greater than src but that fits in buf.
                bufBm.setConfig(gPairs[i].fConfig, subW, subH,
                    SkBitmap::ComputeRowBytes(subset.config(), subW) * 2);
                bufBm.setPixels(buf);
                successExpected = false;
                // Then attempt to copy with a stride that is too large
                // to fit in the buffer.
                REPORTER_ASSERT(reporter,
                    subset.copyPixelsTo(buf, bufSize, bufBm.rowBytes() * 3)
                    == successExpected);

                if (successExpected)
                    reportCopyVerification(subset, bufBm, coords,
                        "copyPixelsTo(buf, bufSize, 1.5*maxRowBytes)",
                        reporter);

                // Test #2 ////////////////////////////////////////////
                // This test should always succeed, but in the case
                // of extracted bitmaps only because we handle the
                // issue of getSafeSize(). Without getSafeSize()
                // buffer overrun/read would occur.
                memset(buf, 0xFF, bufSize);
                bufBm.setConfig(gPairs[i].fConfig, subW, subH,
                                subset.rowBytes());
                bufBm.setPixels(buf);
                successExpected = subset.getSafeSize() <= bufSize;
                REPORTER_ASSERT(reporter,
                    subset.copyPixelsTo(buf, bufSize) ==
                        successExpected);
                if (successExpected)
                    reportCopyVerification(subset, bufBm, coords,
                    "copyPixelsTo(buf, bufSize)", reporter);

                // Test #3 ////////////////////////////////////////////
                // Copy with different stride between src and dst.
                memset(buf, 0xFF, bufSize);
                bufBm.setConfig(gPairs[i].fConfig, subW, subH,
                                subset.rowBytes()+1);
                bufBm.setPixels(buf);
                successExpected = true; // Should always work.
                REPORTER_ASSERT(reporter,
                        subset.copyPixelsTo(buf, bufSize,
                            subset.rowBytes()+1) == successExpected);
                if (successExpected)
                    reportCopyVerification(subset, bufBm, coords,
                    "copyPixelsTo(buf, bufSize, rowBytes+1)", reporter);

                // Test #4 ////////////////////////////////////////////
                // Test copy with stride too small.
                memset(buf, 0xFF, bufSize);
                bufBm.setConfig(gPairs[i].fConfig, subW, subH);
                bufBm.setPixels(buf);
                successExpected = false;
                // Request copy with stride too small.
                REPORTER_ASSERT(reporter,
                    subset.copyPixelsTo(buf, bufSize, bufBm.rowBytes()-1)
                        == successExpected);
                if (successExpected)
                    reportCopyVerification(subset, bufBm, coords,
                    "copyPixelsTo(buf, bufSize, rowBytes()-1)", reporter);

#if 0   // copyPixelsFrom is gone
                // Test #5 ////////////////////////////////////////////
                // Tests the case where the source stride is too small
                // for the source configuration.
                memset(buf, 0xFF, bufSize);
                bufBm.setConfig(gPairs[i].fConfig, subW, subH);
                bufBm.setPixels(buf);
                writeCoordPixels(bufBm, coords);
                REPORTER_ASSERT(reporter,
                    subset.copyPixelsFrom(buf, bufSize, 1) == false);

                // Test #6 ///////////////////////////////////////////
                // Tests basic copy from an external buffer to the bitmap.
                // If the bitmap is "extracted", this also tests the case
                // where the source stride is different from the dest.
                // stride.
                // We've made the buffer large enough to always succeed.
                bufBm.setConfig(gPairs[i].fConfig, subW, subH);
                bufBm.setPixels(buf);
                writeCoordPixels(bufBm, coords);
                REPORTER_ASSERT(reporter,
                    subset.copyPixelsFrom(buf, bufSize, bufBm.rowBytes()) ==
                        true);
                reportCopyVerification(bufBm, subset, coords,
                    "copyPixelsFrom(buf, bufSize)",
                    reporter);

                // Test #7 ////////////////////////////////////////////
                // Tests the case where the source buffer is too small
                // for the transfer.
                REPORTER_ASSERT(reporter,
                    subset.copyPixelsFrom(buf, 1, subset.rowBytes()) ==
                        false);

#endif
            }
        } // for (size_t copyCase ...
    }
}
