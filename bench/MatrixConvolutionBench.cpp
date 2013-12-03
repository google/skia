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
        SkIPoint target = SkIPoint::Make(1, 1);
        fFilter = new SkMatrixConvolutionImageFilter(kernelSize, kernel, gain, bias, target, tileMode, convolveAlpha);
    }

    ~MatrixConvolutionBench() {
        fFilter->unref();
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) {
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
    typedef SkBenchmark INHERITED;
    SkMatrixConvolutionImageFilter* fFilter;
    SkString fName;
};

DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kClamp_TileMode, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kRepeat_TileMode, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kClampToBlack_TileMode, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkMatrixConvolutionImageFilter::kClampToBlack_TileMode, false); )
