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
#include "include/effects/SkRuntimeEffect.h"
#include "tools/Resources.h"

const char* gProg = R"(
    uniform half4 gColor;

    void main(float x, float y, inout half4 color) {
        color = half4(half(x)*(1.0/255), half(y)*(1.0/255), gColor.b, 1);
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
    auto info = SkImageInfo::Make(size.width(), size.height(), kGray_8_SkColorType,
                                  kOpaque_SkAlphaType);
    auto surf = SkSurface::MakeRaster(info);

    SkColor colors[] = {SK_ColorWHITE, SK_ColorBLACK};
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(SkGradientShader::MakeRadial({0,0}, 100, colors, nullptr, 2, SkTileMode::kClamp));

    SkRandom rand;
    for (int i = 0; i < 15; ++i) {
        SkScalar x = rand.nextS() * size.width();
        SkScalar y = rand.nextS() * size.height();
        surf->getCanvas()->drawCircle(x, y, 100, paint);
    }

    return surf->makeImageSnapshot()->makeShader();
}

class ThresholdRT : public skiagm::GM {
    sk_sp<SkShader> fBefore, fAfter, fThreshold;
    sk_sp<SkRuntimeEffect> fEffect;

    float fCutoff = 0;

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

            void main(float x, float y, inout half4 color) {
                half4 before = sample(before_map);
                half4 after = sample(after_map);

                float m = sample(threshold_map).r > cutoff ? 1.0 : 0.0;
                color = sample(threshold_map);
//                color = mix(before, after, half(m));
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

    void onDraw(SkCanvas* canvas) override {
        struct {
            float cutoff;
        } uni = {
            fCutoff
        };
        sk_sp<SkData> data = SkData::MakeWithCopy(&uni, sizeof(uni));
        sk_sp<SkShader> children[] = { fBefore, fAfter, fThreshold };

        SkPaint paint;
        paint.setShader(fEffect->makeShader(data, children, SK_ARRAY_COUNT(children),
                                            nullptr, true));
        canvas->drawRect({0, 0, 256, 256}, paint);

        paint.setShader(fThreshold);
        canvas->translate(256, 0);
        canvas->drawRect({0, 0, 256, 256}, paint);
    }

    bool onAnimate(double nanos) override {
        double t = sin(nanos * 1000 * 1000);
        fCutoff = float(t + 1) * 0.5f;
        return true;
    }
};
DEF_GM(return new ThresholdRT;)
