/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkAAClip.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkString.h"

class AAClipBuilderBench : public SkBenchmark {
    SkString fName;
    SkPath   fPath;
    SkRect   fRect;
    SkRegion fRegion;
    bool     fDoPath;
    bool     fDoAA;

    enum {
        N = SkBENCHLOOP(200),
    };

public:
    AAClipBuilderBench(void* param, bool doPath, bool doAA) : INHERITED(param) {
        fDoPath = doPath;
        fDoAA = doAA;

        fName.printf("aaclip_build_%s_%s", doPath ? "path" : "rect",
                     doAA ? "AA" : "BW");

        fRegion.setRect(0, 0, 640, 480);
        fRect.set(fRegion.getBounds());
        fRect.inset(SK_Scalar1/4, SK_Scalar1/4);
        fPath.addRoundRect(fRect, SkIntToScalar(20), SkIntToScalar(20));
    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        for (int i = 0; i < N; ++i) {
            SkAAClip clip;
            if (fDoPath) {
                clip.setPath(fPath, &fRegion, fDoAA);
            } else {
                clip.setRect(fRect, fDoAA);
            }
        }
    }
private:
    typedef SkBenchmark INHERITED;
};

class AAClipRegionBench : public SkBenchmark {
public:
    AAClipRegionBench(void* param) : INHERITED(param) {
        SkPath path;
        // test conversion of a complex clip to a aaclip
        path.addCircle(0, 0, SkIntToScalar(200));
        path.addCircle(0, 0, SkIntToScalar(180));
        // evenodd means we've constructed basically a stroked circle
        path.setFillType(SkPath::kEvenOdd_FillType);

        SkIRect bounds;
        path.getBounds().roundOut(&bounds);
        fRegion.setPath(path, SkRegion(bounds));
    }

protected:
    virtual const char* onGetName() { return "aaclip_setregion"; }
    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; ++i) {
            SkAAClip clip;
            clip.setRegion(fRegion);
        }
    }

private:
    enum {
        N = SkBENCHLOOP(400),
    };
    SkRegion fRegion;
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* Fact0(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, false, false)); }
static SkBenchmark* Fact1(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, false, true)); }
static SkBenchmark* Fact2(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, true, false)); }
static SkBenchmark* Fact3(void* p) { return SkNEW_ARGS(AAClipBuilderBench, (p, true, true)); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);

static SkBenchmark* Fact01(void* p) { return SkNEW_ARGS(AAClipRegionBench, (p)); }
static BenchRegistry gReg01(Fact01);
