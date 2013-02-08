/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkBicubicImageFilter.h"

// This bench exercises SkBicubicImageFilter, upsampling a 40x40 input to
// 100x100, 400x100, 100x400, and 400x400.

class BicubicBench : public SkBenchmark {
    SkSize         fScale;
    SkString       fName;

public:
    BicubicBench(void* param, float x, float y)
        :  INHERITED(param), fScale(SkSize::Make(SkFloatToScalar(x), SkFloatToScalar(y))) {
        fName.printf("bicubic_%gx%g",
            SkScalarToFloat(fScale.fWidth), SkScalarToFloat(fScale.fHeight));
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
        SkRect r = SkRect::MakeWH(40, 40);
        SkAutoTUnref<SkImageFilter> bicubic(SkBicubicImageFilter::CreateMitchell(fScale));
        paint.setImageFilter(bicubic);
        canvas->save();
        canvas->clipRect(r);
        canvas->drawOval(r, paint);
        canvas->restore();
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact00(void* p) { return new BicubicBench(p, 10.0f, 10.0f); }
static SkBenchmark* Fact01(void* p) { return new BicubicBench(p, 2.5f, 10.0f); }
static SkBenchmark* Fact02(void* p) { return new BicubicBench(p, 10.0f, 2.5f); }
static SkBenchmark* Fact03(void* p) { return new BicubicBench(p, 2.5f, 2.5f); }

static BenchRegistry gReg00(Fact00);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg02(Fact02);
static BenchRegistry gReg03(Fact03);
