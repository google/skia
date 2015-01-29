/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRandom.h"

class AddArcGM : public skiagm::GM {
protected:
    SkString onShortName() SK_OVERRIDE { return SkString("addarc"); }

    SkISize onISize() SK_OVERRIDE { return SkISize::Make(1040, 1040); }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->translate(20, 20);

        SkRect r = SkRect::MakeWH(1000, 1000);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(15);

        const SkScalar inset = paint.getStrokeWidth() + 4;
        const SkScalar sweepAngle = 345;
        SkRandom rand;

        while (r.width() > paint.getStrokeWidth() * 3) {
            paint.setColor(rand.nextU() | (0xFF << 24));
            SkScalar startAngle = rand.nextUScalar1() * 360;

            SkPath path;
            path.addArc(r, startAngle, sweepAngle);
            canvas->drawPath(path, paint);

            r.inset(inset, inset);

        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new AddArcGM; )
