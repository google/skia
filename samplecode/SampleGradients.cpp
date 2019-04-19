/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"

static sk_sp<SkShader> setgrad(const SkRect& r, SkColor c0, SkColor c1) {
    SkColor colors[] = { c0, c1 };
    SkPoint pts[] = { { r.fLeft, r.fTop }, { r.fRight, r.fTop } };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

static void test_alphagradients(SkCanvas* canvas) {
    SkRect r;
    r.set(SkIntToScalar(10), SkIntToScalar(10),
          SkIntToScalar(410), SkIntToScalar(30));
    SkPaint p, p2;
    p2.setStyle(SkPaint::kStroke_Style);

    p.setShader(setgrad(r, 0xFF00FF00, 0x0000FF00));
    canvas->drawRect(r, p);
    canvas->drawRect(r, p2);

    r.offset(0, r.height() + SkIntToScalar(4));
    p.setShader(setgrad(r, 0xFF00FF00, 0x00000000));
    canvas->drawRect(r, p);
    canvas->drawRect(r, p2);

    r.offset(0, r.height() + SkIntToScalar(4));
    p.setShader(setgrad(r, 0xFF00FF00, 0x00FF0000));
    canvas->drawRect(r, p);
    canvas->drawRect(r, p2);
}

///////////////////////////////////////////////////////////////////////////////

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

static const GradData gGradData[] = {
    { 2, gColors, nullptr },
    { 2, gColors, gPos0 },
    { 2, gColors, gPos1 },
    { 5, gColors, nullptr },
    { 5, gColors, gPos2 }
};

static sk_sp<SkShader> MakeLinear(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    return SkGradientShader::MakeLinear(pts, data.fColors, data.fPos, data.fCount, tm);
}

static sk_sp<SkShader> MakeRadial(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeRadial(center, center.fX, data.fColors,
                                        data.fPos, data.fCount, tm);
}

static sk_sp<SkShader> MakeSweep(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeSweep(center.fX, center.fY, data.fColors, data.fPos, data.fCount);
}

static sk_sp<SkShader> Make2Conical(const SkPoint pts[2], const GradData& data, SkTileMode tm) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(
                            center1, (pts[1].fX - pts[0].fX) / 7,
                            center0, (pts[1].fX - pts[0].fX) / 2,
                            data.fColors, data.fPos, data.fCount, tm);
}

static sk_sp<SkShader> Make2ConicalConcentric(const SkPoint pts[2], const GradData& data,
                                       SkTileMode tm) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeTwoPointConical(
                            center, (pts[1].fX - pts[0].fX) / 7,
                            center, (pts[1].fX - pts[0].fX) / 2,
                            data.fColors, data.fPos, data.fCount, tm);
}

typedef sk_sp<SkShader> (*GradMaker)(const SkPoint pts[2], const GradData& data, SkTileMode tm);

static const GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep, Make2Conical, Make2ConicalConcentric
};

///////////////////////////////////////////////////////////////////////////////

class GradientsView : public Sample {
public:
    GradientsView() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Gradients");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setDither(true);

        canvas->save();
        canvas->translate(SkIntToScalar(20), SkIntToScalar(10));

        for (int tm = 0; tm < kSkTileModeCount; ++tm) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
                canvas->save();
                for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
                    paint.setShader(gGradMakers[j](pts, gGradData[i], (SkTileMode)tm));
                    canvas->drawRect(r, paint);
                    canvas->translate(0, SkIntToScalar(120));
                }
                canvas->restore();
                canvas->translate(SkIntToScalar(120), 0);
            }
            canvas->restore();
            canvas->translate(SK_ARRAY_COUNT(gGradData)*SkIntToScalar(120), 0);
        }
        canvas->restore();

        canvas->translate(0, SkIntToScalar(370));
        if (false) { // avoid bit rot, suppress warning
            test_alphagradients(canvas);
        }
    }

private:
    typedef Sample INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new GradientsView(); )
