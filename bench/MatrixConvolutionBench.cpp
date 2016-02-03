/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"

class MatrixConvolutionBench : public Benchmark {
public:
    MatrixConvolutionBench(SkMatrixConvolutionImageFilter::TileMode tileMode, bool convolveAlpha)
        : fName("matrixconvolution") {
        SkISize kernelSize = SkISize::Make(3, 3);
        SkScalar kernel[9] = {
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        };
        SkScalar gain = 0.3f, bias = SkIntToScalar(100);
        SkIPoint kernelOffset = SkIPoint::Make(1, 1);
        fFilter = SkMatrixConvolutionImageFilter::Create(kernelSize, kernel, gain, bias, kernelOffset, tileMode, convolveAlpha);
    }

    ~MatrixConvolutionBench() {
        fFilter->unref();
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(int loops, SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setAntiAlias(true);
        SkRandom rand;
        for (int i = 0; i < loops; i++) {
            SkRect r = SkRect::MakeWH(rand.nextUScalar1() * 400,
                                      rand.nextUScalar1() * 400);
            paint.setImageFilter(fFilter);
            canvas->drawOval(r, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
    SkImageFilter* fFilter;
    SkString fName;
};

DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kClamp_TileMode, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kRepeat_TileMode, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kClampToBlack_TileMode, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kClampToBlack_TileMode, false); )
