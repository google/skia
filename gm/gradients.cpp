/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"

#include <math.h>

namespace {

struct GradData {
    int              fCount;
    const SkColor*   fColors;
    const SkColor4f* fColors4f;
    const SkScalar*  fPos;
};

constexpr SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK
};
constexpr SkColor4f gColors4f[] ={
    { 1.0f, 0.0f, 0.0f, 1.0f }, // Red
    { 0.0f, 1.0f, 0.0f, 1.0f }, // Green
    { 0.0f, 0.0f, 1.0f, 1.0f }, // Blue
    { 1.0f, 1.0f, 1.0f, 1.0f }, // White
    { 0.0f, 0.0f, 0.0f, 1.0f }  // Black
};
constexpr SkScalar gPos0[] = { 0, SK_Scalar1 };
constexpr SkScalar gPos1[] = { SK_Scalar1/4, SK_Scalar1*3/4 };
constexpr SkScalar gPos2[] = {
    0, SK_Scalar1/8, SK_Scalar1/2, SK_Scalar1*7/8, SK_Scalar1
};

constexpr SkScalar gPosClamp[]   = {0.0f, 0.0f, 1.0f, 1.0f};
constexpr SkColor  gColorClamp[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorGREEN, SK_ColorBLUE
};
constexpr SkColor4f gColor4fClamp[] ={
    { 1.0f, 0.0f, 0.0f, 1.0f }, // Red
    { 0.0f, 1.0f, 0.0f, 1.0f }, // Green
    { 0.0f, 1.0f, 0.0f, 1.0f }, // Green
    { 0.0f, 0.0f, 1.0f, 1.0f }  // Blue
};
constexpr GradData gGradData[] = {
    { 2, gColors, gColors4f, nullptr },
    { 2, gColors, gColors4f, gPos0 },
    { 2, gColors, gColors4f, gPos1 },
    { 5, gColors, gColors4f, nullptr },
    { 5, gColors, gColors4f, gPos2 },
    { 4, gColorClamp, gColor4fClamp, gPosClamp }
};

static sk_sp<SkShader> MakeLinear(const SkPoint pts[2], const GradData& data,
                                  SkTileMode tm, const SkMatrix& localMatrix) {
    return SkGradientShader::MakeLinear(pts, data.fColors, data.fPos, data.fCount, tm, 0,
                                        &localMatrix);
}

static sk_sp<SkShader> MakeLinear4f(const SkPoint pts[2], const GradData& data,
                                    SkTileMode tm, const SkMatrix& localMatrix) {
    auto srgb = SkColorSpace::MakeSRGB();
    return SkGradientShader::MakeLinear(pts, data.fColors4f, srgb, data.fPos, data.fCount, tm, 0,
                                        &localMatrix);
}

static sk_sp<SkShader> MakeRadial(const SkPoint pts[2], const GradData& data,
                                  SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeRadial(center, center.fX, data.fColors, data.fPos, data.fCount,
                                        tm, 0, &localMatrix);
}

static sk_sp<SkShader> MakeRadial4f(const SkPoint pts[2], const GradData& data,
                                    SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    auto srgb = SkColorSpace::MakeSRGB();
    return SkGradientShader::MakeRadial(center, center.fX, data.fColors4f, srgb, data.fPos,
                                        data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> MakeSweep(const SkPoint pts[2], const GradData& data,
                                 SkTileMode, const SkMatrix& localMatrix) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeSweep(center.fX, center.fY, data.fColors, data.fPos, data.fCount,
                                       0, &localMatrix);
}

static sk_sp<SkShader> MakeSweep4f(const SkPoint pts[2], const GradData& data,
                                   SkTileMode, const SkMatrix& localMatrix) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    auto srgb = SkColorSpace::MakeSRGB();
    return SkGradientShader::MakeSweep(center.fX, center.fY, data.fColors4f, srgb, data.fPos,
                                       data.fCount, 0, &localMatrix);
}

