/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
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
#include "include/gpu/GrRecordingContext.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
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
    SkString getName() const override { return fName; }
    SkISize getISize() override { return fSize; }

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
        p.setShader(builder.makeShader(&localM));
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
    auto surf = SkSurfaces::Raster(info);
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

        builder.uniform("cutoff") = sinf(fSecs) * 0.55f + 0.5f;
        builder.uniform("slope")  = 10.0f;

        builder.child("before_map")    = fBefore;
        builder.child("after_map")     = fAfter;
        builder.child("threshold_map") = fThreshold;

        SkPaint paint;
        paint.setShader(builder.makeShader());
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
        layout(color) uniform float4 in_colors0;
        layout(color) uniform float4 in_colors1;

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
        builder.uniform("in_colors0") = SkColors::kRed;
        builder.uniform("in_colors1") = SkColors::kGreen;

        SkPaint paint;
        paint.setShader(builder.makeShader());
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
        paint.setShader(builder.makeShader());
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
        paint.setShader(builder.makeShader());
        canvas->translate(256, 0);
        canvas->drawRect({ 0, 0, 256, 256 }, paint);

        // ... and with a sepia-tone color cube. This should match the sepia-toned image.
        builder.child("color_cube") = fSepiaCube->makeShader(sampling, normalize);
        paint.setShader(builder.makeShader());
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
        SkRuntimeColorFilterBuilder builder(fEffect);

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

        SkPaint paint;

        // TODO: Should we add SkImage::makeNormalizedShader() to handle this automatically?
        SkMatrix normalize = SkMatrix::Scale(1.0f / (kSize * kSize), 1.0f / kSize);

        // Now draw the image with an identity color cube - it should look like the original
        builder.child("color_cube") = fIdentityCube->makeShader(sampling, normalize);

        paint.setColorFilter(builder.makeColorFilter());
        canvas->drawImage(fMandrill, 256, 0, sampling, &paint);

        // ... and with a sepia-tone color cube. This should match the sepia-toned image.
        builder.child("color_cube") = fSepiaCube->makeShader(sampling, normalize);

        paint.setColorFilter(builder.makeColorFilter());
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
        paint.setShader(builder.makeShader());
        canvas->drawRect({ 0, 0, 256, 256 }, paint);

        // Now we bind an image shader as the child. This (by convention) scales by the paint alpha
        builder.child("child") = fMandrill->makeShader(SkSamplingOptions());
        paint.setColor4f({ 1.0f, 1.0f, 1.0f, 0.5f });
        paint.setShader(builder.makeShader());
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
        canvas->clipShader(builder.makeShader(&cornerToLocal));

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

class LinearGradientRT : public RuntimeShaderGM {
public:
    LinearGradientRT() : RuntimeShaderGM("linear_gradient_rt", {256 + 10, 128 + 15}, R"(
        layout(color) uniform vec4 in_colors0;
        layout(color) uniform vec4 in_colors1;

        vec4 main(vec2 p) {
            float t = p.x / 256;
            if (p.y < 32) {
                return mix(in_colors0, in_colors1, t);
            } else {
                vec3 linColor0 = toLinearSrgb(in_colors0.rgb);
                vec3 linColor1 = toLinearSrgb(in_colors1.rgb);
                vec3 linColor = mix(linColor0, linColor1, t);
                return fromLinearSrgb(linColor).rgb1;
            }
        }
    )") {}

    void onDraw(SkCanvas* canvas) override {
        // Colors chosen to use values other than 0 and 1 - so that it's obvious if the conversion
        // intrinsics are doing anything. (Most transfer functions map 0 -> 0 and 1 -> 1).
        SkRuntimeShaderBuilder builder(fEffect);
        builder.uniform("in_colors0") = SkColor4f{0.75f, 0.25f, 0.0f, 1.0f};
        builder.uniform("in_colors1") = SkColor4f{0.0f, 0.75f, 0.25f, 1.0f};
        SkPaint paint;
        paint.setShader(builder.makeShader());

        canvas->save();
        canvas->clear(SK_ColorWHITE);
        canvas->translate(5, 5);

        // We draw everything twice. First to a surface with no color management, where the
        // intrinsics should do nothing (eg, the top bar should look the same in the top and bottom
        // halves). Then to an sRGB surface, where they should produce linearly interpolated
        // gradients (the bottom half of the second bar should be brighter than the top half).
        for (auto cs : {static_cast<SkColorSpace*>(nullptr), sk_srgb_singleton()}) {
            SkImageInfo info = SkImageInfo::Make(
                    256, 64, kN32_SkColorType, kPremul_SkAlphaType, sk_ref_sp(cs));
            auto surface = canvas->makeSurface(info);
            if (!surface) {
                surface = SkSurfaces::Raster(info);
            }

            surface->getCanvas()->drawRect({0, 0, 256, 64}, paint);
            canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
            canvas->translate(0, 64 + 5);
        }

        canvas->restore();
    }
};
DEF_GM(return new LinearGradientRT;)

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

    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    surf->getCanvas()->drawLine(0, 0, 100, 100, p);
    auto shader = surf->makeImageSnapshot()->makeShader(SkSamplingOptions(SkFilterMode::kLinear));

    SkRuntimeShaderBuilder builder(SkRuntimeEffect::MakeForShader(SkString(scale)).effect);
    builder.child("child") = shader;
    p.setShader(builder.makeShader());

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
    return effect->makeShader(nullptr, {});
}

