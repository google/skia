/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

namespace skiagm {

// This GM exercises the use case found in crbug.com/423834.
// The following pattern:
//    clipRect(r);
//    drawRect(r, withAA);
//    drawRect(r, noAA);
// can leave 1 pixel wide remnants of the first rect.
class ClipDrawDrawGM : public GM {
public:
    ClipDrawDrawGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() SK_OVERRIDE {
        return SkString("clipdrawdraw");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(512, 512);
    }

    // Vertical remnant
    static void draw1(SkCanvas* canvas) {
        SkPaint p;
        p.setAntiAlias(true);

        const SkRect rect = SkRect::MakeXYWH(8, 9, 404, 313);

        canvas->save();

        canvas->scale(0.5f, 0.5f);
        canvas->translate(265, 265);

        canvas->save();
        canvas->clipRect(rect);
        canvas->drawRect(rect, p);
        canvas->restore();

        p.setColor(SK_ColorWHITE);
        p.setAntiAlias(false);
        canvas->drawRect(rect, p);
        canvas->restore();
    }

    // Horizontal remnant
    static void draw2(SkCanvas* canvas) {
        SkPaint p;
        p.setAntiAlias(true);

        const SkRect rect = SkRect::MakeXYWH(8, 9, 404, 313);

        canvas->save();

        canvas->translate(200.800003f, 172.299988f);
        canvas->scale(0.8f, 0.8f);

        canvas->save();
        canvas->clipRect(rect);
        canvas->drawRect(rect, p);
        canvas->restore();

        p.setColor(SK_ColorWHITE);
        p.setAntiAlias(false);
        canvas->drawRect(rect, p);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        draw1(canvas);
        draw2(canvas);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return SkNEW(ClipDrawDrawGM);)

}
