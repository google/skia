/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "tools/ToolUtils.h"

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
        SkFont font(ToolUtils::create_portable_typeface(), 1500);

        SkRect r;
        (void)font.measureText("/", 1, kUTF8_SkTextEncoding, &r);
        SkPoint pos = {
            this->width()/2 - r.centerX(),
            this->height()/2 - r.centerY()
        };

        paint.setColor(SK_ColorRED);
        canvas->drawSimpleText("/", 1, kUTF8_SkTextEncoding, pos.fX, pos.fY, font, paint);

        paint.setColor(SK_ColorBLUE);
        canvas->drawSimpleText("\\", 1, kUTF8_SkTextEncoding, pos.fX, pos.fY, font, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new BigTextGM;)
