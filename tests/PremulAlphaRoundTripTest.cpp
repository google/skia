/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkCanvas.h"
#include "SkConfig8888.h"
#include "SkBitmapDevice.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "SkGpuDevice.h"
#endif

static void fillCanvas(SkCanvas* canvas, SkCanvas::Config8888 unpremulConfig) {
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
    SkCanvas::kBGRA_Unpremul_Config8888,
    SkCanvas::kRGBA_Unpremul_Config8888,
};

static void PremulAlphaRoundTripTest(skiatest::Reporter* reporter, GrContextFactory* factory) {
    SkAutoTUnref<SkBaseDevice> device;
    for (int dtype = 0; dtype < 2; ++dtype) {

        int glCtxTypeCnt = 1;
#if SK_SUPPORT_GPU
        if (0 != dtype)  {
            glCtxTypeCnt = GrContextFactory::kGLContextTypeCnt;
        }
#endif
        for (int glCtxType = 0; glCtxType < glCtxTypeCnt; ++glCtxType) {
            if (0 == dtype) {
                device.reset(new SkBitmapDevice(SkBitmap::kARGB_8888_Config,
                                                256,
                                                256,
                                                false));
            } else {
#if SK_SUPPORT_GPU
                GrContextFactory::GLContextType type =
                    static_cast<GrContextFactory::GLContextType>(glCtxType);
                if (!GrContextFactory::IsRenderingGLContext(type)) {
                    continue;
                }
                GrContext* context = factory->get(type);
                if (NULL == context) {
                    continue;
                }

                device.reset(new SkGpuDevice(context, SkBitmap::kARGB_8888_Config, 256, 256));
#else
                continue;
#endif
            }
            SkCanvas canvas(device);

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
                bool success = true;
                for (int y = 0; y < 256 && success; ++y) {
                    for (int x = 0; x < 256 && success; ++x) {
                        int i = y * 256 + x;
                        REPORTER_ASSERT(reporter, success = pixels1[i] == pixels2[i]);
                    }
                }
            }
        }
    }
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("PremulAlphaRoundTripTest", PremulAlphaRoundTripTestClass, PremulAlphaRoundTripTest)
