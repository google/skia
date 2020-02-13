/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "tools/Resources.h"

const char* gProg = R"(
    uniform half4 gColor;

    void main(float2 p, inout half4 color) {
        color = half4(half2(p)*(1.0/255), gColor.b, 1);
    }
)";

class RuntimeShader : public skiagm::GM {
    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("runtime_shader"); }

    SkISize onISize() override { return {512, 256}; }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkRuntimeEffect> gEffect = std::get<0>(SkRuntimeEffect::Make(SkString(gProg)));
        SkASSERT(gEffect);

        SkMatrix localM;
        localM.setRotate(90, 128, 128);

        SkColor4f inputColor = { 1, 0, 0, 1 };
        auto shader = gEffect->makeShader(SkData::MakeWithCopy(&inputColor, sizeof(inputColor)),
                                          nullptr, 0, &localM, true);
        SkPaint p;
        p.setShader(std::move(shader));
        canvas->drawRect({0, 0, 256, 256}, p);
    }
};
DEF_GM(return new RuntimeShader;)

static sk_sp<SkShader> make_shader(sk_sp<SkImage> img, SkISize size) {
    SkMatrix scale = SkMatrix::MakeScale(size.width()  / (float)img->width(),
                                         size.height() / (float)img->height());
    return img->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, &scale);
}

#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"

static sk_sp<SkShader> make_threshold(SkISize size) {
    auto info = SkImageInfo::Make(size.width(), size.height(), kAlpha_8_SkColorType,
                                  kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRaster(info);
    auto canvas = surf->getCanvas();

    const SkScalar rad = 50;
    SkColor colors[] = {SK_ColorBLACK, 0};
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(SkGradientShader::MakeRadial({0,0}, rad, colors, nullptr, 2, SkTileMode::kClamp));

    SkPaint layerPaint;
    const SkScalar sigma = 16.0f;
    layerPaint.setImageFilter(SkImageFilters::Blur(sigma, sigma, nullptr));
    canvas->saveLayer(nullptr, &layerPaint);

    SkRandom rand;
    for (int i = 0; i < 25; ++i) {
        SkScalar x = rand.nextF() * size.width();
        SkScalar y = rand.nextF() * size.height();
        canvas->save();
        canvas->translate(x, y);
        canvas->drawCircle(0, 0, rad, paint);
        canvas->restore();
    }

    canvas->restore();  // apply the blur

    return surf->makeImageSnapshot()->makeShader();
}

class ThresholdRT : public skiagm::GM {
    sk_sp<SkShader> fBefore, fAfter, fThreshold;
    sk_sp<SkRuntimeEffect> fEffect;

    float fCutoff = 0.5;    // so we get something interested when we're not animated

    void onOnceBeforeDraw() override {
        const SkISize size = {256, 256};
        fThreshold = make_threshold(size);
        fBefore = make_shader(GetResourceAsImage("images/mandrill_256.png"), size);
        fAfter = make_shader(GetResourceAsImage("images/dog.jpg"), size);

        const char code[] = R"(
            in fragmentProcessor before_map;
            in fragmentProcessor after_map;
            in fragmentProcessor threshold_map;

            uniform float cutoff;
            uniform float slope;

            float smooth_cutoff(float x) {
                x = x * slope + (0.5 - slope * cutoff);
                return clamp(x, 0, 1);
            }

            void main(float2 xy, inout half4 color) {
                half4 before = sample(before_map, xy);
                half4 after = sample(after_map, xy);

                float m = smooth_cutoff(sample(threshold_map, xy).r);
                color = mix(before, after, half(m));
            }
        )";
        auto [effect, error] = SkRuntimeEffect::Make(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("threshold_rt"); }

    SkISize onISize() override { return {256, 256}; }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (canvas->getGrContext() == nullptr) {
            // until SkSL can handle child processors on the raster backend
            return DrawResult::kSkip;
        }

        struct {
            float cutoff, slope;
        } uni = {
            fCutoff, 10
        };
        sk_sp<SkData> data = SkData::MakeWithCopy(&uni, sizeof(uni));
        sk_sp<SkShader> children[] = { fBefore, fAfter, fThreshold };

        SkPaint paint;
        paint.setShader(fEffect->makeShader(data, children, SK_ARRAY_COUNT(children),
                                            nullptr, true));
        canvas->drawRect({0, 0, 256, 256}, paint);

        auto draw = [&](SkScalar x, SkScalar y, sk_sp<SkShader> shader) {
            paint.setShader(shader);
            canvas->save();
            canvas->translate(x, y);
            canvas->drawRect({0, 0, 256, 256}, paint);
            canvas->restore();
        };
        draw(256,   0, fThreshold);
        draw(  0, 256, fBefore);
        draw(256, 256, fAfter);

        return DrawResult::kOk;
    }

    bool onAnimate(double nanos) override {
        double t = sin(nanos / (1000 * 1000 * 1000));
        fCutoff = float(t + 1) * 0.55f - 0.05f;
        return true;
    }
};
DEF_GM(return new ThresholdRT;)
