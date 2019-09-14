/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"
#include "tools/timer/TimeUtils.h"

#define CORNER_RADIUS   12

static const int gXY[] = {
    4, 0, 0, -4, 8, -4, 12, 0, 8, 4, 0, 4
};

static sk_sp<SkPathEffect> make_pe(int flags, SkScalar phase) {
    if (flags == 1) {
        return SkCornerPathEffect::Make(SkIntToScalar(CORNER_RADIUS));
    }

    SkPath  path;
    path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
    for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
        path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
    path.close();
    path.offset(SkIntToScalar(-6), 0);

    auto outer = SkPath1DPathEffect::Make(path, 12, phase, SkPath1DPathEffect::kRotate_Style);

    if (flags == 2)
        return outer;

    auto inner = SkCornerPathEffect::Make(SkIntToScalar(CORNER_RADIUS));

    return SkPathEffect::MakeCompose(outer, inner);
}

static sk_sp<SkPathEffect> make_warp_pe(SkScalar phase) {
    SkPath  path;
    path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
    for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2) {
        path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
    }
    path.close();
    path.offset(SkIntToScalar(-6), 0);

    auto outer = SkPath1DPathEffect::Make(
        path, 12, phase, SkPath1DPathEffect::kMorph_Style);
    auto inner = SkCornerPathEffect::Make(SkIntToScalar(CORNER_RADIUS));

    return SkPathEffect::MakeCompose(outer, inner);
}

///////////////////////////////////////////////////////////

#include "include/core/SkColorFilter.h"

class PathEffectView : public Sample {
    SkPath  fPath;
    SkPoint fClickPt;
    SkScalar fPhase;

public:
    PathEffectView() : fPhase(0) {
        }

protected:
    void onOnceBeforeDraw() override {
        SkRandom    rand;
        int         steps = 20;
        SkScalar    dist = SkIntToScalar(400);
        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = SkIntToScalar(50);

        fPath.moveTo(x, y);
        for (int i = 0; i < steps; i++) {
            x += dist/steps;
            SkScalar tmpY = y + SkIntToScalar(rand.nextS() % 25);
            if (i == steps/2) {
                fPath.moveTo(x, tmpY);
            } else {
                fPath.lineTo(x, tmpY);
            }
        }

        {
            SkRect  oval;
            oval.setLTRB(20, 30, 100, 60);
            oval.offset(x, 0);
            fPath.addRoundRect(oval, SkIntToScalar(8), SkIntToScalar(8));
        }

        fClickPt.set(SkIntToScalar(200), SkIntToScalar(200));

        this->setBGColor(0xFFDDDDDD);
    }

    SkString name() override { return SkString("PathEffects"); }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;

        canvas->translate(0, 50);

        paint.setColor(SK_ColorBLUE);
        paint.setPathEffect(make_pe(2, fPhase));
        canvas->drawPath(fPath, paint);

        canvas->translate(0, 50);

        paint.setARGB(0xFF, 0, 0xBB, 0);
        paint.setPathEffect(make_pe(3, fPhase));
        canvas->drawPath(fPath, paint);

        canvas->translate(0, 50);

        paint.setARGB(0xFF, 0, 0, 0);
        paint.setPathEffect(make_warp_pe(fPhase));
        canvas->drawPath(fPath, paint);
    }

    bool onAnimate(double nanos) override {
        fPhase = TimeUtils::Scaled(1e-9 * nanos, 40);
        return true;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new PathEffectView(); )
