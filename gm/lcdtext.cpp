/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

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
