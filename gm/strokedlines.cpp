/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurMaskFilter.h"
#include "SkDashPathEffect.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPoint3.h"

constexpr int kNumColumns = 6;
constexpr int kNumRows = 8;
constexpr int kRadius = 40;  // radius of the snowflake
constexpr int kPad = 5;      // padding on both sides of the snowflake
constexpr int kNumSpokes = 6;
constexpr SkScalar kStrokeWidth = 5.0f;

static void draw_fins(SkCanvas* canvas, const SkPoint& offset, float angle, const SkPaint& paint) {
    SkScalar cos, sin;

    // first fin
    sin = SkScalarSinCos(angle + (SK_ScalarPI/4), &cos);
    sin *= kRadius / 2.0f;
    cos *= kRadius / 2.0f;

    SkPath p;
    p.moveTo(offset.fX, offset.fY);
    p.lineTo(offset.fX + cos, offset.fY + sin);
    canvas->drawPath(p, paint);

    // second fin
    sin = SkScalarSinCos(angle - (SK_ScalarPI/4), &cos);
    sin *= kRadius / 2.0f;
    cos *= kRadius / 2.0f;

    p.reset();
    p.moveTo(offset.fX, offset.fY);
    p.lineTo(offset.fX + cos, offset.fY + sin);
    canvas->drawPath(p, paint);
}

// draw a snowflake centered at the origin
static void draw_snowflake(SkCanvas* canvas, const SkPaint& paint) {

    canvas->clipRect(SkRect::MakeLTRB(-kRadius-kPad, -kRadius-kPad, kRadius+kPad, kRadius+kPad));

    SkScalar sin, cos, angle = 0.0f;
    for (int i = 0; i < kNumSpokes/2; ++i, angle += SK_ScalarPI/(kNumSpokes/2)) {
        sin = SkScalarSinCos(angle, &cos);
        sin *= kRadius;
        cos *= kRadius;

        // main spoke
        SkPath p;
        p.moveTo(-cos, -sin);
        p.lineTo(cos, sin);
        canvas->drawPath(p, paint);

        // fins on positive side
        const SkPoint posOffset = SkPoint::Make(0.5f * cos, 0.5f * sin);
        draw_fins(canvas, posOffset, angle, paint);

        // fins on negative side
        const SkPoint negOffset = SkPoint::Make(-0.5f * cos, -0.5f * sin);
        draw_fins(canvas, negOffset, angle+SK_ScalarPI, paint);
    }
}

static void draw_row(SkCanvas* canvas, const SkPaint& paint, const SkMatrix& localMatrix) {
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
            draw_snowflake(canvas, tmp);
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
    StrokedLinesGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFF1A65D7));
    }

protected:
    SkString onShortName() override {
        return SkString("strokedlines");
    }

    SkISize onISize() override {
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
            p.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                     SkShader::kClamp_TileMode, 0, nullptr));

            fPaints.push_back(p);
        }
        {
            // dashing
            SkScalar intervals[] = { kStrokeWidth, kStrokeWidth };
            int intervalCount = (int) SK_ARRAY_COUNT(intervals);
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
            m.preScale(3.0f, 3.0f);;

            SkPaint p;
            p.setShader(SkShader::MakeBitmapShader(bm,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode,
                                                   &m));
            fPaints.push_back(p);
        }
        {
            // blur
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setMaskFilter(SkBlurMaskFilter::Make(kOuter_SkBlurStyle, 3.0f,
                                                   SkBlurMaskFilter::kHighQuality_BlurFlag));
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

        SkASSERT(kNumRows == fPaints.count() + fMatrices.count());
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(0, kRadius+kPad);

        for (int i = 0; i < fPaints.count(); ++i) {
            int saveCount = canvas->save();
            draw_row(canvas, fPaints[i], SkMatrix::I());
            canvas->restoreToCount(saveCount);

            canvas->translate(0, 2*(kRadius+kPad));
        }

        for (int i = 0; i < fMatrices.count(); ++i) {
            int saveCount = canvas->save();
            draw_row(canvas, fPaints[0], fMatrices[i]);
            canvas->restoreToCount(saveCount);

            canvas->translate(0, 2*(kRadius+kPad));
        }
    }

private:
    SkTArray<SkPaint> fPaints;
    SkTArray<SkMatrix> fMatrices;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new StrokedLinesGM;)
}
