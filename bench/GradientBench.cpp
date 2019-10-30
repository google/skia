/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkGradientShader.h"

#include "tools/ToolUtils.h"

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
    const char*     fName;
};

static const SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK,
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK, // 10 lines, 50 colors
};

static const SkColor gShallowColors[] = { 0xFF555555, 0xFF444444 };
static const SkScalar gPos[] = {0.25f, 0.75f};

// We have several special-cases depending on the number (and spacing) of colors, so
// try to exercise those here.
static const GradData gGradData[] = {
    { 2, gColors, nullptr, "" },
    { 50, gColors, nullptr, "_hicolor" }, // many color gradient
    { 3, gColors, nullptr, "_3color" },
    { 2, gShallowColors, nullptr, "_shallow" },
    { 2, gColors, gPos, "_pos" },
};

/// Ignores scale
static sk_sp<SkShader> MakeLinear(const SkPoint pts[2], const GradData& data,
                                  SkTileMode tm, float scale) {
    return SkGradientShader::MakeLinear(pts, data.fColors, data.fPos, data.fCount, tm);
}

static sk_sp<SkShader> MakeRadial(const SkPoint pts[2], const GradData& data,
                                  SkTileMode tm, float scale) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeRadial(center, center.fX * scale, data.fColors,
                                        data.fPos, data.fCount, tm);
}

/// Ignores scale
static sk_sp<SkShader> MakeSweep(const SkPoint pts[2], const GradData& data,
                                 SkTileMode tm, float scale) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::MakeSweep(center.fX, center.fY, data.fColors, data.fPos, data.fCount);
}

/// Ignores scale
static sk_sp<SkShader> MakeConical(const SkPoint pts[2], const GradData& data,
                                   SkTileMode tm, float scale) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 data.fColors, data.fPos, data.fCount, tm);
}

/// Ignores scale
static sk_sp<SkShader> MakeConicalZeroRad(const SkPoint pts[2], const GradData& data,
                                          SkTileMode tm, float scale) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center1, 0.0,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 data.fColors, data.fPos, data.fCount, tm);
}

/// Ignores scale
static sk_sp<SkShader> MakeConicalOutside(const SkPoint pts[2], const GradData& data,
                                          SkTileMode tm, float scale) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::MakeTwoPointConical(center0, radius0,
                                                 center1, radius1,
                                                 data.fColors, data.fPos,
                                                 data.fCount, tm);
}

/// Ignores scale
static sk_sp<SkShader> MakeConicalOutsideZeroRad(const SkPoint pts[2], const GradData& data,
                                                 SkTileMode tm, float scale) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::MakeTwoPointConical(center0, 0.0,
                                                 center1, radius1,
                                                 data.fColors, data.fPos,
                                                 data.fCount, tm);
}

typedef sk_sp<SkShader> (*GradMaker)(const SkPoint pts[2], const GradData& data,
                                     SkTileMode tm, float scale);

static const struct {
    GradMaker   fMaker;
    const char* fName;
} gGrads[] = {
    { MakeLinear,                 "linear"  },
    { MakeRadial,                 "radial1" },
    { MakeSweep,                  "sweep"   },
    { MakeConical,                "conical" },
    { MakeConicalZeroRad,         "conicalZero" },
    { MakeConicalOutside,         "conicalOut" },
    { MakeConicalOutsideZeroRad,  "conicalOutZero" },
};

enum GradType { // these must match the order in gGrads
    kLinear_GradType,
    kRadial_GradType,
    kSweep_GradType,
    kConical_GradType,
    kConicalZero_GradType,
    kConicalOut_GradType,
    kConicalOutZero_GradType
};

enum GeomType {
    kRect_GeomType,
    kOval_GeomType
};

static const char* geomtypename(GeomType gt) {
    switch (gt) {
        case kRect_GeomType:
            return "rectangle";
        case kOval_GeomType:
            return "oval";
        default:
            SkDEBUGFAIL("unknown geometry type");
            return "error";
    }
}

///////////////////////////////////////////////////////////////////////////////

class GradientBench : public Benchmark {
public:
    GradientBench(GradType gradType,
                  GradData data = gGradData[0],
                  SkTileMode tm = SkTileMode::kClamp,
                  GeomType geomType = kRect_GeomType,
                  float scale = 1.0f)
        : fGeomType(geomType) {

        fName.printf("gradient_%s_%s", gGrads[gradType].fName,
                     ToolUtils::tilemode_name(tm));
        if (geomType != kRect_GeomType) {
            fName.appendf("_%s", geomtypename(geomType));
        }

        if (scale != 1.f) {
            fName.appendf("_scale_%g", scale);
        }

        fName.append(data.fName);

        this->setupPaint(&fPaint);
        fPaint.setShader(MakeShader(gradType, data, tm, scale));
    }

