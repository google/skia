
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkGradientShader.h"

namespace skiagm {

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
};

static const SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK
};
static const SkScalar gPos0[] = { 0, SK_Scalar1 };
static const SkScalar gPos1[] = { SK_Scalar1/4, SK_Scalar1*3/4 };
static const SkScalar gPos2[] = {
    0, SK_Scalar1/8, SK_Scalar1/2, SK_Scalar1*7/8, SK_Scalar1
};

static const SkScalar gPosClamp[]   = {0.0f, 0.0f, 1.0f, 1.0f};
static const SkColor  gColorClamp[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorGREEN, SK_ColorBLUE
};

static const GradData gGradData[] = {
    { 2, gColors, NULL },
    { 2, gColors, gPos0 },
    { 2, gColors, gPos1 },
    { 5, gColors, NULL },
    { 5, gColors, gPos2 },
    { 4, gColorClamp, gPosClamp }
};

static SkShader* MakeLinear(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper) {
    return SkGradientShader::CreateLinear(pts, data.fColors, data.fPos,
                                          data.fCount, tm, mapper);
}

static SkShader* MakeRadial(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateRadial(center, center.fX, data.fColors,
                                          data.fPos, data.fCount, tm, mapper);
}

static SkShader* MakeSweep(const SkPoint pts[2], const GradData& data,
                           SkShader::TileMode, SkUnitMapper* mapper) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateSweep(center.fX, center.fY, data.fColors,
                                         data.fPos, data.fCount, mapper);
}

static SkShader* Make2Radial(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointRadial(
                                                  center1, (pts[1].fX - pts[0].fX) / 7,
                                                  center0, (pts[1].fX - pts[0].fX) / 2,
                                                  data.fColors, data.fPos, data.fCount, tm, mapper);
}

static SkShader* Make2Conical(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center0, center1;
    SkScalar radius0 = SkScalarDiv(pts[1].fX - pts[0].fX, 10);
    SkScalar radius1 = SkScalarDiv(pts[1].fX - pts[0].fX, 3);
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                   center0, radius0,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, mapper);
}

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data,
                               SkShader::TileMode tm, SkUnitMapper* mapper);
static const GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep, Make2Radial, Make2Conical
};

///////////////////////////////////////////////////////////////////////////////

class GradientsGM : public GM {
public:
    GradientsGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() {
        return SkString("gradients");
    }

    virtual SkISize onISize() { return SkISize::Make(840, 815); }

    virtual void onDraw(SkCanvas* canvas) {

        SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkShader::TileMode tm = SkShader::kClamp_TileMode;
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
                SkShader* shader = gGradMakers[j](pts, gGradData[i], tm, NULL);

                if (i == 5) { // if the clamp case
                    SkMatrix scale;
                    scale.setScale(0.5f, 0.5f);
                    scale.postTranslate(25.f, 25.f);
                    shader->setLocalMatrix(scale);
                }

                paint.setShader(shader);
                canvas->drawRect(r, paint);
                shader->unref();
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
    }

private:
    typedef GM INHERITED;
};

// Based on the original gradient slide, but with perspective applied to the
// gradient shaders' local matrices
class GradientsLocalPerspectiveGM : public GM {
public:
    GradientsLocalPerspectiveGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() {
        return SkString("gradients_local_perspective");
    }

    virtual SkISize onISize() { return SkISize::Make(840, 815); }

    virtual void onDraw(SkCanvas* canvas) {

        SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkShader::TileMode tm = SkShader::kClamp_TileMode;
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
                SkShader* shader = gGradMakers[j](pts, gGradData[i], tm, NULL);

                // apply an increasing y perspective as we move to the right
                SkMatrix perspective;
                perspective.setIdentity();
                perspective.setPerspY(SkScalarDiv(SkIntToScalar((unsigned) i+1),
                                      SkIntToScalar(500)));
                perspective.setSkewX(SkScalarDiv(SkIntToScalar((unsigned) i+1),
                                     SkIntToScalar(10)));

                shader->setLocalMatrix(perspective);

                paint.setShader(shader);
                canvas->drawRect(r, paint);
                shader->unref();
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
    }

private:
    typedef GM INHERITED;
};

// Based on the original gradient slide, but with perspective applied to
// the view matrix
class GradientsViewPerspectiveGM : public GradientsGM {
protected:
    SkString onShortName() {
        return SkString("gradients_view_perspective");
    }

    virtual SkISize onISize() { return SkISize::Make(840, 500); }

    virtual void onDraw(SkCanvas* canvas) {
        SkMatrix perspective;
        perspective.setIdentity();
        perspective.setPerspY(SkScalarDiv(SK_Scalar1, SkIntToScalar(1000)));
        perspective.setSkewX(SkScalarDiv(SkIntToScalar(8), SkIntToScalar(25)));
        canvas->concat(perspective);
        INHERITED::onDraw(canvas);
    }

private:
    typedef GradientsGM INHERITED;
};

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
class GradientsDegenrate2PointGM : public GM {
public:
    GradientsDegenrate2PointGM() {}

protected:
    SkString onShortName() {
        return SkString("gradients_degenerate_2pt");
    }

