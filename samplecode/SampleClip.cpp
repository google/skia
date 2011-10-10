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
#include "SkRandom.h"

#define W   270
#define H   200

static void show_text(SkCanvas* canvas) {
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(SkIntToScalar(20));
    
    for (int i = 0; i < 300; ++i) {
        paint.setColor((SK_A32_MASK << SK_A32_SHIFT) | rand.nextU());
        canvas->drawText("Hamburgefons", 12,
                         rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                         paint);
    }
}

static void show_geo(SkCanvas* canvas) {
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(true);
    
    for (int i = 0; i < 30; ++i) {
        SkRect r;
        SkPath p;

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);
        
        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);
        
        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(rand.nextU());
        p.addOval(r);
        canvas->drawPath(p, paint);
    }
}

typedef void (*CanvasProc)(SkCanvas*);

class ClipView : public SampleView {
public:
    ClipView() {
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
        canvas->drawColor(SK_ColorLTGRAY);
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        static const CanvasProc gProc[] = {
            show_text, show_geo
        };
        
        SkRect r = { 0, 0, SkIntToScalar(W), SkIntToScalar(H) };
        SkPath clipPath;
        r.inset(SK_Scalar1 / 4, SK_Scalar1 / 4);
        clipPath.addRoundRect(r, SkIntToScalar(16), SkIntToScalar(16));

        for (int aa = 0; aa <= 1; ++aa) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(gProc); ++i) {
                canvas->save();
                canvas->clipPath(clipPath);
                canvas->drawColor(SK_ColorWHITE);
                gProc[i](canvas);
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

