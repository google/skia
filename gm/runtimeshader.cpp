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
    SkMatrix scale = SkMatrix::Scale(size.width()  / (float)img->width(),
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
            in shader before_map;
            in shader after_map;
            in shader threshold_map;

            uniform float cutoff;
            uniform float slope;

            float smooth_cutoff(float x) {
                x = x * slope + (0.5 - slope * cutoff);
                return clamp(x, 0, 1);
            }

            void main(float2 xy, inout half4 color) {
                half4 before = sample(before_map, xy);
                half4 after = sample(after_map, xy);

                float m = smooth_cutoff(sample(threshold_map, xy).a);
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

    void onDraw(SkCanvas* canvas) override {
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
    }

    bool onAnimate(double nanos) override {
        double t = sin(nanos / (1000 * 1000 * 1000));
        fCutoff = float(t + 1) * 0.55f - 0.05f;
        return true;
    }
};
DEF_GM(return new ThresholdRT;)

class SpiralRT : public skiagm::GM {
    sk_sp<SkRuntimeEffect> fEffect;
    float                  fSecs = 4;    // so we get something interested when we're not animated

    void onOnceBeforeDraw() override {
        const char code[] = R"(
            uniform float rad_scale;
            uniform float2 in_center;
            layout(srgb_unpremul) uniform float4 in_colors0;
            layout(srgb_unpremul) uniform float4 in_colors1;

            void main(float2 p, inout half4 color) {
                float2 pp = p - in_center;
                float radius = length(pp);
                radius = sqrt(radius);
                float angle = atan(pp.y / pp.x);
                float t = (angle + 3.1415926/2) / (3.1415926);
                t += radius * rad_scale;
                t = fract(t);
                float4 m = in_colors0 * (1-t) + in_colors1 * t;
                color = half4(m);
            }
        )";
        auto [effect, error] = SkRuntimeEffect::Make(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    bool runAsBench() const override { return true; }

    SkString onShortName() override { return SkString("spiral_rt"); }

    SkISize onISize() override { return {512, 512}; }

    void onDraw(SkCanvas* canvas) override {
        struct {
            float rad_scale;
            SkV2  in_center;
            SkV4  in_colors0;
            SkV4  in_colors1;
        } uni {
            std::sin(fSecs / 2) / 5,
            {256, 256},       // center
            {1, 0, 0, 1},     // color0
            {0, 1, 0, 1},    // color1
        };

        SkPaint paint;
        paint.setShader(fEffect->makeShader(SkData::MakeWithCopy(&uni, sizeof(uni)),
                                            nullptr, 0, nullptr, true));
        canvas->drawRect({0, 0, 512, 512}, paint);
    }

    bool onAnimate(double nanos) override {
        fSecs = nanos / (1000 * 1000 * 1000);
        return true;
    }
};
DEF_GM(return new SpiralRT;)

class ColorCubeRT : public skiagm::GM {
    sk_sp<SkImage> fMandrill, fMandrillSepia, fIdentityCube, fSepiaCube;
    sk_sp<SkRuntimeEffect> fEffect;

    void onOnceBeforeDraw() override {
        fMandrill      = GetResourceAsImage("images/mandrill_256.png");
        fMandrillSepia = GetResourceAsImage("images/mandrill_sepia.png");
        fIdentityCube  = GetResourceAsImage("images/lut_identity.png");
        fSepiaCube     = GetResourceAsImage("images/lut_sepia.png");

        const char code[] = R"(
            in shader input;
            in shader color_cube;

            uniform float rg_scale;
            uniform float rg_bias;
            uniform float b_scale;
            uniform float inv_size;

            void main(float2 xy, inout half4 color) {
                float4 c = float4(unpremul(sample(input, xy)));

                // Map to cube coords:
                float3 cubeCoords = float3(c.rg * rg_scale + rg_bias, c.b * b_scale);

                // Compute slice coordinate
                float2 coords1 = float2((floor(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);
                float2 coords2 = float2(( ceil(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);

                // Two bilinear fetches, plus a manual lerp for the third axis:
                color = mix(sample(color_cube, coords1), sample(color_cube, coords2),
                            half(fract(cubeCoords.b)));

                // Premul again
                color.rgb *= color.a;
            }
        )";
        auto [effect, error] = SkRuntimeEffect::Make(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    SkString onShortName() override { return SkString("color_cube_rt"); }

    SkISize onISize() override { return {512, 512}; }

    void onDraw(SkCanvas* canvas) override {
        // First we draw the unmodified image, and a copy that was sepia-toned in Photoshop:
        canvas->drawImage(fMandrill,      0,   0);
        canvas->drawImage(fMandrillSepia, 0, 256);

        // LUT dimensions should be (kSize^2, kSize)
        constexpr float kSize = 16.0f;

        SkRuntimeShaderBuilder builder(fEffect);
        builder.input("rg_scale")     = (kSize - 1) / kSize;
        builder.input("rg_bias")      = 0.5f / kSize;
        builder.input("b_scale")      = kSize - 1;
        builder.input("inv_size")     = 1.0f / kSize;

        builder.child("input")      = fMandrill->makeShader();

        // TODO: Move filter quality to the shader itself. We need to enforce at least kLow here
        // so that we bilerp the color cube image.
        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);

        // TODO: Should we add SkImage::makeNormalizedShader() to handle this automatically?
        SkMatrix normalize = SkMatrix::Scale(1.0f / (kSize * kSize), 1.0f / kSize);

        // Now draw the image with an identity color cube - it should look like the original
        builder.child("color_cube") = fIdentityCube->makeShader(normalize);
        paint.setShader(builder.makeShader(nullptr, true));
        canvas->translate(256, 0);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);

        // ... and with a sepia-tone color cube. This should match the sepia-toned image.
        builder.child("color_cube") = fSepiaCube->makeShader(normalize);
        paint.setShader(builder.makeShader(nullptr, true));
        canvas->translate(0, 256);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);
    }
};
DEF_GM(return new ColorCubeRT;)
