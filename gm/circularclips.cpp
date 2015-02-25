/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

class CircularClipsGM : public skiagm::GM {
    SkScalar fX1, fX2, fY, fR;
    SkPath   fCircle1, fCircle2;

protected:
    void onOnceBeforeDraw() SK_OVERRIDE {
        fX1 = 80;
        fX2 = 120;
        fY = 50;
        fR = 40;

        fCircle1.addCircle(fX1, fY, fR, SkPath::kCW_Direction);
        fCircle2.addCircle(fX2, fY, fR, SkPath::kCW_Direction);
    }


    bool runAsBench() const SK_OVERRIDE { return true; }

    SkString onShortName() SK_OVERRIDE {
        return SkString("circular-clips");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(800, 600);
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkRegion::Op ops[] = {
            SkRegion::kDifference_Op,
            SkRegion::kIntersect_Op,
            SkRegion::kUnion_Op,
            SkRegion::kXOR_Op,
            SkRegion::kReverseDifference_Op,
            SkRegion::kReplace_Op,
        };

        SkRect rect = SkRect::MakeLTRB(fX1 - fR, fY - fR, fX2 + fR, fY + fR);

        SkPaint fillPaint;

        for (size_t i = 0; i < 4; i++) {
            fCircle1.toggleInverseFillType();
            if (i % 2 == 0) {
                fCircle2.toggleInverseFillType();
            }

            canvas->save();
            for (size_t op = 0; op < SK_ARRAY_COUNT(ops); op++) {
                canvas->save();

                canvas->clipPath(fCircle1, SkRegion::kReplace_Op);
                canvas->clipPath(fCircle2, ops[op]);

                canvas->drawRect(rect, fillPaint);

                canvas->restore();
                canvas->translate(0, 2 * fY);
            }
            canvas->restore();
            canvas->translate(fX1 + fX2, 0);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new CircularClipsGM; )