static sk_sp<SkShader> Make2Radial(const SkPoint pts[2], const GradData& data,
                                   SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 data.fColors, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2Radial4f(const SkPoint pts[2], const GradData& data,
                                     SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3) / 5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1) / 4));
    auto srgb = SkColorSpace::MakeSRGB();
    return SkGradientShader::MakeTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 data.fColors4f, srgb, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2Conical(const SkPoint pts[2], const GradData& data,
                                    SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::MakeTwoPointConical(center1, radius1, center0, radius0,
                                                 data.fColors, data.fPos,
                                                 data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2Conical4f(const SkPoint pts[2], const GradData& data,
                                      SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    auto srgb = SkColorSpace::MakeSRGB();
    return SkGradientShader::MakeTwoPointConical(center1, radius1, center0, radius0,
                                                 data.fColors4f, srgb, data.fPos,
                                                 data.fCount, tm, 0, &localMatrix);
}

typedef sk_sp<SkShader> (*GradMaker)(const SkPoint pts[2], const GradData& data,
                                     SkTileMode tm, const SkMatrix& localMatrix);
constexpr GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep, Make2Radial, Make2Conical
};
constexpr GradMaker gGradMakers4f[] ={
    MakeLinear4f, MakeRadial4f, MakeSweep4f, Make2Radial4f, Make2Conical4f
};

///////////////////////////////////////////////////////////////////////////////

class GradientsGM : public skiagm::GM {
public:
    GradientsGM(bool dither) : fDither(dither) {}

protected:
    const bool fDither;

    void onDraw(SkCanvas* canvas) override {
        SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkTileMode tm = SkTileMode::kClamp;
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(fDither);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
                SkMatrix scale = SkMatrix::I();

                if (i == 5) { // if the clamp case
                    scale.setScale(0.5f, 0.5f);
                    scale.postTranslate(25.f, 25.f);
                }

                paint.setShader(gGradMakers[j](pts, gGradData[i], tm, scale));
                canvas->drawRect(r, paint);
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
    }

private:
    void onOnceBeforeDraw() override { this->setBGColor(0xFFDDDDDD); }

    SkString onShortName() override {
        return SkString(fDither ? "gradients" : "gradients_nodither");
    }

    SkISize onISize() override { return {840, 815}; }
};
DEF_GM( return new GradientsGM(true); )
DEF_GM( return new GradientsGM(false); )

// Like the original gradients GM, but using the SkColor4f shader factories. Should be identical.
class Gradients4fGM : public skiagm::GM {
public:
    Gradients4fGM(bool dither) : fDither(dither) {}

private:
    void onOnceBeforeDraw() override { this->setBGColor(0xFFDDDDDD); }

    SkString onShortName() override {
        return SkString(fDither ? "gradients4f" : "gradients4f_nodither");
    }

    SkISize onISize() override { return {840, 815}; }

    void onDraw(SkCanvas* canvas) override {
        SkPoint pts[2] ={
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkTileMode tm = SkTileMode::kClamp;
        SkRect r ={ 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(fDither);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers4f); j++) {
                SkMatrix scale = SkMatrix::I();

                if (i == 5) { // if the clamp case
                    scale.setScale(0.5f, 0.5f);
                    scale.postTranslate(25.f, 25.f);
                }

                paint.setShader(gGradMakers4f[j](pts, gGradData[i], tm, scale));
                canvas->drawRect(r, paint);
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
    }

    bool fDither;
};
DEF_GM(return new Gradients4fGM(true); )
DEF_GM(return new Gradients4fGM(false); )

// Based on the original gradient slide, but with perspective applied to the
// gradient shaders' local matrices
class GradientsLocalPerspectiveGM : public skiagm::GM {
public:
    GradientsLocalPerspectiveGM(bool dither) : fDither(dither) {
        this->setBGColor(0xFFDDDDDD);
    }

private:
    SkString onShortName() override {
        return SkString(fDither ? "gradients_local_perspective" :
                                  "gradients_local_perspective_nodither");
    }

    SkISize onISize() override { return {840, 815}; }

    void onDraw(SkCanvas* canvas) override {
        SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkTileMode tm = SkTileMode::kClamp;
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(fDither);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
                // apply an increasing y perspective as we move to the right
                SkMatrix perspective;
                perspective.setIdentity();
                perspective.setPerspY(SkIntToScalar(i+1) / 500);
                perspective.setSkewX(SkIntToScalar(i+1) / 10);

                paint.setShader(gGradMakers[j](pts, gGradData[i], tm, perspective));
                canvas->drawRect(r, paint);
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
    }

    bool fDither;
};
DEF_GM( return new GradientsLocalPerspectiveGM(true); )
DEF_GM( return new GradientsLocalPerspectiveGM(false); )

// Based on the original gradient slide, but with perspective applied to
// the view matrix
class GradientsViewPerspectiveGM : public GradientsGM {
public:
    GradientsViewPerspectiveGM(bool dither) : INHERITED(dither) { }

private:
    SkString onShortName() override {
        return SkString(fDither ? "gradients_view_perspective" :
                                  "gradients_view_perspective_nodither");
    }

    SkISize onISize() override { return {840, 500}; }

    void onDraw(SkCanvas* canvas) override {
        SkMatrix perspective;
        perspective.setIdentity();
        perspective.setPerspY(0.001f);
        perspective.setSkewX(SkIntToScalar(8) / 25);
        canvas->concat(perspective);
        this->INHERITED::onDraw(canvas);
    }

private:
    typedef GradientsGM INHERITED;
};
DEF_GM( return new GradientsViewPerspectiveGM(true); )
DEF_GM( return new GradientsViewPerspectiveGM(false); )

/*
 Inspired by this <canvas> javascript, where we need to detect that we are not
 solving a quadratic equation, but must instead solve a linear (since our X^2
 coefficient is 0)

 ctx.fillStyle = '#f00';
 ctx.fillRect(0, 0, 100, 50);

 var g = ctx.createRadialGradient(-80, 25, 70, 0, 25, 150);
 g.addColorStop(0, '#f00');
 g.addColorStop(0.01, '#0f0');
 g.addColorStop(0.99, '#0f0');
 g.addColorStop(1, '#f00');
 ctx.fillStyle = g;
 ctx.fillRect(0, 0, 100, 50);
 */
class GradientsDegenrate2PointGM : public skiagm::GM {
public:
    GradientsDegenrate2PointGM(bool dither) : fDither(dither) {}

private:
    SkString onShortName() override {
        return SkString(fDither ? "gradients_degenerate_2pt" : "gradients_degenerate_2pt_nodither");
    }

    SkISize onISize() override { return {320, 320}; }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorBLUE);

        SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorGREEN, SK_ColorRED };
        SkScalar pos[] = { 0, 0.01f, 0.99f, SK_Scalar1 };
        SkPoint c0;
        c0.iset(-80, 25);
        SkScalar r0 = SkIntToScalar(70);
        SkPoint c1;
        c1.iset(0, 25);
        SkScalar r1 = SkIntToScalar(150);
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeTwoPointConical(c0, r0, c1, r1, colors,
                                                              pos, SK_ARRAY_COUNT(pos),
                                                              SkTileMode::kClamp));
        paint.setDither(fDither);
        canvas->drawPaint(paint);
    }

    bool fDither;
};
DEF_GM( return new GradientsDegenrate2PointGM(true); )
DEF_GM( return new GradientsDegenrate2PointGM(false); )

