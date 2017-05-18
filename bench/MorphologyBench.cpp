/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkMorphologyImageFilter.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"

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

class MorphologyBench : public Benchmark {
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
            fName.printf("morph_%d_%s", SkScalarRoundToInt(rad), name);
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setAntiAlias(true);

        SkRandom rand;
        for (int i = 0; i < loops; i++) {
            SkRect r = SkRect::MakeWH(rand.nextUScalar1() * 400,
                                      rand.nextUScalar1() * 400);
            r.offset(fRadius, fRadius);

            if (fRadius > 0) {
                sk_sp<SkImageFilter> mf;
                switch (fStyle) {
                case kDilate_MT:
                    mf = SkDilateImageFilter::Make(SkScalarFloorToInt(fRadius),
                                                   SkScalarFloorToInt(fRadius),
                                                   nullptr);
                    break;
                case kErode_MT:
                    mf = SkErodeImageFilter::Make(SkScalarFloorToInt(fRadius),
                                                  SkScalarFloorToInt(fRadius),
                                                  nullptr);
                    break;
                }
                paint.setImageFilter(std::move(mf));
            }
            canvas->drawOval(r, paint);
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new MorphologyBench(SMALL, kErode_MT); )
DEF_BENCH( return new MorphologyBench(SMALL, kDilate_MT); )

DEF_BENCH( return new MorphologyBench(BIG, kErode_MT); )
DEF_BENCH( return new MorphologyBench(BIG, kDilate_MT); )

DEF_BENCH( return new MorphologyBench(REAL, kErode_MT); )
DEF_BENCH( return new MorphologyBench(REAL, kDilate_MT); )

DEF_BENCH( return new MorphologyBench(0, kErode_MT); )
