/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColor.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"
#include "SkRect.h"
#include "SkShader.h"

class ColorSpaceXformGM : public skiagm::GM {
public:
    ColorSpaceXformGM() {}

protected:
    void onOnceBeforeDraw() override {
        SkColor colors[] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorMAGENTA, SK_ColorCYAN, SK_ColorYELLOW,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorMAGENTA,
        };
        static_assert(kNumColors == SK_ARRAY_COUNT(colors), "Fix number of colors.");

        for (int i = 0; i < kNumColors; i++) {
            fSRGBColors[i] = SkColor4f::FromColor(colors[i]);
        }

        static constexpr float kWideGamutRGB_toXYZD50[]{
            0.7161046f, 0.1009296f, 0.1471858f,
            0.2581874f, 0.7249378f, 0.0168748f,
            0.0000000f, 0.0517813f, 0.7734287f,
        };

        SkMatrix44 wideGamut(SkMatrix44::kUninitialized_Constructor);
        wideGamut.set3x3RowMajorf(kWideGamutRGB_toXYZD50);

        // Test BGRA input.
        sk_sp<SkColorSpace> srcSpace = SkColorSpace::MakeSRGB();
        sk_sp<SkColorSpace> dstSpace =
                SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma, wideGamut);
        std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(srcSpace.get(),
                                                                          dstSpace.get());
        xform->apply(SkColorSpaceXform::kRGBA_F32_ColorFormat, fWideGamutColors0,
                     SkColorSpaceXform::kBGRA_8888_ColorFormat, colors, kNumColors,
                     kOpaque_SkAlphaType);

        // Test F32 input.
        srcSpace = as_CSB(srcSpace)->makeLinearGamma();
        xform = SkColorSpaceXform::New(srcSpace.get(), dstSpace.get());
        xform->apply(SkColorSpaceXform::kRGBA_F32_ColorFormat, fWideGamutColors1,
                     SkColorSpaceXform::kRGBA_F32_ColorFormat, fSRGBColors, kNumColors,
                     kOpaque_SkAlphaType);
    }

    SkString onShortName() override {
        return SkString("colorspacexform");
    }

    SkISize onISize() override {
        return SkISize::Make(500, 300);
    }

    void onDraw(SkCanvas* canvas) override {
        auto drawColors = [canvas](SkColor4f* colors) {
            SkRect r = SkRect::MakeXYWH(0.0f, 0.0f, 50.0f, 100.0f);

            canvas->save();
            for (int i = 0; i < kNumColors; i++) {
                auto space = SkColorSpace::MakeSRGBLinear();
                sk_sp<SkShader> s = SkShader::MakeColorShader(colors[i], space);
                SkPaint paint;
                paint.setShader(s);
                canvas->drawRect(r, paint);
                canvas->translate(50.0f, 0.0f);
            }
            canvas->restore();
        };

        // Wide gamut colors should appear darker - we are simulating a more intense display.
        drawColors(fSRGBColors);
        canvas->translate(0.0f, 100.0f);
        drawColors(fWideGamutColors0);
        canvas->translate(0.0f, 100.0f);
        drawColors(fWideGamutColors1);
    }

private:
    static constexpr int kNumColors = 10;

    SkColor4f fSRGBColors[kNumColors];
    SkColor4f fWideGamutColors0[kNumColors];
    SkColor4f fWideGamutColors1[kNumColors];

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new ColorSpaceXformGM;)