/* bug.skia.org/517
<canvas id="canvas"></canvas>
<script>
var c = document.getElementById("canvas");
var ctx = c.getContext("2d");
ctx.fillStyle = '#ff0';
ctx.fillRect(0, 0, 100, 50);

var g = ctx.createRadialGradient(200, 25, 20, 200, 25, 10);
g.addColorStop(0, '#0f0');
g.addColorStop(0.003, '#f00');  // 0.004 makes this work
g.addColorStop(1, '#ff0');
ctx.fillStyle = g;
ctx.fillRect(0, 0, 100, 50);
</script>
*/

// should draw only green
DEF_SIMPLE_GM(small_color_stop, canvas, 100, 150) {
    SkColor colors[] = { SK_ColorGREEN, SK_ColorRED, SK_ColorYELLOW };
    SkScalar pos[] = { 0, 0.003f, SK_Scalar1 };  // 0.004f makes this work
    SkPoint c0 = { 200, 25 };
    SkScalar r0 = 20;
    SkPoint c1 = { 200, 25 };
    SkScalar r1 = 10;

    SkPaint paint;
    paint.setColor(SK_ColorYELLOW);
    canvas->drawRect(SkRect::MakeWH(100, 150), paint);
    paint.setShader(SkGradientShader::MakeTwoPointConical(c0, r0, c1, r1, colors, pos,
                                                          SK_ARRAY_COUNT(pos),
                                                          SkTileMode::kClamp));
    canvas->drawRect(SkRect::MakeWH(100, 150), paint);
}


/// Tests correctness of *optimized* codepaths in gradients.

class ClampedGradientsGM : public skiagm::GM {
public:
    ClampedGradientsGM(bool dither) : fDither(dither) {}

private:
    SkString onShortName() override {
        return SkString(fDither ? "clamped_gradients" : "clamped_gradients_nodither");
    }

    SkISize onISize() override { return {640, 510}; }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(0xFFDDDDDD);

        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(300) };
        SkPaint paint;
        paint.setDither(fDither);
        paint.setAntiAlias(true);

        SkPoint center;
        center.iset(0, 300);
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        paint.setShader(SkGradientShader::MakeRadial(
            SkPoint(center),
            SkIntToScalar(200), gColors, nullptr, 5,
            SkTileMode::kClamp));
        canvas->drawRect(r, paint);
    }

    bool fDither;
};
DEF_GM( return new ClampedGradientsGM(true); )
DEF_GM( return new ClampedGradientsGM(false); )

/// Checks quality of large radial gradients, which may display
/// some banding.

class RadialGradientGM : public skiagm::GM {
    SkString onShortName() override { return SkString("radial_gradient"); }

    SkISize onISize() override { return {1280, 1280}; }

