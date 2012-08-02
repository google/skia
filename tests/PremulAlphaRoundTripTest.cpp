
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkCanvas.h"
#include "SkConfig8888.h"
#include "SkDevice.h"

#if SK_SUPPORT_GPU
#include "SkGpuDevice.h"
#endif


namespace {

void fillCanvas(SkCanvas* canvas, SkCanvas::Config8888 unpremulConfig) {
    SkBitmap bmp;
    bmp.setConfig(SkBitmap::kARGB_8888_Config, 256, 256);
    bmp.allocPixels();
    SkAutoLockPixels alp(bmp);
    uint32_t* pixels = reinterpret_cast<uint32_t*>(bmp.getPixels());

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            pixels[a * 256 + r] = SkPackConfig8888(unpremulConfig, a, r, 0, 0);
        }
    }
    canvas->writePixels(bmp, 0, 0, unpremulConfig);
}

static const SkCanvas::Config8888 gUnpremulConfigs[] = {
    SkCanvas::kNative_Unpremul_Config8888,
/**
 * There is a bug in Ganesh (http://code.google.com/p/skia/issues/detail?id=438)
 * that causes the readback of pixels from BGRA canvas to an RGBA bitmap to
 * fail. This should be removed as soon as the issue above is resolved.
 */
#if !defined(SK_BUILD_FOR_ANDROID)
    SkCanvas::kBGRA_Unpremul_Config8888,
#endif
    SkCanvas::kRGBA_Unpremul_Config8888,
};

void PremulAlphaRoundTripTest(skiatest::Reporter* reporter,
                              GrContext* context) {
    SkCanvas canvas;
    for (int dtype = 0; dtype < 2; ++dtype) {
        if (0 == dtype) {
            canvas.setDevice(new SkDevice(SkBitmap::kARGB_8888_Config,
                                          256,
                                          256,
                                          false))->unref();
        } else {
#if !SK_SUPPORT_GPU || defined(SK_SCALAR_IS_FIXED)
            // GPU device known not to work in the fixed pt build.
            continue;
#else
            canvas.setDevice(new SkGpuDevice(context,
                                             SkBitmap::kARGB_8888_Config,
                                             256,
                                             256))->unref();
#endif
        }

        SkBitmap readBmp1;
        readBmp1.setConfig(SkBitmap::kARGB_8888_Config, 256, 256);
        readBmp1.allocPixels();
        SkBitmap readBmp2;
        readBmp2.setConfig(SkBitmap::kARGB_8888_Config, 256, 256);
        readBmp2.allocPixels();

        for (size_t upmaIdx = 0;
             upmaIdx < SK_ARRAY_COUNT(gUnpremulConfigs);
             ++upmaIdx) {
            fillCanvas(&canvas, gUnpremulConfigs[upmaIdx]);
            {
                SkAutoLockPixels alp1(readBmp1);
                SkAutoLockPixels alp2(readBmp2);
                sk_bzero(readBmp1.getPixels(), readBmp1.getSafeSize());
                sk_bzero(readBmp2.getPixels(), readBmp2.getSafeSize());
            }

            canvas.readPixels(&readBmp1, 0, 0, gUnpremulConfigs[upmaIdx]);
            canvas.writePixels(readBmp1, 0, 0, gUnpremulConfigs[upmaIdx]);
            canvas.readPixels(&readBmp2, 0, 0, gUnpremulConfigs[upmaIdx]);

            SkAutoLockPixels alp1(readBmp1);
            SkAutoLockPixels alp2(readBmp2);
            uint32_t* pixels1 =
                reinterpret_cast<uint32_t*>(readBmp1.getPixels());
            uint32_t* pixels2 =
                reinterpret_cast<uint32_t*>(readBmp2.getPixels());
            for (int y = 0; y < 256; ++y) {
                for (int x = 0; x < 256; ++x) {
                    int i = y * 256 + x;
                    REPORTER_ASSERT(reporter, pixels1[i] == pixels2[i]);
                }
            }
        }
    }
}
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("PremulAlphaRoundTripTest", PremulAlphaRoundTripTestClass, PremulAlphaRoundTripTest)

