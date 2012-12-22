/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkRandom.h"

static void rotateAbout(SkCanvas* canvas, SkScalar degrees,
                        SkScalar cx, SkScalar cy) {
    canvas->translate(cx, cy);
    canvas->rotate(degrees);
    canvas->translate(-cx, -cy);
}

class RotateCirclesView : public SampleView {
public:
    RotateCirclesView() {
        this->setBGColor(SK_ColorLTGRAY);
        
        fAngle = 0;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "RotateCircles");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkRandom rand;
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(20);

        SkScalar cx = 240;
        SkScalar cy = 240;
        SkScalar DX = 240 * 2;
        SkColor color = 0;

        float scale = 1;
        float sign = 0.3f;
        for (SkScalar rad = 200; rad >= 20; rad -= 15) {
            sign = -sign;
            scale += 0.2f;

            paint.setColor(rand.nextU());
            paint.setAlpha(0xFF);
            color = ~color;
    
            paint.setStyle(SkPaint::kFill_Style);

            canvas->save();
            rotateAbout(canvas, fAngle * scale * sign, cx, cy);
            canvas->drawCircle(cx, cy, rad, paint);
            canvas->restore();

            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(rad*2);

            canvas->save();
            rotateAbout(canvas, fAngle * scale * sign, cx + DX, cy);
            canvas->drawCircle(cx + DX, cy, 10, paint);
            canvas->restore();
            
        }
        
        fAngle = (fAngle + 1) % 360;
        this->inval(NULL);
    }

private:
    int fAngle;
    typedef SkView INHERITED;
};

static SkView* F0() { return new RotateCirclesView; }
static SkViewRegister gR0(F0);