    void onDraw(SkCanvas* canvas) override {
        const SkISize dim = this->getISize();

        canvas->drawColor(0xFF000000);

        SkPaint paint;
        paint.setDither(true);
        SkPoint center;
        center.set(SkIntToScalar(dim.width())/2, SkIntToScalar(dim.height())/2);
        SkScalar radius = SkIntToScalar(dim.width())/2;
        const SkColor colors[] = { 0x7f7f7f7f, 0x7f7f7f7f, 0xb2000000 };
        const SkScalar pos[] = { 0.0f,
                             0.35f,
                             1.0f };
        paint.setShader(SkGradientShader::MakeRadial(center, radius, colors, pos,
                                                     SK_ARRAY_COUNT(pos),
                                                     SkTileMode::kClamp));
        SkRect r = {
            0, 0, SkIntToScalar(dim.width()), SkIntToScalar(dim.height())
        };
        canvas->drawRect(r, paint);
    }
};
DEF_GM( return new RadialGradientGM; )

class RadialGradient2GM : public skiagm::GM {
public:
    RadialGradient2GM(bool dither) : fDither(dither) {}

private:
    SkString onShortName() override {
        return SkString(fDither ? "radial_gradient2" : "radial_gradient2_nodither");
    }

    SkISize onISize() override { return {800, 400}; }

    // Reproduces the example given in bug 7671058.
    void onDraw(SkCanvas* canvas) override {
        SkPaint paint1, paint2, paint3;
        paint1.setStyle(SkPaint::kFill_Style);
        paint2.setStyle(SkPaint::kFill_Style);
        paint3.setStyle(SkPaint::kFill_Style);

        const SkColor sweep_colors[] =
            { 0xFFFF0000, 0xFFFFFF00, 0xFF00FF00, 0xFF00FFFF, 0xFF0000FF, 0xFFFF00FF, 0xFFFF0000 };
        const SkColor colors1[] = { 0xFFFFFFFF, 0x00000000 };
        const SkColor colors2[] = { 0xFF000000, 0x00000000 };

        const SkScalar cx = 200, cy = 200, radius = 150;
        SkPoint center;
        center.set(cx, cy);

        // We can either interpolate endpoints and premultiply each point (default, more precision),
        // or premultiply the endpoints first, avoiding the need to premultiply each point (cheap).
        const uint32_t flags[] = { 0, SkGradientShader::kInterpolateColorsInPremul_Flag };

        for (size_t i = 0; i < SK_ARRAY_COUNT(flags); i++) {
            paint1.setShader(SkGradientShader::MakeSweep(cx, cy, sweep_colors,
                                                         nullptr, SK_ARRAY_COUNT(sweep_colors),
                                                         flags[i], nullptr));
            paint2.setShader(SkGradientShader::MakeRadial(center, radius, colors1,
                                                          nullptr, SK_ARRAY_COUNT(colors1),
                                                          SkTileMode::kClamp,
                                                          flags[i], nullptr));
            paint3.setShader(SkGradientShader::MakeRadial(center, radius, colors2,
                                                          nullptr, SK_ARRAY_COUNT(colors2),
                                                          SkTileMode::kClamp,
                                                          flags[i], nullptr));
            paint1.setDither(fDither);
            paint2.setDither(fDither);
            paint3.setDither(fDither);

            canvas->drawCircle(cx, cy, radius, paint1);
            canvas->drawCircle(cx, cy, radius, paint3);
            canvas->drawCircle(cx, cy, radius, paint2);

            canvas->translate(400, 0);
        }
    }

private:
    bool fDither;

    typedef GM INHERITED;
};
DEF_GM( return new RadialGradient2GM(true); )
DEF_GM( return new RadialGradient2GM(false); )

// Shallow radial (shows banding on raster)
class RadialGradient3GM : public skiagm::GM {
public:
    RadialGradient3GM(bool dither) : fDither(dither) { }

private:
    SkString onShortName() override {
        return SkString(fDither ? "radial_gradient3" : "radial_gradient3_nodither");
    }

    SkISize onISize() override { return {500, 500}; }

    bool runAsBench() const override { return true; }

    void onOnceBeforeDraw() override {
        const SkPoint center = { 0, 0 };
        const SkScalar kRadius = 3000;
        const SkColor gColors[] = { 0xFFFFFFFF, 0xFF000000 };
        fShader = SkGradientShader::MakeRadial(center, kRadius, gColors, nullptr, 2,
                                               SkTileMode::kClamp);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setShader(fShader);
        paint.setDither(fDither);
        canvas->drawRect(SkRect::MakeWH(500, 500), paint);
    }

private:
    sk_sp<SkShader> fShader;
    bool fDither;

    typedef GM INHERITED;
};
DEF_GM( return new RadialGradient3GM(true); )
DEF_GM( return new RadialGradient3GM(false); )

class RadialGradient4GM : public skiagm::GM {
public:
    RadialGradient4GM(bool dither) : fDither(dither) { }

private:
    SkString onShortName() override {
        return SkString(fDither ? "radial_gradient4" : "radial_gradient4_nodither");
    }

    SkISize onISize() override { return {500, 500}; }

