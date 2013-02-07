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

    virtual void onDraw(SkCanvas* canvas) {
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
    BlurRectDirectBench(void *param, SkScalar rad) : BlurRectBench(param, rad) {
        SkString name;

        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_direct_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_direct_%d", SkScalarRound(rad));
        }

        setName(name);
    }
protected:
    virtual void makeBlurryRect(const SkRect& r) SK_OVERRIDE {
        SkMask mask;
        SkBlurMask::BlurRect(&mask, r, radius(), SkBlurMask::kNormal_Style,
                             SkBlurMask::kHigh_Quality);
        SkMask::FreeImage(mask.fImage);
    }
};

class BlurRectSeparableBench: public BlurRectBench {
    SkMask fSrcMask;
public:
    BlurRectSeparableBench(void *param, SkScalar rad) : BlurRectBench(param, rad) {
        SkString name;
        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_separable_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_separable_%d", SkScalarRound(rad));
        }
        setName(name);
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

    virtual void makeBlurryRect(const SkRect& r) SK_OVERRIDE {
        SkMask mask;
        SkBlurMask::BlurSeparable(&mask, fSrcMask, radius(),
                                  SkBlurMask::kNormal_Style,
                                  SkBlurMask::kHigh_Quality);
        SkMask::FreeImage(mask.fImage);
    }
};

DEF_BENCH(return new BlurRectSeparableBench(p, SMALL);)
DEF_BENCH(return new BlurRectSeparableBench(p, BIG);)
DEF_BENCH(return new BlurRectSeparableBench(p, REALBIG);)
DEF_BENCH(return new BlurRectSeparableBench(p, REAL);)
DEF_BENCH(return new BlurRectDirectBench(p, SMALL);)
DEF_BENCH(return new BlurRectDirectBench(p, BIG);)
DEF_BENCH(return new BlurRectDirectBench(p, REALBIG);)
DEF_BENCH(return new BlurRectDirectBench(p, REAL);)
