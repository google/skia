
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkString.h"

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

// We have several special-cases depending on the number (and spacing) of colors, so
// try to exercise those here.
static const GradData gGradData[] = {
    { 2, gColors, NULL, "" },
    { 50, gColors, NULL, "_hicolor" }, // many color gradient
    { 3, gColors, NULL, "_3color" },
    { 2, gShallowColors, NULL, "_shallow" },
};

/// Ignores scale
static SkShader* MakeLinear(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, float scale) {
    return SkGradientShader::CreateLinear(pts, data.fColors, data.fPos, data.fCount, tm);
}

static SkShader* MakeRadial(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, float scale) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateRadial(center, center.fX * scale,
                                          data.fColors,
                                          data.fPos, data.fCount, tm);
}

/// Ignores scale
static SkShader* MakeSweep(const SkPoint pts[2], const GradData& data,
                           SkShader::TileMode tm, float scale) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateSweep(center.fX, center.fY, data.fColors,
                                         data.fPos, data.fCount);
}

/// Ignores scale
static SkShader* MakeConical(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, float scale) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm);
}

/// Ignores scale
static SkShader* MakeConicalZeroRad(const SkPoint pts[2], const GradData& data,
                                    SkShader::TileMode tm, float scale) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center1, 0.0,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm);
}

/// Ignores scale
static SkShader* MakeConicalOutside(const SkPoint pts[2], const GradData& data,
                                    SkShader::TileMode tm, float scale) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center0, radius0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm);
}

/// Ignores scale
static SkShader* MakeConicalOutsideZeroRad(const SkPoint pts[2], const GradData& data,
                                           SkShader::TileMode tm, float scale) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center0, 0.0,
                                                   center1, radius1,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm);
}

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data,
                               SkShader::TileMode tm, float scale);

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

static const char* tilemodename(SkShader::TileMode tm) {
    switch (tm) {
        case SkShader::kClamp_TileMode:
            return "clamp";
        case SkShader::kRepeat_TileMode:
            return "repeat";
        case SkShader::kMirror_TileMode:
            return "mirror";
        default:
            SkDEBUGFAIL("unknown tilemode");
            return "error";
    }
}

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
    SkString fName;
    SkShader* fShader;
    bool fDither;
    enum {
        W   = 400,
        H   = 400,
    };
public:
    SkShader* makeShader(GradType gradType, GradData data, SkShader::TileMode tm, float scale) {
        const SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(W), SkIntToScalar(H) }
        };

        return gGrads[gradType].fMaker(pts, data, tm, scale);
    }

    GradientBench(GradType gradType,
                  GradData data = gGradData[0],
                  SkShader::TileMode tm = SkShader::kClamp_TileMode,
                  GeomType geomType = kRect_GeomType,
                  float scale = 1.0f) {
        fName.printf("gradient_%s_%s", gGrads[gradType].fName,
                     tilemodename(tm));
        if (geomType != kRect_GeomType) {
            fName.append("_");
            fName.append(geomtypename(geomType));
        }

        if (scale != 1.f) {
            fName.appendf("_scale_%g", scale);
        }

        fName.append(data.fName);

        fDither = false;
        fShader = this->makeShader(gradType, data, tm, scale);
        fGeomType = geomType;
    }

    GradientBench(GradType gradType, GradData data, bool dither) {
        const char *tmname = tilemodename(SkShader::kClamp_TileMode);
        fName.printf("gradient_%s_%s", gGrads[gradType].fName, tmname);
        fName.append(data.fName);

        fDither = dither;
        if (dither) {
            fName.appendf("_dither");
        }

        fShader = this->makeShader(gradType, data, SkShader::kClamp_TileMode, 1.0f);
        fGeomType = kRect_GeomType;
    }

    virtual ~GradientBench() {
        fShader->unref();
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setShader(fShader);
        if (fDither) {
            paint.setDither(true);
        }

        SkRect r = { 0, 0, SkIntToScalar(W), SkIntToScalar(H) };
        for (int i = 0; i < loops; i++) {
            switch (fGeomType) {
               case kRect_GeomType:
                   canvas->drawRect(r, paint);
                   break;
               case kOval_GeomType:
                   canvas->drawOval(r, paint);
                   break;
            }
        }
    }

private:
    typedef Benchmark INHERITED;

    GeomType fGeomType;
};

DEF_BENCH( return new GradientBench(kLinear_GradType); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[2]); )
DEF_BENCH( return new GradientBench(kLinear_GradType, gGradData[0], SkShader::kMirror_TileMode); )

DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0]); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[1]); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[2]); )
// Draw a radial gradient of radius 1/2 on a rectangle; half the lines should
// be completely pinned, the other half should pe partially pinned
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkShader::kClamp_TileMode, kRect_GeomType, 0.5f); )

// Draw a radial gradient on a circle of equal size; all the lines should
// hit the unpinned fast path (so long as GradientBench.W == H)
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkShader::kClamp_TileMode, kOval_GeomType); )

DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkShader::kMirror_TileMode); )
DEF_BENCH( return new GradientBench(kRadial_GradType, gGradData[0], SkShader::kRepeat_TileMode); )
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

    virtual void onDraw(const int loops, SkCanvas* canvas) {
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
            SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL,
                                                         SK_ARRAY_COUNT(colors),
                                                         SkShader::kClamp_TileMode);
            paint.setShader(s)->unref();
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new Gradient2Bench(false); )
DEF_BENCH( return new Gradient2Bench(true); )
