#include "Test.h"
#include "SkBitmap.h"

static void TestGetColor(skiatest::Reporter* reporter) {
    static const struct Rec {
        SkBitmap::Config    fConfig;
        SkColor             fInColor;
        SkColor             fOutColor;
    } gRec[] = {
        // todo: add some tests that involve alpha, so we exercise the
        // unpremultiply aspect of getColor()
        {   SkBitmap::kA8_Config,           0xFF000000,     0xFF000000  },
        {   SkBitmap::kA8_Config,           0,              0           },
        {   SkBitmap::kARGB_4444_Config,    0xFF224466,     0xFF224466  },
        {   SkBitmap::kARGB_4444_Config,    0,              0           },
        {   SkBitmap::kRGB_565_Config,      0xFF00FF00,     0xFF00FF00  },
        {   SkBitmap::kRGB_565_Config,      0xFFFF00FF,     0xFFFF00FF  },
        {   SkBitmap::kARGB_8888_Config,    0xFFFFFFFF,     0xFFFFFFFF  },
        {   SkBitmap::kARGB_8888_Config,    0,              0           },
        {   SkBitmap::kARGB_8888_Config,    0xFF224466,     0xFF224466  },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        SkBitmap bm;
        uint32_t storage[1];
        bm.setConfig(gRec[i].fConfig, 1, 1);
        bm.setPixels(storage);
        bm.eraseColor(gRec[i].fInColor);

        SkColor c = bm.getColor(0, 0);
        REPORTER_ASSERT(reporter, c == gRec[i].fOutColor);
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("GetColor", TestGetColorClass, TestGetColor)
