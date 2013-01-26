
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
#define REALBIG SkFloatToScalar(100.5f)

static const char* gStyleName[] = {
    "normal",
    "solid",
    "outer",
    "inner"
};

class BlurBench : public SkBenchmark {
    SkScalar    fRadius;
    SkBlurMaskFilter::BlurStyle fStyle;
    uint32_t                    fFlags;
    SkString    fName;

public:
    BlurBench(void* param, SkScalar rad, SkBlurMaskFilter::BlurStyle bs, uint32_t flags = 0) : INHERITED(param) {
        fRadius = rad;
        fStyle = bs;
        fFlags = flags;
        const char* name = rad > 0 ? gStyleName[bs] : "none";
        const char* quality = flags & SkBlurMaskFilter::kHighQuality_BlurFlag ? "high_quality" : "low_quality";
        if (SkScalarFraction(rad) != 0) {
            fName.printf("blur_%.2f_%s_%s", SkScalarToFloat(rad), name, quality);
        } else {
            fName.printf("blur_%d_%s_%s", SkScalarRound(rad), name, quality);
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
                SkMaskFilter* mf = SkBlurMaskFilter::Create(fRadius, fStyle, fFlags);
                paint.setMaskFilter(mf)->unref();
            }
            canvas->drawOval(r, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH(return new BlurBench(p, SMALL, SkBlurMaskFilter::kNormal_BlurStyle);)
DEF_BENCH(return new BlurBench(p, SMALL, SkBlurMaskFilter::kSolid_BlurStyle);)
DEF_BENCH(return new BlurBench(p, SMALL, SkBlurMaskFilter::kOuter_BlurStyle);)
DEF_BENCH(return new BlurBench(p, SMALL, SkBlurMaskFilter::kInner_BlurStyle);)

DEF_BENCH(return new BlurBench(p, BIG, SkBlurMaskFilter::kNormal_BlurStyle);)
DEF_BENCH(return new BlurBench(p, BIG, SkBlurMaskFilter::kSolid_BlurStyle);)
DEF_BENCH(return new BlurBench(p, BIG, SkBlurMaskFilter::kOuter_BlurStyle);)
DEF_BENCH(return new BlurBench(p, BIG, SkBlurMaskFilter::kInner_BlurStyle);)

DEF_BENCH(return new BlurBench(p, REALBIG, SkBlurMaskFilter::kNormal_BlurStyle);)
DEF_BENCH(return new BlurBench(p, REALBIG, SkBlurMaskFilter::kSolid_BlurStyle);)
DEF_BENCH(return new BlurBench(p, REALBIG, SkBlurMaskFilter::kOuter_BlurStyle);)
DEF_BENCH(return new BlurBench(p, REALBIG, SkBlurMaskFilter::kInner_BlurStyle);)

DEF_BENCH(return new BlurBench(p, REAL, SkBlurMaskFilter::kNormal_BlurStyle);)
DEF_BENCH(return new BlurBench(p, REAL, SkBlurMaskFilter::kSolid_BlurStyle);)
DEF_BENCH(return new BlurBench(p, REAL, SkBlurMaskFilter::kOuter_BlurStyle);)
DEF_BENCH(return new BlurBench(p, REAL, SkBlurMaskFilter::kInner_BlurStyle);)

DEF_BENCH(return new BlurBench(p, SMALL, SkBlurMaskFilter::kNormal_BlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(p, BIG, SkBlurMaskFilter::kNormal_BlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(p, REALBIG, SkBlurMaskFilter::kNormal_BlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(p, REAL, SkBlurMaskFilter::kNormal_BlurStyle, SkBlurMaskFilter::kHighQuality_BlurFlag);)

DEF_BENCH(return new BlurBench(p, 0, SkBlurMaskFilter::kNormal_BlurStyle);)
