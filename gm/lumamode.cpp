/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkLumaXfermode.h"

static void show_scene(SkCanvas* canvas, SkXfermode* mode, const char* label) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRect r, c, bounds = { 10, 10, 110, 110 };

    c = bounds;
    c.fRight = bounds.centerX();
    r = bounds;
    r.fBottom = bounds.centerY();
    canvas->drawRect(r, paint);

    SkScalar tw = paint.measureText(label, strlen(label));
    canvas->drawText(label, strlen(label), bounds.centerX() - tw / 2,
                     bounds.bottom() + 15, paint);

    canvas->saveLayer(&bounds, NULL);

    r = bounds;
    r.inset(20, 0);
    paint.setColor(0x80FFFF00);
    canvas->drawOval(r, paint);
    canvas->save();
    canvas->clipRect(c);
    paint.setColor(0xFFFFFF00);
    canvas->drawOval(r, paint);
    canvas->restore();

    SkPaint xferPaint;
    xferPaint.setXfermode(mode);
    canvas->saveLayer(&bounds, &xferPaint);

    r = bounds;
    r.inset(0, 20);
    paint.setColor(0x8080FF00);
    canvas->drawOval(r, paint);
    canvas->save();
    canvas->clipRect(c);
    paint.setColor(0xFF80FF00);
    canvas->drawOval(r, paint);
    canvas->restore();

    canvas->restore();
    canvas->restore();
}

class LumaXfermodeGM : public skiagm::GM {
public:
    LumaXfermodeGM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("lumamode");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(450, 140);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        show_scene(canvas, NULL, "SrcOver");
        canvas->translate(150, 0);
        SkAutoTUnref<SkXfermode> src_in(SkLumaMaskXfermode::Create(SkXfermode::kSrcIn_Mode));
        show_scene(canvas, src_in.get(), "SrcInLuma");
        canvas->translate(150, 0);
        SkAutoTUnref<SkXfermode> dst_in(SkLumaMaskXfermode::Create(SkXfermode::kDstIn_Mode));
        show_scene(canvas, dst_in.get(), "DstInLuma");
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(LumaXfermodeGM); )
