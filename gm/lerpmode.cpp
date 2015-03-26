/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkLerpXfermode.h"

static void show_circlelayers(SkCanvas* canvas, SkXfermode* mode) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRect r, bounds = { 10, 10, 110, 110 };

    r = bounds;
    r.fRight = bounds.centerX();
    canvas->drawRect(r, paint);

    canvas->saveLayer(&bounds, NULL);

    paint.setColor(0x80FF0000);
    r = bounds;
    r.inset(20, 0);
    canvas->drawOval(r, paint);

    paint.setColor(0x800000FF);
    r = bounds;
    r.inset(0, 20);
    paint.setXfermode(mode);
    canvas->drawOval(r, paint);

    canvas->restore();
}

class LerpXfermodeGM : public skiagm::GM {
public:
    LerpXfermodeGM() {}

protected:
    SkString onShortName() override {
        return SkString("lerpmode");
    }

    SkISize onISize() override {
        return SkISize::Make(240, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        show_circlelayers(canvas, NULL);
        canvas->translate(150, 0);
        SkAutoTUnref<SkXfermode> mode(SkLerpXfermode::Create(0.5f));
        show_circlelayers(canvas, mode.get());
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(LerpXfermodeGM); )
