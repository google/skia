/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkRect.h"
#include "ToolUtils.h"
#include "gm.h"

namespace skiagm {

class PathOpsInverseGM : public GM {
public:
    PathOpsInverseGM() {
    }

protected:
    void onOnceBeforeDraw() override {
        const unsigned oneColor   = ToolUtils::color_to_565(0xFF8080FF);
        const unsigned twoColor = 0x807F1f1f;
        SkColor blendColor = blend(oneColor, twoColor);
        makePaint(&fOnePaint, oneColor);
        makePaint(&fTwoPaint, twoColor);
        makePaint(&fOpPaint[kDifference_SkPathOp], oneColor);
        makePaint(&fOpPaint[kIntersect_SkPathOp], blendColor);
        makePaint(&fOpPaint[kUnion_SkPathOp], ToolUtils::color_to_565(0xFFc0FFc0));
        makePaint(&fOpPaint[kReverseDifference_SkPathOp], twoColor);
        makePaint(&fOpPaint[kXOR_SkPathOp], ToolUtils::color_to_565(0xFFa0FFe0));
        makePaint(&fOutlinePaint, 0xFF000000);
        fOutlinePaint.setStyle(SkPaint::kStroke_Style);
    }

    SkColor blend(SkColor one, SkColor two) {
        SkBitmap temp;
        temp.allocN32Pixels(1, 1);
        SkCanvas canvas(temp);
        canvas.drawColor(one);
        canvas.drawColor(two);
        void* pixels = temp.getPixels();
        return *(SkColor*) pixels;
    }

    void makePaint(SkPaint* paint, SkColor color) {
        paint->setAntiAlias(true);
        paint->setStyle(SkPaint::kFill_Style);
        paint->setColor(color);
    }

    SkString onShortName() override {
        return SkString("pathopsinverse");
    }

    SkISize onISize() override {
        return SkISize::Make(1200, 900);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPath one, two;
        int yPos = 0;
        for (int oneFill = 0; oneFill <= 1; ++oneFill) {
            SkPath::FillType oneF = oneFill ? SkPath::kInverseEvenOdd_FillType
                    : SkPath::kEvenOdd_FillType;
            for (int twoFill = 0; twoFill <= 1; ++twoFill) {
                SkPath::FillType twoF = twoFill ? SkPath::kInverseEvenOdd_FillType
                        : SkPath::kEvenOdd_FillType;
                one.reset();
                one.setFillType(oneF);
                one.addRect(10, 10, 70, 70);
                two.reset();
                two.setFillType(twoF);
                two.addRect(40, 40, 100, 100);
                canvas->save();
                canvas->translate(0, SkIntToScalar(yPos));
                canvas->clipRect(SkRect::MakeWH(110, 110), true);
                canvas->drawPath(one, fOnePaint);
                canvas->drawPath(one, fOutlinePaint);
                canvas->drawPath(two, fTwoPaint);
                canvas->drawPath(two, fOutlinePaint);
                canvas->restore();
                int xPos = 150;
                for (int op = kDifference_SkPathOp; op <= kReverseDifference_SkPathOp; ++op) {
                    SkPath result;
                    Op(one, two, (SkPathOp) op, &result);
                    canvas->save();
                    canvas->translate(SkIntToScalar(xPos), SkIntToScalar(yPos));
                    canvas->clipRect(SkRect::MakeWH(110, 110), true);
                    canvas->drawPath(result, fOpPaint[op]);
                    canvas->drawPath(result, fOutlinePaint);
                    canvas->restore();
                    xPos += 150;
                }
                yPos += 150;
            }
        }
    }

private:
    SkPaint fOnePaint;
    SkPaint fTwoPaint;
    SkPaint fOutlinePaint;
    SkPaint fOpPaint[kReverseDifference_SkPathOp - kDifference_SkPathOp + 1];
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PathOpsInverseGM; )

}
