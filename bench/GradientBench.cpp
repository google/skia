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
                           SkShader::TileMode tm, SkUnitMapper* mapper) {
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

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data,
                               SkShader::TileMode tm, SkUnitMapper* mapper);

static const struct {
    GradMaker   fMaker;
    const char* fName;
    int         fRepeat;
} gGrads[] = {
    { MakeLinear,   "linear",  15 },
    { MakeRadial,   "radial1", 10 },
    { MakeSweep,    "sweep",    1 },
    { Make2Radial,  "radial2",  5 },
};

enum GradType { // these must match the order in gGrads
    kLinear_GradType,
    kRadial_GradType,
    kSweep_GradType,
    kRadial2_GradType
};

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
    GradientBench(void* param, GradType gt) : INHERITED(param) {
        fName.printf("gradient_%s", gGrads[gt].fName);

        const SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(W), SkIntToScalar(H) }
        };
        
        fCount = N * gGrads[gt].fRepeat;
        fShader = gGrads[gt].fMaker(pts, gGradData[0],
                                    SkShader::kClamp_TileMode, NULL);
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
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact0(void* p) { return new GradientBench(p, kLinear_GradType); }
static SkBenchmark* Fact1(void* p) { return new GradientBench(p, kRadial_GradType); }
static SkBenchmark* Fact2(void* p) { return new GradientBench(p, kSweep_GradType); }
static SkBenchmark* Fact3(void* p) { return new GradientBench(p, kRadial2_GradType); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);

