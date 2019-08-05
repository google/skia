/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkRandom.h"

#include "tools/ToolUtils.h"

class MatrixConvolutionBench : public Benchmark {
public:
    MatrixConvolutionBench(SkTileMode tileMode, bool convolveAlpha)
        : fName(SkStringPrintf("matrixconvolution_%s%s",
                               ToolUtils::tilemode_name(tileMode),
                               convolveAlpha ? "" : "_noConvolveAlpha")) {
        SkISize kernelSize = SkISize::Make(3, 3);
        SkScalar kernel[9] = {
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
            SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        };
        SkScalar gain = 0.3f, bias = SkIntToScalar(100);
        SkIPoint kernelOffset = SkIPoint::Make(1, 1);
        fFilter = SkImageFilters::MatrixConvolution(kernelSize, kernel, gain, bias,
                                                    kernelOffset, tileMode, convolveAlpha, nullptr);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setImageFilter(fFilter);
        paint.setAntiAlias(true);

        SkRandom rand;
        for (int i = 0; i < loops; i++) {
            SkRect r = SkRect::MakeWH(rand.nextUScalar1() * 400,
                                      rand.nextUScalar1() * 400);
            canvas->drawOval(r, paint);
        }
    }

private:
    sk_sp<SkImageFilter> fFilter;
    SkString fName;

    typedef Benchmark INHERITED;
};

DEF_BENCH( return new MatrixConvolutionBench(SkTileMode::kClamp, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkTileMode::kRepeat, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkTileMode::kMirror, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkTileMode::kDecal, true); )
DEF_BENCH( return new MatrixConvolutionBench(SkTileMode::kDecal, false); )
