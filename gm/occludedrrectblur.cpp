/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurMaskFilter.h"
#include "SkRRect.h"

static SkRect offset_center_to(const SkIRect& src, SkScalar x, SkScalar y) {
    SkScalar halfW = 0.5f * src.width();
    SkScalar halfH = 0.5f * src.height();

    return SkRect::MakeLTRB(x - halfW, y - halfH, x + halfW, y + halfH);
}

static void draw_rrect(SkCanvas* canvas, const SkRRect& rr, const SkRRect& occRR) {
    const SkScalar kBlurSigma = 5.0f;

    SkRect occRect;
    SkColor strokeColor;

    {
        SkRect occRect1 = sk_tool_utils::compute_central_occluder(occRR);
        SkRect occRect2 = sk_tool_utils::compute_widest_occluder(occRR);
        SkRect occRect3 = sk_tool_utils::compute_tallest_occluder(occRR);

        SkScalar area1 = occRect1.width() * occRect1.height();
        SkScalar area2 = occRect2.width() * occRect2.height();
        SkScalar area3 = occRect3.width() * occRect3.height();

        if (area1 >= area2 && area1 >= area3) {
            strokeColor = SK_ColorRED;
            occRect = occRect1;
        } else if (area2 > area3) {
            strokeColor = SK_ColorYELLOW;
            occRect = occRect2;
        } else {
            strokeColor = SK_ColorCYAN;
            occRect = occRect3;
        }
    }

    // draw the blur
    SkPaint paint;
    paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, kBlurSigma, occRect));
    canvas->drawRRect(rr, paint);

    // draw the stroked geometry of the full occluder
    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setColor(SK_ColorBLUE);
    canvas->drawRRect(occRR, stroke);

    // draw the geometry of the occluding rect
    stroke.setColor(strokeColor);
    canvas->drawRect(occRect, stroke);
}

static void draw_45(SkCanvas* canvas, SkRRect::Corner corner,
                    SkScalar dist, const SkPoint& center) {
    SkRRect::Corner left = SkRRect::kUpperLeft_Corner, right = SkRRect::kUpperLeft_Corner;
    SkVector dir = { 0, 0 };

    constexpr SkScalar kSize = 64.0f / SK_ScalarSqrt2;

    switch (corner) {
    case SkRRect::kUpperLeft_Corner:
        left = SkRRect::kUpperRight_Corner;
        right = SkRRect::kLowerLeft_Corner;

        dir.set(-SK_ScalarRoot2Over2, -SK_ScalarRoot2Over2);
        break;
    case SkRRect::kUpperRight_Corner:
        left = SkRRect::kUpperLeft_Corner;
        right = SkRRect::kLowerRight_Corner;
        dir.set(SK_ScalarRoot2Over2, -SK_ScalarRoot2Over2);
        break;
    case SkRRect::kLowerRight_Corner:
        left = SkRRect::kLowerLeft_Corner;
        right = SkRRect::kUpperRight_Corner;
        dir.set(SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);
        break;
    case SkRRect::kLowerLeft_Corner:
        left = SkRRect::kLowerRight_Corner;
        right = SkRRect::kUpperLeft_Corner;
        dir.set(-SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);
        break;
    default:
        SkFAIL("Invalid shape.");
    }

    SkRect r = SkRect::MakeWH(kSize, kSize);
    // UL, UR, LR, LL
    SkVector radii[4] = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };
    radii[left] = SkVector::Make(kSize, kSize);
    radii[right] = SkVector::Make(kSize, kSize);
    SkRRect rr;
    rr.setRectRadii(
            offset_center_to(r.roundOut(), center.fX + dist*dir.fX, center.fY + dist*dir.fY),
            radii);

    SkRRect occRR;
    dist -= 10.0f;
    occRR.setRectRadii(
            offset_center_to(r.roundOut(), center.fX + dist*dir.fX, center.fY + dist*dir.fY),
            radii);

    draw_rrect(canvas, rr, occRR);
}

