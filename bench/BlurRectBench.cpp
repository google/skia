
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
    SkScalar    fRadius;
    SkString    fName;

public:
    BlurRectBench(void *param, SkScalar rad) : INHERITED(param) {
        fRadius = rad;
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    SkScalar radius() const {
        return fRadius;
    }

    void setName( SkString name ) {
        fName = name;
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setAntiAlias(true);

        int pad = fRadius * 1.5 + 1;
        SkRect r = SkRect::MakeWH(2 * pad + 1, 2 * pad + 1);

        int loop_count;

        if (fRadius > SkIntToScalar(50)) {
          loop_count = 10;
        } else if (fRadius > SkIntToScalar(5)) {
          loop_count = 1000;
        } else {
          loop_count = 10000;
        }

        preBenchSetup( r );

        for (int i = 0; i < SkBENCHLOOP(loop_count); i++) {
            makeBlurryRect( r );
        }
    }

    virtual void makeBlurryRect( SkRect &r ) = 0;
    virtual void preBenchSetup( SkRect &r ) {}
private:
    typedef SkBenchmark INHERITED;
};


class BlurRectDirectBench: public BlurRectBench {
 public:
    BlurRectDirectBench( void *param, SkScalar rad ) : BlurRectBench( param, rad ) {
        SkString name;

        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_direct_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_direct_%d", SkScalarRound(rad));
        }

        setName( name );
    }
protected:
    virtual void makeBlurryRect( SkRect &r ) {
        SkMask mask;
        SkBlurMask::BlurRect( &mask, r, radius(), SkBlurMask::kNormal_Style, SkBlurMask::kHigh_Quality );
    }
};

class BlurRectSeparableBench: public BlurRectBench {
    SkMask fSrcMask;
public:
    BlurRectSeparableBench(void *param, SkScalar rad) : BlurRectBench( param, rad ) {
        SkString name;
        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_separable_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_separable_%d", SkScalarRound(rad));
        }

        setName( name );
    }

protected:
    virtual void preBenchSetup( SkRect &r ) {
        fSrcMask.fFormat = SkMask::kA8_Format;
        fSrcMask.fRowBytes = r.width();
        fSrcMask.fBounds = SkIRect::MakeWH(r.width(), r.height());
        fSrcMask.fImage = SkMask::AllocImage( fSrcMask.computeTotalImageSize() );

        memset( fSrcMask.fImage, 0xff, fSrcMask.computeTotalImageSize() );
    }

    virtual void makeBlurryRect( SkRect &r ) {
        SkMask mask;
        SkBlurMask::BlurSeparable( &mask, fSrcMask, radius(), SkBlurMask::kNormal_Style, SkBlurMask::kHigh_Quality );
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
