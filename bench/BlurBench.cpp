
/*
 * Copyright 2011 Google Inc.
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
#include "SkBlurMaskFilter.h"

#define SMALL   SkIntToScalar(2)
#define REAL    SkFloatToScalar(1.5f)
#define BIG     SkIntToScalar(10)

static const char* gStyleName[] = {
    "normal",
    "solid",
    "outer",
    "inner"
};

class BlurBench : public SkBenchmark {
    SkScalar    fRadius;
    SkBlurMaskFilter::BlurStyle fStyle;
    SkString    fName;

public:
    BlurBench(void* param, SkScalar rad, SkBlurMaskFilter::BlurStyle bs) : INHERITED(param) {
        fRadius = rad;
        fStyle = bs;
        const char* name = rad > 0 ? gStyleName[bs] : "none";
        if (SkScalarFraction(rad) != 0) {
            fName.printf("blur_%.2f_%s", SkScalarToFloat(rad), name);
        } else {
            fName.printf("blur_%d_%s", SkScalarRound(rad), name);
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
        for (int i = 0; i < SkBENCHLOOP(10); i++) {
            SkRect r = SkRect::MakeWH(rand.nextUScalar1() * 400,
                                      rand.nextUScalar1() * 400);
            r.offset(fRadius, fRadius);

            if (fRadius > 0) {
                SkMaskFilter* mf = SkBlurMaskFilter::Create(fRadius, fStyle, 0);
                paint.setMaskFilter(mf)->unref();
            }
            canvas->drawOval(r, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact00(void* p) { return new BlurBench(p, SMALL, SkBlurMaskFilter::kNormal_BlurStyle); }
static SkBenchmark* Fact01(void* p) { return new BlurBench(p, SMALL, SkBlurMaskFilter::kSolid_BlurStyle); }
static SkBenchmark* Fact02(void* p) { return new BlurBench(p, SMALL, SkBlurMaskFilter::kOuter_BlurStyle); }
static SkBenchmark* Fact03(void* p) { return new BlurBench(p, SMALL, SkBlurMaskFilter::kInner_BlurStyle); }

static SkBenchmark* Fact10(void* p) { return new BlurBench(p, BIG, SkBlurMaskFilter::kNormal_BlurStyle); }
static SkBenchmark* Fact11(void* p) { return new BlurBench(p, BIG, SkBlurMaskFilter::kSolid_BlurStyle); }
static SkBenchmark* Fact12(void* p) { return new BlurBench(p, BIG, SkBlurMaskFilter::kOuter_BlurStyle); }
static SkBenchmark* Fact13(void* p) { return new BlurBench(p, BIG, SkBlurMaskFilter::kInner_BlurStyle); }

static SkBenchmark* Fact20(void* p) { return new BlurBench(p, REAL, SkBlurMaskFilter::kNormal_BlurStyle); }
static SkBenchmark* Fact21(void* p) { return new BlurBench(p, REAL, SkBlurMaskFilter::kSolid_BlurStyle); }
static SkBenchmark* Fact22(void* p) { return new BlurBench(p, REAL, SkBlurMaskFilter::kOuter_BlurStyle); }
static SkBenchmark* Fact23(void* p) { return new BlurBench(p, REAL, SkBlurMaskFilter::kInner_BlurStyle); }

static SkBenchmark* FactNone(void* p) { return new BlurBench(p, 0, SkBlurMaskFilter::kNormal_BlurStyle); }

static BenchRegistry gReg00(Fact00);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg02(Fact02);
static BenchRegistry gReg03(Fact03);

static BenchRegistry gReg10(Fact10);
static BenchRegistry gReg11(Fact11);
static BenchRegistry gReg12(Fact12);
static BenchRegistry gReg13(Fact13);

static BenchRegistry gReg20(Fact20);
static BenchRegistry gReg21(Fact21);
static BenchRegistry gReg22(Fact22);
static BenchRegistry gReg23(Fact23);

static BenchRegistry gRegNone(FactNone);