    void onOnceBeforeDraw() override {
        const SkPoint center = { 250, 250 };
        const SkScalar kRadius = 250;
        const SkColor colors[] = { SK_ColorRED, SK_ColorRED, SK_ColorWHITE, SK_ColorWHITE,
                SK_ColorRED };
        const SkScalar pos[] = { 0, .4f, .4f, .8f, .8f, 1 };
        fShader = SkGradientShader::MakeRadial(center, kRadius, colors, pos,
                                               SK_ARRAY_COUNT(gColors), SkTileMode::kClamp);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(fDither);
        paint.setShader(fShader);
        canvas->drawRect(SkRect::MakeWH(500, 500), paint);
    }

private:
    sk_sp<SkShader> fShader;
    bool fDither;

    typedef GM INHERITED;
};
DEF_GM( return new RadialGradient4GM(true); )
DEF_GM( return new RadialGradient4GM(false); )

class LinearGradientGM : public skiagm::GM {
public:
    LinearGradientGM(bool dither) : fDither(dither) { }

private:
    SkString onShortName() override {
        return SkString(fDither ? "linear_gradient" : "linear_gradient_nodither");
    }

    const SkScalar kWidthBump = 30.f;
    const SkScalar kHeight = 5.f;
    const SkScalar kMinWidth = 540.f;

    SkISize onISize() override { return {500, 500}; }

    void onOnceBeforeDraw() override {
        SkPoint pts[2] = { {0, 0}, {0, 0} };
        const SkColor colors[] = { SK_ColorWHITE, SK_ColorWHITE, 0xFF008200, 0xFF008200,
                SK_ColorWHITE, SK_ColorWHITE };
        const SkScalar unitPos[] = { 0, 50, 70, 500, 540 };
        SkScalar pos[6];
        pos[5] = 1;
        for (int index = 0; index < (int) SK_ARRAY_COUNT(fShader); ++index) {
            pts[1].fX = 500.f + index * kWidthBump;
            for (int inner = 0; inner < (int) SK_ARRAY_COUNT(unitPos); ++inner) {
                pos[inner] = unitPos[inner] / (kMinWidth + index * kWidthBump);
            }
            fShader[index] = SkGradientShader::MakeLinear(pts, colors, pos,
                    SK_ARRAY_COUNT(gColors), SkTileMode::kClamp);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(fDither);
        for (int index = 0; index < (int) SK_ARRAY_COUNT(fShader); ++index) {
            paint.setShader(fShader[index]);
            canvas->drawRect(SkRect::MakeLTRB(0, index * kHeight, kMinWidth + index * kWidthBump,
                    (index + 1) * kHeight), paint);
        }
    }

private:
    sk_sp<SkShader> fShader[100];
    bool fDither;

    typedef GM INHERITED;
};
DEF_GM( return new LinearGradientGM(true); )
DEF_GM( return new LinearGradientGM(false); )

class LinearGradientTinyGM : public skiagm::GM {
    static constexpr uint32_t kFlags = 0;

    SkString onShortName() override { return SkString("linear_gradient_tiny"); }

    SkISize onISize() override { return {600, 500}; }

    void onDraw(SkCanvas* canvas) override {
        const SkScalar kRectSize = 100;
        const unsigned kStopCount = 3;
        const SkColor colors[kStopCount] = { SK_ColorGREEN, SK_ColorRED, SK_ColorGREEN };
        const struct {
            SkPoint pts[2];
            SkScalar pos[kStopCount];
        } configs[] = {
            { { SkPoint::Make(0, 0),        SkPoint::Make(10, 0) },       { 0, 0.999999f,    1 }},
            { { SkPoint::Make(0, 0),        SkPoint::Make(10, 0) },       { 0, 0.000001f,    1 }},
            { { SkPoint::Make(0, 0),        SkPoint::Make(10, 0) },       { 0, 0.999999999f, 1 }},
            { { SkPoint::Make(0, 0),        SkPoint::Make(10, 0) },       { 0, 0.000000001f, 1 }},

            { { SkPoint::Make(0, 0),        SkPoint::Make(0, 10) },       { 0, 0.999999f,    1 }},
            { { SkPoint::Make(0, 0),        SkPoint::Make(0, 10) },       { 0, 0.000001f,    1 }},
            { { SkPoint::Make(0, 0),        SkPoint::Make(0, 10) },       { 0, 0.999999999f, 1 }},
            { { SkPoint::Make(0, 0),        SkPoint::Make(0, 10) },       { 0, 0.000000001f, 1 }},

            { { SkPoint::Make(0, 0),        SkPoint::Make(0.00001f, 0) }, { 0, 0.5f, 1 }},
            { { SkPoint::Make(9.99999f, 0), SkPoint::Make(10, 0) },       { 0, 0.5f, 1 }},
            { { SkPoint::Make(0, 0),        SkPoint::Make(0, 0.00001f) }, { 0, 0.5f, 1 }},
            { { SkPoint::Make(0, 9.99999f), SkPoint::Make(0, 10) },       { 0, 0.5f, 1 }},
        };

        SkPaint paint;
        for (unsigned i = 0; i < SK_ARRAY_COUNT(configs); ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            paint.setShader(SkGradientShader::MakeLinear(configs[i].pts, colors, configs[i].pos,
                                                         kStopCount, SkTileMode::kClamp,
                                                         kFlags, nullptr));
            canvas->translate(kRectSize * ((i % 4) * 1.5f + 0.25f),
                              kRectSize * ((i / 4) * 1.5f + 0.25f));

            canvas->drawRect(SkRect::MakeWH(kRectSize, kRectSize), paint);
        }
    }
};

DEF_GM( return new LinearGradientTinyGM; )
}  // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////

