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
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkRandom.h"
#include "tools/Resources.h"

enum RT_Flags {
    kAnimate_RTFlag = 0x1,
    kBench_RTFlag   = 0x2,
};

class RuntimeShaderGM : public skiagm::GM {
public:
    RuntimeShaderGM(const char* name, SkISize size, const char* sksl, uint32_t flags = 0)
            : fName(name), fSize(size), fFlags(flags), fSkSL(sksl) {}

    void onOnceBeforeDraw() override {
        auto [effect, error] = SkRuntimeEffect::Make(fSkSL);
        if (!effect) {
            SkDebugf("RuntimeShader error: %s\n", error.c_str());
        }
        fEffect = std::move(effect);
    }

    bool runAsBench() const override { return SkToBool(fFlags & kBench_RTFlag); }
    SkString onShortName() override { return fName; }
    SkISize onISize() override { return fSize; }

    bool onAnimate(double nanos) override {
        fSecs = nanos / (1000 * 1000 * 1000);
        return SkToBool(fFlags & kAnimate_RTFlag);
    }

protected:
    SkString fName;
    SkISize  fSize;
    uint32_t fFlags;
    float    fSecs = 0.0f;

    SkString fSkSL;
    sk_sp<SkRuntimeEffect> fEffect;
};

class SimpleRT : public RuntimeShaderGM {
public:
    SimpleRT() : RuntimeShaderGM("runtime_shader", {512, 256}, R"(
        uniform half4 gColor;

        half4 main(float2 p) {
            return half4(p*(1.0/255), gColor.b, 1);
        }
    )", kBench_RTFlag) {}

    void onDraw(SkCanvas* canvas) override {
        SkRuntimeShaderBuilder builder(fEffect);

        SkMatrix localM;
        localM.setRotate(90, 128, 128);
        builder.uniform("gColor") = SkColor4f{1, 0, 0, 1};

        SkPaint p;
        p.setShader(builder.makeShader(&localM, true));
        canvas->drawRect({0, 0, 256, 256}, p);
    }
};
DEF_GM(return new SimpleRT;)

static sk_sp<SkShader> make_shader(sk_sp<SkImage> img, SkISize size) {
    SkMatrix scale = SkMatrix::Scale(size.width()  / (float)img->width(),
                                     size.height() / (float)img->height());
    return img->makeShader(SkSamplingOptions(), scale);
}

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

    return surf->makeImageSnapshot()->makeShader(SkSamplingOptions());
}

class ThresholdRT : public RuntimeShaderGM {
public:
    ThresholdRT() : RuntimeShaderGM("threshold_rt", {256, 256}, R"(
        uniform shader before_map;
        uniform shader after_map;
        uniform shader threshold_map;

        uniform float cutoff;
        uniform float slope;

        float smooth_cutoff(float x) {
            x = x * slope + (0.5 - slope * cutoff);
            return clamp(x, 0, 1);
        }

        half4 main(float2 xy) {
            half4 before = sample(before_map);
            half4 after = sample(after_map);

            float m = smooth_cutoff(sample(threshold_map).a);
            return mix(before, after, m);
        }
    )", kAnimate_RTFlag | kBench_RTFlag) {}

    sk_sp<SkShader> fBefore, fAfter, fThreshold;

    void onOnceBeforeDraw() override {
        const SkISize size = {256, 256};
        fThreshold = make_threshold(size);
        fBefore = make_shader(GetResourceAsImage("images/mandrill_256.png"), size);
        fAfter = make_shader(GetResourceAsImage("images/dog.jpg"), size);

        this->RuntimeShaderGM::onOnceBeforeDraw();
    }

    void onDraw(SkCanvas* canvas) override {
        SkRuntimeShaderBuilder builder(fEffect);

        builder.uniform("cutoff") = sin(fSecs) * 0.55f + 0.5f;
        builder.uniform("slope")  = 10.0f;

        builder.child("before_map")    = fBefore;
        builder.child("after_map")     = fAfter;
        builder.child("threshold_map") = fThreshold;

        SkPaint paint;
        paint.setShader(builder.makeShader(nullptr, true));
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
};
DEF_GM(return new ThresholdRT;)

class SpiralRT : public RuntimeShaderGM {
public:
    SpiralRT() : RuntimeShaderGM("spiral_rt", {512, 512}, R"(
        uniform float rad_scale;
        uniform float2 in_center;
        layout(srgb_unpremul) uniform float4 in_colors0;
        layout(srgb_unpremul) uniform float4 in_colors1;

        half4 main(float2 p) {
            float2 pp = p - in_center;
            float radius = length(pp);
            radius = sqrt(radius);
            float angle = atan(pp.y / pp.x);
            float t = (angle + 3.1415926/2) / (3.1415926);
            t += radius * rad_scale;
            t = fract(t);
            return in_colors0 * (1-t) + in_colors1 * t;
        }
    )", kAnimate_RTFlag | kBench_RTFlag) {}

