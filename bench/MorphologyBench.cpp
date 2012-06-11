/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkMorphologyImageFilter.h"

#define SMALL   SkIntToScalar(2)
#define REAL    SkFloatToScalar(1.5f)
#define BIG     SkIntToScalar(10)

namespace {

enum MorphologyType {
    kErode_MT,
    kDilate_MT
};

}

static const char* gStyleName[] = {
    "erode",
    "dilate"
};

class MorphologyBench : public SkBenchmark {
    SkScalar       fRadius;
    MorphologyType fStyle;
    SkString       fName;

public:
    MorphologyBench(void* param, SkScalar rad, MorphologyType style)
        :  INHERITED(param) {
        fRadius = rad;
        fStyle = style;
        const char* name = rad > 0 ? gStyleName[style] : "none";
        if (SkScalarFraction(rad) != 0) {
            fName.printf("morph_%.2f_%s", SkScalarToFloat(rad), name);
        } else {
            fName.printf("morph_%d_%s", SkScalarRound(rad), name);
        }
    }
    
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setAntiAlias(true);

        SkRandom rand;
        for (int i = 0; i < SkBENCHLOOP(3); i++) {
            SkRect r = SkRect::MakeWH(rand.nextUScalar1() * 400,
                                      rand.nextUScalar1() * 400);
            r.offset(fRadius, fRadius);

            if (fRadius > 0) {
                SkMorphologyImageFilter* mf = NULL;
                switch (fStyle) {
                case kDilate_MT:
                    mf = new SkDilateImageFilter(fRadius, fRadius);
                    break;
                case kErode_MT:
                    mf = new SkErodeImageFilter(fRadius, fRadius);
                    break;
                }
                paint.setImageFilter(mf)->unref();
            }
            canvas->drawOval(r, paint);
        }
    }
    
private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact00(void* p) { return new MorphologyBench(p, SMALL, kErode_MT); }
static SkBenchmark* Fact01(void* p) { return new MorphologyBench(p, SMALL, kDilate_MT); }

static SkBenchmark* Fact10(void* p) { return new MorphologyBench(p, BIG, kErode_MT); }
static SkBenchmark* Fact11(void* p) { return new MorphologyBench(p, BIG, kDilate_MT); }

static SkBenchmark* Fact20(void* p) { return new MorphologyBench(p, REAL, kErode_MT); }
static SkBenchmark* Fact21(void* p) { return new MorphologyBench(p, REAL, kDilate_MT); }

static SkBenchmark* FactNone(void* p) { return new MorphologyBench(p, 0, kErode_MT); }

// Fixed point can be 100x slower than float on these tests, causing
// bench to timeout.
#ifndef SK_SCALAR_IS_FIXED

static BenchRegistry gReg00(Fact00);
static BenchRegistry gReg01(Fact01);

static BenchRegistry gReg10(Fact10);
static BenchRegistry gReg11(Fact11);

static BenchRegistry gReg20(Fact20);
static BenchRegistry gReg21(Fact21);

static BenchRegistry gRegNone(FactNone);

#endif

