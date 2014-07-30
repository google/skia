/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"

/** This benchmark tests rendering rotated rectangles. It can optionally apply AA and/or change the
    paint color between each rect. */
class RotRectBench: public Benchmark {
public:
    RotRectBench(bool aa, bool changeColor)
        : fAA(aa)
        , fChangeColor(changeColor) {
        this->makeName();
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE { return fName.c_str(); }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        paint.setAntiAlias(fAA);
        SkColor color = 0xFF000000;

        int w = canvas->getBaseLayerSize().width();
        int h = canvas->getBaseLayerSize().height();

        static const SkScalar kRectW = 25.1f;
        static const SkScalar kRectH = 25.9f;

        SkMatrix rotate;
        // This value was chosen so that we frequently hit the axis-aligned case.
        rotate.setRotate(30.f, kRectW / 2, kRectH / 2);
        SkMatrix m = rotate;

        SkScalar tx = 0, ty = 0;

        for (int i = 0; i < loops; ++i) {
            canvas->save();
            canvas->translate(tx, ty);
            canvas->concat(m);
            paint.setColor(color);
            if (fChangeColor) {
                color += 0x010203;
                color |= 0xFF000000;
            }
            canvas->drawRect(SkRect::MakeWH(kRectW, kRectH), paint);
            canvas->restore();

            tx += kRectW + 2;
            if (tx > w) {
                tx = 0;
                ty += kRectH + 2;
                if (ty > h) {
                    ty = 0;
                }
            }

            m.postConcat(rotate);
        }
    }

private:
    void makeName() {
        fName = "rotated_rects";
        if (fAA) {
            fName.append("_aa");
        } else {
            fName.append("_bw");
        }
        if (fChangeColor) {
            fName.append("_change_color");
        } else {
            fName.append("_same_color");
        }
    }

    bool     fAA;
    bool     fChangeColor;
    SkString fName;

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new RotRectBench(true, true);)
DEF_BENCH(return new RotRectBench(true, false);)
DEF_BENCH(return new RotRectBench(false, true);)
DEF_BENCH(return new RotRectBench(false, false);)
