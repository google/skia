/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"

#define W   SkIntToScalar(100)
#define H   SkIntToScalar(60)

typedef void (*Proc)(SkCanvas*, const SkPaint&);

static void draw_hair(SkCanvas* canvas, const SkPaint& paint) {
    SkPaint p(paint);
    p.setStrokeWidth(0);
    canvas->drawLine(0, 0, W, H, p);
}

static void draw_thick(SkCanvas* canvas, const SkPaint& paint) {
    SkPaint p(paint);
    p.setStrokeWidth(H/5);
    canvas->drawLine(0, 0, W, H, p);
}

static void draw_rect(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawRect(SkRect::MakeWH(W, H), paint);
}

static void draw_oval(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawOval(SkRect::MakeWH(W, H), paint);
}

static void draw_text(SkCanvas* canvas, const SkPaint& paint) {
    SkPaint p(paint);
    p.setTextSize(H/4);
    canvas->drawText("Hamburgefons", 12, 0, H*2/3, p);
}

class SrcModeGM : public skiagm::GM {
    SkPath fPath;
public:
    SrcModeGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    virtual SkString onShortName() {
        return SkString("srcmode");
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0x80FF0000);

        const Proc procs[] = {
            draw_hair, draw_thick, draw_rect, draw_oval, draw_text
        };

        const SkXfermode::Mode modes[] = {
            SkXfermode::kSrcOver_Mode, SkXfermode::kSrc_Mode, SkXfermode::kClear_Mode
        };

        for (size_t x = 0; x < SK_ARRAY_COUNT(modes); ++x) {
            paint.setXfermodeMode(modes[x]);
            canvas->save();
            for (size_t y = 0; y < SK_ARRAY_COUNT(procs); ++y) {
                procs[y](canvas, paint);
                canvas->translate(0, H * 5 / 4);
            }
            canvas->restore();
            canvas->translate(W * 5 / 4, 0);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SrcModeGM;)

