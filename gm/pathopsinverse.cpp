/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/pathops/SkPathOps.h"
#include "tools/ToolUtils.h"

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

    SkString getName() const override { return SkString("pathopsinverse"); }

    SkISize getISize() override { return SkISize::Make(1200, 900); }

    void onDraw(SkCanvas* canvas) override {
        SkPath one, two;
        int yPos = 0;
        for (int oneFill = 0; oneFill <= 1; ++oneFill) {
            SkPathFillType oneF = oneFill ? SkPathFillType::kInverseEvenOdd
                    : SkPathFillType::kEvenOdd;
            for (int twoFill = 0; twoFill <= 1; ++twoFill) {
                SkPathFillType twoF = twoFill ? SkPathFillType::kInverseEvenOdd
                        : SkPathFillType::kEvenOdd;
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
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PathOpsInverseGM; )

}  // namespace skiagm

#include "include/utils/SkParsePath.h"

DEF_SIMPLE_GM(pathops_skbug_10155, canvas, 256, 256) {
    const char* svgStr[] = {
        "M474.889 27.0952C474.889 27.1002 474.888 27.1018 474.889 27.1004L479.872 27.5019C479.883 27.3656 479.889 27.2299 479.889 27.0952L474.889 27.0952L474.889 27.0952Z",
        "M474.94 26.9405C474.93 26.9482 474.917 26.9576 474.901 26.9683L477.689 31.1186C477.789 31.0512 477.888 30.9804 477.985 30.9059L474.94 26.9405L474.94 26.9405Z"
    };

    SkPath path[2], resultPath;
    SkOpBuilder builder;

    for (int i = 0; i < 2; i++)
    {
        SkParsePath::FromSVGString(svgStr[i], &path[i]);
        builder.add(path[i], kUnion_SkPathOp);
    }

    builder.resolve(&resultPath);

    auto r = path[0].getBounds();
    canvas->translate(30, 30);
    canvas->scale(200 / r.width(), 200 / r.width());
    canvas->translate(-r.fLeft, -r.fTop);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);

    canvas->drawPath(path[0], paint);
    canvas->drawPath(path[1], paint);

    // The blue draw should (nearly) overdraw all of the red (except where the two paths intersect)
    paint.setColor(SK_ColorBLUE);
    canvas->drawPath(resultPath, paint);
}
