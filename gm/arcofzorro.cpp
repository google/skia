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
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "include/utils/SkRandom.h"

namespace skiagm {

#define CROSS_INNER_RADIUS 3
#define CROSS_SIZE 18

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM(bool old) : fOld(old) {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        if (fOld) {
            return SkString("arcofzorro_old");
        } else {
            return SkString("arcofzorro_new");
        }
    }

    SkISize onISize() override {
        return SkISize::Make(200, 200);
    }

    /*
        Budget: 268435456 bytes
                        Entry Count: current 818 (818 budgeted, 0 wrapped, 800 locked, 818 scratch), high 818
                        Entry Bytes: current 3993856 (budgeted 3993856, 1.5% full, 0 unbudgeted) high 3993856
        Textures Created: 801
     */
    void drawCross1(float x, float y, SkPaint crossPaint, SkCanvas* canvas) {
        canvas->drawLine(x - CROSS_SIZE / 2, y, x - CROSS_INNER_RADIUS, y, crossPaint);
        canvas->drawLine(x + CROSS_SIZE / 2, y, x + CROSS_INNER_RADIUS, y, crossPaint);
        canvas->drawLine(x, y - CROSS_SIZE / 2, x, y - CROSS_INNER_RADIUS, crossPaint);
        canvas->drawLine(x, y + CROSS_SIZE / 2, x, y + CROSS_INNER_RADIUS, crossPaint);
    }

    /*
        Budget: 268435456 bytes
                        Entry Count: current 34 (34 budgeted, 0 wrapped, 17 locked, 33 scratch), high 34
                        Entry Bytes: current 798976 (budgeted 798976, 0.3% full, 0 unbudgeted) high 798976
        Textures Created: 17
     */
    void drawCross2(float x, float y, SkPaint crossPaint, SkCanvas* canvas) {
        float quantX = 0.25f * SkScalarRoundToScalar(4 * x);
        float quantY = 0.25f * SkScalarRoundToScalar(4 * y);

        canvas->save();
           canvas->translate(quantX, quantY);
           canvas->drawPath(fPath, crossPaint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {

        fPath.moveTo(-CROSS_SIZE / 2, 0);
        fPath.lineTo(-CROSS_INNER_RADIUS, 0);
        fPath.moveTo(CROSS_SIZE / 2, 0);
        fPath.lineTo(CROSS_INNER_RADIUS, 0);
        fPath.moveTo(0, -CROSS_SIZE / 2);
        fPath.lineTo(0, -CROSS_INNER_RADIUS);
        fPath.moveTo(0, CROSS_SIZE / 2);
        fPath.lineTo(0, CROSS_INNER_RADIUS);
        fPath.setIsVolatile(false);

        SkPaint p;
        p.setColor(SK_ColorYELLOW);
        p.setStrokeWidth(3);
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeCap(SkPaint::kRound_Cap);

        SkPaint shadowP;
        shadowP.setColor(SK_ColorBLACK);
        shadowP.setStrokeWidth(3);
        shadowP.setAntiAlias(true);
        shadowP.setStyle(SkPaint::kStroke_Style);
        shadowP.setStrokeCap(SkPaint::kRound_Cap);
        shadowP.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6.85085f));

        SkRandom random;
        for (int i = 0; i < 200; ++i) {
            float x = 100.0f + random.nextRangeF(-50.0f, 50.0f);
            float y = 100.0f + random.nextRangeF(-50.0f, 50.0f);

            if (fOld) {
                this->drawCross1(x + 4, y + 1, shadowP, canvas);
                this->drawCross1(x, y, p, canvas);
            } else {
                this->drawCross2(x + 4, y + 1, shadowP, canvas);
                this->drawCross1(x, y, p, canvas);
            }
        }
    }

private:
    SkPath fPath;
    bool   fOld;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM(true);)
DEF_GM(return new ArcOfZorroGM(false);)
}  // namespace skiagm