struct GradRun {
    SkColor  fColors[4];
    SkScalar fPos[4];
    int      fCount;
};

#define SIZE 121

static sk_sp<SkShader> make_linear(const GradRun& run, SkTileMode mode) {
    const SkPoint pts[] { { 30, 30 }, { SIZE - 30, SIZE - 30 } };
    return SkGradientShader::MakeLinear(pts, run.fColors, run.fPos, run.fCount, mode);
}

static sk_sp<SkShader> make_radial(const GradRun& run, SkTileMode mode) {
    const SkScalar half = SIZE * 0.5f;
    return SkGradientShader::MakeRadial({half,half}, half - 10, run.fColors, run.fPos,
                                        run.fCount, mode);
}

static sk_sp<SkShader> make_conical(const GradRun& run, SkTileMode mode) {
    const SkScalar half = SIZE * 0.5f;
    const SkPoint center { half, half };
    return SkGradientShader::MakeTwoPointConical(center, 20, center, half - 10,
                                                 run.fColors, run.fPos, run.fCount, mode);
}

static sk_sp<SkShader> make_sweep(const GradRun& run, SkTileMode) {
    const SkScalar half = SIZE * 0.5f;
    return SkGradientShader::MakeSweep(half, half, run.fColors, run.fPos, run.fCount);
}

/*
 *  Exercise duplicate color-stops, at the ends, and in the middle
 *
 *  At the time of this writing, only Linear correctly deals with duplicates at the ends,
 *  and then only correctly on CPU backend.
 */
DEF_SIMPLE_GM(gradients_dup_color_stops, canvas, 704, 564) {
    const SkColor preColor  = 0xFFFF0000;   // clamp color before start
    const SkColor postColor = 0xFF0000FF;   // clamp color after end
    const SkColor color0    = 0xFF000000;
    const SkColor color1    = 0xFF00FF00;
    const SkColor badColor  = 0xFF3388BB;   // should never be seen, fills out fixed-size array

    const GradRun runs[] = {
        {   { color0, color1, badColor, badColor },
            { 0, 1, -1, -1 },
            2,
        },
        {   { preColor, color0, color1, badColor },
            { 0, 0, 1, -1 },
            3,
        },
        {   { color0, color1, postColor, badColor },
            { 0, 1, 1, -1 },
            3,
        },
        {   { preColor, color0, color1, postColor },
            { 0, 0, 1, 1 },
            4,
        },
        {   { color0, color0, color1, color1 },
            { 0, 0.5f, 0.5f, 1 },
            4,
        },
    };
    sk_sp<SkShader> (*factories[])(const GradRun&, SkTileMode) {
        make_linear, make_radial, make_conical, make_sweep
    };

    const SkRect rect = SkRect::MakeWH(SIZE, SIZE);
    const SkScalar dx = SIZE + 20;
    const SkScalar dy = SIZE + 20;
    const SkTileMode mode = SkTileMode::kClamp;

    SkPaint paint;
    canvas->translate(10, 10 - dy);
    for (auto factory : factories) {
        canvas->translate(0, dy);
        SkAutoCanvasRestore acr(canvas, true);
        for (const auto& run : runs) {
            paint.setShader(factory(run, mode));
            canvas->drawRect(rect, paint);
            canvas->translate(dx, 0);
        }
    }
}

static void draw_many_stops(SkCanvas* canvas) {
    const unsigned kStopCount = 200;
    const SkPoint pts[] = { {50, 50}, {450, 465}};

    SkColor colors[kStopCount];
    for (unsigned i = 0; i < kStopCount; i++) {
        switch (i % 5) {
        case 0: colors[i] = SK_ColorRED; break;
        case 1: colors[i] = SK_ColorGREEN; break;
        case 2: colors[i] = SK_ColorGREEN; break;
        case 3: colors[i] = SK_ColorBLUE; break;
        case 4: colors[i] = SK_ColorRED; break;
        }
    }

    SkPaint p;
    p.setShader(SkGradientShader::MakeLinear(
        pts, colors, nullptr, SK_ARRAY_COUNT(colors), SkTileMode::kClamp));

    canvas->drawRect(SkRect::MakeXYWH(0, 0, 500, 500), p);
}

