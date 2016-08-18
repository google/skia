/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkPath.h"

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
        return SkISize::Make(800, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkScalar min = -20;
        const SkScalar max = 800;
        const SkScalar size = 20;

        SkRandom rand;
        SkPaint paint;
        for (int i = 0; i < 10000; i++) {
            paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawRect(SkRect::MakeXYWH(rand.nextRangeScalar(min, max),
                                              rand.nextRangeScalar(min, max),
                                              rand.nextRangeScalar(0, size),
                                              rand.nextRangeScalar(0, size)),
                             paint);
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        return true;
    }

private:

    typedef GM INHERITED;
};
DEF_GM(return new SimpleRectGM;)
