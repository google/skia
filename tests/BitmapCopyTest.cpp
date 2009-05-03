#include "Test.h"
#include "SkBitmap.h"

static const char* boolStr(bool value) {
    return value ? "true" : "false";
}

// these are in the same order as the SkBitmap::Config enum
static const char* gConfigName[] = {
    "None", "A1", "A8", "Index8", "565", "4444", "8888", "RLE_Index8"
};

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
        { SkBitmap::kRLE_Index8_Config, "00000000"  }
    };

    const int W = 20;
    const int H = 33;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        for (size_t j = 0; j < SK_ARRAY_COUNT(gPairs); j++) {
            SkBitmap src, dst;
            SkColorTable* ct = NULL;

            src.setConfig(gPairs[i].fConfig, W, H);
            if (SkBitmap::kIndex8_Config == src.config()) {
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

            if (success) {
                REPORTER_ASSERT(reporter, src.width() == dst.width());
                REPORTER_ASSERT(reporter, src.height() == dst.height());
                REPORTER_ASSERT(reporter, dst.config() == gPairs[j].fConfig);
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
