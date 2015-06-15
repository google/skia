/*
 * Copyright 2014 Google Inc.
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
    { 2, gColors, gPos0 },
    { 2, gColors, gPos1 },
    { 5, gColors, gPos2 },
    { 4, gColorClamp, gPosClamp }
};

static SkShader* Make2ConicalOutside(const SkPoint pts[2], const GradData& data,
                                     SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalOutsideFlip(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                   center0, radius0,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalInside(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

static SkShader* Make2ConicalInsideFlip(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center0, (pts[1].fX - pts[0].fX) / 2,
                                                   center1, (pts[1].fX - pts[0].fX) / 7,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

static SkShader* Make2ConicalInsideCenter(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center0, (pts[1].fX - pts[0].fX) / 7,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

static SkShader* Make2ConicalZeroRad(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center1, 0.f,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

static SkShader* Make2ConicalZeroRadFlip(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 2,
                                                   center0, 0.f,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

static SkShader* Make2ConicalZeroRadCenter(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center0, 0.f,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

static SkShader* Make2ConicalZeroRadOutside(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalZeroRadFlipOutside(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                   center0, radius0,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalEdgeX(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX + radius1, center1.fY);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalEdgeY(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX, center1.fY + radius1);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalZeroRadEdgeX(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX + radius1, center1.fY);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalZeroRadEdgeY(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX, center1.fY + radius1);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalTouchX(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX - radius1 + radius0, center1.fY);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalTouchY(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX, center1.fY + radius1 - radius0);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm, 0, &localMatrix);
}

static SkShader* Make2ConicalInsideSmallRad(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center0, 0.0000000000000000001f,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data,
                               SkShader::TileMode tm, const SkMatrix& localMatrix);

static const GradMaker gGradMakersOutside[] = {
    Make2ConicalOutside, Make2ConicalOutsideFlip,
    Make2ConicalZeroRadOutside, Make2ConicalZeroRadFlipOutside
};

static const GradMaker gGradMakersInside[] = {
    Make2ConicalInside, Make2ConicalInsideFlip, Make2ConicalInsideCenter,
    Make2ConicalZeroRad, Make2ConicalZeroRadFlip, Make2ConicalZeroRadCenter,
};

static const GradMaker gGradMakersEdgeCases[] = {
    Make2ConicalEdgeX, Make2ConicalEdgeY,
    Make2ConicalZeroRadEdgeX, Make2ConicalZeroRadEdgeY,
    Make2ConicalTouchX, Make2ConicalTouchY,
    Make2ConicalInsideSmallRad
};


static const struct {
    const GradMaker*   fMaker;
    const int fCount;
    const char* fName;
} gGradCases[] = {
    { gGradMakersOutside,   SK_ARRAY_COUNT(gGradMakersOutside),     "outside"  },
    { gGradMakersInside,    SK_ARRAY_COUNT(gGradMakersInside),      "inside"  },
    { gGradMakersEdgeCases, SK_ARRAY_COUNT(gGradMakersEdgeCases),   "edge"  },
};

enum GradCaseType { // these must match the order in gGradCases
    kOutside_GradCaseType,
    kInside_GradCaseType,
    kEdge_GradCaseType,
};

///////////////////////////////////////////////////////////////////////////////

class ConicalGradientsGM : public GM {
public:
    ConicalGradientsGM(GradCaseType gradCaseType) {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
        fName.printf("gradients_2pt_conical_%s", gGradCases[gradCaseType].fName);
        fGradCaseType = gradCaseType;
    }

protected:
    SkString onShortName() {
        return fName;
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

        const GradMaker* gradMaker = gGradCases[fGradCaseType].fMaker;
        const int count = gGradCases[fGradCaseType].fCount;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (int j = 0; j < count; j++) {
                SkMatrix scale = SkMatrix::I();

                if (i == 3) { // if the clamp case
                    scale.setScale(0.5f, 0.5f);
                    scale.postTranslate(25.f, 25.f);
                }

                SkShader* shader = gradMaker[j](pts, gGradData[i], tm, scale);
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

    GradCaseType fGradCaseType;
    SkString fName;
};
///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory1(void*) { return new ConicalGradientsGM(kInside_GradCaseType); }
static GMRegistry reg1(MyFactory1);

static GM* MyFactory2(void*) { return new ConicalGradientsGM(kOutside_GradCaseType); }
static GMRegistry reg2(MyFactory2);

static GM* MyFactory3(void*) { return new ConicalGradientsGM(kEdge_GradCaseType); }
static GMRegistry reg3(MyFactory3);
}
