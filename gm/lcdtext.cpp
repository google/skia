
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/* Tests text rendering with LCD and subpixel rendering turned on and off.
 */

#include "gm.h"
#include "SkCanvas.h"

namespace skiagm {

class LcdTextGM : public GM {
public:
    LcdTextGM() {
        const int pointSize = 36;
        textHeight = SkIntToScalar(pointSize);
    }

protected:

    SkString onShortName() {
        return SkString("lcdtext");
    }

    SkISize onISize() { return make_isize(640, 480); }

    virtual void onDraw(SkCanvas* canvas) {

        y = textHeight;
        drawText(canvas, SkString("TEXT: SubpixelTrue LCDRenderTrue"),
                 true,  true);
        drawText(canvas, SkString("TEXT: SubpixelTrue LCDRenderFalse"),
                 true,  false);
        drawText(canvas, SkString("TEXT: SubpixelFalse LCDRenderTrue"),
                 false, true);
        drawText(canvas, SkString("TEXT: SubpixelFalse LCDRenderFalse"),
                 false, false);
    }

    void drawText(SkCanvas* canvas, const SkString& string,
                  bool subpixelTextEnabled, bool lcdRenderTextEnabled) {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setDither(true);
        paint.setAntiAlias(true);
        paint.setSubpixelText(subpixelTextEnabled);
        paint.setLCDRenderText(lcdRenderTextEnabled);
        paint.setTextSize(textHeight);

        canvas->drawText(string.c_str(), string.size(), 0, y, paint);
        y += textHeight;
    }

private:
    typedef GM INHERITED;
    SkScalar y, textHeight;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new LcdTextGM; }
static GMRegistry reg(MyFactory);

}