static sk_sp<SkImage> normal_map_image() {
    // Above, baked into an image:
    auto info = SkImageInfo::Make(256, 256, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::Raster(info);
    SkPaint p;
    p.setShader(normal_map_shader());
    surface->getCanvas()->drawPaint(p);
    return surface->makeImageSnapshot();
}

static sk_sp<SkShader> normal_map_image_shader() {
    return normal_map_image()->makeShader(SkFilterMode::kNearest);
}

static sk_sp<SkShader> normal_map_raw_image_shader() {
    return normal_map_image()->makeRawShader(SkFilterMode::kNearest);
}

static sk_sp<SkImage> normal_map_unpremul_image() {
    auto image = normal_map_image();
    SkPixmap pm;
    SkAssertResult(image->peekPixels(&pm));
    SkBitmap bmp;
    bmp.allocPixels(image->imageInfo().makeAlphaType(kUnpremul_SkAlphaType));
    // Copy all pixels over, but set alpha to 0
    for (int y = 0; y < pm.height(); y++) {
        for (int x = 0; x < pm.width(); x++) {
            *bmp.getAddr32(x, y) = *pm.addr32(x, y) & 0x00FFFFFF;
        }
    }
    return bmp.asImage();
}

static sk_sp<SkShader> normal_map_unpremul_image_shader() {
    return normal_map_unpremul_image()->makeShader(SkFilterMode::kNearest);
}

static sk_sp<SkShader> normal_map_raw_unpremul_image_shader() {
    return normal_map_unpremul_image()->makeRawShader(SkFilterMode::kNearest);
}

static sk_sp<SkShader> lit_shader(sk_sp<SkShader> normals) {
    // Simple N-dot-L against a fixed, directional light:
    static const char* kSrc = R"(
        uniform shader normals;
        half4 main(vec2 p) {
            vec3 n = normalize(normals.eval(p).xyz * 2 - 1);
            vec3 l = normalize(vec3(1, -1, 1));
            return saturate(dot(n, l)).xxx1;
        }
    )";
    auto effect = SkRuntimeEffect::MakeForShader(SkString(kSrc)).effect;
    return effect->makeShader(nullptr, &normals, 1);
}

static sk_sp<SkShader> lit_shader_linear(sk_sp<SkShader> normals) {
    // Simple N-dot-L against a fixed, directional light, done in linear space:
    static const char* kSrc = R"(
        uniform shader normals;
        half4 main(vec2 p) {
            vec3 n = normalize(normals.eval(p).xyz * 2 - 1);
            vec3 l = normalize(vec3(1, -1, 1));
            return fromLinearSrgb(saturate(dot(n, l)).xxx).xxx1;
        }
    )";
    auto effect = SkRuntimeEffect::MakeForShader(SkString(kSrc)).effect;
    return effect->makeShader(nullptr, &normals, 1);
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

