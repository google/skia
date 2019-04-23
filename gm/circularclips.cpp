/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

class CircularClipsGM : public skiagm::GM {
    SkScalar fX1, fX2, fY, fR;
    SkPath   fCircle1, fCircle2;

protected:
    void onOnceBeforeDraw() override {
        fX1 = 80;
        fX2 = 120;
        fY = 50;
        fR = 40;

        fCircle1.addCircle(fX1, fY, fR, SkPath::kCW_Direction);
        fCircle2.addCircle(fX2, fY, fR, SkPath::kCW_Direction);
    }


    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        return SkString("circular-clips");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkClipOp ops[] = {
            kDifference_SkClipOp,
            kIntersect_SkClipOp,
            kUnion_SkClipOp,
            kXOR_SkClipOp,
            kReverseDifference_SkClipOp,
            kReplace_SkClipOp,
        };

        SkRect rect = SkRect::MakeLTRB(fX1 - fR, fY - fR, fX2 + fR, fY + fR);

        SkPaint fillPaint;

        // Giant background circular clips (AA, non-inverted, replace/isect)
        fillPaint.setColor(0x80808080);
        canvas->save();
        canvas->scale(10, 10);
        canvas->translate(-((fX1 + fX2)/2 - fR), -(fY - 2*fR/3));
        canvas->clipPath(fCircle1, true);
        canvas->clipPath(fCircle2, true);

        canvas->drawRect(rect, fillPaint);

        canvas->restore();

        fillPaint.setColor(0xFF000000);

        for (size_t i = 0; i < 4; i++) {
            fCircle1.toggleInverseFillType();
            if (i % 2 == 0) {
                fCircle2.toggleInverseFillType();
            }

            canvas->save();
            for (size_t op = 0; op < SK_ARRAY_COUNT(ops); op++) {
                canvas->save();

                canvas->clipPath(fCircle1);
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
