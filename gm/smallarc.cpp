/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

namespace skiagm {

// this draws a small arc scaled up
// see https://code.google.com/p/chromium/issues/detail?id=102411
// and https://code.google.com/p/skia/issues/detail?id=2769
class SmallArcGM : public GM {
public:
    SmallArcGM() {
    }

protected:

    SkString onShortName() override {
        return SkString("smallarc");
    }

    SkISize onISize() override {
        return SkISize::Make(762, 762);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(120);

        SkPath path;
        path.moveTo(75, 0);
        path.cubicTo(33.5, 0, 0, 33.5, 0, 75);

        canvas->translate(-400, -400);
        canvas->scale(8, 8);
        canvas->drawPath(path, p);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(SmallArcGM); )

}
