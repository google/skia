#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"

static bool check_for_all_zeros(const SkBitmap& bm) {
    SkAutoLockPixels alp(bm);

    size_t count = bm.width() * bm.bytesPerPixel();
    for (int y = 0; y < bm.height(); y++) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(bm.getAddr(0, y));
        for (size_t i = 0; i < count; i++) {
            if (ptr[i]) {
                return false;
            }
        }
    }
    return true;
}

static const int gWidth = 256;
static const int gHeight = 256;

static void create(SkBitmap* bm, SkBitmap::Config config, SkColor color) {
    bm->setConfig(config, gWidth, gHeight);
    bm->allocPixels();
    bm->eraseColor(color);
}

static void TestDrawBitmapRect(skiatest::Reporter* reporter) {
    SkBitmap src, dst;

    create(&src, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    create(&dst, SkBitmap::kARGB_8888_Config, 0);

    SkCanvas canvas(dst);

    SkIRect srcR = { gWidth, 0, gWidth + 16, 16 };
    SkRect  dstR = { 0, 0, SkIntToScalar(16), SkIntToScalar(16) };

    canvas.drawBitmapRect(src, &srcR, dstR, NULL);

    // ensure that we draw nothing if srcR does not intersect the bitmap
    REPORTER_ASSERT(reporter, check_for_all_zeros(dst));
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DrawBitmapRect", TestDrawBitmapRectClass, TestDrawBitmapRect)
