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

typedef void (*InsetProc)(const SkRRect&, SkScalar dx, SkScalar dy, SkRRect*);

static void inset0(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }
    
    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    for (int i = 0; i < 4; ++i) {
        radii[i].fX -= dx;
        radii[i].fY -= dy;
    }
    dst->setRectRadii(r, radii);
}

static void inset1(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
    SkRect r = src.rect();

    r.inset(dx, dy);
    if (r.isEmpty()) {
        dst->setEmpty();
        return;
    }
    
    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        radii[i] = src.radii((SkRRect::Corner)i);
    }
    dst->setRectRadii(r, radii);
}

static void draw_rrect_color(SkCanvas* canvas, const SkRRect& rrect) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    if (rrect.isRect()) {
        paint.setColor(SK_ColorRED);
    } else if (rrect.isOval()) {
        paint.setColor(0xFF008800);
    } else if (rrect.isSimple()) {
        paint.setColor(SK_ColorBLUE);
    } else {
        paint.setColor(SK_ColorBLACK);
    }
    canvas->drawRRect(rrect, paint);
}

static void drawrr(SkCanvas* canvas, const SkRRect& rrect, InsetProc proc) {
    SkRRect rr;
    for (SkScalar d = -30; d <= 30; d += 10) {
        proc(rrect, d, d, &rr);
        draw_rrect_color(canvas, rr);
    }
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
        static const InsetProc insetProcs[] = { inset0, inset1 };

        SkRRect rrect[4];
        SkRect r = { 0, 0, 120, 160 };
        SkVector radii[4] = {
            { 0, 0 }, { 20, 20 }, { 10, 40 }, { 40, 40 }
        };
        
        rrect[0].setRect(r);
        rrect[1].setOval(r);
        rrect[2].setRectXY(r, 20, 20);
        rrect[3].setRectRadii(r, radii);

        canvas->translate(50, 50);
        for (size_t j = 0; j < SK_ARRAY_COUNT(insetProcs); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(rrect); ++i) {
                drawrr(canvas, rrect[i], insetProcs[j]);
                canvas->translate(rrect[i].width() * 5 / 3, 0);
            }
            canvas->restore();
            canvas->translate(0, rrect[0].height() * 5 / 3);
        }
    }
    
private:
    typedef GM INHERITED;
};

DEF_GM( return new RRectGM; )

