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
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"

namespace skiagm {

class DegenerateSegmentsGM : public GM {
    struct PathAndName {
        SkPath      fPath;
        const char* fName1;
        const char* fName2;
    };

    SkString getName() const override { return SkString("degeneratesegments"); }

    SkISize getISize() override { return {896, 930}; }

    typedef SkPoint (*AddSegmentFunc)(SkPathBuilder&, SkPoint&);

    // We need to use explicit commands here, instead of addPath, because we
    // do not want the moveTo that is added at the beginning of a path to
    // appear in the appended path.
    static SkPoint AddMove(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveClose(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddDegenLine(SkPathBuilder& path, SkPoint& startPt) {
        path.lineTo(startPt);
        return startPt;
    }

    static SkPoint AddMoveDegenLine(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.lineTo(moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveDegenLineClose(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.lineTo(moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddDegenQuad(SkPathBuilder& path, SkPoint& startPt) {
        path.quadTo(startPt, startPt);
        return startPt;
    }

    static SkPoint AddMoveDegenQuad(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.quadTo(moveToPt, moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveDegenQuadClose(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.quadTo(moveToPt, moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddDegenCubic(SkPathBuilder& path, SkPoint& startPt) {
        path.cubicTo(startPt, startPt, startPt);
        return startPt;
    }

    static SkPoint AddMoveDegenCubic(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.cubicTo(moveToPt, moveToPt, moveToPt);
        return moveToPt;
    }

    static SkPoint AddMoveDegenCubicClose(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        path.moveTo(moveToPt);
        path.cubicTo(moveToPt, moveToPt, moveToPt);
        path.close();
        return moveToPt;
    }

    static SkPoint AddClose(SkPathBuilder& path, SkPoint& startPt) {
        path.close();
        return startPt;
    }

    static SkPoint AddLine(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint endPt = startPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.lineTo(endPt);
        return endPt;
    }

    static SkPoint AddMoveLine(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.lineTo(endPt);
        return endPt;
    }

    static SkPoint AddMoveLineClose(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.lineTo(endPt);
        path.close();
        return endPt;
    }

    static SkPoint AddQuad(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint midPt = startPt + SkPoint::Make(20*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = startPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.quadTo(midPt, endPt);
        return endPt;
    }

    static SkPoint AddMoveQuad(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint midPt = moveToPt + SkPoint::Make(20*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.quadTo(midPt, endPt);
        return endPt;
    }

    static SkPoint AddMoveQuadClose(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint midPt = moveToPt + SkPoint::Make(20*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.quadTo(midPt, endPt);
        path.close();
        return endPt;
    }

    static SkPoint AddCubic(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint t1Pt = startPt + SkPoint::Make(15*SK_Scalar1, 5*SK_Scalar1);
        SkPoint t2Pt = startPt + SkPoint::Make(25*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = startPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.cubicTo(t1Pt, t2Pt, endPt);
        return endPt;
    }

    static SkPoint AddMoveCubic(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint t1Pt = moveToPt + SkPoint::Make(15*SK_Scalar1, 5*SK_Scalar1);
        SkPoint t2Pt = moveToPt + SkPoint::Make(25*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.cubicTo(t1Pt, t2Pt, endPt);
        return endPt;
    }

    static SkPoint AddMoveCubicClose(SkPathBuilder& path, SkPoint& startPt) {
        SkPoint moveToPt = startPt + SkPoint::Make(0, 10*SK_Scalar1);
        SkPoint t1Pt = moveToPt + SkPoint::Make(15*SK_Scalar1, 5*SK_Scalar1);
        SkPoint t2Pt = moveToPt + SkPoint::Make(25*SK_Scalar1, 5*SK_Scalar1);
        SkPoint endPt = moveToPt + SkPoint::Make(40*SK_Scalar1, 0);
        path.moveTo(moveToPt);
        path.cubicTo(t1Pt, t2Pt, endPt);
        path.close();
        return endPt;
    }

    void drawPath(SkPath path, SkCanvas* canvas, SkColor color,
                  const SkRect& clip, SkPaint::Cap cap, SkPaint::Join join,
                  SkPaint::Style style, SkPathFillType fill,
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

    void onDraw(SkCanvas* canvas) override {
        constexpr AddSegmentFunc gSegmentFunctions[] = {
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
        const char* gSegmentNames[] = {
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
            SkPathFillType fFill;
            const char*      fName;
        };
        constexpr FillAndName gFills[] = {
            {SkPathFillType::kWinding, "Winding"},
            {SkPathFillType::kEvenOdd, "Even / Odd"},
            {SkPathFillType::kInverseWinding, "Inverse Winding"},
            {SkPathFillType::kInverseEvenOdd, "Inverse Even / Odd"}
        };
        struct StyleAndName {
            SkPaint::Style fStyle;
            const char*    fName;
        };
        constexpr StyleAndName gStyles[] = {
            {SkPaint::kFill_Style, "Fill"},
            {SkPaint::kStroke_Style, "Stroke 10"},
            {SkPaint::kStrokeAndFill_Style, "Stroke 10 And Fill"}
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

        SkPaint titlePaint;
        titlePaint.setColor(SK_ColorBLACK);
        titlePaint.setAntiAlias(true);
        SkFont     font(ToolUtils::create_portable_typeface(), 15);
        const char title[] = "Random Paths Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, "
                             "with Stroke width 6";
        canvas->drawString(title, 20, 20, font, titlePaint);

        SkRandom rand;
        SkRect rect = SkRect::MakeWH(220*SK_Scalar1, 50*SK_Scalar1);
        canvas->save();
        canvas->translate(2*SK_Scalar1, 30 * SK_Scalar1); // The title
        canvas->save();
        unsigned numSegments = std::size(gSegmentFunctions);
        unsigned numCaps = std::size(gCaps);
        unsigned numStyles = std::size(gStyles);
        unsigned numFills = std::size(gFills);
        for (size_t row = 0; row < 6; ++row) {
            if (0 < row) {
                canvas->translate(0, rect.height() + 100*SK_Scalar1);
            }
            canvas->save();
            for (size_t column = 0; column < 4; ++column) {
                if (0 < column) {
                    canvas->translate(rect.width() + 4*SK_Scalar1, 0);
                }

                SkColor      color = ToolUtils::color_to_565(0xff007000);
                StyleAndName style = gStyles[(rand.nextU() >> 16) % numStyles];
                CapAndName cap = gCaps[(rand.nextU() >> 16) % numCaps];
                FillAndName fill = gFills[(rand.nextU() >> 16) % numFills];
                unsigned s1 = (rand.nextU() >> 16) % numSegments;
                unsigned s2 = (rand.nextU() >> 16) % numSegments;
                unsigned s3 = (rand.nextU() >> 16) % numSegments;
                unsigned s4 = (rand.nextU() >> 16) % numSegments;
                unsigned s5 = (rand.nextU() >> 16) % numSegments;
                SkPoint pt = SkPoint::Make(10*SK_Scalar1, 0);
                SkPathBuilder path;
                pt = gSegmentFunctions[s1](path, pt);
                pt = gSegmentFunctions[s2](path, pt);
                pt = gSegmentFunctions[s3](path, pt);
                pt = gSegmentFunctions[s4](path, pt);
                pt = gSegmentFunctions[s5](path, pt);

                this->drawPath(path.detach(), canvas, color, rect,
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
                font.setSize(10);
                canvas->drawString(style.fName, 0, rect.height() + 12, font, labelPaint);
                canvas->drawString(fill.fName, 0, rect.height() + 24, font, labelPaint);
                canvas->drawString(cap.fName, 0, rect.height() + 36, font, labelPaint);
                canvas->drawString(gSegmentNames[s1], 0, rect.height() + 48, font, labelPaint);
                canvas->drawString(gSegmentNames[s2], 0, rect.height() + 60, font, labelPaint);
                canvas->drawString(gSegmentNames[s3], 0, rect.height() + 72, font, labelPaint);
                canvas->drawString(gSegmentNames[s4], 0, rect.height() + 84, font, labelPaint);
                canvas->drawString(gSegmentNames[s5], 0, rect.height() + 96, font, labelPaint);
            }
            canvas->restore();
        }
        canvas->restore();
        canvas->restore();
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new DegenerateSegmentsGM; )

}  // namespace skiagm
