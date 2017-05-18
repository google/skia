/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
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

static void inset2(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
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
        if (radii[i].fX) {
            radii[i].fX -= dx;
        }
        if (radii[i].fY) {
            radii[i].fY -= dy;
        }
    }
    dst->setRectRadii(r, radii);
}

static SkScalar prop(SkScalar radius, SkScalar newSize, SkScalar oldSize) {
    return newSize * radius / oldSize;
}

static void inset3(const SkRRect& src, SkScalar dx, SkScalar dy, SkRRect* dst) {
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
        radii[i].fX = prop(radii[i].fX, r.width(), src.rect().width());
        radii[i].fY = prop(radii[i].fY, r.height(), src.rect().height());
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
        paint.setColor(sk_tool_utils::color_to_565(0xFF008800));
    } else if (rrect.isSimple()) {
        paint.setColor(SK_ColorBLUE);
    } else {
        paint.setColor(SK_ColorBLACK);
    }
    canvas->drawRRect(rrect, paint);
}

static void drawrr(SkCanvas* canvas, const SkRRect& rrect, InsetProc proc) {
    SkRRect rr;
    for (SkScalar d = -30; d <= 30; d += 5) {
        proc(rrect, d, d, &rr);
        draw_rrect_color(canvas, rr);
    }
}

class RRectGM : public skiagm::GM {
public:
    RRectGM() {}

protected:

    SkString onShortName() override {
        return SkString("rrect");
    }

    SkISize onISize() override {
        return SkISize::Make(820, 710);
    }

    void onDraw(SkCanvas* canvas) override {
        constexpr InsetProc insetProcs[] = {
            inset0, inset1, inset2, inset3
        };

        SkRRect rrect[4];
        SkRect r = { 0, 0, 120, 100 };
        SkVector radii[4] = {
            { 0, 0 }, { 30, 1 }, { 10, 40 }, { 40, 40 }
        };

        rrect[0].setRect(r);
        rrect[1].setOval(r);
        rrect[2].setRectXY(r, 20, 20);
        rrect[3].setRectRadii(r, radii);

        canvas->translate(50.5f, 50.5f);
        for (size_t j = 0; j < SK_ARRAY_COUNT(insetProcs); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(rrect); ++i) {
                drawrr(canvas, rrect[i], insetProcs[j]);
                canvas->translate(200, 0);
            }
            canvas->restore();
            canvas->translate(0, 170);
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new RRectGM; )