DEF_SIMPLE_GM(gradient_many_stops, canvas, 500, 500) {
    draw_many_stops(canvas);
}

static void draw_circle_shader(SkCanvas* canvas, SkScalar cx, SkScalar cy, SkScalar r,
                               sk_sp<SkShader> (*shaderFunc)()) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setShader(shaderFunc());
    canvas->drawCircle(cx, cy, r, p);

    p.setShader(nullptr);
    p.setColor(SK_ColorGRAY);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(2);
    canvas->drawCircle(cx, cy, r, p);
}

DEF_SIMPLE_GM(fancy_gradients, canvas, 800, 300) {
    draw_circle_shader(canvas, 150, 150, 100, []() -> sk_sp<SkShader> {
        // Checkerboard using two linear gradients + picture shader.
        SkScalar kTileSize = 80 / sqrtf(2);
        SkColor colors1[] = { 0xff000000, 0xff000000,
                              0xffffffff, 0xffffffff,
                              0xff000000, 0xff000000 };
        SkColor colors2[] = { 0xff000000, 0xff000000,
                              0x00000000, 0x00000000,
                              0xff000000, 0xff000000 };
        SkScalar pos[] = { 0, .25f, .25f, .75f, .75f, 1 };
        static_assert(SK_ARRAY_COUNT(colors1) == SK_ARRAY_COUNT(pos), "color/pos size mismatch");
        static_assert(SK_ARRAY_COUNT(colors2) == SK_ARRAY_COUNT(pos), "color/pos size mismatch");

        SkPictureRecorder recorder;
        recorder.beginRecording(SkRect::MakeWH(kTileSize, kTileSize));

        SkPaint p;

        SkPoint pts1[] = { { 0, 0 }, { kTileSize, kTileSize }};
        p.setShader(SkGradientShader::MakeLinear(pts1, colors1, pos, SK_ARRAY_COUNT(colors1),
                                                 SkTileMode::kClamp, 0, nullptr));
        recorder.getRecordingCanvas()->drawPaint(p);

        SkPoint pts2[] = { { 0, kTileSize }, { kTileSize, 0 }};
        p.setShader(SkGradientShader::MakeLinear(pts2, colors2, pos, SK_ARRAY_COUNT(colors2),
                                                 SkTileMode::kClamp, 0, nullptr));
        recorder.getRecordingCanvas()->drawPaint(p);

        SkMatrix m = SkMatrix::I();
        m.preRotate(45);
        return recorder.finishRecordingAsPicture()->makeShader(
                                           SkTileMode::kRepeat,
                                           SkTileMode::kRepeat, &m, nullptr);
    });

    draw_circle_shader(canvas, 400, 150, 100, []() -> sk_sp<SkShader> {
        // Checkerboard using a sweep gradient + picture shader.
        SkScalar kTileSize = 80;
        SkColor colors[] = { 0xff000000, 0xff000000,
                             0xffffffff, 0xffffffff,
                             0xff000000, 0xff000000,
                             0xffffffff, 0xffffffff };
        SkScalar pos[] = { 0, .25f, .25f, .5f, .5f, .75f, .75f, 1 };
        static_assert(SK_ARRAY_COUNT(colors) == SK_ARRAY_COUNT(pos), "color/pos size mismatch");

        SkPaint p;
        p.setShader(SkGradientShader::MakeSweep(kTileSize / 2, kTileSize / 2,
                                                colors, pos, SK_ARRAY_COUNT(colors), 0, nullptr));
        SkPictureRecorder recorder;
        recorder.beginRecording(SkRect::MakeWH(kTileSize, kTileSize))->drawPaint(p);

        return recorder.finishRecordingAsPicture()->makeShader(
                                           SkTileMode::kRepeat,
                                           SkTileMode::kRepeat);
    });

    draw_circle_shader(canvas, 650, 150, 100, []() -> sk_sp<SkShader> {
        // Dartboard using sweep + radial.
        const SkColor a = 0xffffffff;
        const SkColor b = 0xff000000;
        SkColor colors[] = { a, a, b, b, a, a, b, b, a, a, b, b, a, a, b, b};
        SkScalar pos[] = { 0, .125f, .125f, .25f, .25f, .375f, .375f, .5f, .5f,
                           .625f, .625f, .75f, .75f, .875f, .875f, 1};
        static_assert(SK_ARRAY_COUNT(colors) == SK_ARRAY_COUNT(pos), "color/pos size mismatch");

        SkPoint center = { 650, 150 };
        sk_sp<SkShader> sweep1 = SkGradientShader::MakeSweep(center.x(), center.y(), colors, pos,
                                                             SK_ARRAY_COUNT(colors), 0, nullptr);
        SkMatrix m = SkMatrix::I();
        m.preRotate(22.5f, center.x(), center.y());
        sk_sp<SkShader> sweep2 = SkGradientShader::MakeSweep(center.x(), center.y(), colors, pos,
                                                             SK_ARRAY_COUNT(colors), 0, &m);

        sk_sp<SkShader> sweep(SkShaders::Blend(SkBlendMode::kExclusion, sweep1, sweep2));

        SkScalar radialPos[] = { 0, .02f, .02f, .04f, .04f, .08f, .08f, .16f, .16f, .31f, .31f,
                                 .62f, .62f, 1, 1, 1 };
        static_assert(SK_ARRAY_COUNT(colors) == SK_ARRAY_COUNT(radialPos),
                      "color/pos size mismatch");

        return SkShaders::Blend(SkBlendMode::kExclusion, sweep,
                                SkGradientShader::MakeRadial(center, 100, colors,
                                                             radialPos,
                                                             SK_ARRAY_COUNT(radialPos),
                                                             SkTileMode::kClamp));
    });
}

