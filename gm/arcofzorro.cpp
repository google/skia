/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkRandom.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 1000);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPoint pts[2] = {
            { 540.467f, 685.803f },
            { 540.467f, 704.803f },
        };

        SkPaint p;
        p.setStrokeWidth(3);
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeCap(SkPaint::kRound_Cap);
//        p.setImageFilter(SkImageFilters::Blur(6.85085f, 6.85085f, SkTileMode::kClamp, nullptr));
        p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6.85085f));

        for (int i = 0; i < 10; ++i) {
            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, p);
            pts[0] += {0.01f, 0.02f};
            pts[1] += {0.01f, 0.02f};
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
