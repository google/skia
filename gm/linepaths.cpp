/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

static void drawPath(SkPath& path,SkCanvas* canvas,SkColor color,
                     const SkRect& clip,SkPaint::Cap cap, SkPaint::Join join,
                     SkPaint::Style style, SkPath::FillType fill,
                     SkScalar strokeWidth) {
        path.setFillType(fill);
        SkPaint paint;
        paint.setStrokeCap(cap);
        paint.setStrokeWidth(strokeWidth);
        paint.setStrokeJoin(join);
        paint.setColor(color);
        paint.setStyle(style);
        canvas->save();
        canvas->clipRect(clip);
        canvas->drawPath(path, paint);
        canvas->restore();
}

static void draw(SkCanvas* canvas, bool doClose) {
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
        struct CapAndName {
            SkPaint::Cap  fCap;
            SkPaint::Join fJoin;
            const char*   fName;
        };
        constexpr CapAndName gCaps[] = {
            {SkPaint::kButt_Cap, SkPaint::kBevel_Join, "Butt"},
            {SkPaint::kRound_Cap, SkPaint::kRound_Join, "Round"},
            {SkPaint::kSquare_Cap, SkPaint::kBevel_Join, "Square"}
        };
        struct PathAndName {
            SkPath      fPath;
            const char* fName;
        };
        PathAndName path;
        path.fPath.moveTo(25*SK_Scalar1, 15*SK_Scalar1);
        path.fPath.lineTo(75*SK_Scalar1, 15*SK_Scalar1);
        if (doClose) {
            path.fPath.close();
            path.fName = "moveTo-line-close";
        } else {
            path.fName = "moveTo-line";
        }

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);

        SkFont font(ToolUtils::create_portable_typeface(), 15.0f);

        const char titleNoClose[] = "Line Drawn Into Rectangle Clips With "
            "Indicated Style, Fill and Linecaps, with stroke width 10";
        const char titleClose[] = "Line Closed Drawn Into Rectangle Clips With "
            "Indicated Style, Fill and Linecaps, with stroke width 10";
        const char* title = doClose ? titleClose : titleNoClose;
        canvas->drawString(title, 20.0f, 20.0f, font, titlePaint);

        SkRandom rand;
        SkRect rect = SkRect::MakeWH(100*SK_Scalar1, 30*SK_Scalar1);
        canvas->save();
        canvas->translate(10 * SK_Scalar1, 30 * SK_Scalar1);
        canvas->save();
        for (size_t cap = 0; cap < SK_ARRAY_COUNT(gCaps); ++cap) {
            if (0 < cap) {
                canvas->translate((rect.width() + 40 * SK_Scalar1) * SK_ARRAY_COUNT(gStyles), 0);
            }
            canvas->save();
            for (size_t fill = 0; fill < SK_ARRAY_COUNT(gFills); ++fill) {
                if (0 < fill) {
                    canvas->translate(0, rect.height() + 40 * SK_Scalar1);
                }
                canvas->save();
                for (size_t style = 0; style < SK_ARRAY_COUNT(gStyles); ++style) {
                    if (0 < style) {
                        canvas->translate(rect.width() + 40 * SK_Scalar1, 0);
                    }

                    SkColor color = ToolUtils::color_to_565(0xff007000);
                    drawPath(path.fPath, canvas, color, rect,
                                    gCaps[cap].fCap, gCaps[cap].fJoin, gStyles[style].fStyle,
                                    gFills[fill].fFill, SK_Scalar1*10);

                    SkPaint rectPaint;
                    rectPaint.setColor(SK_ColorBLACK);
                    rectPaint.setStyle(SkPaint::kStroke_Style);
                    rectPaint.setStrokeWidth(-1);
                    rectPaint.setAntiAlias(true);
                    canvas->drawRect(rect, rectPaint);

                    SkPaint labelPaint;
                    labelPaint.setColor(color);
                    font.setSize(10);
                    canvas->drawString(gStyles[style].fName, 0, rect.height() + 12.0f,
                                       font, labelPaint);
                    canvas->drawString(gFills[fill].fName, 0, rect.height() + 24.0f,
                                       font, labelPaint);
                    canvas->drawString(gCaps[cap].fName, 0, rect.height() + 36.0f,
                                       font, labelPaint);
                }
                canvas->restore();
            }
            canvas->restore();
        }
        canvas->restore();
        canvas->restore();
}
DEF_SIMPLE_GM(linepath, canvas, 1240, 390) {
    draw(canvas, false);
}
DEF_SIMPLE_GM(lineclosepath, canvas, 1240, 390) {
    draw(canvas, true);
}
