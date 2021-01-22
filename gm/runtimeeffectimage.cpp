/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

class RuntimeEffectImage : public skiagm::GM {
public:
    RuntimeEffectImage() = default;

    SkString onShortName() override { return SkString("runtime_effect_image"); }

    SkISize onISize() override { return {800, 200}; }

protected:
    void onOnceBeforeDraw() override {
        SkString sksl(R"(
            uniform shader child;
            uniform half gAlphaType; // 0 is premul, non-zero unpremul.

            half4 main(float2 p) {
                half r = fract(p.x/20);
                half g = fract(p.y/20);
                half b = fract((p.x + 5)/10);
                half a = min(distance(p, vec2(50, 50))/50 + 0.3, 1);
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

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        GrRecordingContext* rc = canvas->recordingContext();
        SkRuntimeShaderBuilder builder(fEffect);
        SkImageInfo info = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        SkSamplingOptions nn{SkFilterMode::kNearest};
        SkSamplingOptions mm{SkFilterMode::kLinear, SkMipmapMode::kNearest};
        auto whiteShader = SkShaders::Color(SK_ColorWHITE);
        auto imageForShader = GetResourceAsImage("images/ducky.jpg");
        if (!imageForShader) {
            *errorMsg = "Could not load image";
            return DrawResult::kFail;
        }
        auto imageShader = imageForShader->makeShader(SkTileMode::kRepeat,
                                                      SkTileMode::kRepeat,
                                                      SkSamplingOptions(SkFilterMode::kLinear),
                                                      SkMatrix::Scale(0.2f, 0.2f));

        builder.uniform("gAlphaType") = 0.f;
        builder.child("child") = whiteShader;
        ToolUtils::draw_checkerboard(canvas, SK_ColorWHITE, SK_ColorLTGRAY, 50);
        canvas->translate(5, 5);
        SkPaint aaPaint;
        aaPaint.setAntiAlias(true);
        for (const auto& vm : {SkMatrix::Scale(1.5f, 1.5f), SkMatrix::Scale(0.3f, 0.3f)}) {
            canvas->save();
            canvas->concat(vm);

            sk_sp<SkImage> image;

            // basic
            image = builder.makeImage(rc, nullptr, info, false);
            canvas->drawImage(image, 0, 0, nn, &aaPaint);
            canvas->translate(105, 0);

            // local matrix
            SkMatrix lm = SkMatrix::RotateDeg(45, {50, 50});
            lm.preTranslate(10, 10);
            image = builder.makeImage(rc, &lm, info, false);
            canvas->drawImage(image, 0, 0, nn, &aaPaint);
            canvas->translate(105, 0);

            // unpremul
            if (rc) {
                // use a uniform to make the effect output be unpremul so it looks the same as the
                // premul case when drawn to the canvas.
                builder.uniform("gAlphaType") = 1.f;
                image = builder.makeImage(rc,
                                          nullptr,
                                          info.makeAlphaType(kUnpremul_SkAlphaType),
                                          false);
                builder.uniform("gAlphaType") = 0.f;
                canvas->drawImage(image, 0, 0, nn, &aaPaint);
                canvas->translate(105, 0);
            } else {
                // CPU doesn't yet support making kUnpremul images. Just draw the basic one again.
                image = builder.makeImage(nullptr, nullptr, info, false);
                canvas->drawImage(image, 0, 0, nn, &aaPaint);
                canvas->translate(105, 0);
            }

            // color space
            sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB()->makeColorSpin();
            image = builder.makeImage(rc, nullptr, info.makeColorSpace(std::move(cs)), false);
            canvas->drawImage(image, 0, 0, nn, &aaPaint);
            canvas->translate(105, 0);

            // mipmapped and different child
            builder.child("child") = imageShader;
            image = builder.makeImage(rc, nullptr, info, true);
            builder.child("child") = whiteShader;
            canvas->drawImage(image, 0, 0, mm, &aaPaint);
            canvas->translate(105, 0);

            canvas->restore();
            canvas->translate(0, 105*vm.getScaleY());
        }
        return DrawResult::kOk;
    }

private:
    sk_sp<SkRuntimeEffect> fEffect;
};
DEF_GM(return new RuntimeEffectImage;)
