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
#include "include/core/SkRRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkRandom.h"
#include "tools/Resources.h"

enum RT_Flags {
    kAnimate_RTFlag     = 0x1,
    kBench_RTFlag       = 0x2,
    kColorFilter_RTFlag = 0x4,
};

class RuntimeShaderGM : public skiagm::GM {
public:
    RuntimeShaderGM(const char* name, SkISize size, const char* sksl, uint32_t flags = 0)
            : fName(name), fSize(size), fFlags(flags), fSkSL(sksl) {}

    void onOnceBeforeDraw() override {
        auto [effect, error] = (fFlags & kColorFilter_RTFlag)
                                       ? SkRuntimeEffect::MakeForColorFilter(fSkSL)
                                       : SkRuntimeEffect::MakeForShader(fSkSL);
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
            half4 before = before_map.eval(xy);
            half4 after = after_map.eval(xy);

            float m = smooth_cutoff(threshold_map.eval(xy).a);
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

// Test case for sampling with both unmodified input coordinates, and explicit coordinates.
// The first version of skbug.com/11869 suffered a bug where all samples of a child were treated
// as pass-through if *at least one* used the unmodified coordinates. This was detected & tracked
// in b/181092919. This GM is similar, and demonstrates the bug before the fix was applied.
class UnsharpRT : public RuntimeShaderGM {
public:
    UnsharpRT() : RuntimeShaderGM("unsharp_rt", {512, 256}, R"(
        uniform shader child;
        half4 main(float2 xy) {
            half4 c = child.eval(xy) * 5;
            c -= child.eval(xy + float2( 1,  0));
            c -= child.eval(xy + float2(-1,  0));
            c -= child.eval(xy + float2( 0,  1));
            c -= child.eval(xy + float2( 0, -1));
            return c;
        }
    )") {}

    sk_sp<SkImage> fMandrill;

    void onOnceBeforeDraw() override {
        fMandrill      = GetResourceAsImage("images/mandrill_256.png");
        this->RuntimeShaderGM::onOnceBeforeDraw();
    }

    void onDraw(SkCanvas* canvas) override {
        // First we draw the unmodified image
        canvas->drawImage(fMandrill,      0,   0);

        // Now draw the image with our unsharp mask applied
        SkRuntimeShaderBuilder builder(fEffect);
        const SkSamplingOptions sampling(SkFilterMode::kNearest);
        builder.child("child") = fMandrill->makeShader(sampling);

        SkPaint paint;
        paint.setShader(builder.makeShader(nullptr, true));
        canvas->translate(256, 0);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);
    }
};
DEF_GM(return new UnsharpRT;)

class ColorCubeRT : public RuntimeShaderGM {
public:
    ColorCubeRT() : RuntimeShaderGM("color_cube_rt", {512, 512}, R"(
        uniform shader child;
        uniform shader color_cube;

        uniform float rg_scale;
        uniform float rg_bias;
        uniform float b_scale;
        uniform float inv_size;

        half4 main(float2 xy) {
            float4 c = unpremul(child.eval(xy));

            // Map to cube coords:
            float3 cubeCoords = float3(c.rg * rg_scale + rg_bias, c.b * b_scale);

            // Compute slice coordinate
            float2 coords1 = float2((floor(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);
            float2 coords2 = float2(( ceil(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);

            // Two bilinear fetches, plus a manual lerp for the third axis:
            half4 color = mix(color_cube.eval(coords1), color_cube.eval(coords2),
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

        builder.child("child")        = fMandrill->makeShader(sampling);

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

// Same as above, but demonstrating how to implement this as a runtime color filter (that samples
// a shader child for the LUT).
class ColorCubeColorFilterRT : public RuntimeShaderGM {
public:
    ColorCubeColorFilterRT() : RuntimeShaderGM("color_cube_cf_rt", {512, 512}, R"(
        uniform shader color_cube;

        uniform float rg_scale;
        uniform float rg_bias;
        uniform float b_scale;
        uniform float inv_size;

        half4 main(half4 inColor) {
            float4 c = unpremul(inColor);

            // Map to cube coords:
            float3 cubeCoords = float3(c.rg * rg_scale + rg_bias, c.b * b_scale);

            // Compute slice coordinate
            float2 coords1 = float2((floor(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);
            float2 coords2 = float2(( ceil(cubeCoords.b) + cubeCoords.r) * inv_size, cubeCoords.g);

            // Two bilinear fetches, plus a manual lerp for the third axis:
            half4 color = mix(color_cube.eval(coords1), color_cube.eval(coords2),
                              fract(cubeCoords.b));

            // Premul again
            color.rgb *= color.a;

            return color;
        }
    )", kColorFilter_RTFlag) {}

    sk_sp<SkImage> fMandrill, fMandrillSepia, fIdentityCube, fSepiaCube;

    void onOnceBeforeDraw() override {
        fMandrill      = GetResourceAsImage("images/mandrill_256.png");
        fMandrillSepia = GetResourceAsImage("images/mandrill_sepia.png");
        fIdentityCube  = GetResourceAsImage("images/lut_identity.png");
        fSepiaCube     = GetResourceAsImage("images/lut_sepia.png");

        this->RuntimeShaderGM::onOnceBeforeDraw();
    }

    void onDraw(SkCanvas* canvas) override {
        // First we draw the unmodified image, and a copy that was sepia-toned in Photoshop:
        canvas->drawImage(fMandrill,      0,   0);
        canvas->drawImage(fMandrillSepia, 0, 256);

        // LUT dimensions should be (kSize^2, kSize)
        constexpr float kSize = 16.0f;

        const SkSamplingOptions sampling(SkFilterMode::kLinear);

        float uniforms[] = {
                (kSize - 1) / kSize,  // rg_scale
                0.5f / kSize,         // rg_bias
                kSize - 1,            // b_scale
                1.0f / kSize,         // inv_size
        };

        SkPaint paint;

        // TODO: Should we add SkImage::makeNormalizedShader() to handle this automatically?
        SkMatrix normalize = SkMatrix::Scale(1.0f / (kSize * kSize), 1.0f / kSize);

        // Now draw the image with an identity color cube - it should look like the original
        SkRuntimeEffect::ChildPtr children[] = {fIdentityCube->makeShader(sampling, normalize)};
        paint.setColorFilter(fEffect->makeColorFilter(
                SkData::MakeWithCopy(uniforms, sizeof(uniforms)), SkMakeSpan(children)));
        canvas->drawImage(fMandrill, 256, 0, sampling, &paint);

        // ... and with a sepia-tone color cube. This should match the sepia-toned image.
        children[0] = fSepiaCube->makeShader(sampling, normalize);
        paint.setColorFilter(fEffect->makeColorFilter(
                SkData::MakeWithCopy(uniforms, sizeof(uniforms)), SkMakeSpan(children)));
        canvas->drawImage(fMandrill, 256, 256, sampling, &paint);
    }
};
DEF_GM(return new ColorCubeColorFilterRT;)

class DefaultColorRT : public RuntimeShaderGM {
public:
    DefaultColorRT() : RuntimeShaderGM("default_color_rt", {512, 256}, R"(
        uniform shader child;
        half4 main(float2 xy) {
            return child.eval(xy);
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
        builder.child("child") = fMandrill->makeShader(SkSamplingOptions());
        paint.setColor4f({ 1.0f, 1.0f, 1.0f, 0.5f });
        paint.setShader(builder.makeShader(nullptr, false));
        canvas->translate(256, 0);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);

    }
};
DEF_GM(return new DefaultColorRT;)

// Emits coverage for a rounded rectangle whose corners are superellipses defined by the boundary:
//
//   x^n + y^n == 1
//
// Where x and y are normalized, clamped coordinates ranging from 0..1 inside the nearest corner's
// bounding box.
//
// See: https://en.wikipedia.org/wiki/Superellipse
class ClipSuperRRect : public RuntimeShaderGM {
public:
    ClipSuperRRect(const char* name, float power) : RuntimeShaderGM(name, {500, 500}, R"(
        uniform float power_minus1;
        uniform float2 stretch_factor;
        uniform float2x2 derivatives;
        half4 main(float2 xy) {
            xy = max(abs(xy) + stretch_factor, 0);
            float2 exp_minus1 = pow(xy, power_minus1.xx);  // If power == 3.5: xy * xy * sqrt(xy)
            float f = dot(exp_minus1, xy) - 1;  // f = x^n + y^n - 1
            float2 grad = exp_minus1 * derivatives;
            float fwidth = abs(grad.x) + abs(grad.y) + 1e-12;  // 1e-12 to avoid a divide by zero.
            return half4(saturate(.5 - f/fwidth)); // Approx coverage by riding the gradient to f=0.
        }
    )"), fPower(power) {}

    void drawSuperRRect(SkCanvas* canvas, const SkRect& superRRect, float radX, float radY,
                        SkColor color) {
        SkPaint paint;
        paint.setColor(color);

        if (fPower == 2) {
            // Draw a normal round rect for the sake of testing.
            SkRRect rrect = SkRRect::MakeRectXY(superRRect, radX, radY);
            paint.setAntiAlias(true);
            canvas->drawRRect(rrect, paint);
            return;
        }

        SkRuntimeShaderBuilder builder(fEffect);
        builder.uniform("power_minus1") = fPower - 1;

        // Size the corners such that the "apex" of our "super" rounded corner is in the same
        // location that the apex of a circular rounded corner would be with the given radii. We
        // define the apex as the point on the rounded corner that is 45 degrees between the
        // horizontal and vertical edges.
        float scale = (1 - SK_ScalarRoot2Over2) / (1 - exp2f(-1/fPower));
        float cornerWidth = radX * scale;
        float cornerHeight = radY * scale;
        cornerWidth = std::min(cornerWidth, superRRect.width() * .5f);
        cornerHeight = std::min(cornerHeight, superRRect.height() * .5f);
        // The stretch factor controls how long the flat edge should be between rounded corners.
        builder.uniform("stretch_factor") = SkV2{1 - superRRect.width()*.5f / cornerWidth,
                                                 1 - superRRect.height()*.5f / cornerHeight};

        // Calculate a 2x2 "derivatives" matrix that the shader will use to find the gradient.
        //
        //     f = s^n + t^n - 1   [s,t are "super" rounded corner coords in normalized 0..1 space]
        //
        //     gradient = [df/dx  df/dy] = [ns^(n-1)  nt^(n-1)] * |ds/dx  ds/dy|
        //                                                        |dt/dx  dt/dy|
        //
        //              = [s^(n-1)  t^(n-1)] * |n  0| * |ds/dx  ds/dy|
        //                                     |0  n|   |dt/dx  dt/dy|
        //
        //              = [s^(n-1)  t^(n-1)] * |2n/cornerWidth   0| * mat2x2(canvasMatrix)^-1
        //                                     |0  2n/cornerHeight|
        //
        //              = [s^(n-1)  t^(n-1)] * "derivatives"
        //
        const SkMatrix& M = canvas->getTotalMatrix();
        float a=M.getScaleX(), b=M.getSkewX(), c=M.getSkewY(), d=M.getScaleY();
        float determinant = a*d - b*c;
        float dx = fPower / (cornerWidth * determinant);
        float dy = fPower / (cornerHeight * determinant);
        builder.uniform("derivatives") = SkV4{d*dx, -c*dy, -b*dx, a*dy};

        // This matrix will be inverted by the effect system, giving a matrix that converts local
        // coordinates to (almost) coner coordinates. To get the rest of the way to the nearest
        // corner's space, the shader will have to take the absolute value, add the stretch_factor,
        // then clamp above zero.
        SkMatrix cornerToLocal;
        cornerToLocal.setScaleTranslate(cornerWidth, cornerHeight, superRRect.centerX(),
                                        superRRect.centerY());
        canvas->clipShader(builder.makeShader(&cornerToLocal, false));

        // Bloat the outer edges of the rect we will draw so it contains all the antialiased pixels.
        // Bloat by a full pixel instead of half in case Skia is in a mode that draws this rect with
        // unexpected AA of its own.
        float inverseDet = 1 / fabsf(determinant);
        float bloatX = (fabsf(d) + fabsf(c)) * inverseDet;
        float bloatY = (fabsf(b) + fabsf(a)) * inverseDet;
        canvas->drawRect(superRRect.makeOutset(bloatX, bloatY), paint);
    }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand(2);

        canvas->save();
        canvas->translate(canvas->imageInfo().width() / 2.f, canvas->imageInfo().height() / 2.f);

        canvas->save();
        canvas->rotate(21);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(-5, 25, 175, 100), 50, 30,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->save();
        canvas->rotate(94);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(95, 75, 125, 100), 30, 30,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->save();
        canvas->rotate(132);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(0, 75, 150, 100), 40, 30,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->save();
        canvas->rotate(282);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(15, -20, 100, 100), 20, 20,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->save();
        canvas->rotate(0);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(140, -50, 90, 110), 25, 25,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->save();
        canvas->rotate(-35);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(160, -60, 60, 90), 18, 18,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->save();
        canvas->rotate(65);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(220, -120, 60, 90), 18, 18,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->save();
        canvas->rotate(265);
        this->drawSuperRRect(canvas, SkRect::MakeXYWH(150, -129, 80, 160), 24, 39,
                             rand.nextU() | 0xff808080);
        canvas->restore();

        canvas->restore();
    }

private:
    const float fPower;
};
DEF_GM(return new ClipSuperRRect("clip_super_rrect_pow2", 2);)
// DEF_GM(return new ClipSuperRRect("clip_super_rrect_pow3", 3);)
DEF_GM(return new ClipSuperRRect("clip_super_rrect_pow3.5", 3.5);)
// DEF_GM(return new ClipSuperRRect("clip_super_rrect_pow4", 4);)
// DEF_GM(return new ClipSuperRRect("clip_super_rrect_pow4.5", 4.5);)
// DEF_GM(return new ClipSuperRRect("clip_super_rrect_pow5", 5);)

DEF_SIMPLE_GM(child_sampling_rt, canvas, 256,256) {
    static constexpr char scale[] =
        "uniform shader child;"
        "half4 main(float2 xy) {"
        "    return child.eval(xy*0.1);"
        "}";

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(1);

    auto surf = SkSurface::MakeRasterN32Premul(100,100);
    surf->getCanvas()->drawLine(0, 0, 100, 100, p);
    auto shader = surf->makeImageSnapshot()->makeShader(SkSamplingOptions(SkFilterMode::kLinear));

    SkRuntimeShaderBuilder builder(SkRuntimeEffect::MakeForShader(SkString(scale)).effect);
    builder.child("child") = shader;
    p.setShader(builder.makeShader(nullptr, false));

    canvas->drawPaint(p);
}

static sk_sp<SkShader> normal_map_shader() {
    // Produces a hemispherical normal:
    static const char* kSrc = R"(
        half4 main(vec2 p) {
            p = (p / 256) * 2 - 1;
            float p2 = dot(p, p);
            vec3 v = (p2 > 1) ? vec3(0, 0, 1) : vec3(p, sqrt(1 - p2));
            return (v * 0.5 + 0.5).xyz1;
        }
    )";
    auto effect = SkRuntimeEffect::MakeForShader(SkString(kSrc)).effect;
    return effect->makeShader(nullptr, {}, nullptr, true);
}

static sk_sp<SkShader> normal_map_image_shader() {
    // Above, baked into an image:
    auto surface = SkSurface::MakeRasterN32Premul(256, 256);
    SkPaint p;
    p.setShader(normal_map_shader());
    surface->getCanvas()->drawPaint(p);
    auto image = surface->makeImageSnapshot();
    return image->makeShader(SkSamplingOptions{});
}

static sk_sp<SkShader> lit_shader(sk_sp<SkShader> normals) {
    // Simple N.L against a fixed, directional light:
    static const char* kSrc = R"(
        uniform shader normals;
        half4 main(vec2 p) {
            vec3 n = normalize(normals.eval(p).xyz * 2 - 1);
            vec3 l = normalize(vec3(1, -1, 1));
            return saturate(dot(n, l)).xxx1;
        }
    )";
    auto effect = SkRuntimeEffect::MakeForShader(SkString(kSrc)).effect;
    return effect->makeShader(nullptr, &normals, 1, nullptr, true);
}

DEF_SIMPLE_GM(paint_alpha_normals_rt, canvas, 512,512) {
    // Various draws, with non-opaque paint alpha. This demonstrates several issues around how
    // paint alpha is applied differently on CPU (globally, after all shaders) and GPU (per shader,
    // inconsistently). See: skbug.com/11942
    //
    // When this works, it will be a demo of applying paint alpha to fade out a complex effect.
    auto draw_shader = [=](int x, int y, sk_sp<SkShader> shader) {
        SkPaint p;
        p.setAlpha(164);
        p.setShader(shader);

        canvas->save();
        canvas->translate(x, y);
        canvas->clipRect({0, 0, 256, 256});
        canvas->drawPaint(p);
        canvas->restore();
    };

    draw_shader(0, 0, normal_map_shader());
    draw_shader(0, 256, normal_map_image_shader());

    draw_shader(256, 0, lit_shader(normal_map_shader()));
    draw_shader(256, 256, lit_shader(normal_map_image_shader()));
}
