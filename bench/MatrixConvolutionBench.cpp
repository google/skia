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
#include "SkString.h"
#include "SkMatrixConvolutionImageFilter.h"

class MatrixConvolutionBench : public SkBenchmark {
    SkMatrixConvolutionImageFilter::TileMode  fTileMode;

public:
    MatrixConvolutionBench(void* param, SkMatrixConvolutionImageFilter::TileMode tileMode)
        : INHERITED(param), fName("matrixconvolution") {
        SkISize kernelSize = SkISize::Make(3, 3);
        SkScalar kernel[9] = {
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        };
        SkScalar gain = SkFloatToScalar(0.3f), bias = SkIntToScalar(100);
        SkIPoint target = SkIPoint::Make(1, 1);
        fFilter = new SkMatrixConvolutionImageFilter(kernelSize, kernel, gain, bias, target, tileMode);
    }

    ~MatrixConvolutionBench() {
        fFilter->unref();
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
            paint.setImageFilter(fFilter);
            canvas->drawOval(r, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
    SkMatrixConvolutionImageFilter* fFilter;
    SkString fName;
};

static SkBenchmark* Fact00(void* p) { return new MatrixConvolutionBench(p, SkMatrixConvolutionImageFilter::kClamp_TileMode); }
static SkBenchmark* Fact01(void* p) { return new MatrixConvolutionBench(p, SkMatrixConvolutionImageFilter::kRepeat_TileMode); }
static SkBenchmark* Fact02(void* p) { return new MatrixConvolutionBench(p, SkMatrixConvolutionImageFilter::kClampToBlack_TileMode); }

static BenchRegistry gReg00(Fact00);
static BenchRegistry gReg01(Fact01);
static BenchRegistry gReg02(Fact02);
