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
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

namespace skiagm {

class QuadPathGM : public GM {
public:
    QuadPathGM() {}

protected:
    SkString getName() const override { return SkString("quadpath"); }

    SkISize getISize() override { return SkISize::Make(1240, 390); }

    void drawPath(SkPath& path,SkCanvas* canvas,SkColor color,
                  const SkRect& clip,SkPaint::Cap cap, SkPaint::Join join,
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
        path.fPath.moveTo(25*SK_Scalar1, 10*SK_Scalar1);
        path.fPath.quadTo(50*SK_Scalar1, 20*SK_Scalar1,
                          75*SK_Scalar1, 10*SK_Scalar1);
        path.fName = "moveTo-quad";

        SkPaint titlePaint;
        SkFont  font(ToolUtils::DefaultPortableTypeface(), 15);
        SkFont  labelFont(ToolUtils::DefaultPortableTypeface(), 10);

        const char title[] = "Quad Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, with stroke width 10";
        canvas->drawString(title, 20.0f, 20.0f, font, titlePaint);

        SkRandom rand;
        SkRect rect = SkRect::MakeWH(100*SK_Scalar1, 30*SK_Scalar1);
        canvas->save();
        canvas->translate(10 * SK_Scalar1, 30 * SK_Scalar1);
        canvas->save();
        for (size_t cap = 0; cap < std::size(gCaps); ++cap) {
            if (0 < cap) {
                canvas->translate((rect.width() + 40 * SK_Scalar1) * std::size(gStyles), 0);
            }
            canvas->save();
            for (size_t fill = 0; fill < std::size(gFills); ++fill) {
                if (0 < fill) {
                    canvas->translate(0, rect.height() + 40 * SK_Scalar1);
                }
                canvas->save();
                for (size_t style = 0; style < std::size(gStyles); ++style) {
                    if (0 < style) {
                        canvas->translate(rect.width() + 40 * SK_Scalar1, 0);
                    }

                    SkColor color = ToolUtils::color_to_565(0xff007000);
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
                    canvas->drawString(gStyles[style].fName, 0, rect.height() + 12.0f,
                                       labelFont, labelPaint);
                    canvas->drawString(gFills[fill].fName, 0, rect.height() + 24.0f,
                                       labelFont, labelPaint);
                    canvas->drawString(gCaps[cap].fName, 0, rect.height() + 36.0f,
                                       labelFont, labelPaint);
                }
                canvas->restore();
            }
            canvas->restore();
        }
        canvas->restore();
        canvas->restore();
    }

private:
    using INHERITED = GM;
};

class QuadClosePathGM : public GM {
public:
    QuadClosePathGM() {}

protected:
    SkString getName() const override { return SkString("quadclosepath"); }

    SkISize getISize() override { return SkISize::Make(1240, 390); }

    void drawPath(SkPath& path,SkCanvas* canvas,SkColor color,
                  const SkRect& clip,SkPaint::Cap cap, SkPaint::Join join,
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
        path.fPath.moveTo(25*SK_Scalar1, 10*SK_Scalar1);
        path.fPath.quadTo(50*SK_Scalar1, 20*SK_Scalar1,
                          75*SK_Scalar1, 10*SK_Scalar1);
        path.fPath.close();
        path.fName = "moveTo-quad-close";

        SkPaint titlePaint;
        SkFont     font(ToolUtils::DefaultPortableTypeface(), 15);
        SkFont     labelFont(ToolUtils::DefaultPortableTypeface(), 10);
        const char title[] = "Quad Closed Drawn Into Rectangle Clips With "
                             "Indicated Style, Fill and Linecaps, with stroke width 10";
        canvas->drawString(title, 20.0f, 20.0f, font, titlePaint);

        SkRandom rand;
        SkRect rect = SkRect::MakeWH(100*SK_Scalar1, 30*SK_Scalar1);
        canvas->save();
        canvas->translate(10 * SK_Scalar1, 30 * SK_Scalar1);
        canvas->save();
        for (size_t cap = 0; cap < std::size(gCaps); ++cap) {
            if (0 < cap) {
                canvas->translate((rect.width() + 40 * SK_Scalar1) * std::size(gStyles), 0);
            }
            canvas->save();
            for (size_t fill = 0; fill < std::size(gFills); ++fill) {
                if (0 < fill) {
                    canvas->translate(0, rect.height() + 40 * SK_Scalar1);
                }
                canvas->save();
                for (size_t style = 0; style < std::size(gStyles); ++style) {
                    if (0 < style) {
                        canvas->translate(rect.width() + 40 * SK_Scalar1, 0);
                    }

                    SkColor color = ToolUtils::color_to_565(0xff007000);
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
                    canvas->drawString(gStyles[style].fName, 0, rect.height() + 12.0f,
                                       labelFont, labelPaint);
                    canvas->drawString(gFills[fill].fName, 0, rect.height() + 24.0f,
                                       labelFont, labelPaint);
                    canvas->drawString(gCaps[cap].fName, 0, rect.height() + 36.0f,
                                       labelFont, labelPaint);
                }
                canvas->restore();
            }
            canvas->restore();
        }
        canvas->restore();
        canvas->restore();
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new QuadPathGM; )

DEF_GM( return new QuadClosePathGM; )

}  // namespace skiagm
