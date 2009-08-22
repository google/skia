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

static void init_src(const SkBitmap& bitmap) {
    SkAutoLockPixels lock(bitmap);
    if (bitmap.getPixels()) {
        memset(bitmap.getPixels(), 4, bitmap.getSize());
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
            ct->safeRef();

            init_src(src);
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
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("BitmapCopy", TestBitmapCopyClass, TestBitmapCopy)
