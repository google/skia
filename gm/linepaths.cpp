/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"

namespace skiagm {

class LinePathGM : public GM {
public:
    LinePathGM() {}

protected:
    SkString onShortName() {
        return SkString("linepath");
    }

    SkISize onISize() { return make_isize(1240, 390); }

    void drawPath(SkPath& path,SkCanvas* canvas,SkColor color,
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
        struct CapAndName {
            SkPaint::Cap  fCap;
            SkPaint::Join fJoin;
            const char*   fName;
        };
        static const CapAndName gCaps[] = {
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
        path.fName = "moveTo-line";

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);
        titlePaint.setLCDRenderText(true);
        titlePaint.setTextSize(15 * SK_Scalar1);
        const char title[] = "Line Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, with stroke width 10";
        canvas->drawText(title, strlen(title),
                            20 * SK_Scalar1,
                            20 * SK_Scalar1,
                            titlePaint);

        SkLCGRandom rand;
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

                    SkColor color = 0xff007000;
                    this->drawPath(path.fPath, canvas, color, rect,
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
                    labelPaint.setAntiAlias(true);
                    labelPaint.setLCDRenderText(true);
                    labelPaint.setTextSize(10 * SK_Scalar1);
                    canvas->drawText(gStyles[style].fName,
                                        strlen(gStyles[style].fName),
                                        0, rect.height() + 12 * SK_Scalar1,
                                        labelPaint);
                    canvas->drawText(gFills[fill].fName,
                                        strlen(gFills[fill].fName),
                                        0, rect.height() + 24 * SK_Scalar1,
                                        labelPaint);
                    canvas->drawText(gCaps[cap].fName,
                                        strlen(gCaps[cap].fName),
                                        0, rect.height() + 36 * SK_Scalar1,
                                        labelPaint);
                }
                canvas->restore();
            }
            canvas->restore();
        }
        canvas->restore();
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

class LineClosePathGM : public GM {
public:
    LineClosePathGM() {}

protected:
    SkString onShortName() {
        return SkString("lineclosepath");
    }

    SkISize onISize() { return make_isize(1240, 390); }

    void drawPath(SkPath& path,SkCanvas* canvas,SkColor color,
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
        struct CapAndName {
            SkPaint::Cap  fCap;
            SkPaint::Join fJoin;
            const char*   fName;
        };
        static const CapAndName gCaps[] = {
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
        path.fPath.close();
        path.fName = "moveTo-line-close";

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);
        titlePaint.setLCDRenderText(true);
        titlePaint.setTextSize(15 * SK_Scalar1);
        const char title[] = "Line Closed Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, with stroke width 10";
        canvas->drawText(title, strlen(title),
                            20 * SK_Scalar1,
                            20 * SK_Scalar1,
                            titlePaint);

        SkLCGRandom rand;
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

                    SkColor color = 0xff007000;
                    this->drawPath(path.fPath, canvas, color, rect,
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
                    labelPaint.setAntiAlias(true);
                    labelPaint.setLCDRenderText(true);
                    labelPaint.setTextSize(10 * SK_Scalar1);
                    canvas->drawText(gStyles[style].fName,
                                        strlen(gStyles[style].fName),
                                        0, rect.height() + 12 * SK_Scalar1,
                                        labelPaint);
                    canvas->drawText(gFills[fill].fName,
                                        strlen(gFills[fill].fName),
                                        0, rect.height() + 24 * SK_Scalar1,
                                        labelPaint);
                    canvas->drawText(gCaps[cap].fName,
                                        strlen(gCaps[cap].fName),
                                        0, rect.height() + 36 * SK_Scalar1,
                                        labelPaint);
                }
                canvas->restore();
            }
            canvas->restore();
        }
        canvas->restore();
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* LinePathFactory(void*) { return new LinePathGM; }
static GMRegistry regLinePath(LinePathFactory);

static GM* LineClosePathFactory(void*) { return new LineClosePathGM; }
static GMRegistry regLineClosePath(LineClosePathFactory);

}
