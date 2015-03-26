/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

/**
 *  Skia may draw from outlines when the size is very large, so we exercise that
 *  here.
*/

class BigTextGM : public skiagm::GM {
public:
    BigTextGM() {}

protected:

    SkString onShortName() override {
        return SkString("bigtext");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(1500);

        SkRect r;
        (void)paint.measureText("/", 1, &r);
        SkPoint pos = {
            this->width()/2 - r.centerX(),
            this->height()/2 - r.centerY()
        };

        paint.setColor(SK_ColorRED);
        canvas->drawText("/", 1, pos.fX, pos.fY, paint);

        paint.setColor(SK_ColorBLUE);
        canvas->drawPosText("\\", 1, &pos, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(BigTextGM); )
