/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

class BlurRectsBench : public Benchmark {
public:
    BlurRectsBench(SkRect outer, SkRect inner, SkScalar radius) {
        fRadius = radius;
        fOuter = outer;
        fInner = inner;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void setName(const SkString& name) {
        fName = name;
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, fRadius));

        SkPath path;
        path.addRect(fOuter, SkPath::kCW_Direction);
        path.addRect(fInner, SkPath::kCW_Direction);

        for (int i = 0; i < loops; i++) {
            canvas->drawPath(path, paint);
        }
    }

private:
    SkString    fName;
    SkRect      fOuter;
    SkRect      fInner;
    SkScalar    fRadius;

    typedef     Benchmark INHERITED;
};

class BlurRectsNinePatchBench: public BlurRectsBench {
public:
    BlurRectsNinePatchBench(SkRect outer, SkRect inner, SkScalar radius)
        : INHERITED(outer, inner, radius) {
        this->setName(SkString("blurrectsninepatch"));
    }
private:
    typedef BlurRectsBench INHERITED;
};

class BlurRectsNonNinePatchBench: public BlurRectsBench {
public:
    BlurRectsNonNinePatchBench(SkRect outer, SkRect inner, SkScalar radius)
        : INHERITED(outer, inner, radius) {
        SkString name;
        this->setName(SkString("blurrectsnonninepatch"));
    }
private:
    typedef BlurRectsBench INHERITED;
};

DEF_BENCH(return new BlurRectsNinePatchBench(SkRect::MakeXYWH(10, 10, 100, 100),
                                             SkRect::MakeXYWH(20, 20, 60, 60),
                                             2.3f);)
DEF_BENCH(return new BlurRectsNonNinePatchBench(SkRect::MakeXYWH(10, 10, 100, 100),
                                                SkRect::MakeXYWH(50, 50, 10, 10),
                                                4.3f);)
