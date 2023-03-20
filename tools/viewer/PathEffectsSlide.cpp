/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "src/base/SkRandom.h"
#include "src/base/SkUTF.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

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
    for (unsigned i = 2; i < std::size(gXY); i += 2)
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
    for (unsigned i = 2; i < std::size(gXY); i += 2) {
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

class PathEffectSlide : public Slide {
    SkPath  fPath;
    SkPoint fClickPt;
    SkScalar fPhase;

public:
    PathEffectSlide() : fPhase(0) { fName = "PathEffects"; }

    void load(SkScalar w, SkScalar h) override {
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
    }

    void draw(SkCanvas* canvas) override {
        canvas->clear(0xFFDDDDDD);

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

    bool animate(double nanos) override {
        fPhase = TimeUtils::Scaled(1e-9 * nanos, 40);
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new PathEffectSlide(); )
