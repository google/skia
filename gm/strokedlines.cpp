/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "tools/ToolUtils.h"

#include <initializer_list>

using namespace skia_private;

constexpr int kNumColumns = 6;
constexpr int kNumRows = 8;
constexpr int kRadius = 40;  // radius of the snowflake
constexpr int kPad = 5;      // padding on both sides of the snowflake
constexpr int kNumSpokes = 6;
constexpr SkScalar kStrokeWidth = 5.0f;

static void draw_line(SkCanvas* canvas, const SkPoint& p0, const SkPoint& p1,
                      const SkPaint& paint, bool useDrawPath) {
    if (useDrawPath) {
        canvas->drawPath(SkPath::Line(p0, p1), paint);
    } else {
        canvas->drawLine(p0, p1, paint);
    }
}

static void draw_fins(SkCanvas* canvas, const SkPoint& offset, float angle, const SkPaint& paint,
                      bool useDrawPath) {
    SkScalar cos, sin;

    // first fin
    sin = SkScalarSin(angle + (SK_ScalarPI/4));
    cos = SkScalarCos(angle + (SK_ScalarPI/4));
    sin *= kRadius / 2.0f;
    cos *= kRadius / 2.0f;

    draw_line(canvas, offset, offset + SkPoint{cos, sin}, paint, useDrawPath);

    // second fin
    sin = SkScalarSin(angle - (SK_ScalarPI/4));
    cos = SkScalarCos(angle - (SK_ScalarPI/4));
    sin *= kRadius / 2.0f;
    cos *= kRadius / 2.0f;

    draw_line(canvas, offset, offset + SkPoint{cos, sin}, paint, useDrawPath);
}

// draw a snowflake centered at the origin
static void draw_snowflake(SkCanvas* canvas, const SkPaint& paint, bool useDrawPath) {

    canvas->clipRect(SkRect::MakeLTRB(-kRadius-kPad, -kRadius-kPad, kRadius+kPad, kRadius+kPad));

    SkScalar sin, cos, angle = 0.0f;
    for (int i = 0; i < kNumSpokes/2; ++i, angle += SK_ScalarPI/(kNumSpokes/2)) {
        sin = SkScalarSin(angle);
        cos = SkScalarCos(angle);
        sin *= kRadius;
        cos *= kRadius;

        // main spoke
        draw_line(canvas, {-cos, -sin}, {cos, sin}, paint, useDrawPath);

        // fins on positive side
        const SkPoint posOffset = SkPoint::Make(0.5f * cos, 0.5f * sin);
        draw_fins(canvas, posOffset, angle, paint, useDrawPath);

        // fins on negative side
        const SkPoint negOffset = SkPoint::Make(-0.5f * cos, -0.5f * sin);
        draw_fins(canvas, negOffset, angle+SK_ScalarPI, paint, useDrawPath);
    }
}

static void draw_row(SkCanvas* canvas, const SkPaint& paint, const SkMatrix& localMatrix,
                     bool useDrawPath) {
    canvas->translate(kRadius+kPad, 0.0f);

    for (auto cap : { SkPaint::kButt_Cap, SkPaint::kRound_Cap, SkPaint::kSquare_Cap }) {
        for (auto isAA : { true, false }) {
            SkPaint tmp(paint);
            tmp.setStrokeWidth(kStrokeWidth);
            tmp.setStyle(SkPaint::kStroke_Style);
            tmp.setStrokeCap(cap);
            tmp.setAntiAlias(isAA);

            int saveCount = canvas->save();
            canvas->concat(localMatrix);
            draw_snowflake(canvas, tmp, useDrawPath);
            canvas->restoreToCount(saveCount);

            canvas->translate(2*(kRadius+kPad), 0.0f);
        }
    }
}

namespace skiagm {

// This GM exercises the special case of a stroked lines.
// Various shaders are applied to ensure the coordinate spaces work out right.
class StrokedLinesGM : public GM {
public:
    StrokedLinesGM(bool useDrawPath) : fUseDrawPath(useDrawPath) {
        this->setBGColor(ToolUtils::color_to_565(0xFF1A65D7));
    }

protected:
    SkString getName() const override {
        // To preserve history, useDrawPath==true has no suffix.
        SkString name{"strokedlines"};
        if (!fUseDrawPath) {
            name.append("_drawPoints");
        }
        return name;
    }

    SkISize getISize() override {
        return SkISize::Make(kNumColumns * (2*kRadius+2*kPad), kNumRows * (2*kRadius+2*kPad));
    }

    void onOnceBeforeDraw() override {
        // paints
        {
            // basic white
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            fPaints.push_back(p);
        }
        {
            // gradient
            SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
            SkPoint pts[] = { {-kRadius-kPad, -kRadius-kPad }, { kRadius+kPad, kRadius+kPad } };

            SkPaint p;
            p.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));

