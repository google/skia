/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "Sk1DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"
#include "SkColorPriv.h"

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

#include "SkColorFilter.h"
#include "SkLayerRasterizer.h"

class TestRastBuilder : public SkLayerRasterizer::Builder {
public:
    TestRastBuilder() {
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setAlpha(0x66);
        this->addLayer(paint, SkIntToScalar(4), SkIntToScalar(4));

        paint.setAlpha(0xFF);
        this->addLayer(paint);
    }
};

class PathEffectView : public SampleView {
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
            oval.set(SkIntToScalar(20), SkIntToScalar(30),
                     SkIntToScalar(100), SkIntToScalar(60));
            oval.offset(x, 0);
            fPath.addRoundRect(oval, SkIntToScalar(8), SkIntToScalar(8));
        }

        fClickPt.set(SkIntToScalar(200), SkIntToScalar(200));

        this->setBGColor(0xFFDDDDDD);
    }

    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PathEffects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

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
        TestRastBuilder testRastBuilder;
        paint.setRasterizer(testRastBuilder.detach());
        canvas->drawPath(fPath, paint);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fPhase = timer.scaled(40);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PathEffectView; }
static SkViewRegister reg(MyFactory);
