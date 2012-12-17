/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRRect.h"
#include "SkPath.h"

static void draw_rrect_color(SkCanvas* canvas, const SkRRect& rrect) {
    SkPaint paint;
    paint.setAntiAlias(true);
    
    if (rrect.isRect()) {
        paint.setColor(SK_ColorRED);
    } else if (rrect.isOval()) {
        paint.setColor(SK_ColorGREEN);
    } else if (rrect.isSimple()) {
        paint.setColor(SK_ColorBLUE);
    } else {
        paint.setColor(SK_ColorGRAY);
    }
    canvas->drawRRect(rrect, paint);
}

static void drawrr(SkCanvas* canvas, const SkRRect& rrect) {
    SkRRect inner, outer, inner2;
    
    SkScalar dx = 30;
    SkScalar dy = 30;

    rrect.outset(dx, dy, &outer);
    rrect.inset(dx/2, dy/2, &inner);
    rrect.inset(dx, dy, &inner2);

    draw_rrect_color(canvas, outer);
    draw_rrect_color(canvas, rrect);
    draw_rrect_color(canvas, inner);
    draw_rrect_color(canvas, inner2);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorDKGRAY);
    canvas->drawRRect(rrect, paint);
    canvas->drawRRect(inner, paint);
    canvas->drawRRect(inner2, paint);
}

class RRectGM : public skiagm::GM {
public:
    RRectGM() {}
    
protected:
    virtual SkString onShortName() {
        return SkString("rrect");
    }
    
    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        SkRRect rrect[4];
        SkRect r = { 0, 0, 120, 240 };
        SkVector radii[4] = {
            { 0, 0 }, { 20, 20 }, { 10, 40 }, { 40, 40 }
        };
        
        rrect[0].setRect(r);
        rrect[1].setOval(r);
        rrect[2].setRectXY(r, 20, 20);
        rrect[3].setRectRadii(r, radii);

        canvas->translate(50, 50);
        for (size_t i = 0; i < SK_ARRAY_COUNT(rrect); ++i) {
            drawrr(canvas, rrect[i]);
            canvas->translate(rrect[i].width() * 2, 0);
        }
    }
    
private:
    typedef GM INHERITED;
};

DEF_GM( return new RRectGM; )

