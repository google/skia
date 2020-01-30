// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(pathops, 1000, 600, false, 0) {
void makePaint(SkPaint * paint, SkColor color) {
    paint->setAntiAlias(true);
    paint->setStyle(SkPaint::kFill_Style);
    paint->setColor(color);
}

SkColor blend(SkColor one, SkColor two) {
    SkBitmap temp;
    temp.allocN32Pixels(1, 1);
    SkCanvas canvas(temp);
    canvas.drawColor(one);
    canvas.drawColor(two);
    void* pixels = temp.getPixels();
    return *(SkColor*)pixels;
}

void draw(SkCanvas* canvas) {
    SkPaint fOnePaint;
    SkPaint fTwoPaint;
    SkPaint fOutlinePaint;
    SkPaint fOpPaint[kReverseDifference_SkPathOp - kDifference_SkPathOp + 1];

    const unsigned oneColor = 0xFF8080FF;
    const unsigned twoColor = 0x807F1f1f;
    SkColor blendColor = blend(oneColor, twoColor);
    makePaint(&fOnePaint, oneColor);
    makePaint(&fTwoPaint, twoColor);
    makePaint(&fOpPaint[kDifference_SkPathOp], oneColor);
    makePaint(&fOpPaint[kIntersect_SkPathOp], blendColor);
    makePaint(&fOpPaint[kUnion_SkPathOp], 0xFFc0FFc0);
    makePaint(&fOpPaint[kReverseDifference_SkPathOp], twoColor);
    makePaint(&fOpPaint[kXOR_SkPathOp], 0xFFa0FFe0);
    makePaint(&fOutlinePaint, 0xFF000000);
    fOutlinePaint.setStyle(SkPaint::kStroke_Style);

    SkPath one, two;
    int yPos = 0;
    for (int oneFill = 0; oneFill <= 1; ++oneFill) {
        SkPathFillType oneF =
                oneFill ? SkPathFillType::kInverseEvenOdd : SkPathFillType::kEvenOdd;
        for (int twoFill = 0; twoFill <= 1; ++twoFill) {
            SkPathFillType twoF =
                    twoFill ? SkPathFillType::kInverseEvenOdd : SkPathFillType::kEvenOdd;
            one.reset();
            one.setFillType(oneF);

            one.moveTo(10, 10);
            one.conicTo(0, 90, 50, 50, 3);
            one.conicTo(90, 0, 90, 90, 2);
            one.close();

            two.reset();
            two.setFillType(twoF);
            two.addRect(40, 40, 100, 100);
            canvas->save();
            canvas->translate(0, SkIntToScalar(yPos));
            canvas->clipRect(SkRect::MakeWH(110, 110), SkClipOp::kIntersect, true);
            canvas->drawPath(one, fOnePaint);
            canvas->drawPath(one, fOutlinePaint);
            canvas->drawPath(two, fTwoPaint);
            canvas->drawPath(two, fOutlinePaint);
            canvas->restore();
            int xPos = 150;
            for (int op = kDifference_SkPathOp; op <= kReverseDifference_SkPathOp; ++op) {
                SkPath result;
                Op(one, two, (SkPathOp)op, &result);
                canvas->save();
                canvas->translate(SkIntToScalar(xPos), SkIntToScalar(yPos));
                canvas->clipRect(SkRect::MakeWH(110, 110), SkClipOp::kIntersect, true);
                canvas->drawPath(result, fOpPaint[op]);
                canvas->drawPath(result, fOutlinePaint);
                canvas->restore();
                xPos += 150;
            }
            yPos += 150;
        }
    }
}

}  // END FIDDLE