DEF_SIMPLE_GM(raw_image_shader_normals_rt, canvas, 768, 512) {
    // Demonstrates the utility of SkImage::makeRawShader, for non-color child shaders.

    // First, make an offscreen surface, so we can control the destination color space:
    auto surfInfo = SkImageInfo::Make(512, 512,
                                      kN32_SkColorType,
                                      kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB()->makeColorSpin());
    auto surface = canvas->makeSurface(surfInfo);
    if (!surface) {
        surface = SkSurfaces::Raster(surfInfo);
    }

    auto draw_shader = [](int x, int y, sk_sp<SkShader> shader, SkCanvas* canvas) {
        SkPaint p;
        p.setShader(shader);

        canvas->save();
        canvas->translate(x, y);
        canvas->clipRect({0, 0, 256, 256});
        canvas->drawPaint(p);
        canvas->restore();
    };

    sk_sp<SkShader> colorNormals = normal_map_image_shader(),
                    rawNormals = normal_map_raw_image_shader();

    // Draw our normal map as colors (will be color-rotated), and raw (untransformed)
    draw_shader(0, 0, colorNormals, surface->getCanvas());
    draw_shader(0, 256, rawNormals, surface->getCanvas());

    // Now draw our lighting shader using the normal and raw versions of the normals as children.
    // The top image will have the normals rotated (incorrectly), so the lighting is very dark.
    draw_shader(256, 0, lit_shader(colorNormals), surface->getCanvas());
    draw_shader(256, 256, lit_shader(rawNormals), surface->getCanvas());

    // Now draw the offscreen surface back to our original canvas. If we do this naively, the image
    // will be un-transformed back to the canvas' color space. That will have the effect of undoing
    // the color spin on the upper-left, and APPLYING a color-spin on the bottom left. To preserve
    // the intent of this GM (and make it draw consistently whether or not the original surface has
    // a color space attached), we reinterpret the offscreen image as being in sRGB:
    canvas->drawImage(
            surface->makeImageSnapshot()->reinterpretColorSpace(SkColorSpace::MakeSRGB()), 0, 0);

    // Finally, to demonstrate that raw unpremul image shaders don't premul, draw lighting two more
    // times, with an unpremul normal map (containing ZERO in the alpha channel). THe top will
    // premultiply the normals, resulting in totally dark lighting. The bottom will retain the RGB
    // encoded normals, even with zero alpha:
    draw_shader(512, 0, lit_shader(normal_map_unpremul_image_shader()), canvas);
    draw_shader(512, 256, lit_shader(normal_map_raw_unpremul_image_shader()), canvas);
}

DEF_SIMPLE_GM(lit_shader_linear_rt, canvas, 512, 256) {
    // First, make an offscreen surface, so we can control the destination color space:
    auto surfInfo = SkImageInfo::Make(512, 256,
                                      kN32_SkColorType,
                                      kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());
    auto surface = canvas->makeSurface(surfInfo);
    if (!surface) {
        surface = SkSurfaces::Raster(surfInfo);
    }

    auto draw_shader = [](int x, int y, sk_sp<SkShader> shader, SkCanvas* canvas) {
        SkPaint p;
        p.setShader(shader);

        canvas->save();
        canvas->translate(x, y);
        canvas->clipRect({0, 0, 256, 256});
        canvas->drawPaint(p);
        canvas->restore();
    };

    // We draw two lit spheres - one does math in the working space (so gamma-encoded). The second
    // works in linear space, then converts to sRGB. This produces (more accurate) sharp falloff:
    draw_shader(0, 0, lit_shader(normal_map_shader()), surface->getCanvas());
    draw_shader(256, 0, lit_shader_linear(normal_map_shader()), surface->getCanvas());

    // Now draw the offscreen surface back to our original canvas:
    canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
}

// skbug.com/13598 GPU was double applying the local matrix.
DEF_SIMPLE_GM(local_matrix_shader_rt, canvas, 256, 256) {
    SkString passthrough(R"(
        uniform shader s;
        half4 main(float2 p) { return s.eval(p); }
    )");
    auto [rte, error] = SkRuntimeEffect::MakeForShader(passthrough, {});
    if (!rte) {
        SkDebugf("%s\n", error.c_str());
        return;
    }

    auto image     = GetResourceAsImage("images/mandrill_128.png");
    auto imgShader = image->makeShader(SkFilterMode::kNearest);

    auto r = SkRect::MakeWH(image->width(), image->height());

    auto lm = SkMatrix::RotateDeg(90.f, {image->width()/2.f, image->height()/2.f});

    SkPaint paint;

    // image
    paint.setShader(imgShader);
    canvas->drawRect(r, paint);

    // passthrough(image)
    canvas->save();
    canvas->translate(image->width(), 0);
    paint.setShader(rte->makeShader(nullptr, &imgShader, 1));
    canvas->drawRect(r, paint);
    canvas->restore();

    // localmatrix(image)
    canvas->save();
    canvas->translate(0, image->height());
    paint.setShader(imgShader->makeWithLocalMatrix(lm));
    canvas->drawRect(r, paint);
    canvas->restore();

    // localmatrix(passthrough(image)) This was the bug.
    canvas->save();
    canvas->translate(image->width(), image->height());
    paint.setShader(rte->makeShader(nullptr, &imgShader, 1)->makeWithLocalMatrix(lm));
    canvas->drawRect(r, paint);
    canvas->restore();
}