DEF_SIMPLE_GM(sweep_tiling, canvas, 690, 512) {
    static constexpr SkScalar size = 160;
    static constexpr SkColor colors[] = { SK_ColorBLUE, SK_ColorYELLOW, SK_ColorGREEN };
    static constexpr SkScalar   pos[] = { 0, .25f, .50f };
    static_assert(SK_ARRAY_COUNT(colors) == SK_ARRAY_COUNT(pos), "size mismatch");

    static constexpr SkTileMode modes[] = { SkTileMode::kClamp,
                                            SkTileMode::kRepeat,
                                            SkTileMode::kMirror };

    static const struct {
        SkScalar start, end;
    } angles[] = {
        { -330, -270 },
        {   30,   90 },
        {  390,  450 },
        {  -30,  800 },
    };

    SkPaint p;
    const SkRect r = SkRect::MakeWH(size, size);

    for (auto mode : modes) {
        {
            SkAutoCanvasRestore acr(canvas, true);

            for (auto angle : angles) {
                p.setShader(SkGradientShader::MakeSweep(size / 2, size / 2, colors, pos,
                                                        SK_ARRAY_COUNT(colors), mode,
                                                        angle.start, angle.end, 0, nullptr));

                canvas->drawRect(r, p);
                canvas->translate(size * 1.1f, 0);
            }
        }
        canvas->translate(0, size * 1.1f);
    }
}

// Exercises the special-case Ganesh gradient effects.
DEF_SIMPLE_GM(gradients_interesting, canvas, 640, 1300) {
    static const SkColor colors2[] = { SK_ColorRED, SK_ColorBLUE };
    static const SkColor colors3[] = { SK_ColorRED, SK_ColorYELLOW, SK_ColorBLUE };
    static const SkColor colors4[] = { SK_ColorRED, SK_ColorYELLOW, SK_ColorYELLOW, SK_ColorBLUE };

    static const SkScalar softRight[]  = { 0, .999f,   1 }; // Based on Android launcher "clipping"
    static const SkScalar hardLeft[]   = { 0,     0,   1 };
    static const SkScalar hardRight[]  = { 0,     1,   1 };
    static const SkScalar hardCenter[] = { 0,   .5f, .5f, 1 };

    static const struct {
        const SkColor*  colors;
        const SkScalar* pos;
        int             count;
    } configs[] = {
        { colors2,    nullptr, 2 }, // kTwo_ColorType
        { colors3,    nullptr, 3 }, // kThree_ColorType (simple)
        { colors3,  softRight, 3 }, // kThree_ColorType (tricky)
        { colors3,   hardLeft, 3 }, // kHardStopLeftEdged_ColorType
        { colors3,  hardRight, 3 }, // kHardStopRightEdged_ColorType
        { colors4, hardCenter, 4 }, // kSingleHardStop_ColorType
    };

    static const SkTileMode modes[] = {
        SkTileMode::kClamp,
        SkTileMode::kRepeat,
        SkTileMode::kMirror,
    };

    static constexpr SkScalar size = 200;
    static const SkPoint pts[] = { { size / 3, size / 3 }, { size * 2 / 3, size * 2 / 3} };

    SkPaint p;
    for (const auto& cfg : configs) {
        {
            SkAutoCanvasRestore acr(canvas, true);
            for (auto mode : modes) {
                p.setShader(SkGradientShader::MakeLinear(pts, cfg.colors, cfg.pos, cfg.count,
                                                         mode));
                canvas->drawRect(SkRect::MakeWH(size, size), p);
                canvas->translate(size * 1.1f, 0);
            }
        }
        canvas->translate(0, size * 1.1f);
    }
}
