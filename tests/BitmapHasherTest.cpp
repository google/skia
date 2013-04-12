
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"

#include "SkBitmap.h"
#include "SkBitmapHasher.h"
#include "SkColor.h"

// Word size that is large enough to hold results of any checksum type.
typedef uint64_t checksum_result;

namespace skiatest {
    class BitmapHasherTestClass : public Test {
    public:
        static Test* Factory(void*) {return SkNEW(BitmapHasherTestClass); }
    protected:
        virtual void onGetName(SkString* name) { name->set("BitmapHasher"); }
        virtual void onRun(Reporter* reporter) {
            this->fReporter = reporter;
            RunTest();
        }
    private:

        // Fill in bitmap with test data.
        void CreateTestBitmap(SkBitmap &bitmap, SkBitmap::Config config, int width, int height,
                              SkColor color) {
            bitmap.setConfig(config, width, height);
            REPORTER_ASSERT(fReporter, bitmap.allocPixels());
            bitmap.setIsOpaque(true);
            bitmap.eraseColor(color);
        }

        void RunTest() {
            // Test SkBitmapHasher
            SkBitmap bitmap;
            SkHashDigest digest;
            // initial test case
            CreateTestBitmap(bitmap, SkBitmap::kARGB_8888_Config, 333, 555, SK_ColorBLUE);
            REPORTER_ASSERT(fReporter, SkBitmapHasher::ComputeDigest(bitmap, &digest));
            REPORTER_ASSERT(fReporter, digest == 0x18f9df68b1b02f38ULL);
            // same pixel data but different dimensions should yield a different checksum
            CreateTestBitmap(bitmap, SkBitmap::kARGB_8888_Config, 555, 333, SK_ColorBLUE);
            REPORTER_ASSERT(fReporter, SkBitmapHasher::ComputeDigest(bitmap, &digest));
            REPORTER_ASSERT(fReporter, digest == 0x6b0298183f786c8eULL);
            // same dimensions but different color should yield a different checksum
            CreateTestBitmap(bitmap, SkBitmap::kARGB_8888_Config, 555, 333, SK_ColorGREEN);
            REPORTER_ASSERT(fReporter, SkBitmapHasher::ComputeDigest(bitmap, &digest));
            REPORTER_ASSERT(fReporter, digest == 0xc6b4b3f6fadaaf37ULL);
            // same pixel colors in a different config should yield the same checksum
            CreateTestBitmap(bitmap, SkBitmap::kARGB_4444_Config, 555, 333, SK_ColorGREEN);
            REPORTER_ASSERT(fReporter, SkBitmapHasher::ComputeDigest(bitmap, &digest));
            REPORTER_ASSERT(fReporter, digest == 0xc6b4b3f6fadaaf37ULL);
        }

        Reporter* fReporter;
    };

    static TestRegistry gReg(BitmapHasherTestClass::Factory);
}
