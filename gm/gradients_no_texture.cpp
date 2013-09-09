/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkGradientShader.h"

using namespace skiagm;

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
};

static const SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE,
};

static const GradData gGradData[] = {
    { 1, gColors, NULL },
    { 2, gColors, NULL },
    { 3, gColors, NULL },
    { 4, gColors, NULL },
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
    MakeLinear, MakeRadial, MakeSweep, Make2Radial, Make2Conical,
};

///////////////////////////////////////////////////////////////////////////////

class GradientsNoTextureGM : public GM {
public:
    GradientsNoTextureGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() SK_OVERRIDE { return SkString("gradients_no_texture"); }
    virtual SkISize onISize() SK_OVERRIDE { return make_isize(640, 615); }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        static const SkPoint kPts[2] = { { 0, 0 },
                                         { SkIntToScalar(50), SkIntToScalar(50) } };
        static const SkShader::TileMode kTM = SkShader::kClamp_TileMode;
        SkRect kRect = { 0, 0, SkIntToScalar(50), SkIntToScalar(50) };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        static const uint8_t kAlphas[] = { 0xff, 0x40 };
        for (size_t a = 0; a < SK_ARRAY_COUNT(kAlphas); ++a) {
            for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); ++i) {
                canvas->save();
                for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); ++j) {
                    SkShader* shader = gGradMakers[j](kPts, gGradData[i], kTM, NULL);
                    paint.setShader(shader)->unref();
                    paint.setAlpha(kAlphas[a]);
                    canvas->drawRect(kRect, paint);
                    canvas->translate(0, SkIntToScalar(kRect.height() + 20));
                }
                canvas->restore();
                canvas->translate(SkIntToScalar(kRect.width() + 20), 0);
            }
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(GradientsNoTextureGM));
