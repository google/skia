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
    SkString onShortName() override { return SkString("emptypath"); }

    SkISize onISize() override { return {600, 280}; }

    void drawEmpty(SkCanvas* canvas,
                    SkColor color,
                    const SkRect& clip,
                    SkPaint::Style style,
                    SkPathFillType fill) {
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

    void onDraw(SkCanvas* canvas) override {
        struct FillAndName {
            SkPathFillType fFill;
            const char*      fName;
        };
        constexpr FillAndName gFills[] = {
            {SkPathFillType::kWinding, "Winding"},
            {SkPathFillType::kEvenOdd, "Even / Odd"},
            {SkPathFillType::kInverseWinding, "Inverse Winding"},
            {SkPathFillType::kInverseEvenOdd, "Inverse Even / Odd"},
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
};
DEF_GM( return new EmptyPathGM; )

//////////////////////////////////////////////////////////////////////////////

static constexpr int kPtsCount = 3;
static constexpr SkPoint kPts[kPtsCount] = {
    {40, 40},
    {80, 40},
    {120, 40},
};

static void make_path_move(SkPath* path) {
    for (SkPoint p : kPts) {
        path->moveTo(p);
    }
}

static void make_path_move_close(SkPath* path) {
    for (SkPoint p : kPts) {
        path->moveTo(p);
        path->close();
    }
}

static void make_path_move_line(SkPath* path) {
    for (SkPoint p : kPts) {
        path->moveTo(p);
        path->lineTo(p);
    }
}

static void make_path_move_mix(SkPath* path) {
    path->moveTo(kPts[0]);
    path->moveTo(kPts[1]); path->close();
    path->moveTo(kPts[2]); path->lineTo(kPts[2]);
}

class EmptyStrokeGM : public GM {
    SkString onShortName() override { return SkString("emptystroke"); }

    SkISize onISize() override { return {200, 240}; }

    void onDraw(SkCanvas* canvas) override {
        static constexpr void (*kProcs[])(SkPath*) = {
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

        for (auto proc : kProcs) {
            SkPath path;
            proc(&path);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, kPtsCount, kPts, dotPaint);
            canvas->drawPath(path, strokePaint);
            canvas->translate(0, 40);
        }
    }
};
DEF_GM( return new EmptyStrokeGM; )

}
