/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

namespace skiagm {

class CircularClipsGM : public GM {
public:
    CircularClipsGM() {}

protected:
    virtual SkString onShortName() {
        return SkString("circular-clips");
    }

    virtual SkISize onISize() {
        return SkISize::Make(800, 600);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkRegion::Op ops[] = {
            SkRegion::kDifference_Op,
            SkRegion::kIntersect_Op,
            SkRegion::kUnion_Op,
            SkRegion::kXOR_Op,
            SkRegion::kReverseDifference_Op,
            SkRegion::kReplace_Op,
        };

        SkScalar x1 = 80, x2 = 120;
        SkScalar y = 50;
        SkScalar r = 40;

        SkPath circle1, circle2;
        circle1.addCircle(x1, y, r, SkPath::kCW_Direction);
        circle2.addCircle(x2, y, r, SkPath::kCW_Direction);
        SkRect rect = SkRect::MakeLTRB(x1 - r, y - r, x2 + r, y + r);

        SkPaint fillPaint;

        for (size_t i = 0; i < 4; i++) {
            circle1.toggleInverseFillType();
            if (i % 2 == 0) {
                circle2.toggleInverseFillType();
            }

            canvas->save();
            for (size_t op = 0; op < SK_ARRAY_COUNT(ops); op++) {
                canvas->save();

                canvas->clipPath(circle1, SkRegion::kReplace_Op);
                canvas->clipPath(circle2, ops[op]);

                canvas->drawRect(rect, fillPaint);

                canvas->restore();
                canvas->translate(0, 2 * y);
            }
            canvas->restore();
            canvas->translate(x1 + x2, 0);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new CircularClipsGM; )
}
