#include "Test.h"
#include "SkBitmap.h"
#include "SkRect.h"

static const char* boolStr(bool value) {
    return value ? "true" : "false";
}

// these are in the same order as the SkBitmap::Config enum
static const char* gConfigName[] = {
    "None", "A1", "A8", "Index8", "565", "4444", "8888", "RLE_Index8"
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
static void test_isOpaque(skiatest::Reporter* reporter, const SkBitmap& src,
                          SkBitmap::Config dstConfig) {
    SkBitmap bitmap(src);
    SkBitmap dst;

    // we need the lock so that we get a valid colorTable (when available)
    SkAutoLockPixels alp(bitmap);
    SkColorTable* ctable = bitmap.getColorTable();
    unsigned ctableFlags = ctable ? ctable->getFlags() : 0;

    if (canHaveAlpha(bitmap.config()) && canHaveAlpha(dstConfig)) {
        bitmap.setIsOpaque(false);
        if (ctable) {
            ctable->setFlags(ctableFlags & ~SkColorTable::kColorsAreOpaque_Flag);
        }
        REPORTER_ASSERT(reporter, bitmap.copyTo(&dst, dstConfig));
        REPORTER_ASSERT(reporter, dst.config() == dstConfig);
        if (bitmap.isOpaque() != dst.isOpaque()) {
            report_opaqueness(reporter, bitmap, dst);
        }
    }

    bitmap.setIsOpaque(true);
    if (ctable) {
        ctable->setFlags(ctableFlags | SkColorTable::kColorsAreOpaque_Flag);
    }
    REPORTER_ASSERT(reporter, bitmap.copyTo(&dst, dstConfig));
    REPORTER_ASSERT(reporter, dst.config() == dstConfig);
    if (bitmap.isOpaque() != dst.isOpaque()) {
        report_opaqueness(reporter, bitmap, dst);
    }

    if (ctable) {
        ctable->setFlags(ctableFlags);
    }
}

static void init_src(const SkBitmap& bitmap, const SkColorTable* ct) {
    SkAutoLockPixels lock(bitmap);
    if (bitmap.getPixels()) {
        if (ct) {
            sk_bzero(bitmap.getPixels(), bitmap.getSize());
        } else {
            bitmap.eraseColor(SK_ColorWHITE);
        }
    }
}

SkColorTable* init_ctable() {
    static const SkColor colors[] = {
        SK_ColorBLACK, SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE
    };
    return new SkColorTable(colors, SK_ARRAY_COUNT(colors));
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
uint32_t getPixel(int x, int y, const SkBitmap& bm) {
    uint32_t val = 0;
    uint16_t val16;
    uint8_t val8, shift;
    SkAutoLockPixels lock(bm);
    const void* rawAddr = bm.getAddr(x,y);

    switch (bm.getConfig()) {
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
        case SkBitmap::kA1_Config:
            memcpy(&val8, rawAddr, sizeof(uint8_t));
            shift = x % 8;
            val = (val8 >> shift) & 0x1 ;
            break;
        default:
            break;
    }
    return val;
}

// Utility function to set value of any pixel in bm.
// bm.getConfig() specifies what format 'val' must be
// converted to, but at present uint32_t can handle all formats.
void setPixel(int x, int y, uint32_t val, SkBitmap& bm) {
    uint16_t val16;
    uint8_t val8, shift;
    SkAutoLockPixels lock(bm);
    void* rawAddr = bm.getAddr(x,y);

    switch (bm.getConfig()) {
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
        case SkBitmap::kA1_Config:
            shift = x % 8; // We assume we're in the right byte.
            memcpy(&val8, rawAddr, sizeof(uint8_t));
            if (val & 0x1) // Turn bit on.
                val8 |= (0x1 << shift);
            else // Turn bit off.
                val8 &= ~(0x1 << shift);
            memcpy(rawAddr, &val8, sizeof(uint8_t));
            break;
        default:
            // Ignore.
            break;
    }
}

// Utility to return string containing name of each format, to
// simplify diagnostic output.
const char* getSkConfigName(const SkBitmap& bm) {
    switch (bm.getConfig()) {
        case SkBitmap::kNo_Config: return "SkBitmap::kNo_Config";
        case SkBitmap::kA1_Config: return "SkBitmap::kA1_Config";
        case SkBitmap::kA8_Config: return "SkBitmap::kA8_Config";
        case SkBitmap::kIndex8_Config: return "SkBitmap::kIndex8_Config";
        case SkBitmap::kRGB_565_Config: return "SkBitmap::kRGB_565_Config";
        case SkBitmap::kARGB_4444_Config: return "SkBitmap::kARGB_4444_Config";
        case SkBitmap::kARGB_8888_Config: return "SkBitmap::kARGB_8888_Config";
        case SkBitmap::kRLE_Index8_Config:
            return "SkBitmap::kRLE_Index8_Config,";
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
void reportCopyVerification(const SkBitmap& bm1, const SkBitmap& bm2,
                            Coordinates& coords,
                            const char* msg,
                            skiatest::Reporter* reporter){
    bool success = true;

    // Confirm all pixels in the list match.
    for (int i = 0; i < coords.length; ++i)
        success = success &&
                  (getPixel(coords[i]->fX, coords[i]->fY, bm1) ==
                   getPixel(coords[i]->fX, coords[i]->fY, bm2));

    if (!success) {
        SkString str;
        str.printf("%s [config = %s]",
                   msg, getSkConfigName(bm1));
        reporter->reportFailed(str);
    }
}

// Writes unique pixel values at locations specified by coords.
void writeCoordPixels(SkBitmap& bm, const Coordinates& coords) {
    for (int i = 0; i < coords.length; ++i)
        setPixel(coords[i]->fX, coords[i]->fY, i, bm);
}

static void TestBitmapCopy(skiatest::Reporter* reporter) {
    static const Pair gPairs[] = {
        { SkBitmap::kNo_Config,         "00000000"  },
        { SkBitmap::kA1_Config,         "01000000"  },
        { SkBitmap::kA8_Config,         "00101110"  },
        { SkBitmap::kIndex8_Config,     "00111110"  },
        { SkBitmap::kRGB_565_Config,    "00101110"  },
        { SkBitmap::kARGB_4444_Config,  "00101110"  },
        { SkBitmap::kARGB_8888_Config,  "00101110"  },
// TODO: create valid RLE bitmap to test with
 //       { SkBitmap::kRLE_Index8_Config, "00101111"  }
    };

    static const bool isExtracted[] = {
        false, true
    };

    const int W = 20;
    const int H = 33;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        for (size_t j = 0; j < SK_ARRAY_COUNT(gPairs); j++) {
            SkBitmap src, dst;
            SkColorTable* ct = NULL;

            src.setConfig(gPairs[i].fConfig, W, H);
            if (SkBitmap::kIndex8_Config == src.config() ||
                    SkBitmap::kRLE_Index8_Config == src.config()) {
                ct = init_ctable();
            }
            src.allocPixels(ct);
            SkSafeUnref(ct);

            init_src(src, ct);
            bool success = src.copyTo(&dst, gPairs[j].fConfig);
            bool expected = gPairs[i].fValid[j] != '0';
            if (success != expected) {
                SkString str;
                str.printf("SkBitmap::copyTo from %s to %s. expected %s returned %s",
                           gConfigName[i], gConfigName[j], boolStr(expected),
                           boolStr(success));
                reporter->reportFailed(str);
            }

            bool canSucceed = src.canCopyTo(gPairs[j].fConfig);
            if (success != canSucceed) {
                SkString str;
                str.printf("SkBitmap::copyTo from %s to %s. returned %s canCopyTo %s",
                           gConfigName[i], gConfigName[j], boolStr(success),
                           boolStr(canSucceed));
                reporter->reportFailed(str);
            }

            if (success) {
                REPORTER_ASSERT(reporter, src.width() == dst.width());
                REPORTER_ASSERT(reporter, src.height() == dst.height());
                REPORTER_ASSERT(reporter, dst.config() == gPairs[j].fConfig);
                test_isOpaque(reporter, src, dst.config());
                if (src.config() == dst.config()) {
                    SkAutoLockPixels srcLock(src);
                    SkAutoLockPixels dstLock(dst);
                    REPORTER_ASSERT(reporter, src.readyToDraw());
                    REPORTER_ASSERT(reporter, dst.readyToDraw());
                    const char* srcP = (const char*)src.getAddr(0, 0);
                    const char* dstP = (const char*)dst.getAddr(0, 0);
                    REPORTER_ASSERT(reporter, srcP != dstP);
                    REPORTER_ASSERT(reporter, !memcmp(srcP, dstP,
                                                      src.getSize()));
                }
                // test extractSubset
                {
                    SkBitmap subset;
                    SkIRect r;
                    r.set(1, 1, 2, 2);
                    if (src.extractSubset(&subset, r)) {
                        REPORTER_ASSERT(reporter, subset.width() == 1);
                        REPORTER_ASSERT(reporter, subset.height() == 1);

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
            if (gPairs[i].fConfig != SkBitmap::kRLE_Index8_Config) {

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

                    case SkBitmap::kA1_Config:
                        if (safeSize.fHi != 0x470DE ||
                            safeSize.fLo != 0x4DF82000)
                            sizeFail = true;
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

                    case SkBitmap::kRLE_Index8_Config:
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

                size_t subW, subH;
                // Set sizes to be height = 2 to force the last row of the
                // source to be used, thus verifying correct operation if
                // the bitmap is an extracted subset.
                if (gPairs[i].fConfig == SkBitmap::kA1_Config) {
                    // If one-bit per pixel, use 9 pixels to force more than
                    // one byte per row.
                    subW = 9;
                    subH = 2;
                } else {
                    // All other configurations are at least one byte per pixel,
                    // and different configs will test copying different numbers
                    // of bytes.
                    subW = subH = 2;
                }

                // Create bitmap to act as source for copies and subsets.
                SkBitmap src, subset;
                SkColorTable* ct = NULL;
                if (isExtracted[copyCase]) { // A larger image to extract from.
                    src.setConfig(gPairs[i].fConfig, 2 * subW + 1, subH);
                } else // Tests expect a 2x2 bitmap, so make smaller.
                    src.setConfig(gPairs[i].fConfig, subW, subH);
                if (SkBitmap::kIndex8_Config == src.config() ||
                        SkBitmap::kRLE_Index8_Config == src.config()) {
                    ct = init_ctable();
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
                    if (gPairs[i].fConfig == SkBitmap::kA1_Config)
                        // This config seems to need byte-alignment of
                        // extracted subset bits.
                        r.set(0, 0, subW, subH);
                    else
                        r.set(1, 0, 1 + subW, subH); // 2x2 extracted bitmap

                    srcReady = src.extractSubset(&subset, r);
                } else {
                    srcReady = src.copyTo(&subset, src.getConfig());
                }

                // Not all configurations will generate a valid 'subset'.
                if (srcReady) {

                    // Allocate our target buffer 'buf' for all copies.
                    // To simplify verifying correctness of copies attach
                    // buf to a SkBitmap, but copies are done using the
                    // raw buffer pointer.
                    const uint32_t bufSize = subH *
                        SkBitmap::ComputeRowBytes(src.getConfig(), subW) * 2;
                    uint8_t* buf = new uint8_t[bufSize];

                    SkBitmap bufBm; // Attach buf to this bitmap.
                    bool successExpected;

                    // Set up values for each pixel being copied.
                    Coordinates coords(subW * subH);
                    for (size_t x = 0; x < subW; ++x)
                        for (size_t y = 0; y < subH; ++y)
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
                        SkBitmap::ComputeRowBytes(subset.getConfig(), subW)
                                                  * 2);
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

                    delete [] buf;
                }
            }
        } // for (size_t copyCase ...
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("BitmapCopy", TestBitmapCopyClass, TestBitmapCopy)
