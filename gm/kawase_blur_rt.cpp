/*
 * Copyright 2020 Google LLC
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
#include "include/effects/SkRuntimeEffect.h"
#include "tools/Resources.h"

class KawaseBlurFilter {
public:
    // Downsample scale factor used to improve performance
    static constexpr float kInputScale = 0.25f;
    // Downsample scale factor used to improve performance
    static constexpr float kInverseInputScale = 1.0f / kInputScale;
    // Maximum number of render passes
    static constexpr uint32_t kMaxPasses = 4;
    // To avoid downscaling artifacts, we interpolate the blurred fbo with the full composited
    // image, up to this radius.
    static constexpr float kMaxCrossFadeRadius = 30.0f;

    KawaseBlurFilter() {
        SkString blurString(R"(
            uniform shader src;
            uniform float in_inverseScale;
            uniform float2 in_blurOffset;

            half4 main(float2 xy) {
                float2 scaled_xy = float2(xy.x * in_inverseScale, xy.y * in_inverseScale);

                half4 c = src.eval(scaled_xy);
                c += src.eval(scaled_xy + float2( in_blurOffset.x,  in_blurOffset.y));
                c += src.eval(scaled_xy + float2( in_blurOffset.x, -in_blurOffset.y));
                c += src.eval(scaled_xy + float2(-in_blurOffset.x,  in_blurOffset.y));
                c += src.eval(scaled_xy + float2(-in_blurOffset.x, -in_blurOffset.y));

                return half4(c.rgb * 0.2, 1.0);
            }
        )");

        SkString mixString(R"(
            uniform shader in_blur;
            uniform shader in_original;
            uniform float in_inverseScale;
            uniform float in_mix;

            half4 main(float2 xy) {
                float2 scaled_xy = float2(xy.x * in_inverseScale, xy.y * in_inverseScale);

                half4 blurred = in_blur.eval(scaled_xy);
                half4 composition = in_original.eval(xy);
                return mix(composition, blurred, in_mix);
            }
        )");

        auto [blurEffect, error] = SkRuntimeEffect::MakeForShader(blurString);
        if (!blurEffect) {
            SkDEBUGFAILF("RuntimeShader error: %s\n", error.c_str());
        }
        fBlurEffect = std::move(blurEffect);

        auto [mixEffect, error2] = SkRuntimeEffect::MakeForShader(mixString);
        if (!mixEffect) {
            SkDEBUGFAILF("RuntimeShader error: %s\n", error2.c_str());
        }
        fMixEffect = std::move(mixEffect);
    }

    static sk_sp<SkSurface> MakeSurface(SkCanvas* canvas, const SkImageInfo& info) {
        if (sk_sp<SkSurface> surface = canvas->makeSurface(info)) {
            return surface;
        }
        // serialize-8888 returns null from makeSurface; fallback to a raster surface.
        return SkSurfaces::Raster(info);
    }

    void draw(SkCanvas* canvas, sk_sp<SkImage> input, int blurRadius) {
        // NOTE: this is only experimental and the current blur params cause points to be sampled
        // beyond the input blur radius.

        // Kawase is an approximation of Gaussian, but it behaves differently from it.
        // A radius transformation is required for approximating them, and also to introduce
        // non-integer steps, necessary to smoothly interpolate large radii.
        float tmpRadius = (float)blurRadius / 6.0f;
        float numberOfPasses = std::min(kMaxPasses, (uint32_t)ceil(tmpRadius));
        float radiusByPasses = tmpRadius / (float)numberOfPasses;

        SkImageInfo scaledInfo = SkImageInfo::MakeN32Premul((float)input->width() * kInputScale,
                                                            (float)input->height() * kInputScale);
        auto drawSurface = MakeSurface(canvas, scaledInfo);

        const float stepX = radiusByPasses;
        const float stepY = radiusByPasses;

        // start by drawing and downscaling and doing the first blur pass
        SkRuntimeShaderBuilder blurBuilder(fBlurEffect);
        blurBuilder.child("src") = input->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
        blurBuilder.uniform("in_inverseScale") = kInverseInputScale;
        blurBuilder.uniform("in_blurOffset") = SkV2{stepX * kInverseInputScale,
                                                    stepY * kInverseInputScale};
        SkPaint paint;
        paint.setShader(blurBuilder.makeShader());
        drawSurface->getCanvas()->drawIRect(scaledInfo.bounds(), paint);

        // DEBUG draw each of the stages
        canvas->save();
        canvas->drawImage(drawSurface->makeImageSnapshot(), input->width() / 4, 0,
                          SkSamplingOptions());
        canvas->translate(input->width() / 4, input->height() * 0.75);

        // And now we'll ping pong between our surfaces, to accumulate the result of various
        // offsets.
        auto lastDrawTarget = drawSurface;
        if (numberOfPasses > 1) {
            auto readSurface = drawSurface;
            drawSurface = MakeSurface(canvas, scaledInfo);

            for (auto i = 1; i < numberOfPasses; i++) {
                const float stepScale = (float)i * kInputScale;

                blurBuilder.child("src") = readSurface->makeImageSnapshot()->makeShader(
                        SkSamplingOptions(SkFilterMode::kLinear));
                blurBuilder.uniform("in_inverseScale") = 1.0f;
                blurBuilder.uniform("in_blurOffset") = SkV2{stepX * stepScale , stepY * stepScale};

                paint.setShader(blurBuilder.makeShader());
                drawSurface->getCanvas()->drawIRect(scaledInfo.bounds(), paint);

                // DEBUG draw each of the stages
                canvas->drawImage(drawSurface->makeImageSnapshot(), 0, 0, SkSamplingOptions());
                canvas->translate(0, input->height() * 0.75);

                // Swap buffers for next iteration
                auto tmp = drawSurface;
                drawSurface = readSurface;
                readSurface = tmp;
            }
            lastDrawTarget = readSurface;
        }

        // restore translations done for debug and offset
        canvas->restore();
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(input->width(), 0);

        // do the final composition and when we scale our blur up. It will be interpolated
        // with the larger composited texture to hide downscaling artifacts.
        SkRuntimeShaderBuilder mixBuilder(fMixEffect);
        mixBuilder.child("in_blur") = lastDrawTarget->makeImageSnapshot()->makeShader(
                SkSamplingOptions(SkFilterMode::kLinear));
        mixBuilder.child("in_original") =
                input->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
        mixBuilder.uniform("in_inverseScale") = kInputScale;
        mixBuilder.uniform("in_mix") = std::min(1.0f, (float)blurRadius / kMaxCrossFadeRadius);

        paint.setShader(mixBuilder.makeShader());
        canvas->drawIRect(input->bounds(), paint);
    }

private:
    sk_sp<SkRuntimeEffect> fBlurEffect;
    sk_sp<SkRuntimeEffect> fMixEffect;
};

class KawaseBlurRT : public skiagm::GM {
public:
    KawaseBlurRT() {}
    SkString getName() const override { return SkString("kawase_blur_rt"); }
    SkISize getISize() override { return {1280, 768}; }

    void onOnceBeforeDraw() override {
        fMandrill = GetResourceAsImage("images/mandrill_256.png");
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawImage(fMandrill, 0, 0);
        canvas->translate(256, 0);
        KawaseBlurFilter blurFilter;
        blurFilter.draw(canvas, fMandrill, 45);
        canvas->translate(512, 0);
        blurFilter.draw(canvas, fMandrill, 55);
    }

private:
    sk_sp<SkImage> fMandrill;
};
DEF_GM(return new KawaseBlurRT;)
