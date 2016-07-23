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

// https://bug.skia.org/1316 shows that this cubic, when slightly clipped, creates big
// (incorrect) changes to its control points.
class ClippedCubicGM : public skiagm::GM {
public:
    ClippedCubicGM() {}

protected:

    SkString onShortName() {
        return SkString("clippedcubic");
    }

    SkISize onISize() { return SkISize::Make(1240, 390); }

    virtual void onDraw(SkCanvas* canvas) {
        SkPath path;
        path.moveTo(0, 0);
        path.cubicTo(140, 150, 40, 10, 170, 150);

        SkPaint paint;
        SkRect bounds = path.getBounds();

        for (SkScalar dy = -1; dy <= 1; dy += 1) {
            canvas->save();
            for (SkScalar dx = -1; dx <= 1; dx += 1) {
                canvas->save();
                canvas->clipRect(bounds);
                canvas->translate(dx, dy);
                canvas->drawPath(path, paint);
                canvas->restore();

                canvas->translate(bounds.width(), 0);
            }
            canvas->restore();
            canvas->translate(0, bounds.height());
        }
    }

private:
    typedef skiagm::GM INHERITED;
};


class ClippedCubic2GM : public skiagm::GM {
public:
    ClippedCubic2GM() {}

protected:

    SkString onShortName() override {
        return SkString("clippedcubic2");
    }

    SkISize onISize() override { return SkISize::Make(1240, 390); }

    void onDraw(SkCanvas* canvas) override {
        canvas->save();
        canvas->translate(-2, 120);
        drawOne(canvas, fPath, SkRect::MakeLTRB(0, 0, 80, 150));
        canvas->translate(0, 170);
        drawOne(canvas, fPath, SkRect::MakeLTRB(0, 0, 80, 100));
        canvas->translate(0, 170);
        drawOne(canvas, fPath, SkRect::MakeLTRB(0, 0, 30, 150));
        canvas->translate(0, 170);
        drawOne(canvas, fPath, SkRect::MakeLTRB(0, 0, 10, 150));
        canvas->restore();
        canvas->save();
        canvas->translate(20, -2);
        drawOne(canvas, fFlipped, SkRect::MakeLTRB(0, 0, 150, 80));
        canvas->translate(170, 0);
        drawOne(canvas, fFlipped, SkRect::MakeLTRB(0, 0, 100, 80));
        canvas->translate(170, 0);
        drawOne(canvas, fFlipped, SkRect::MakeLTRB(0, 0, 150, 30));
        canvas->translate(170, 0);
        drawOne(canvas, fFlipped, SkRect::MakeLTRB(0, 0, 150, 10));
        canvas->restore();
    }

    void drawOne(SkCanvas* canvas, const SkPath& path, const SkRect& clip) {
        SkPaint framePaint, fillPaint;
        framePaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(clip, framePaint);
        canvas->drawPath(path, framePaint);
        canvas->save();
        canvas->clipRect(clip);
        canvas->drawPath(path, fillPaint);
        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        fPath.moveTo(69.7030518991886f, 0);
        fPath.cubicTo( 69.7030518991886f, 21.831149999999997f,
                58.08369508178456f, 43.66448333333333f, 34.8449814469765f, 65.5f);
        fPath.cubicTo( 11.608591683531916f, 87.33115f, -0.010765133872116195f, 109.16448333333332f,
                -0.013089005235602302f, 131);
        fPath.close();
        fFlipped = fPath;
        SkMatrix matrix;
        matrix.reset();
        matrix.setScaleX(0);
        matrix.setScaleY(0);
        matrix.setSkewX(1);
        matrix.setSkewY(1);
        fFlipped.transform(matrix);
    }

    SkPath fPath;
    SkPath fFlipped;
private:
    typedef skiagm::GM INHERITED;
};


class CubicPathGM : public skiagm::GM {
public:
    CubicPathGM() {}

protected:

    SkString onShortName() {
        return SkString("cubicpath");
    }

    SkISize onISize() { return SkISize::Make(1240, 390); }

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
        path.fPath.moveTo(25*SK_Scalar1, 10*SK_Scalar1);
        path.fPath.cubicTo(40*SK_Scalar1, 20*SK_Scalar1,
                           60*SK_Scalar1, 20*SK_Scalar1,
                           75*SK_Scalar1, 10*SK_Scalar1);
        path.fName = "moveTo-cubic";

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&titlePaint);
        titlePaint.setTextSize(15 * SK_Scalar1);
        const char title[] = "Cubic Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, with stroke width 10";
        canvas->drawText(title, strlen(title),
                            20 * SK_Scalar1,
                            20 * SK_Scalar1,
                            titlePaint);

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
                    sk_tool_utils::set_portable_typeface(&labelPaint);
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
    typedef skiagm::GM INHERITED;
};

class CubicClosePathGM : public skiagm::GM {
public:
    CubicClosePathGM() {}

protected:

    SkString onShortName() {
        return SkString("cubicclosepath");
    }

    SkISize onISize() { return SkISize::Make(1240, 390); }

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
        path.fPath.moveTo(25*SK_Scalar1, 10*SK_Scalar1);
        path.fPath.cubicTo(40*SK_Scalar1, 20*SK_Scalar1,
                           60*SK_Scalar1, 20*SK_Scalar1,
                           75*SK_Scalar1, 10*SK_Scalar1);
        path.fPath.close();
        path.fName = "moveTo-cubic-close";

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&titlePaint);
        titlePaint.setTextSize(15 * SK_Scalar1);
        const char title[] = "Cubic Closed Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, with stroke width 10";
        canvas->drawText(title, strlen(title),
                            20 * SK_Scalar1,
                            20 * SK_Scalar1,
                            titlePaint);

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
                    sk_tool_utils::set_portable_typeface(&labelPaint);
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
    typedef skiagm::GM INHERITED;
};

DEF_SIMPLE_GM(bug5099, canvas, 50, 50) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);

    SkPath path;
    path.moveTo(6, 27);
    path.cubicTo(31.5f, 1.5f, 3.5f, 4.5f, 29, 29);
    canvas->drawPath(path, p);
}

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new CubicPathGM; )
DEF_GM( return new CubicClosePathGM; )
DEF_GM( return new ClippedCubicGM; )
DEF_GM( return new ClippedCubic2GM; )