    void onDraw(SkCanvas* canvas) override {
        SkRuntimeShaderBuilder builder(fEffect);

        builder.uniform("rad_scale")  = std::sin(fSecs * 0.5f + 2.0f) / 5;
        builder.uniform("in_center")  = SkV2{256, 256};
        builder.uniform("in_colors0") = SkV4{1, 0, 0, 1};
        builder.uniform("in_colors1") = SkV4{0, 1, 0, 1};

        SkPaint paint;
        paint.setShader(builder.makeShader(nullptr, true));
        canvas->drawRect({0, 0, 512, 512}, paint);
    }
};
DEF_GM(return new SpiralRT;)

class ColorCubeRT : public RuntimeShaderGM {
public:
    ColorCubeRT() : RuntimeShaderGM("color_cube_rt", {512, 512}, R"(
        uniform shader input;
        uniform shader color_cube;

        uniform float rg_scale;
        uniform float rg_bias;
        uniform float b_scale;
        uniform float inv_size;

        half4 main(float2 xy) {
            float4 c = unpremul(sample(input));

            // Map to cube coords:
            float3 cubeCoords = float3(c.rg * rg_scale + rg_bias, c.b * b_scale);

            // Compute slice coordinate
            float2 coords1 = float2((floor(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);
            float2 coords2 = float2(( ceil(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);

            // Two bilinear fetches, plus a manual lerp for the third axis:
            half4 color = mix(sample(color_cube, coords1), sample(color_cube, coords2),
                              fract(cubeCoords.b));

            // Premul again
            color.rgb *= color.a;

            return color;
        }
    )") {}

    sk_sp<SkImage> fMandrill, fMandrillSepia, fIdentityCube, fSepiaCube;

    void onOnceBeforeDraw() override {
        fMandrill      = GetResourceAsImage("images/mandrill_256.png");
        fMandrillSepia = GetResourceAsImage("images/mandrill_sepia.png");
        fIdentityCube  = GetResourceAsImage("images/lut_identity.png");
        fSepiaCube     = GetResourceAsImage("images/lut_sepia.png");

        this->RuntimeShaderGM::onOnceBeforeDraw();
    }

    void onDraw(SkCanvas* canvas) override {
        SkRuntimeShaderBuilder builder(fEffect);

        // First we draw the unmodified image, and a copy that was sepia-toned in Photoshop:
        canvas->drawImage(fMandrill,      0,   0);
        canvas->drawImage(fMandrillSepia, 0, 256);

        // LUT dimensions should be (kSize^2, kSize)
        constexpr float kSize = 16.0f;

        const SkSamplingOptions sampling(SkFilterMode::kLinear);

        builder.uniform("rg_scale")     = (kSize - 1) / kSize;
        builder.uniform("rg_bias")      = 0.5f / kSize;
        builder.uniform("b_scale")      = kSize - 1;
        builder.uniform("inv_size")     = 1.0f / kSize;

        builder.child("input")        = fMandrill->makeShader(sampling);

        SkPaint paint;

        // TODO: Should we add SkImage::makeNormalizedShader() to handle this automatically?
        SkMatrix normalize = SkMatrix::Scale(1.0f / (kSize * kSize), 1.0f / kSize);

        // Now draw the image with an identity color cube - it should look like the original
        builder.child("color_cube") = fIdentityCube->makeShader(sampling, normalize);
        paint.setShader(builder.makeShader(nullptr, true));
        canvas->translate(256, 0);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);

        // ... and with a sepia-tone color cube. This should match the sepia-toned image.
        builder.child("color_cube") = fSepiaCube->makeShader(sampling, normalize);
        paint.setShader(builder.makeShader(nullptr, true));
        canvas->translate(0, 256);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);
    }
};
DEF_GM(return new ColorCubeRT;)

class DefaultColorRT : public RuntimeShaderGM {
public:
    // This test also *explicitly* doesn't include coords in main's parameter list, to test that
    // runtime shaders work without them being declared (when they're not used).
    DefaultColorRT() : RuntimeShaderGM("default_color_rt", {512, 256}, R"(
        uniform shader input;
        half4 main() {
            return sample(input);
        }
    )") {}

    sk_sp<SkImage> fMandrill;

    void onOnceBeforeDraw() override {
        fMandrill      = GetResourceAsImage("images/mandrill_256.png");
        this->RuntimeShaderGM::onOnceBeforeDraw();
    }

    void onDraw(SkCanvas* canvas) override {
        SkRuntimeShaderBuilder builder(fEffect);

        // First, we leave the child as null, so sampling it returns the default (paint) color
        SkPaint paint;
        paint.setColor4f({ 0.25f, 0.75f, 0.75f, 1.0f });
        paint.setShader(builder.makeShader(nullptr, false));
        canvas->drawRect({ 0, 0, 256, 256 }, paint);

        // Now we bind an image shader as the child. This (by convention) scales by the paint alpha
        builder.child("input") = fMandrill->makeShader(SkSamplingOptions());
        paint.setColor4f({ 1.0f, 1.0f, 1.0f, 0.5f });
        paint.setShader(builder.makeShader(nullptr, false));
        canvas->translate(256, 0);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);

    }
};
DEF_GM(return new DefaultColorRT;)

// Emits coverage for a general superellipse defined by the boundary:
//
//   x^m + y^n == 1
//
// Where x and y are normalized coordinates ranging from -1..+1 inside the squircle's bounding box.
//
// See: https://en.wikipedia.org/wiki/Superellipse#Generalizations
class ClipSquircle : public RuntimeShaderGM {
public:
    ClipSquircle() : RuntimeShaderGM("clip_squircle", {512, 256}, R"(
        uniform float2 exponentsMinus1;
        uniform float2x2 derivatives;
        half4 main(float2 xy) {
            xy = abs(xy);
            float2 expMinus1 = pow(xy, exponentsMinus1);
            float f = dot(expMinus1, xy) - 1;  // f = x^m + y^n - 1
            float2 grad = expMinus1 * derivatives;
            float fwidth = abs(grad.x) + abs(grad.y);
            return half4(saturate(.5 - f/fwidth)); // Approx coverage by riding the gradient to f=0.
        }
    )") {}