            fPaints.push_back(p);
        }
        {
            // dashing
            SkScalar intervals[] = { kStrokeWidth, kStrokeWidth };
            int intervalCount = (int) std::size(intervals);
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setPathEffect(SkDashPathEffect::Make(intervals, intervalCount, kStrokeWidth));

            fPaints.push_back(p);
        }
        {
            // Bitmap shader
            SkBitmap bm;
            bm.allocN32Pixels(2, 2);
            *bm.getAddr32(0, 0) = *bm.getAddr32(1, 1) = 0xFFFFFFFF;
            *bm.getAddr32(1, 0) = *bm.getAddr32(0, 1) = 0x0;

            SkMatrix m;
            m.setRotate(12.0f);
            m.preScale(3.0f, 3.0f);

            SkPaint p;
            p.setShader(bm.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                      SkSamplingOptions(), m));
            fPaints.push_back(p);
        }
        {
            // blur
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 3.0f));
            fPaints.push_back(p);
        }

        // matrices
        {
            // rotation
            SkMatrix m;
            m.setRotate(12.0f);

            fMatrices.push_back(m);
        }
        {
            // skew
            SkMatrix m;
            m.setSkew(0.3f, 0.5f);

            fMatrices.push_back(m);
        }
        {
            // perspective
            SkMatrix m;
            m.reset();
            m.setPerspX(-SK_Scalar1 / 300);
            m.setPerspY(SK_Scalar1 / 300);

            fMatrices.push_back(m);
        }

        SkASSERT(kNumRows == fPaints.size() + fMatrices.size());
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(0, kRadius+kPad);

        for (int i = 0; i < fPaints.size(); ++i) {
            int saveCount = canvas->save();
            draw_row(canvas, fPaints[i], SkMatrix::I(), fUseDrawPath);
            canvas->restoreToCount(saveCount);

            canvas->translate(0, 2*(kRadius+kPad));
        }

        for (int i = 0; i < fMatrices.size(); ++i) {
            int saveCount = canvas->save();
            draw_row(canvas, fPaints[0], fMatrices[i], fUseDrawPath);
            canvas->restoreToCount(saveCount);

            canvas->translate(0, 2*(kRadius+kPad));
        }
    }

private:
    TArray<SkPaint> fPaints;
    TArray<SkMatrix> fMatrices;

    const bool fUseDrawPath;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new StrokedLinesGM(true);)
DEF_GM(return new StrokedLinesGM(false);)

//////////////////////////////////////////////////////////////////////////////

static constexpr float kStrokeWidth = 20.f;

static void draw_path(SkCanvas* canvas, const SkPoint& p0, const SkPoint& p1, SkPaint::Cap cap) {
    // Add a gradient *not* aligned with the line's points to show local coords are tracked properly
    constexpr SkRect kRect {-kStrokeWidth, -kStrokeWidth, 2*kStrokeWidth, 4*kStrokeWidth};
    constexpr SkPoint kPts[] {{kRect.fLeft, kRect.fTop}, {kRect.fRight, kRect.fBottom}};
    constexpr SkColor kColors[] {SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE};
    constexpr SkScalar kStops[] {0.f, 0.75f, 1.f};
    sk_sp<SkShader> shader = SkGradientShader::MakeLinear(kPts, kColors, kStops, 3,
                                                          SkTileMode::kClamp);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    paint.setShader(std::move(shader));
    paint.setStrokeWidth(kStrokeWidth);
    paint.setStrokeCap(cap);
    canvas->drawLine(p0, p1, paint);

    // Show outline and control points
    SkPath fillPath;
    SkPath path = SkPath::Line(p0, p1);
    skpathutils::FillPathWithPaint(path, paint, &fillPath);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setShader(nullptr);
    paint.setColor(SK_ColorRED);
    canvas->drawPath(fillPath, paint);

    paint.setStrokeWidth(3);
    paint.setStrokeCap(SkPaint::kSquare_Cap);
    int n = fillPath.countPoints();
    AutoTArray<SkPoint> points(n);
    fillPath.getPoints(points.get(), n);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, n, points.get(), paint);
}

DEF_SIMPLE_GM(strokedline_caps, canvas, 1400, 740) {
    canvas->translate(kStrokeWidth*3/2, kStrokeWidth*3/2);

    constexpr SkPaint::Cap kCaps[] = {
        SkPaint::kSquare_Cap, SkPaint::kButt_Cap, SkPaint::kRound_Cap
    };

    constexpr float kLengths[] = {
        4*kStrokeWidth, kStrokeWidth, kStrokeWidth/2, kStrokeWidth/4
    };

    for (size_t i = 0; i < std::size(kCaps); ++i) {
        SkAutoCanvasRestore acr(canvas, true);

        auto drawLine = [&](float x0, float y0, float x1, float y1) {
            draw_path(canvas, {x0, y0}, {x1, y1}, kCaps[i]);
            canvas->translate(std::max(x0, x1) + 2 * kStrokeWidth, 0);
        };

        for (size_t j = 0; j < std::size(kLengths); ++j) {
            float l = kLengths[j];

            drawLine(0.f, 0.f, l, l);
            drawLine(l, l, 0.f, 0.f);
            drawLine(l/2, 0, l/2, l);
            drawLine(0, l/2, l, l/2);
        }

        drawLine(kStrokeWidth/2, kStrokeWidth/2, kStrokeWidth/2, kStrokeWidth/2);

        acr.restore();
        canvas->translate(0, kLengths[0] + 2 * kStrokeWidth);
    }
}

}  // namespace skiagm
