/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

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
        constexpr FillAndName gFills[] = {
            {SkPath::kWinding_FillType, "Winding"},
            {SkPath::kEvenOdd_FillType, "Even / Odd"},
            {SkPath::kInverseWinding_FillType, "Inverse Winding"},
            {SkPath::kInverseEvenOdd_FillType, "Inverse Even / Odd"},
        };
        struct StyleAndName {
            SkPaint::Style fStyle;
            const char*    fName;
        };
        constexpr StyleAndName gStyles[] = {
            {SkPaint::kFill_Style, "Fill"},
            {SkPaint::kStroke_Style, "Stroke"},
            {SkPaint::kStrokeAndFill_Style, "Stroke And Fill"},
        };

        SkFont     font(ToolUtils::create_portable_typeface(), 15);
        const char title[] = "Empty Paths Drawn Into Rectangle Clips With "
                             "Indicated Style and Fill";
        canvas->drawString(title, 20.0f, 20.0f, font, SkPaint());

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
                color         = ToolUtils::color_to_565(color);
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
                SkFont labelFont(ToolUtils::create_portable_typeface(), 12);
                canvas->drawString(gStyles[style].fName, 0, rect.height() + 15.0f,
                                   labelFont, labelPaint);
                canvas->drawString(gFills[fill].fName, 0, rect.height() + 28.0f,
                                   labelFont, labelPaint);
            }
        }
        canvas->restore();
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new EmptyPathGM; )

//////////////////////////////////////////////////////////////////////////////

static void make_path_move(SkPath* path, const SkPoint pts[3]) {
    for (int i = 0; i < 3; ++i) {
        path->moveTo(pts[i]);
    }
}

static void make_path_move_close(SkPath* path, const SkPoint pts[3]) {
    for (int i = 0; i < 3; ++i) {
        path->moveTo(pts[i]);
        path->close();
    }
}

static void make_path_move_line(SkPath* path, const SkPoint pts[3]) {
    for (int i = 0; i < 3; ++i) {
        path->moveTo(pts[i]);
        path->lineTo(pts[i]);
    }
}

typedef void (*MakePathProc)(SkPath*, const SkPoint pts[3]);

static void make_path_move_mix(SkPath* path, const SkPoint pts[3]) {
    path->moveTo(pts[0]);
    path->moveTo(pts[1]); path->close();
    path->moveTo(pts[2]); path->lineTo(pts[2]);
}

class EmptyStrokeGM : public GM {
    SkPoint fPts[3];

public:
    EmptyStrokeGM() {
        fPts[0].set(40, 40);
        fPts[1].set(80, 40);
        fPts[2].set(120, 40);
    }

protected:
    SkString onShortName() override {
        return SkString("emptystroke");
    }

    SkISize onISize() override { return SkISize::Make(200, 240); }

    void onDraw(SkCanvas* canvas) override {
        const MakePathProc procs[] = {
            make_path_move,             // expect red red red
            make_path_move_close,       // expect black black black
            make_path_move_line,        // expect black black black
            make_path_move_mix,         // expect red black black,
        };

        SkPaint strokePaint;
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setStrokeWidth(21);
        strokePaint.setStrokeCap(SkPaint::kSquare_Cap);

        SkPaint dotPaint;
        dotPaint.setColor(SK_ColorRED);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        dotPaint.setStrokeWidth(7);

        for (size_t i = 0; i < SK_ARRAY_COUNT(procs); ++i) {
            SkPath path;
            procs[i](&path, fPts);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, 3, fPts, dotPaint);
            canvas->drawPath(path, strokePaint);
            canvas->translate(0, 40);
        }
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new EmptyStrokeGM; )

}