static void draw_45_simple(SkCanvas* canvas, const SkVector& v,
                           SkScalar dist, const SkPoint& center) {
    SkIRect r = SkIRect::MakeWH(64, 64);
    SkRRect rr = SkRRect::MakeRectXY(
                            offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY),
                            8, 8);

    dist -= 10.0f;
    SkRRect occRR = SkRRect::MakeRectXY(
                            offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY),
                            8, 8);

    draw_rrect(canvas, rr, occRR);
}

static void draw_90(SkCanvas* canvas, const SkVector& v, SkScalar dist, const SkPoint& center) {
    constexpr int kWidth = 25;

    SkIRect r;
    if (fabs(v.fX) < fabs(v.fY)) {
        r = SkIRect::MakeWH(kWidth, 64);
    } else {
        r = SkIRect::MakeWH(64, kWidth);
    }
    SkRRect rr = SkRRect::MakeOval(
                            offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY));

    dist -= 10.0f;
    SkRRect occRR = SkRRect::MakeOval(
                            offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY));

    draw_rrect(canvas, rr, occRR);
}

static void draw_90_simple(SkCanvas* canvas, const SkVector& v,
                           SkScalar dist, const SkPoint& center) {
    constexpr int kLength = 128;
    // The width needs to be larger than 2*3*blurRadii+2*cornerRadius for the analytic
    // RRect blur to kick in
    constexpr int kWidth = 47;

    SkIRect r;
    if (fabs(v.fX) < fabs(v.fY)) {
        r = SkIRect::MakeWH(kLength, kWidth);
    } else {
        r = SkIRect::MakeWH(kWidth, kLength);
    }
    SkRRect rr = SkRRect::MakeRectXY(
                            offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY),
                            8, 8);

    dist -= 10.0f;
    SkRRect occRR = SkRRect::MakeRectXY(
                            offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY),
                            8, 8);

    draw_rrect(canvas, rr, occRR);
}

static void draw_30_60(SkCanvas* canvas, SkRRect::Corner corner, const SkVector& v,
                       SkScalar dist, const SkPoint& center) {
    SkRRect::Corner left = SkRRect::kUpperLeft_Corner, right = SkRRect::kUpperLeft_Corner;

    constexpr int kLength = 64;
    constexpr int kWidth = 30;

    switch (corner) {
    case SkRRect::kUpperLeft_Corner:
        left = SkRRect::kUpperRight_Corner;
        right = SkRRect::kLowerLeft_Corner;
        break;
    case SkRRect::kUpperRight_Corner:
        left = SkRRect::kUpperLeft_Corner;
        right = SkRRect::kLowerRight_Corner;
        break;
    case SkRRect::kLowerRight_Corner:
        left = SkRRect::kLowerLeft_Corner;
        right = SkRRect::kUpperRight_Corner;
        break;
    case SkRRect::kLowerLeft_Corner:
        left = SkRRect::kLowerRight_Corner;
        right = SkRRect::kUpperLeft_Corner;
        break;
    default:
        SkFAIL("Invalid shape.");
    }

    SkIRect r;
    if (fabs(v.fX) < fabs(v.fY)) {
        r = SkIRect::MakeWH(kLength, kWidth);
    } else {
        r = SkIRect::MakeWH(kWidth, kLength);
    }
    // UL, UR, LR, LL
    SkVector radii[4] = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };
    radii[left] = SkVector::Make(SkIntToScalar(kWidth), SkIntToScalar(kWidth));
    radii[right] = SkVector::Make(SkIntToScalar(kWidth), SkIntToScalar(kWidth));
    SkRRect rr;
    rr.setRectRadii(offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY), radii);

    dist -= 10.0f;
    SkRRect occRR;
    occRR.setRectRadii(offset_center_to(r, center.fX + dist*v.fX, center.fY + dist*v.fY), radii);
    draw_rrect(canvas, rr, occRR);
}

namespace skiagm {

class OccludedRRectBlurGM : public GM {
public:
    OccludedRRectBlurGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("occludedrrectblur");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkPoint center = SkPoint::Make(kWidth/2, kHeight/2);

