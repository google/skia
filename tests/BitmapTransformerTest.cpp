
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * Tests for SkBitmapTransformer.h and SkBitmapTransformer.cpp
 */

#include "Test.h"
#include "SkBitmap.h"
#include "SkBitmapTransformer.h"

namespace skiatest {
    class BitmapTransformerTestClass : public Test {
    public:
        static Test* Factory(void*) {return SkNEW(BitmapTransformerTestClass); }
    protected:
        virtual void onGetName(SkString* name) { name->set("BitmapTransformer"); }
        virtual void onRun(Reporter* reporter) {
            this->fReporter = reporter;
            RunTest();
        }
    private:
        void RunTest() {
            SkBitmap bitmap;
            SkBitmap::Config supportedConfig = SkBitmap::kARGB_8888_Config;
            SkBitmap::Config unsupportedConfig = SkBitmap::kARGB_4444_Config;
            SkBitmapTransformer::PixelFormat supportedPixelFormat =
                SkBitmapTransformer::kARGB_8888_Premul_PixelFormat;
            const int kWidth = 55;
            const int kHeight = 333;

            // Transformations that we know are unsupported:
            {
                bitmap.setConfig(unsupportedConfig, kWidth, kHeight);
                SkBitmapTransformer transformer = SkBitmapTransformer(bitmap, supportedPixelFormat);
                REPORTER_ASSERT(fReporter, !transformer.isValid());
            }

            // Valid transformations:
            {
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

                // A bitmap that should generate the above bytes:
                bitmap.setConfig(supportedConfig, kWidth, kHeight);
                REPORTER_ASSERT(fReporter, bitmap.allocPixels());
                bitmap.setIsOpaque(true);
                bitmap.eraseColor(SK_ColorBLUE);
                bitmap.lockPixels();
                // Change rows [0,1] from blue to [red,green].
                SkColor oldColor = SK_ColorBLUE;
                SkColor newColors[] = {SK_ColorRED, SK_ColorGREEN};
                for (int y = 0; y <= 1; y++) {
                    for (int x = 0; x < kWidth; x++) {
                        REPORTER_ASSERT(fReporter, bitmap.getColor(x, y) == oldColor);
                        SkPMColor* pixel = static_cast<SkPMColor *>(bitmap.getAddr(x, y));
                        *pixel = SkPreMultiplyColor(newColors[y]);
                        REPORTER_ASSERT(fReporter, bitmap.getColor(x, y) == newColors[y]);
                    }
                }
                bitmap.unlockPixels();

                // Transform the bitmap and confirm we got the expected results.
                SkBitmapTransformer transformer = SkBitmapTransformer(bitmap, supportedPixelFormat);
                REPORTER_ASSERT(fReporter, transformer.isValid());
                REPORTER_ASSERT(fReporter, transformer.bytesNeededPerRow() == kWidth * 4);
                REPORTER_ASSERT(fReporter, transformer.bytesNeededTotal() == kWidth * kHeight * 4);
                int bufferSize = transformer.bytesNeededTotal();
                SkAutoMalloc pixelBufferManager(bufferSize);
                char *pixelBuffer = static_cast<char *>(pixelBufferManager.get());
                REPORTER_ASSERT(fReporter,
                                transformer.copyBitmapToPixelBuffer(pixelBuffer, bufferSize));
                REPORTER_ASSERT(fReporter, bufferSize == sizeof(comparisonBuffer));
                REPORTER_ASSERT(fReporter, memcmp(pixelBuffer, comparisonBuffer, bufferSize) == 0);
            }

        }

        Reporter* fReporter;
    };

    static TestRegistry gReg(BitmapTransformerTestClass::Factory);
}
