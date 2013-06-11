/*
 * Copyright 2013 Google Inc.
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
#include "SkBlurMask.h"

#define SMALL   SkIntToScalar(2)
#define REAL    SkFloatToScalar(1.5f)
#define BIG     SkIntToScalar(10)
#define REALBIG SkFloatToScalar(30.5f)

class BlurRectBench: public SkBenchmark {
    int         fLoopCount;
    SkScalar    fRadius;
    SkString    fName;

public:
    BlurRectBench(void *param, SkScalar rad) : INHERITED(param) {
        fRadius = rad;

        if (fRadius > SkIntToScalar(25)) {
            fLoopCount = 100;
        } else if (fRadius > SkIntToScalar(5)) {
            fLoopCount = 1000;
        } else {
            fLoopCount = 10000;
        }
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    SkScalar radius() const {
        return fRadius;
    }

    void setName(const SkString& name) {
        fName = name;
    }

    virtual void onDraw(SkCanvas*) {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setAntiAlias(true);

        SkScalar pad = fRadius*3/2 + SK_Scalar1;
        SkRect r = SkRect::MakeWH(2 * pad + SK_Scalar1, 2 * pad + SK_Scalar1);

        preBenchSetup(r);

        for (int i = 0; i < SkBENCHLOOP(fLoopCount); i++) {
            makeBlurryRect(r);
        }
    }

    virtual void makeBlurryRect(const SkRect&) = 0;
    virtual void preBenchSetup(const SkRect&) {}
private:
    typedef SkBenchmark INHERITED;
};


class BlurRectDirectBench: public BlurRectBench {
 public:
    BlurRectDirectBench(void *param, SkScalar rad) : INHERITED(param, rad) {
        SkString name;

        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_direct_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_direct_%d", SkScalarRoundToInt(rad));
        }

        setName(name);
    }
protected:
    virtual void makeBlurryRect(const SkRect& r) SK_OVERRIDE {
        SkMask mask;
        SkBlurMask::BlurRect(&mask, r, this->radius(), SkBlurMask::kNormal_Style);
        SkMask::FreeImage(mask.fImage);
    }
private:
    typedef BlurRectBench INHERITED;
};

class BlurRectSeparableBench: public BlurRectBench {

public:
    BlurRectSeparableBench(void *param, SkScalar rad) : INHERITED(param, rad) {
        fSrcMask.fImage = NULL;
    }

    ~BlurRectSeparableBench() {
        SkMask::FreeImage(fSrcMask.fImage);
    }

protected:
    virtual void preBenchSetup(const SkRect& r) SK_OVERRIDE {
        SkMask::FreeImage(fSrcMask.fImage);

        r.roundOut(&fSrcMask.fBounds);
        fSrcMask.fFormat = SkMask::kA8_Format;
        fSrcMask.fRowBytes = fSrcMask.fBounds.width();
        fSrcMask.fImage = SkMask::AllocImage(fSrcMask.computeTotalImageSize());

        memset(fSrcMask.fImage, 0xff, fSrcMask.computeTotalImageSize());
    }

    SkMask fSrcMask;
private:
    typedef BlurRectBench INHERITED;
};

class BlurRectBoxFilterBench: public BlurRectSeparableBench {
public:
    BlurRectBoxFilterBench(void *param, SkScalar rad) : INHERITED(param, rad) {
        SkString name;
        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_boxfilter_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_boxfilter_%d", SkScalarRoundToInt(rad));
        }
        setName(name);
    }

protected:

    virtual void makeBlurryRect(const SkRect&) SK_OVERRIDE {
        SkMask mask;
        mask.fImage = NULL;
        SkBlurMask::Blur(&mask, fSrcMask, this->radius(),
                                  SkBlurMask::kNormal_Style,
                                  SkBlurMask::kHigh_Quality);
        SkMask::FreeImage(mask.fImage);
    }
private:
    typedef BlurRectSeparableBench INHERITED;
};

class BlurRectGaussianBench: public BlurRectSeparableBench {
public:
    BlurRectGaussianBench(void *param, SkScalar rad) : INHERITED(param, rad) {
        SkString name;
        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_gaussian_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_gaussian_%d", SkScalarRoundToInt(rad));
        }
        setName(name);
    }

protected:

    virtual void makeBlurryRect(const SkRect&) SK_OVERRIDE {
        SkMask mask;
        mask.fImage = NULL;
        SkBlurMask::BlurGroundTruth(&mask, fSrcMask, this->radius(),
                                    SkBlurMask::kNormal_Style);
        SkMask::FreeImage(mask.fImage);
    }
private:
    typedef BlurRectSeparableBench INHERITED;
};

DEF_BENCH(return new BlurRectBoxFilterBench(p, SMALL);)
DEF_BENCH(return new BlurRectBoxFilterBench(p, BIG);)
DEF_BENCH(return new BlurRectBoxFilterBench(p, REALBIG);)
DEF_BENCH(return new BlurRectBoxFilterBench(p, REAL);)
DEF_BENCH(return new BlurRectGaussianBench(p, SMALL);)
DEF_BENCH(return new BlurRectGaussianBench(p, BIG);)
DEF_BENCH(return new BlurRectGaussianBench(p, REALBIG);)
DEF_BENCH(return new BlurRectGaussianBench(p, REAL);)
DEF_BENCH(return new BlurRectDirectBench(p, SMALL);)
DEF_BENCH(return new BlurRectDirectBench(p, BIG);)
DEF_BENCH(return new BlurRectDirectBench(p, REALBIG);)
DEF_BENCH(return new BlurRectDirectBench(p, REAL);)

DEF_BENCH(return new BlurRectDirectBench(p, SkIntToScalar(5));)
DEF_BENCH(return new BlurRectDirectBench(p, SkIntToScalar(20));)

DEF_BENCH(return new BlurRectBoxFilterBench(p, SkIntToScalar(5));)
DEF_BENCH(return new BlurRectBoxFilterBench(p, SkIntToScalar(20));)

#if 0
// disable Gaussian benchmarks; the algorithm works well enough
// and serves as a baseline for ground truth, but it's too slow
// to use in production for non-trivial radii, so no real point
// in having the bots benchmark it all the time.

DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(1));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(2));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(3));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(4));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(5));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(6));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(7));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(8));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(9));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(10));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(11));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(12));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(13));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(14));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(15));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(16));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(17));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(18));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(19));)
DEF_BENCH(return new BlurRectGaussianBench(p, SkIntToScalar(20));)
#endif
