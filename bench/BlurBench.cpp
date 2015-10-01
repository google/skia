
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"

#define MINI   0.01f
#define SMALL   SkIntToScalar(2)
#define REAL    1.5f
#define BIG     SkIntToScalar(10)
#define REALBIG 100.5f

static const char* gStyleName[] = {
    "normal",
    "solid",
    "outer",
    "inner"
};

class BlurBench : public Benchmark {
    SkScalar    fRadius;
    SkBlurStyle fStyle;
    uint32_t    fFlags;
    SkString    fName;

public:
    BlurBench(SkScalar rad, SkBlurStyle bs, uint32_t flags = 0) {
        fRadius = rad;
        fStyle = bs;
        fFlags = flags;
        const char* name = rad > 0 ? gStyleName[bs] : "none";
        const char* quality = flags & SkBlurMaskFilter::kHighQuality_BlurFlag ? "high_quality"
                                                                              : "low_quality";
        if (SkScalarFraction(rad) != 0) {
            fName.printf("blur_%.2f_%s_%s", SkScalarToFloat(rad), name, quality);
        } else {
            fName.printf("blur_%d_%s_%s", SkScalarRoundToInt(rad), name, quality);
        }
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
            r.offset(fRadius, fRadius);

            if (fRadius > 0) {
                SkMaskFilter* mf = SkBlurMaskFilter::Create(fStyle,
                                            SkBlurMask::ConvertRadiusToSigma(fRadius),
                                            fFlags);
                paint.setMaskFilter(mf)->unref();
            }
            canvas->drawOval(r, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new BlurBench(MINI, kNormal_SkBlurStyle);)
DEF_BENCH(return new BlurBench(MINI, kSolid_SkBlurStyle);)
DEF_BENCH(return new BlurBench(MINI, kOuter_SkBlurStyle);)
DEF_BENCH(return new BlurBench(MINI, kInner_SkBlurStyle);)

DEF_BENCH(return new BlurBench(SMALL, kNormal_SkBlurStyle);)
DEF_BENCH(return new BlurBench(SMALL, kSolid_SkBlurStyle);)
DEF_BENCH(return new BlurBench(SMALL, kOuter_SkBlurStyle);)
DEF_BENCH(return new BlurBench(SMALL, kInner_SkBlurStyle);)

DEF_BENCH(return new BlurBench(BIG, kNormal_SkBlurStyle);)
DEF_BENCH(return new BlurBench(BIG, kSolid_SkBlurStyle);)
DEF_BENCH(return new BlurBench(BIG, kOuter_SkBlurStyle);)
DEF_BENCH(return new BlurBench(BIG, kInner_SkBlurStyle);)

DEF_BENCH(return new BlurBench(REALBIG, kNormal_SkBlurStyle);)
DEF_BENCH(return new BlurBench(REALBIG, kSolid_SkBlurStyle);)
DEF_BENCH(return new BlurBench(REALBIG, kOuter_SkBlurStyle);)
DEF_BENCH(return new BlurBench(REALBIG, kInner_SkBlurStyle);)

DEF_BENCH(return new BlurBench(REAL, kNormal_SkBlurStyle);)
DEF_BENCH(return new BlurBench(REAL, kSolid_SkBlurStyle);)
DEF_BENCH(return new BlurBench(REAL, kOuter_SkBlurStyle);)
DEF_BENCH(return new BlurBench(REAL, kInner_SkBlurStyle);)

DEF_BENCH(return new BlurBench(MINI, kNormal_SkBlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(SMALL, kNormal_SkBlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(BIG, kNormal_SkBlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(REALBIG, kNormal_SkBlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(REAL, kNormal_SkBlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(0, kNormal_SkBlurStyle);)