DEF_SIMPLE_GM(null_child_rt, canvas, 150, 150) {
    using ChildPtr = SkRuntimeEffect::ChildPtr;

    // Every swatch should evaluate to the same shade of purple.
    // Paint with a shader evaluating a null shader.
    // Point passed to eval() is ignored; paint color is returned.
    {
        const SkString kEvalShader{R"(
            uniform shader s;
            half4 main(float2 p) { return s.eval(p); }
        )"};
        auto [rtShader, error] = SkRuntimeEffect::MakeForShader(kEvalShader);
        SkASSERT(rtShader);

        SkPaint paint;
        ChildPtr children[1] = {ChildPtr{sk_sp<SkShader>{nullptr}}};
        paint.setShader(rtShader->makeShader(/*uniforms=*/nullptr, children));
        paint.setColor(SkColorSetARGB(0xFF, 0x80, 0x00, 0x80));  // purple (contributes)
        canvas->drawRect({0, 0, 48, 48}, paint);
        canvas->translate(50, 0);
    }
    // Paint with a shader evaluating a null color filter.
    // Color passed to eval() is returned; paint color is ignored.
    {
        const SkString kEvalColorFilter{R"(
            uniform colorFilter cf;
            half4 main(float2 p) { return cf.eval(half4(0.5, 0, 0.5, 1)); }
        )"};
        auto [rtShader, error] = SkRuntimeEffect::MakeForShader(kEvalColorFilter);
        SkASSERT(rtShader);

        SkPaint paint;
        ChildPtr children[1] = {ChildPtr{sk_sp<SkColorFilter>{nullptr}}};
        paint.setShader(rtShader->makeShader(/*uniforms=*/nullptr, children));
        paint.setColor(SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF));  // green (does not contribute)
        canvas->drawRect({0, 0, 48, 48}, paint);
        canvas->translate(50, 0);
    }
    // Paint with a shader evaluating a null blender.
    // Colors passed to eval() are blended via src-over; paint color is ignored.
    {
        const SkString kEvalBlender{R"(
            uniform blender b;
            half4 main(float2 p) { return b.eval(half4(0.5, 0, 0, 0.5), half4(0, 0, 1, 1)); }
        )"};
        auto [rtShader, error] = SkRuntimeEffect::MakeForShader(kEvalBlender);
        SkASSERT(rtShader);

        SkPaint paint;
        ChildPtr children[1] = {ChildPtr{sk_sp<SkBlender>{nullptr}}};
        paint.setShader(rtShader->makeShader(/*uniforms=*/nullptr, children));
        paint.setColor(SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF));  // green (does not contribute)
        canvas->drawRect({0, 0, 48, 48}, paint);
        canvas->translate(50, 0);
    }

    canvas->translate(-150, 50);

    // Paint with a color filter evaluating a null shader.
    // Point passed to eval() is ignored; previous-stage color (the paint color) is returned.
    {
        const SkString kEvalShader{R"(
            uniform shader s;
            half4 main(half4 c) { return s.eval(float2(0)); }
        )"};
        auto [rtFilter, error] = SkRuntimeEffect::MakeForColorFilter(kEvalShader);
        SkASSERT(rtFilter);

        SkPaint paint;
        ChildPtr children[1] = {ChildPtr{sk_sp<SkShader>{nullptr}}};
        paint.setColorFilter(rtFilter->makeColorFilter(/*uniforms=*/nullptr, children));
        paint.setColor(SkColorSetARGB(0xFF, 0x80, 0x00, 0x80));  // purple (contributes)
        canvas->drawRect({0, 0, 48, 48}, paint);
        canvas->translate(50, 0);
    }
    // Paint with a color filter evaluating a null color filter.
    // Color passed to eval() is returned; paint color is ignored.
    {
        const SkString kEvalColorFilter{R"(
            uniform colorFilter cf;
            half4 main(half4 c) { return cf.eval(half4(0.5, 0, 0.5, 1)); }
        )"};
        auto [rtFilter, error] = SkRuntimeEffect::MakeForColorFilter(kEvalColorFilter);
        SkASSERT(rtFilter);

        SkPaint paint;
        ChildPtr children[1] = {ChildPtr{sk_sp<SkColorFilter>{nullptr}}};
        paint.setColorFilter(rtFilter->makeColorFilter(/*uniforms=*/nullptr, children));
        paint.setColor(SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF));  // green (does not contribute)
        canvas->drawRect({0, 0, 48, 48}, paint);
        canvas->translate(50, 0);
    }
    // Paint with a color filter evaluating a null blender.
    // Colors passed to eval() are blended via src-over; paint color is ignored.
    {
        const SkString kEvalBlender{R"(
            uniform blender b;
            half4 main(half4 c) { return b.eval(half4(0.5, 0, 0, 0.5), half4(0, 0, 1, 1)); }
        )"};
        auto [rtFilter, error] = SkRuntimeEffect::MakeForColorFilter(kEvalBlender);
        SkASSERT(rtFilter);

        SkPaint paint;
        ChildPtr children[1] = {ChildPtr{sk_sp<SkBlender>{nullptr}}};
        paint.setColorFilter(rtFilter->makeColorFilter(/*uniforms=*/nullptr, children));
        paint.setColor(SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF));  // green (does not contribute)
        canvas->drawRect({0, 0, 48, 48}, paint);
        canvas->translate(50, 0);
    }

    canvas->translate(-150, 50);

    // Paint with a shader evaluating a null shader.
    // Point passed to eval() is ignored; paint color is returned.
    // We draw to an offscreen surface in a different color space, then draw that surface back to
    // the main canvas. This ensures the paint color is correctly transformed. (skbug.com/14153)
    {
        const SkString kEvalShader{R"(
            uniform shader s;
            half4 main(float2 p) { return s.eval(p); }
        )"};
        auto [rtShader, error] = SkRuntimeEffect::MakeForShader(kEvalShader);
        SkASSERT(rtShader);

        sk_sp<SkColorSpace> spin = SkColorSpace::MakeSRGB()->makeColorSpin();
        SkImageInfo spinInfo =
                SkImageInfo::Make(50, 50, kN32_SkColorType, kPremul_SkAlphaType, spin);
        auto surface = canvas->makeSurface(spinInfo);
        if (!surface) {
            surface = SkSurfaces::Raster(spinInfo);
        }

        SkPaint paint;
        ChildPtr children[1] = {ChildPtr{sk_sp<SkShader>{nullptr}}};
        paint.setShader(rtShader->makeShader(/*uniforms=*/nullptr, children));
        paint.setColor(SkColorSetARGB(0xFF, 0x80, 0x00, 0x80));  // purple (contributes)
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->drawRect({0, 0, 48, 48}, paint);

        // Ideally, we'd just draw the offscreen surface back to the canvas. But if `canvas` isn't
        // color managed, we won't convert it BACK, so we'll still see a color-spin happen.
        // Instead, convert the image back to sRGB, and the resulting image will look correct for
        // all modes (assuming the paint color was handled correctly above):
        auto image = surface->makeImageSnapshot();
#if defined(SK_GRAPHITE)
        if (auto recorder = canvas->recorder()) {
            image = image->makeColorSpace(recorder, SkColorSpace::MakeSRGB(), {});
        } else
#endif
        {
            auto direct = GrAsDirectContext(canvas->recordingContext());
            image = image->makeColorSpace(direct, SkColorSpace::MakeSRGB());
        }

        canvas->drawImage(image, 0, 0);
        canvas->translate(50, 0);
    }
}

