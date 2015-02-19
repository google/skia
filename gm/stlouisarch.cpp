/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkTArray.h"

namespace skiagm {

// this GM tests hairlines which fill nearly the entire render target
class StLouisArchGM : public GM {
protected:
    SkString onShortName() SK_OVERRIDE {
        return SkString("stlouisarch");
    }

    SkISize onISize() SK_OVERRIDE { return SkISize::Make((int)kWidth, (int)kHeight); }

    void onOnceBeforeDraw() SK_OVERRIDE {
        {
            SkPath* bigQuad = &fPaths.push_back();
            bigQuad->moveTo(0, 0);
            bigQuad->quadTo(kWidth/2, kHeight, kWidth, 0);
        }

        {
            SkPath* degenBigQuad = &fPaths.push_back();
            SkScalar yPos = kHeight / 2 + 10;
            degenBigQuad->moveTo(0, yPos);
            degenBigQuad->quadTo(0, yPos, kWidth, yPos);
        }


        {
            SkPath* bigCubic = &fPaths.push_back();
            bigCubic->moveTo(0, 0);
            bigCubic->cubicTo(0, kHeight,
                              kWidth, kHeight,
                              kWidth, 0);
        }

        {
            SkPath* degenBigCubic = &fPaths.push_back();
            SkScalar yPos = kHeight / 2;
            degenBigCubic->moveTo(0, yPos);
            degenBigCubic->cubicTo(0, yPos,
                                   0, yPos,
                                   kWidth, yPos);
        }

        {
            SkPath* bigConic = &fPaths.push_back();
            bigConic->moveTo(0, 0);
            bigConic->conicTo(kWidth/2, kHeight, kWidth, 0, .5);
        }

        {
            SkPath* degenBigConic = &fPaths.push_back();
            SkScalar yPos = kHeight / 2 - 10;
            degenBigConic->moveTo(0, yPos);
            degenBigConic->conicTo(0, yPos, kWidth, yPos, .5);
        }
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->save();
        canvas->scale(1, -1);
        canvas->translate(0, -kHeight);
        for (int p = 0; p < fPaths.count(); ++p) {
            SkPaint paint;
            paint.setARGB(0xff, 0, 0, 0);
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(0);
            canvas->drawPath(fPaths[p], paint);
        }
        canvas->restore();
    }

    const SkScalar kWidth = 256;
    const SkScalar kHeight = 256;

private:
    SkTArray<SkPath> fPaths;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new StLouisArchGM; }
static GMRegistry reg(MyFactory);

}
