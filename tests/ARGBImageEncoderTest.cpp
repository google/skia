/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoder.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "Test.h"

static SkColorType gColorTypes[] = {
    kRGB_565_SkColorType,
    kN32_SkColorType,
};

DEF_TEST(ARGBImageEncoder, reporter) {
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
    for (size_t ctIndex = 0; ctIndex < SK_ARRAY_COUNT(gColorTypes); ++ctIndex) {
        // A bitmap that should generate the above bytes:
        SkBitmap bitmap;
        {
            bitmap.allocPixels(SkImageInfo::Make(kWidth, kHeight, gColorTypes[ctIndex],
                                                 kOpaque_SkAlphaType));
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
