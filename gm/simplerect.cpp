/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkPath.h"
#include "ToolUtils.h"
#include "gm.h"

class SimpleRectGM : public skiagm::GM {
public:
    SimpleRectGM() {}

protected:
    SkString onShortName() override {
        SkString name;
        name.printf("simplerect");
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(1, 1);    // want to exercise non-identity ctm performance

        const SkScalar min = -20;
        const SkScalar max = 800;
        const SkScalar size = 20;

        SkRandom rand;
        SkPaint paint;
        for (int i = 0; i < 10000; i++) {
            paint.setColor(ToolUtils::color_to_565(rand.nextU() | (0xFF << 24)));
            SkScalar x = rand.nextRangeScalar(min, max);
            SkScalar y = rand.nextRangeScalar(min, max);
            SkScalar w = rand.nextRangeScalar(0, size);
            SkScalar h = rand.nextRangeScalar(0, size);
            canvas->drawRect(SkRect::MakeXYWH(x, y, w, h), paint);
        }
    }

    bool onAnimate(const AnimTimer& timer) override { return true; }

private:

    typedef GM INHERITED;
};
DEF_GM(return new SimpleRectGM;)