    virtual SkISize onISize() { return SkISize::Make(320, 320); }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorBLUE);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorGREEN, SK_ColorRED };
        SkScalar pos[] = { 0, 0.01f, 0.99f, SK_Scalar1 };
        SkPoint c0;
        c0.iset(-80, 25);
        SkScalar r0 = SkIntToScalar(70);
        SkPoint c1;
        c1.iset(0, 25);
        SkScalar r1 = SkIntToScalar(150);
        SkShader* s = SkGradientShader::CreateTwoPointRadial(c0, r0, c1, r1, colors,
                                                             pos, SK_ARRAY_COUNT(pos),
                                                             SkShader::kClamp_TileMode);
        SkPaint paint;
        paint.setShader(s)->unref();
        canvas->drawPaint(paint);
    }

private:
    typedef GM INHERITED;
};

/// Tests correctness of *optimized* codepaths in gradients.

class ClampedGradientsGM : public GM {
public:
    ClampedGradientsGM() {}

protected:
    SkString onShortName() { return SkString("clamped_gradients"); }

    virtual SkISize onISize() { return SkISize::Make(640, 510); }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(300) };
        SkPaint paint;
        paint.setAntiAlias(true);

        SkPoint center;
        center.iset(0, 300);
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        SkShader* shader = SkGradientShader::CreateRadial(
            SkPoint(center),
            SkIntToScalar(200), gColors, NULL, 5,
            SkShader::kClamp_TileMode, NULL);
        paint.setShader(shader);
        canvas->drawRect(r, paint);
        shader->unref();
    }

private:
    typedef GM INHERITED;
};

/// Checks quality of large radial gradients, which may display
/// some banding.

class RadialGradientGM : public GM {
public:
    RadialGradientGM() {}

protected:
    SkString onShortName() { return SkString("radial_gradient"); }
    virtual SkISize onISize() { return SkISize::Make(1280, 1280); }
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFF000000);
    }
    virtual void onDraw(SkCanvas* canvas) {
        const SkISize dim = this->getISize();

        this->drawBG(canvas);

        SkPaint paint;
        paint.setDither(true);
        SkPoint center;
        center.set(SkIntToScalar(dim.width())/2, SkIntToScalar(dim.height())/2);
        SkScalar radius = SkIntToScalar(dim.width())/2;
        const SkColor colors[] = { 0x7f7f7f7f, 0x7f7f7f7f, 0xb2000000 };
        const SkScalar pos[] = { 0.0f,
                             0.35f,
                             1.0f };
        SkShader* shader =
            SkGradientShader::CreateRadial(center, radius, colors,
                                           pos, SK_ARRAY_COUNT(pos),
                                           SkShader::kClamp_TileMode);
        paint.setShader(shader)->unref();
        SkRect r = {
            0, 0, SkIntToScalar(dim.width()), SkIntToScalar(dim.height())
        };
        canvas->drawRect(r, paint);
    }
private:
    typedef GM INHERITED;
};


class RadialGradient2GM : public GM {
public:
    RadialGradient2GM() {}

protected:
    SkString onShortName() { return SkString("radial_gradient2"); }
    virtual SkISize onISize() { return SkISize::Make(800, 400); }
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFF000000);
    }

    // Reproduces the example given in bug 7671058.
    virtual void onDraw(SkCanvas* canvas) {
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
            SkAutoTUnref<SkShader> sweep(
                    SkGradientShader::CreateSweep(cx, cy, sweep_colors,
                                                  NULL, SK_ARRAY_COUNT(sweep_colors),
                                                  NULL, flags[i]));
            SkAutoTUnref<SkShader> radial1(
                    SkGradientShader::CreateRadial(center, radius, colors1,
                                                   NULL, SK_ARRAY_COUNT(colors1),
                                                   SkShader::kClamp_TileMode,
                                                   NULL, flags[i]));
            SkAutoTUnref<SkShader> radial2(
                    SkGradientShader::CreateRadial(center, radius, colors2,
                                                   NULL, SK_ARRAY_COUNT(colors2),
                                                   SkShader::kClamp_TileMode,
                                                   NULL, flags[i]));
            paint1.setShader(sweep);
            paint2.setShader(radial1);
            paint3.setShader(radial2);

            canvas->drawCircle(cx, cy, radius, paint1);
            canvas->drawCircle(cx, cy, radius, paint3);
            canvas->drawCircle(cx, cy, radius, paint2);

            canvas->translate(400, 0);
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new GradientsGM; }
static GMRegistry reg(MyFactory);

static GM* MyFactory2(void*) { return new GradientsDegenrate2PointGM; }
static GMRegistry reg2(MyFactory2);

static GM* MyFactory3(void*) { return new ClampedGradientsGM; }
static GMRegistry reg3(MyFactory3);

static GM* MyFactory4(void*) { return new RadialGradientGM; }
static GMRegistry reg4(MyFactory4);

static GM* MyFactory5(void*) { return new GradientsLocalPerspectiveGM; }
static GMRegistry reg5(MyFactory5);

static GM* MyFactory6(void*) { return new GradientsViewPerspectiveGM; }
static GMRegistry reg6(MyFactory6);

static GM* MyFactory7(void*) { return new RadialGradient2GM; }
static GMRegistry reg7(MyFactory7);
}
