/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/** Tests for ARGBImageEncoder. */

#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImageEncoder.h"
#include "SkStream.h"

namespace skiatest {

class BitmapTransformerTestClass : public Test {
public:
    static Test* Factory(void*) { return SkNEW(BitmapTransformerTestClass); }
protected:
    virtual void onGetName(SkString* name) SK_OVERRIDE { name->set("ARGBImageEncoder"); }
    virtual void onRun(Reporter* reporter) SK_OVERRIDE;
};

static SkBitmap::Config configs[] = {
        SkBitmap::kRGB_565_Config,
        SkBitmap::kARGB_8888_Config,
};

void BitmapTransformerTestClass::onRun(Reporter* reporter) {
    // Bytes we expect to get:
    const int kWidth = 3;
    const int kHeight = 5;
    const unsigned char comparisonBuffer[] = {
        // kHeight rows, each with kWidth pixels, premultiplied ARGB for each pixel
        0xff,0xff,0x00,0x00, 0xff,0xff,0x00,0x00, 0xff,0xff,0x00,0x00, // red
        0xff,0x00,0xff,0x00, 0xff,0x00,0xff,0x00, 0xff,0x00,0xff,0x00, // green
        0xff,0x00,0x00,0xff, 0xff,0x00,0x00,0xff, 0xff,0x00,0x00,0xff, // blue
        0xff,0x00,0x00,0xff, 0xff,0x00,0x00,0xff, 0xff,0x00,0x00,0xff, // blue
        0xff,0x00,0x00,0xff, 0xff,0x00,0x00,0xff, 0xff,0x00,0x00,0xff, // blue
    };

    SkAutoTDelete<SkImageEncoder> enc(CreateARGBImageEncoder());
    for (size_t configIndex = 0; configIndex < SK_ARRAY_COUNT(configs); ++configIndex) {
        // A bitmap that should generate the above bytes:
        SkBitmap bitmap;
        {
            bitmap.setConfig(configs[configIndex], kWidth, kHeight);
            REPORTER_ASSERT(reporter, bitmap.allocPixels());
            bitmap.setAlphaType(kOpaque_SkAlphaType);
            bitmap.eraseColor(SK_ColorBLUE);
            // Change rows [0,1] from blue to [red,green].
            SkCanvas canvas(bitmap);
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            canvas.drawIRect(SkIRect::MakeLTRB(0, 0, kWidth, 1), paint);
            paint.setColor(SK_ColorGREEN);
            canvas.drawIRect(SkIRect::MakeLTRB(0, 1, kWidth, 2), paint);
        }

        // Transform the bitmap.
        int bufferSize = bitmap.width() * bitmap.height() * 4;
        SkAutoMalloc pixelBufferManager(bufferSize);
        char *pixelBuffer = static_cast<char *>(pixelBufferManager.get());
        SkMemoryWStream out(pixelBuffer, bufferSize);
        REPORTER_ASSERT(reporter, enc->encodeStream(&out, bitmap, SkImageEncoder::kDefaultQuality));

        // Confirm we got the expected results.
        REPORTER_ASSERT(reporter, bufferSize == sizeof(comparisonBuffer));
        REPORTER_ASSERT(reporter, memcmp(pixelBuffer, comparisonBuffer, bufferSize) == 0);
    }
}

static TestRegistry gReg(BitmapTransformerTestClass::Factory);

}
