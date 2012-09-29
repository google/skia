
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkUnitMapper.h"

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
    { 2, gColors, NULL },
    { 2, gColors, gPos0 },
    { 2, gColors, gPos1 },
    { 5, gColors, NULL },
    { 5, gColors, gPos2 }
};

/// Ignores scale
static SkShader* MakeLinear(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper,
                            float scale) {
    return SkGradientShader::CreateLinear(pts, data.fColors, data.fPos,
                                          data.fCount, tm, mapper);
}

static SkShader* MakeRadial(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper,
                            float scale) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateRadial(center, center.fX * scale,
                                          data.fColors,
                                          data.fPos, data.fCount, tm, mapper);
}

/// Ignores scale
static SkShader* MakeSweep(const SkPoint pts[2], const GradData& data,
                           SkShader::TileMode tm, SkUnitMapper* mapper,
                           float scale) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateSweep(center.fX, center.fY, data.fColors,
                                         data.fPos, data.fCount, mapper);
}

/// Ignores scale
static SkShader* Make2Radial(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, SkUnitMapper* mapper,
                             float scale) {
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

/// Ignores scale
static SkShader* MakeConical(const SkPoint pts[2], const GradData& data,
                             SkShader::TileMode tm, SkUnitMapper* mapper,
                             float scale) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                   center0, (pts[1].fX - pts[0].fX) / 2,
                                                   data.fColors, data.fPos, data.fCount, tm, mapper);
}

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data,
                               SkShader::TileMode tm, SkUnitMapper* mapper,
                               float scale);

static const struct {
    GradMaker   fMaker;
    const char* fName;
    int         fRepeat;
} gGrads[] = {
    { MakeLinear,   "linear",  15 },
    { MakeRadial,   "radial1", 10 },
    { MakeSweep,    "sweep",    1 },
    { Make2Radial,  "radial2",  5 },
    { MakeConical,  "conical",  5 },
};

enum GradType { // these must match the order in gGrads
    kLinear_GradType,
    kRadial_GradType,
    kSweep_GradType,
    kRadial2_GradType,
    kConical_GradType
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
            SkASSERT(!"unknown tilemode");
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
            SkASSERT(!"unknown geometry type");
            return "error";
    }
}

///////////////////////////////////////////////////////////////////////////////

class GradientBench : public SkBenchmark {
    SkString fName;
    SkShader* fShader;
    int      fCount;
    enum {
        W   = 400,
        H   = 400,
        N   = 1
    };
public:
    GradientBench(void* param, GradType gradType,
                  SkShader::TileMode tm = SkShader::kClamp_TileMode,
                  GeomType geomType = kRect_GeomType,
                  float scale = 1.0f)
        : INHERITED(param) {
        fName.printf("gradient_%s_%s", gGrads[gradType].fName,
                     tilemodename(tm));
        if (geomType != kRect_GeomType) {
            fName.append("_");
            fName.append(geomtypename(geomType));
        }

        const SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(W), SkIntToScalar(H) }
        };

        fCount = SkBENCHLOOP(N * gGrads[gradType].fRepeat);
        fShader = gGrads[gradType].fMaker(pts, gGradData[0], tm, NULL, scale);
        fGeomType = geomType;
    }

    virtual ~GradientBench() {
        fShader->unref();
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setShader(fShader);

        SkRect r = { 0, 0, SkIntToScalar(W), SkIntToScalar(H) };
        for (int i = 0; i < fCount; i++) {
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
    typedef SkBenchmark INHERITED;

    GeomType fGeomType;
};

class Gradient2Bench : public SkBenchmark {
public:
    Gradient2Bench(void* param) : INHERITED(param) {}

protected:
    virtual const char* onGetName() {
        return "gradient_create";
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        const SkRect r = { 0, 0, SkIntToScalar(4), SkIntToScalar(4) };
        const SkPoint pts[] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) },
        };

        for (int i = 0; i < SkBENCHLOOP(1000); i++) {
            const int a = i % 256;
            SkColor colors[] = {
                SK_ColorBLACK,
                SkColorSetARGB(a, a, a, a),
                SK_ColorWHITE };
            SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL,
                                                         SK_ARRAY_COUNT(colors),
                                                         SkShader::kClamp_TileMode);
            paint.setShader(s)->unref();
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact0(void* p) { return new GradientBench(p, kLinear_GradType); }
static SkBenchmark* Fact01(void* p) { return new GradientBench(p, kLinear_GradType, SkShader::kMirror_TileMode); }

// Draw a radial gradient of radius 1/2 on a rectangle; half the lines should
// be completely pinned, the other half should pe partially pinned
static SkBenchmark* Fact1(void* p) { return new GradientBench(p, kRadial_GradType, SkShader::kClamp_TileMode, kRect_GeomType, 0.5f); }

// Draw a radial gradient on a circle of equal size; all the lines should
// hit the unpinned fast path (so long as GradientBench.W == H)
static SkBenchmark* Fact1o(void* p) { return new GradientBench(p, kRadial_GradType, SkShader::kClamp_TileMode, kOval_GeomType); }

static SkBenchmark* Fact11(void* p) { return new GradientBench(p, kRadial_GradType, SkShader::kMirror_TileMode); }
static SkBenchmark* Fact2(void* p) { return new GradientBench(p, kSweep_GradType); }
static SkBenchmark* Fact3(void* p) { return new GradientBench(p, kRadial2_GradType); }
static SkBenchmark* Fact31(void* p) { return new GradientBench(p, kRadial2_GradType, SkShader::kMirror_TileMode); }
static SkBenchmark* Fact5(void* p) { return new GradientBench(p, kConical_GradType); }

static SkBenchmark* Fact4(void* p) { return new Gradient2Bench(p); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg1o(Fact1o);
static BenchRegistry gReg11(Fact11);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);
static BenchRegistry gReg31(Fact31);
static BenchRegistry gReg5(Fact5);

static BenchRegistry gReg4(Fact4);

