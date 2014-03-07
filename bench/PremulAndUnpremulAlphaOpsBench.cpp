/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkConfig8888.h"
#include "SkString.h"
#include "sk_tool_utils.h"

class PremulAndUnpremulAlphaOpsBench : public SkBenchmark {
public:
    PremulAndUnpremulAlphaOpsBench(SkCanvas::Config8888 config) {
        fUnPremulConfig = config;
        fName.printf("premul_and_unpremul_alpha_%s",
                     (config ==  SkCanvas::kRGBA_Unpremul_Config8888) ?
                     "RGBA8888" : "Native8888");
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        canvas->clear(SK_ColorBLACK);
        SkISize size = canvas->getDeviceSize();

        SkBitmap bmp1;
        bmp1.setConfig(SkBitmap::kARGB_8888_Config, size.width(),
                       size.height());
        bmp1.allocPixels();
        SkAutoLockPixels alp(bmp1);
        uint32_t* pixels = reinterpret_cast<uint32_t*>(bmp1.getPixels());
        for (int h = 0; h < size.height(); ++h) {
            for (int w = 0; w < size.width(); ++w)
                pixels[h * size.width() + w] = SkPackConfig8888(fUnPremulConfig,
                    h & 0xFF, w & 0xFF, w & 0xFF, w & 0xFF);
        }

        SkBitmap bmp2;
        bmp2.setConfig(SkBitmap::kARGB_8888_Config, size.width(),
                       size.height());

        SkColorType ct;
        SkAlphaType at;
        sk_tool_utils::config8888_to_imagetypes(fUnPremulConfig, &ct, &at);
        if (bmp1.isOpaque()) {
            at = kOpaque_SkAlphaType;
        }

        for (int loop = 0; loop < loops; ++loop) {
            // Unpremul -> Premul
            sk_tool_utils::write_pixels(canvas, bmp1, 0, 0, ct, at);
            // Premul -> Unpremul
            canvas->readPixels(&bmp2, 0, 0, fUnPremulConfig);
        }
    }

private:
    SkCanvas::Config8888 fUnPremulConfig;
    SkString fName;
    typedef SkBenchmark INHERITED;
};


DEF_BENCH(return new PremulAndUnpremulAlphaOpsBench(SkCanvas::kRGBA_Unpremul_Config8888));
DEF_BENCH(return new PremulAndUnpremulAlphaOpsBench(SkCanvas::kNative_Unpremul_Config8888));
