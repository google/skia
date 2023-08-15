/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"

class SimpleRectGM : public skiagm::GM {
public:
    SimpleRectGM() {}

protected:
    SkString getName() const override {
        SkString name;
        name.printf("simplerect");
        return name;
    }

    SkISize getISize() override { return SkISize::Make(800, 800); }

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

    bool onAnimate(double nanos) override { return true; }

private:

    using INHERITED = GM;
};
DEF_GM(return new SimpleRectGM;)