        // outer-most big RR
        {
            SkIRect r = SkIRect::MakeWH(420, 420);
            SkRRect rr = SkRRect::MakeRectXY(offset_center_to(r, center.fX, center.fY), 64, 64);
            draw_rrect(canvas, rr, rr);

#if 1
            // TODO: remove this. Until we actually start skipping the middle draw we need this
            // to provide contrast
            SkPaint temp;
            temp.setColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
            r.inset(32, 32);
            canvas->drawRect(offset_center_to(r, center.fX, center.fY), temp);
#endif
        }

        // center circle
        {
            SkIRect r = SkIRect::MakeWH(32, 32);
            SkRRect rr = SkRRect::MakeOval(offset_center_to(r, center.fX, center.fY));
            draw_rrect(canvas, rr, rr);
        }

        draw_45(canvas, SkRRect::kUpperLeft_Corner, 64, center);
        draw_45(canvas, SkRRect::kUpperRight_Corner, 64, center);
        draw_45(canvas, SkRRect::kLowerRight_Corner, 64, center);
        draw_45(canvas, SkRRect::kLowerLeft_Corner, 64, center);

        draw_90(canvas, SkVector::Make(-1.0f, 0.0f), 64, center);
        draw_90(canvas, SkVector::Make(0.0f, -1.0f), 64, center);
        draw_90(canvas, SkVector::Make(1.0f, 0.0f), 64, center);
        draw_90(canvas, SkVector::Make(0.0f, 1.0f), 64, center);

        constexpr SkScalar kRoot3Over2 = 0.8660254037844386f;

        draw_30_60(canvas, SkRRect::kLowerLeft_Corner,
                   SkVector::Make(0.5f, kRoot3Over2), 120, center);
        draw_30_60(canvas, SkRRect::kUpperRight_Corner,
                   SkVector::Make(kRoot3Over2, 0.5f), 120, center);

        draw_30_60(canvas, SkRRect::kUpperLeft_Corner,
                   SkVector::Make(-0.5f, kRoot3Over2), 120, center);
        draw_30_60(canvas, SkRRect::kLowerRight_Corner,
                   SkVector::Make(-kRoot3Over2, 0.5f), 120, center);

        draw_30_60(canvas, SkRRect::kLowerLeft_Corner,
                   SkVector::Make(-0.5f, -kRoot3Over2), 120, center);
        draw_30_60(canvas, SkRRect::kUpperRight_Corner,
                   SkVector::Make(-kRoot3Over2, -0.5f), 120, center);

        draw_30_60(canvas, SkRRect::kUpperLeft_Corner,
                   SkVector::Make(0.5f, -kRoot3Over2), 120, center);
        draw_30_60(canvas, SkRRect::kLowerRight_Corner,
                   SkVector::Make(kRoot3Over2, -0.5f), 120, center);

        draw_45_simple(canvas, SkVector::Make(-SK_ScalarRoot2Over2, -SK_ScalarRoot2Over2),
                       210, center);
        draw_45_simple(canvas, SkVector::Make(SK_ScalarRoot2Over2, -SK_ScalarRoot2Over2),
                       210, center);
        draw_45_simple(canvas, SkVector::Make(SK_ScalarRoot2Over2, SK_ScalarRoot2Over2),
                       210, center);
        draw_45_simple(canvas, SkVector::Make(-SK_ScalarRoot2Over2, SK_ScalarRoot2Over2),
                       210, center);

        draw_90_simple(canvas, SkVector::Make(-1.0f, 0.0f), 160, center);
        draw_90_simple(canvas, SkVector::Make(0.0f, -1.0f), 160, center);
        draw_90_simple(canvas, SkVector::Make(1.0f, 0.0f), 160, center);
        draw_90_simple(canvas, SkVector::Make(0.0f, 1.0f), 160, center);
    }

private:
    static constexpr int kWidth = 440;
    static constexpr int kHeight = 440;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new OccludedRRectBlurGM;)
}
