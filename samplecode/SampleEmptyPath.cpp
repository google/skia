/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkString.h"

class EmptyPathView : public SampleView {
public:
    EmptyPathView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "EmptyPath");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

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

    virtual void onDrawContent(SkCanvas* canvas) {
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
        titlePaint.setLCDRenderText(true);
        titlePaint.setTextSize(24 * SK_Scalar1);
        const char title[] = "Empty Paths Drawn Into Rectangle Clips With Indicated Style and Fill";
        canvas->drawText(title, strlen(title),
                         40 * SK_Scalar1,
                         100*SK_Scalar1,
                         titlePaint);

        SkRandom rand;
        SkRect rect = SkRect::MakeWH(125*SK_Scalar1, 100*SK_Scalar1);
        int i = 0;
        canvas->save();
        canvas->translate(80 * SK_Scalar1, 0);
        canvas->save();
        for (size_t style = 0; style < SK_ARRAY_COUNT(gStyles); ++style) {
            for (size_t fill = 0; fill < SK_ARRAY_COUNT(gFills); ++fill) {
                if (0 == i % 4) {
                    canvas->restore();
                    canvas->translate(0, rect.height() + 50 * SK_Scalar1);
                    canvas->save();
                } else {
                    canvas->translate(rect.width() + 100 * SK_Scalar1, 0);
                }
                ++i;


                SkColor color = rand.nextU();
                color = 0xff000000| color; // force solid
                this->drawEmpty(canvas, color, rect,
                                gStyles[style].fStyle, gFills[fill].fFill);

                SkPaint rectPaint;
                rectPaint.setColor(SK_ColorBLACK);
                rectPaint.setStyle(SkPaint::kStroke_Style);
                rectPaint.setStrokeWidth(-1);
                rectPaint.setAntiAlias(true);
                canvas->drawRect(rect, rectPaint);

                SkString label;
                label.appendf("%s, %s", gStyles[style].fName, gFills[fill].fName);

                SkPaint labelPaint;
                labelPaint.setColor(color);
                labelPaint.setAntiAlias(true);
                labelPaint.setLCDRenderText(true);
                canvas->drawText(label.c_str(), label.size(),
                                 0, rect.height() + 15 * SK_Scalar1,
                                 labelPaint);
            }
        }
        canvas->restore();
        canvas->restore();
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new EmptyPathView; }
static SkViewRegister reg(MyFactory);
