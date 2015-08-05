/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"

#define W   150
#define H   200

static void show_text(SkCanvas* canvas, bool doAA) {
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(doAA);
    paint.setLCDRenderText(true);
    paint.setTextSize(SkIntToScalar(20));

    for (int i = 0; i < 200; ++i) {
        paint.setColor((SK_A32_MASK << SK_A32_SHIFT) | rand.nextU());
        canvas->drawText("Hamburgefons", 12,
                         rand.nextSScalar1() * W, rand.nextSScalar1() * H + 20,
                         paint);
    }
}

static void show_fill(SkCanvas* canvas, bool doAA) {
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(doAA);

    for (int i = 0; i < 50; ++i) {
        SkRect r;
        SkPath p;

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        p.addOval(r);
        canvas->drawPath(p, paint);
    }
}

static SkScalar randRange(SkRandom& rand, SkScalar min, SkScalar max) {
    SkASSERT(min <= max);
    return min + SkScalarMul(rand.nextUScalar1(), max - min);
}

static void show_stroke(SkCanvas* canvas, bool doAA, SkScalar strokeWidth, int n) {
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(doAA);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(strokeWidth);

    for (int i = 0; i < n; ++i) {
        SkRect r;
        SkPath p;

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        p.addOval(r);
        canvas->drawPath(p, paint);

        const SkScalar minx = -SkIntToScalar(W)/4;
        const SkScalar maxx = 5*SkIntToScalar(W)/4;
        const SkScalar miny = -SkIntToScalar(H)/4;
        const SkScalar maxy = 5*SkIntToScalar(H)/4;
        paint.setColor(rand.nextU());
        canvas->drawLine(randRange(rand, minx, maxx), randRange(rand, miny, maxy),
                         randRange(rand, minx, maxx), randRange(rand, miny, maxy),
                         paint);
    }
}

static void show_hair(SkCanvas* canvas, bool doAA) {
    show_stroke(canvas, doAA, 0, 150);
}

static void show_thick(SkCanvas* canvas, bool doAA) {
    show_stroke(canvas, doAA, SkIntToScalar(5), 50);
}

typedef void (*CanvasProc)(SkCanvas*, bool);

#include "SkAAClip.h"

class ClipView : public SampleView {
public:
    ClipView() {
        SkAAClip clip;
        SkIRect r = { -2, -3, 842, 18 };
        clip.setRect(r);
    }

    virtual ~ClipView() {
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Clip");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        static const CanvasProc gProc[] = {
            show_text, show_thick, show_hair, show_fill
        };

        SkRect r = { 0, 0, SkIntToScalar(W), SkIntToScalar(H) };
        SkPath clipPath;
        r.inset(SK_Scalar1 / 4, SK_Scalar1 / 4);
        clipPath.addRoundRect(r, SkIntToScalar(20), SkIntToScalar(20));

//        clipPath.toggleInverseFillType();

        for (int aa = 0; aa <= 1; ++aa) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(gProc); ++i) {
                canvas->save();
                canvas->clipPath(clipPath, SkRegion::kIntersect_Op, SkToBool(aa));
//                canvas->drawColor(SK_ColorWHITE);
                gProc[i](canvas, SkToBool(aa));
                canvas->restore();
                canvas->translate(W * SK_Scalar1 * 8 / 7, 0);
            }
            canvas->restore();
            canvas->translate(0, H * SK_Scalar1 * 8 / 7);
        }
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ClipView; }
static SkViewRegister reg(MyFactory);