DEF_SIMPLE_GM_CAN_FAIL(deferred_shader_rt, canvas, errorMsg, 150, 50) {
    // Skip this GM on recording devices. It actually works okay on serialize-8888, but pic-8888
    // does not. Ultimately, behavior on CPU is potentially strange (especially with SkRP), because
    // SkRP will build the shader more than once per draw.
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        return skiagm::DrawResult::kSkip;
    }

    const SkString kShader{R"(
        uniform half4 color;
        half4 main(float2 p) { return color; }
    )"};
    auto [effect, error] = SkRuntimeEffect::MakeForShader(kShader);
    SkASSERT(effect);

    SkColor4f color = SkColors::kRed;
    auto makeUniforms = [color](const SkRuntimeEffectPriv::UniformsCallbackContext&) mutable {
        auto result = SkData::MakeWithCopy(&color, sizeof(color));
        color = {color.fB, color.fR, color.fG, color.fA};
        return result;
    };

    auto shader =
            SkRuntimeEffectPriv::MakeDeferredShader(effect.get(), makeUniforms, /*children=*/{});
    SkASSERT(shader);

    SkPaint paint;
    paint.setShader(shader);

    for (int i = 0; i < 3; ++i) {
        canvas->drawRect({0, 0, 50, 50}, paint);
        canvas->translate(50, 0);
    }

    return skiagm::DrawResult::kOk;
}
