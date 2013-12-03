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
#include "SkShader.h"
#include "SkString.h"
#include "SkMorphologyImageFilter.h"

#define SMALL   SkIntToScalar(2)
#define REAL    1.5f
#define BIG     SkIntToScalar(10)

enum MorphologyType {
    kErode_MT,
    kDilate_MT
};

static const char* gStyleName[] = {
    "erode",
    "dilate"
};

class MorphologyBench : public SkBenchmark {
    SkScalar       fRadius;
    MorphologyType fStyle;
    SkString       fName;

public:
    MorphologyBench(SkScalar rad, MorphologyType style)
         {
        fRadius = rad;
        fStyle = style;
        const char* name = rad > 0 ? gStyleName[style] : "none";
        if (SkScalarFraction(rad) != 0) {
            fName.printf("morph_%.2f_%s", SkScalarToFloat(rad), name);
        } else {
            fName.printf("morph_%d_%s", SkScalarRound(rad), name);
        }
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
            r.offset(fRadius, fRadius);

            if (fRadius > 0) {
                SkMorphologyImageFilter* mf = NULL;
                switch (fStyle) {
                case kDilate_MT:
                    mf = new SkDilateImageFilter(SkScalarFloorToInt(fRadius),
                                                 SkScalarFloorToInt(fRadius));
                    break;
                case kErode_MT:
                    mf = new SkErodeImageFilter(SkScalarFloorToInt(fRadius),
                                                SkScalarFloorToInt(fRadius));
                    break;
                }
                paint.setImageFilter(mf)->unref();
            }
            canvas->drawOval(r, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

// Fixed point can be 100x slower than float on these tests, causing
// bench to timeout.
#ifndef SK_SCALAR_IS_FIXED
DEF_BENCH( return new MorphologyBench(SMALL, kErode_MT); )
DEF_BENCH( return new MorphologyBench(SMALL, kDilate_MT); )

DEF_BENCH( return new MorphologyBench(BIG, kErode_MT); )
DEF_BENCH( return new MorphologyBench(BIG, kDilate_MT); )

DEF_BENCH( return new MorphologyBench(REAL, kErode_MT); )
DEF_BENCH( return new MorphologyBench(REAL, kDilate_MT); )

DEF_BENCH( return new MorphologyBench(0, kErode_MT); )
#endif
