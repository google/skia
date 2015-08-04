
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"

namespace skiagm {

class EmptyPathGM : public GM {
public:
    EmptyPathGM() {}

protected:
    SkString onShortName() {
        return SkString("emptypath");
    }

    SkISize onISize() { return SkISize::Make(600, 280); }

    void drawEmpty(SkCanvas* canvas,
                    SkColor color,
                    const SkRect& clip,
                    SkPaint::Style style,
                    SkPath::FillType fill) {
        SkPath path;
        path.setFillType(fill);
        SkPaint paint;
        paint.setColor(color);
        paint.setStyle(style);
        canvas->save();
        canvas->clipRect(clip);
        canvas->drawPath(path, paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {
        struct FillAndName {
            SkPath::FillType fFill;
            const char*      fName;
        };
        static const FillAndName gFills[] = {
            {SkPath::kWinding_FillType, "Winding"},
            {SkPath::kEvenOdd_FillType, "Even / Odd"},
            {SkPath::kInverseWinding_FillType, "Inverse Winding"},
            {SkPath::kInverseEvenOdd_FillType, "Inverse Even / Odd"},
        };
        struct StyleAndName {
            SkPaint::Style fStyle;
            const char*    fName;
        };
        static const StyleAndName gStyles[] = {
            {SkPaint::kFill_Style, "Fill"},
            {SkPaint::kStroke_Style, "Stroke"},
            {SkPaint::kStrokeAndFill_Style, "Stroke And Fill"},
        };

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&titlePaint);
        titlePaint.setTextSize(15 * SK_Scalar1);
        const char title[] = "Empty Paths Drawn Into Rectangle Clips With "
                             "Indicated Style and Fill";
        canvas->drawText(title, strlen(title),
                            20 * SK_Scalar1,
                            20 * SK_Scalar1,
                            titlePaint);

        SkRandom rand;
        SkRect rect = SkRect::MakeWH(100*SK_Scalar1, 30*SK_Scalar1);
        int i = 0;
        canvas->save();
        canvas->translate(10 * SK_Scalar1, 0);
        canvas->save();
        for (size_t style = 0; style < SK_ARRAY_COUNT(gStyles); ++style) {
            for (size_t fill = 0; fill < SK_ARRAY_COUNT(gFills); ++fill) {
                if (0 == i % 4) {
                    canvas->restore();
                    canvas->translate(0, rect.height() + 40 * SK_Scalar1);
                    canvas->save();
                } else {
                    canvas->translate(rect.width() + 40 * SK_Scalar1, 0);
                }
                ++i;


                SkColor color = rand.nextU();
                color = 0xff000000 | color; // force solid
                color = sk_tool_utils::color_to_565(color);
                this->drawEmpty(canvas, color, rect,
                                gStyles[style].fStyle, gFills[fill].fFill);

                SkPaint rectPaint;
                rectPaint.setColor(SK_ColorBLACK);
                rectPaint.setStyle(SkPaint::kStroke_Style);
                rectPaint.setStrokeWidth(-1);
                rectPaint.setAntiAlias(true);
                canvas->drawRect(rect, rectPaint);

                SkPaint labelPaint;
                labelPaint.setColor(color);
                labelPaint.setAntiAlias(true);
                sk_tool_utils::set_portable_typeface(&labelPaint);
                labelPaint.setTextSize(12 * SK_Scalar1);
                canvas->drawText(gStyles[style].fName,
                                 strlen(gStyles[style].fName),
                                 0, rect.height() + 15 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(gFills[fill].fName,
                                 strlen(gFills[fill].fName),
                                 0, rect.height() + 28 * SK_Scalar1,
                                 labelPaint);
            }
        }
        canvas->restore();
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new EmptyPathGM; }
static GMRegistry reg(MyFactory);

}