    void onDraw(SkCanvas* canvas) override {
        SkRect squircle = SkRect::MakeXYWH(7, 3, 300, 185.41f);
        float m = 5.32f;
        float n = 3.14f;

        canvas->save();
        canvas->rotate(9.2f, 5, 185);

        SkRuntimeShaderBuilder builder(fEffect);
        builder.uniform("exponentsMinus1") = SkV2{m - 1, n - 1};

        // Calculate a 2x2 "derivatives" matrix that the shader will use to find the gradient.
        //
        //     f = s^m + t^n - 1   [s,t are "squircle" coordinates in normalized -1..+1 space]
        //
        //     gradient = [df/dx  df/dy] = [ms^(m-1)  nt^(n-1)] * |ds/dx  ds/dy|
        //                                                        |dt/dx  dt/dy|
        //
        //              = [s^(m-1)  t^(n-1)] * |m  0| * |ds/dx  ds/dy|
        //                                     |0  n|   |dt/dx  dt/dy|
        //
        //              = [s^(m-1)  t^(n-1)] * |2m/squircleWidth   0| * mat2x2(canvasMatrix)^-1
        //                                     |0  2n/squricleHeight|
        //
        //              = [s^(m-1)  t^(n-1)] * "derivatives"
        //
        const SkMatrix& M = canvas->getTotalMatrix();
        float a=M.getScaleX(), b=M.getSkewX(), c=M.getSkewY(), d=M.getScaleY();
        float determinantTimesHalf = (a*d - b*c) * .5f;
        float dx = m / (squircle.width() * determinantTimesHalf);
        float dy = n / (squircle.height() * determinantTimesHalf);
        builder.uniform("derivatives") = SkV4{d*dx, -c*dy, -b*dx, a*dy};

        SkMatrix squircleToLocal;
        squircleToLocal.setScaleTranslate(squircle.width()*.5f, squircle.height()*.5f,
                                          squircle.centerX(), squircle.centerY());
        canvas->clipShader(builder.makeShader(&squircleToLocal, false));
        canvas->clear(SkColorSetARGB(255, 144, 123, 189));

        canvas->restore();
    }
};
DEF_GM(return new ClipSquircle;)

DEF_SIMPLE_GM(child_sampling_rt, canvas, 256,256) {
    static constexpr char scale[] =
        "uniform shader child;"
        "half4 main(float2 xy) {"
        "    return sample(child, xy*0.1);"
        "}";

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(1);

    auto surf = SkSurface::MakeRasterN32Premul(100,100);
    surf->getCanvas()->drawLine(0, 0, 100, 100, p);
    auto shader = surf->makeImageSnapshot()->makeShader(SkSamplingOptions(SkFilterMode::kLinear));

    SkRuntimeShaderBuilder builder(SkRuntimeEffect::Make(SkString(scale)).effect);
    builder.child("child") = shader;
    p.setShader(builder.makeShader(nullptr, false));

    canvas->drawPaint(p);
}
