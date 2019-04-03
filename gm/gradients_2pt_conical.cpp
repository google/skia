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

constexpr SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK
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

constexpr GradData gGradData[] = {
    { 2, gColors, gPos0 },
    { 2, gColors, gPos1 },
    { 5, gColors, gPos2 },
    { 4, gColorClamp, gPosClamp }
};

static sk_sp<SkShader> Make2ConicalOutside(const SkPoint pts[2], const GradData& data,
                                           SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalOutsideStrip(const SkPoint pts[2], const GradData& data,
                                                SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX, pts[0].fY);
    center1.set(pts[1].fX, pts[1].fY);
    return SkGradientShader::MakeTwoPointConical(center0, radius, center1, radius, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalOutsideFlip(const SkPoint pts[2], const GradData& data,
                             SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::MakeTwoPointConical(center1, radius1, center0, radius0, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalInside(const SkPoint pts[2], const GradData& data,
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

static sk_sp<SkShader> Make2ConicalInsideFlip(const SkPoint pts[2], const GradData& data,
                                              SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center0, (pts[1].fX - pts[0].fX) / 2,
                                                 center1, (pts[1].fX - pts[0].fX) / 7,
                                                 data.fColors, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalInsideCenter(const SkPoint pts[2], const GradData& data,
                             SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeTwoPointConical(center0, (pts[1].fX - pts[0].fX) / 7,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 data.fColors, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalInsideCenterReversed(const SkPoint pts[2], const GradData& data,
                             SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeTwoPointConical(center0, (pts[1].fX - pts[0].fX) / 2,
                                                 center0, (pts[1].fX - pts[0].fX) / 7,
                                                 data.fColors, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalZeroRad(const SkPoint pts[2], const GradData& data,
                                           SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center1, 0.f,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 data.fColors, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalZeroRadFlip(const SkPoint pts[2], const GradData& data,
                                               SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 2,
                                                 center0, 0.f,
                                                 data.fColors, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalZeroRadCenter(const SkPoint pts[2], const GradData& data,
                             SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center0, 0.f, center0, (pts[1].fX - pts[0].fX) / 2,
                                                 data.fColors, data.fPos, data.fCount, tm,
                                                 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalZeroRadOutside(const SkPoint pts[2], const GradData& data,
                                                  SkTileMode tm,
                                                  const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1,
                                                 data.fColors, data.fPos,
                                                 data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalZeroRadFlipOutside(const SkPoint pts[2], const GradData& data,
                                                      SkTileMode tm,
                                                      const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::MakeTwoPointConical(center1, radius1, center0, radius0, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalEdgeX(const SkPoint pts[2], const GradData& data,
                                         SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX + radius1, center1.fY);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalEdgeY(const SkPoint pts[2], const GradData& data,
                                         SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX, center1.fY + radius1);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalZeroRadEdgeX(const SkPoint pts[2], const GradData& data,
                                                SkTileMode tm,
                                                const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX + radius1, center1.fY);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalZeroRadEdgeY(const SkPoint pts[2], const GradData& data,
                                                SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = 0.f;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX, center1.fY + radius1);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalTouchX(const SkPoint pts[2], const GradData& data,
                                          SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX - radius1 + radius0, center1.fY);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalTouchY(const SkPoint pts[2], const GradData& data,
                                          SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 7;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center1.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center0.set(center1.fX, center1.fY + radius1 - radius0);
    return SkGradientShader::MakeTwoPointConical(center0, radius0, center1, radius1, data.fColors,
                                                 data.fPos, data.fCount, tm, 0, &localMatrix);
}

static sk_sp<SkShader> Make2ConicalInsideSmallRad(const SkPoint pts[2], const GradData& data,
                             SkTileMode tm, const SkMatrix& localMatrix) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center0, 0.0000000000000000001f,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm,
                                                   0, &localMatrix);
}

typedef sk_sp<SkShader> (*GradMaker)(const SkPoint pts[2], const GradData& data,
                                     SkTileMode tm, const SkMatrix& localMatrix);

constexpr GradMaker gGradMakersOutside[] = {
    Make2ConicalOutside, Make2ConicalOutsideFlip,
    Make2ConicalZeroRadOutside, Make2ConicalZeroRadFlipOutside,
    Make2ConicalOutsideStrip
};

constexpr GradMaker gGradMakersInside[] = {
    Make2ConicalInside, Make2ConicalInsideFlip, Make2ConicalInsideCenter,
    Make2ConicalZeroRad, Make2ConicalZeroRadFlip, Make2ConicalZeroRadCenter,
    Make2ConicalInsideCenterReversed
};

constexpr GradMaker gGradMakersEdgeCases[] = {
    Make2ConicalEdgeX, Make2ConicalEdgeY,
    Make2ConicalZeroRadEdgeX, Make2ConicalZeroRadEdgeY,
    Make2ConicalTouchX, Make2ConicalTouchY,
    Make2ConicalInsideSmallRad
};


constexpr struct {
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
    ConicalGradientsGM(GradCaseType gradCaseType, bool dither,
                       SkTileMode mode = SkTileMode::kClamp)
        : fGradCaseType(gradCaseType)
        , fDither(dither)
        , fMode(mode) {
        this->setBGColor(0xFFDDDDDD);
        fName.printf("gradients_2pt_conical_%s%s", gGradCases[gradCaseType].fName,
                     fDither ? "" : "_nodither");
        switch (mode) {
        case SkTileMode::kRepeat:
            fName.appendf("_repeat");
            break;
        case SkTileMode::kMirror:
            fName.appendf("_mirror");
            break;
        default:
            break;
        }
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
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(fDither);

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

                paint.setShader(gradMaker[j](pts, gGradData[i], fMode, scale));
                canvas->drawRect(r, paint);
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
    bool fDither;
    SkTileMode fMode;
};
///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ConicalGradientsGM(kInside_GradCaseType, true); )
DEF_GM( return new ConicalGradientsGM(kOutside_GradCaseType, true); )
DEF_GM( return new ConicalGradientsGM(kEdge_GradCaseType, true); )

DEF_GM( return new ConicalGradientsGM(kInside_GradCaseType, true, SkTileMode::kRepeat); )
DEF_GM( return new ConicalGradientsGM(kOutside_GradCaseType, true, SkTileMode::kRepeat); )
DEF_GM( return new ConicalGradientsGM(kEdge_GradCaseType, true, SkTileMode::kRepeat); )

DEF_GM( return new ConicalGradientsGM(kInside_GradCaseType, true, SkTileMode::kMirror); )
DEF_GM( return new ConicalGradientsGM(kOutside_GradCaseType, true, SkTileMode::kMirror); )
DEF_GM( return new ConicalGradientsGM(kEdge_GradCaseType, true, SkTileMode::kMirror); )

DEF_GM( return new ConicalGradientsGM(kInside_GradCaseType, false); )
DEF_GM( return new ConicalGradientsGM(kOutside_GradCaseType, false); )
DEF_GM( return new ConicalGradientsGM(kEdge_GradCaseType, false); )

}
