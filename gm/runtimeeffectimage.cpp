/*
 * Copyright 2021 Google LLC
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
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkRandom.h"
#include "tools/Resources.h"

class RuntimeEffectImage : public skiagm::GM {
public:
    RuntimeEffectImage() = default;

    SkString onShortName() override { return SkString("runtime_effect_image"); }

    SkISize onISize() override { return {800, 200}; }

    void onOnceBeforeDraw() override {
        SkString sksl(R"(
            uniform shader child;
            uniform half gAlphaType; // 0 is premul, non-zero unpremul.

            half4 main(float2 p) {
                half r = fract(p.x/20);
                half g = fract(p.y/20);
                half b = fract((p.x + 5)/5);
                half a = min(distance(p, vec2(50, 50))/100, 1);
                half4 result = half4(r, g, b, a);
                result *= sample(child);
                if (gAlphaType == 0) {
                   result.rgb *= a;
                }
                return result;
            }
        )");
        auto [effect, error] = SkRuntimeEffect::Make(sksl);
        if (!effect) {
            SkDebugf("RuntimeShader error: %s\n", error.c_str());
        }
        fEffect = std::move(effect);
    }
    void onDraw(SkCanvas* canvas) override {
        GrRecordingContext* rc = canvas->recordingContext();
        SkRuntimeShaderBuilder builder(fEffect);
        SkImageInfo info = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        sk_sp<SkImage> image;

        SkSamplingOptions nn{SkFilterMode::kNearest};
        SkSamplingOptions mm{SkFilterMode::kLinear, SkMipmapMode::kNearest};
        auto white = SkShaders::Color(SK_ColorWHITE);
        SkColor gradColors[] = {SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN, SK_ColorRED};
        auto gradient = SkGradientShader::MakeSweep(50, 50,
                                                    gradColors,
                                                    /*pos*/ nullptr,
                                                    SK_ARRAY_COUNT(gradColors));

        builder.uniform("gAlphaType") = 0.f;
        builder.child("child") = white;
        canvas->translate(5, 5);
        for (const auto& vm : {SkMatrix::Scale(1.5f, 1.5f), SkMatrix::Scale(0.3, 0.3f)}) {
            canvas->save();
            canvas->concat(vm);

            // basic
            image = builder.makeImage(rc, nullptr, info, false);
            canvas->drawImage(image, 0, 0, nn);
            canvas->translate(105, 0);

            // local matrix
            SkMatrix lm = SkMatrix::RotateDeg(45, {50, 50});
            lm.preTranslate(10, 10);
            image = builder.makeImage(rc, &lm, info, false);
            canvas->drawImage(image, 0, 0, nn);
            canvas->translate(105, 0);

            // unpremul (use uniform to make the output be unpremul so we don't get double premul).
            builder.uniform("gAlphaType") = 1.f;
            image = builder.makeImage(rc,
                                      nullptr,
                                      info.makeAlphaType(kUnpremul_SkAlphaType),
                                      false);
            builder.uniform("gAlphaType") = 0.f;
            canvas->drawImage(image, 0, 0, nn);
            canvas->translate(105, 0);

            sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB()->makeColorSpin();
            image = builder.makeImage(rc, nullptr, info.makeColorSpace(std::move(cs)), false);
            canvas->drawImage(image, 0, 0, nn);
            canvas->translate(105, 0);

            // mipmapped and different child
            builder.child("child") = gradient;
            image = builder.makeImage(rc, nullptr, info, true);
            builder.child("child") = white;
            canvas->drawImage(image, 0, 0, mm);
            canvas->translate(105, 0);

            canvas->restore();
            canvas->translate(0, 105*vm.getScaleY());
        }
    }
private:
    sk_sp<SkRuntimeEffect> fEffect;
};
DEF_GM(return new RuntimeEffectImage;)