    GradientBench(GradType gradType, GradData data, bool dither)
        : fGeomType(kRect_GeomType) {

        const char *tmname = ToolUtils::tilemode_name(SkTileMode::kClamp);
        fName.printf("gradient_%s_%s", gGrads[gradType].fName, tmname);
        fName.append(data.fName);

        if (dither) {
            fName.appendf("_dither");
        }

        this->setupPaint(&fPaint);
        fPaint.setShader(MakeShader(gradType, data, SkTileMode::kClamp, 1.0f));
        fPaint.setDither(dither);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(kSize, kSize);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect r = SkRect::MakeIWH(kSize, kSize);

        for (int i = 0; i < loops; i++) {
            switch (fGeomType) {
               case kRect_GeomType:
                   canvas->drawRect(r, fPaint);
                   break;
               case kOval_GeomType:
                   canvas->drawOval(r, fPaint);
                   break;
            }
        }
    }

private:
    typedef Benchmark INHERITED;

    sk_sp<SkShader> MakeShader(GradType gradType, GradData data,
                               SkTileMode tm, float scale) {
        const SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(kSize), SkIntToScalar(kSize) }
        };

        return gGrads[gradType].fMaker(pts, data, tm, scale);
    }

    static const int kSize = 400;

    SkString       fName;
    SkPaint        fPaint;
    const GeomType fGeomType;
};

DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[0]); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[2]); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[4]); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[0], SkTileMode::kRepeat); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[1], SkTileMode::kRepeat); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[2], SkTileMode::kRepeat); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[0], SkTileMode::kMirror); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[1], SkTileMode::kMirror); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[2], SkTileMode::kMirror); )

DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0]); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[2]); )
// Draw a radial gradient of radius 1/2 on a rectangle; half the lines should
// be completely pinned, the other half should pe partially pinned
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkTileMode::kClamp, kRect_GeomType, 0.5f); )

// Draw a radial gradient on a circle of equal size; all the lines should
// hit the unpinned fast path (so long as GradientBench.W == H)
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkTileMode::kClamp, kOval_GeomType); )

DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkTileMode::kMirror); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkTileMode::kRepeat); )
DEF_BENCH( return new GradientBench(kSweep_GradType); )
DEF_BENCH( return new GradientBench(kSweep_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kSweep_GradType, gGradData[2]); )
DEF_BENCH( return new GradientBench(kConical_GradType); )
DEF_BENCH( return new GradientBench(kConical_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kConical_GradType, gGradData[2]); )
DEF_BENCH( return new GradientBench(kConicalZero_GradType); )
DEF_BENCH( return new GradientBench(kConicalZero_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kConicalZero_GradType, gGradData[2]); )
DEF_BENCH( return new GradientBench(kConicalOut_GradType); )
DEF_BENCH( return new GradientBench(kConicalOut_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kConicalOut_GradType, gGradData[2]); )
DEF_BENCH( return new GradientBench(kConicalOutZero_GradType); )
DEF_BENCH( return new GradientBench(kConicalOutZero_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kConicalOutZero_GradType, gGradData[2]); )

// Dithering
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[3], true); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[3], false); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[3], true); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[3], false); )
DEF_BENCH( return new GradientBench(kSweep_GradType, gGradData[3], true); )
DEF_BENCH( return new GradientBench(kSweep_GradType, gGradData[3], false); )
DEF_BENCH( return new GradientBench(kConical_GradType, gGradData[3], true); )
DEF_BENCH( return new GradientBench(kConical_GradType, gGradData[3], false); )

///////////////////////////////////////////////////////////////////////////////

class Gradient2Bench : public Benchmark {
    SkString fName;
    bool     fHasAlpha;

public:
    Gradient2Bench(bool hasAlpha)  {
        fName.printf("gradient_create_%s", hasAlpha ? "alpha" : "opaque");
        fHasAlpha = hasAlpha;
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(int loops, SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        const SkRect r = { 0, 0, SkIntToScalar(4), SkIntToScalar(4) };
        const SkPoint pts[] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) },
        };

        for (int i = 0; i < loops; i++) {
            const int gray = i % 256;
            const int alpha = fHasAlpha ? gray : 0xFF;
            SkColor colors[] = {
                SK_ColorBLACK,
                SkColorSetARGB(alpha, gray, gray, gray),
                SK_ColorWHITE };
            paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr,
                                                         SK_ARRAY_COUNT(colors),
                                                         SkTileMode::kClamp));
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new Gradient2Bench(false); )
DEF_BENCH( return new Gradient2Bench(true); )
