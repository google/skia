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

class DegenerateSegmentsGM : public GM {
public:
    DegenerateSegmentsGM() {}

protected:
    struct PathAndName {
        SkPath      fPath;
        const char* fName1;
        const char* fName2;
    };

    SkString onShortName() {
        return SkString("degeneratesegments");
    }

    SkISize onISize() { return SkISize::Make(896, 930); }

    typedef SkPoint (*AddSegmentFunc)(SkPath&, SkPoint&);

    // We need to use explicit commands here, instead of addPath, because we
    // do not want the moveTo that is added at the beginning of a path to
    // appear in the appended path.
    static SkPoint AddMove(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveClose(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddDegenLine(SkPath& path, SkPoint& startPt) {
        path.lineTo(startPt);
        return startPt;
    }

    static SkPoint AddMoveDegenLine(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.lineTo(moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveDegenLineClose(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.lineTo(moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddDegenQuad(SkPath& path, SkPoint& startPt) {
        path.quadTo(startPt, startPt);
        return startPt;
    }

    static SkPoint AddMoveDegenQuad(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.quadTo(moveToPt, moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveDegenQuadClose(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.quadTo(moveToPt, moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddDegenCubic(SkPath& path, SkPoint& startPt) {
        path.cubicTo(startPt, startPt, startPt);
        return startPt;
    }

    static SkPoint AddMoveDegenCubic(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.cubicTo(moveToPt, moveToPt, moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveDegenCubicClose(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.cubicTo(moveToPt, moveToPt, moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddClose(SkPath& path, SkPoint& startPt) {
        path.close();
        return startPt;
    }

    static SkPoint AddLine(SkPath& path, SkPoint& startPt) {
        SkPoint endPt = startPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.lineTo(endPt);
        return endPt;
    }

    static SkPoint AddMoveLine(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.lineTo(endPt);
        return endPt;
    }

    static SkPoint AddMoveLineClose(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.lineTo(endPt);
        path.close();
        return endPt;
    }

    static SkPoint AddQuad(SkPath& path, SkPoint& startPt) {
        SkPoint midPt = startPt + SkPoint::Make(20*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = startPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.quadTo(midPt, endPt);
        return endPt;
    }

    static SkPoint AddMoveQuad(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint midPt = moveToPt + SkPoint::Make(20*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.quadTo(midPt, endPt);
        return endPt;
    }

    static SkPoint AddMoveQuadClose(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint midPt = moveToPt + SkPoint::Make(20*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.quadTo(midPt, endPt);
        path.close();
        return endPt;
    }

    static SkPoint AddCubic(SkPath& path, SkPoint& startPt) {
        SkPoint t1Pt = startPt + SkPoint::Make(15*SK_Scalar1, 5*SK_Scalar1);
        SkPoint t2Pt = startPt + SkPoint::Make(25*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = startPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.cubicTo(t1Pt, t2Pt, endPt);
        return endPt;
    }

    static SkPoint AddMoveCubic(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint t1Pt = moveToPt + SkPoint::Make(15*SK_Scalar1, 5*SK_Scalar1);
        SkPoint t2Pt = moveToPt + SkPoint::Make(25*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.cubicTo(t1Pt, t2Pt, endPt);
        return endPt;
    }

    static SkPoint AddMoveCubicClose(SkPath& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint t1Pt = moveToPt + SkPoint::Make(15*SK_Scalar1, 5*SK_Scalar1);
        SkPoint t2Pt = moveToPt + SkPoint::Make(25*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.cubicTo(t1Pt, t2Pt, endPt);
        path.close();
        return endPt;
    }

    void drawPath(SkPath& path, SkCanvas* canvas, SkColor color,
                  const SkRect& clip, SkPaint::Cap cap, SkPaint::Join join,
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
    static const AddSegmentFunc gSegmentFunctions[] = {
        AddMove,
        AddMoveClose,
        AddDegenLine,
        AddMoveDegenLine,
        AddMoveDegenLineClose,
        AddDegenQuad,
        AddMoveDegenQuad,
        AddMoveDegenQuadClose,
        AddDegenCubic,
        AddMoveDegenCubic,
        AddMoveDegenCubicClose,
        AddClose,
        AddLine,
        AddMoveLine,
        AddMoveLineClose,
        AddQuad,
        AddMoveQuad,
        AddMoveQuadClose,
        AddCubic,
        AddMoveCubic,
        AddMoveCubicClose
    };
    static const char* gSegmentNames[] = {
        "Move",
        "MoveClose",
        "DegenLine",
        "MoveDegenLine",
        "MoveDegenLineClose",
        "DegenQuad",
        "MoveDegenQuad",
        "MoveDegenQuadClose",
        "DegenCubic",
        "MoveDegenCubic",
        "MoveDegenCubicClose",
        "Close",
        "Line",
        "MoveLine",
        "MoveLineClose",
        "Quad",
        "MoveQuad",
        "MoveQuadClose",
        "Cubic",
        "MoveCubic",
        "MoveCubicClose"
    };

        struct FillAndName {
            SkPath::FillType fFill;
            const char*      fName;
        };
        static const FillAndName gFills[] = {
            {SkPath::kWinding_FillType, "Winding"},
            {SkPath::kEvenOdd_FillType, "Even / Odd"},
            {SkPath::kInverseWinding_FillType, "Inverse Winding"},
            {SkPath::kInverseEvenOdd_FillType, "Inverse Even / Odd"}
        };
        struct StyleAndName {
            SkPaint::Style fStyle;
            const char*    fName;
        };
        static const StyleAndName gStyles[] = {
            {SkPaint::kFill_Style, "Fill"},
            {SkPaint::kStroke_Style, "Stroke 10"},
            {SkPaint::kStrokeAndFill_Style, "Stroke 10 And Fill"}
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

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&titlePaint);
        titlePaint.setTextSize(15 * SK_Scalar1);
        const char title[] = "Random Paths Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, "
                             "with Stroke width 6";
        canvas->drawText(title, strlen(title),
                            20 * SK_Scalar1,
                            20 * SK_Scalar1,
                            titlePaint);

        SkRandom rand;
        SkRect rect = SkRect::MakeWH(220*SK_Scalar1, 50*SK_Scalar1);
        canvas->save();
        canvas->translate(2*SK_Scalar1, 30 * SK_Scalar1); // The title
        canvas->save();
        unsigned numSegments = SK_ARRAY_COUNT(gSegmentFunctions);
        unsigned numCaps = SK_ARRAY_COUNT(gCaps);
        unsigned numStyles = SK_ARRAY_COUNT(gStyles);
        unsigned numFills = SK_ARRAY_COUNT(gFills);
        for (size_t row = 0; row < 6; ++row) {
            if (0 < row) {
                canvas->translate(0, rect.height() + 100*SK_Scalar1);
            }
            canvas->save();
            for (size_t column = 0; column < 4; ++column) {
                if (0 < column) {
                    canvas->translate(rect.width() + 4*SK_Scalar1, 0);
                }

                SkColor color = sk_tool_utils::color_to_565(0xff007000);
                StyleAndName style = gStyles[(rand.nextU() >> 16) % numStyles];
                CapAndName cap = gCaps[(rand.nextU() >> 16) % numCaps];
                FillAndName fill = gFills[(rand.nextU() >> 16) % numFills];
                SkPath path;
                unsigned s1 = (rand.nextU() >> 16) % numSegments;
                unsigned s2 = (rand.nextU() >> 16) % numSegments;
                unsigned s3 = (rand.nextU() >> 16) % numSegments;
                unsigned s4 = (rand.nextU() >> 16) % numSegments;
                unsigned s5 = (rand.nextU() >> 16) % numSegments;
                SkPoint pt = SkPoint::Make(10*SK_Scalar1, 0);
                pt = gSegmentFunctions[s1](path, pt);
                pt = gSegmentFunctions[s2](path, pt);
                pt = gSegmentFunctions[s3](path, pt);
                pt = gSegmentFunctions[s4](path, pt);
                pt = gSegmentFunctions[s5](path, pt);

                this->drawPath(path, canvas, color, rect,
                               cap.fCap, cap.fJoin, style.fStyle,
                               fill.fFill, SK_Scalar1*6);

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
                canvas->drawText(style.fName,
                                 strlen(style.fName),
                                 0, rect.height() + 12 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(fill.fName,
                                 strlen(fill.fName),
                                 0, rect.height() + 24 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(cap.fName,
                                 strlen(cap.fName),
                                 0, rect.height() + 36 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(gSegmentNames[s1],
                                 strlen(gSegmentNames[s1]),
                                 0, rect.height() + 48 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(gSegmentNames[s2],
                                 strlen(gSegmentNames[s2]),
                                 0, rect.height() + 60 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(gSegmentNames[s3],
                                 strlen(gSegmentNames[s3]),
                                 0, rect.height() + 72 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(gSegmentNames[s4],
                                 strlen(gSegmentNames[s4]),
                                 0, rect.height() + 84 * SK_Scalar1,
                                 labelPaint);
                canvas->drawText(gSegmentNames[s5],
                                 strlen(gSegmentNames[s5]),
                                 0, rect.height() + 96 * SK_Scalar1,
                                 labelPaint);
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

static GM* MyFactory(void*) { return new DegenerateSegmentsGM; }
static GMRegistry reg(MyFactory);

}
