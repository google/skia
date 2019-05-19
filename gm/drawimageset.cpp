/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/ToolUtils.h"

#include <algorithm>
#include <initializer_list>
#include <utility>

// Makes a set of m x n tiled images to be drawn with SkCanvas::experimental_drawImageSetV1().
static void make_image_tiles(int tileW, int tileH, int m, int n, const SkColor colors[4],
                             SkCanvas::ImageSetEntry set[]) {
    const int w = tileW * m;
    const int h = tileH * n;
    auto surf = SkSurface::MakeRaster(
            SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    surf->getCanvas()->clear(SK_ColorLTGRAY);

    static constexpr SkScalar kStripeW = 10;
    static constexpr SkScalar kStripeSpacing = 30;
    SkPaint paint;

    SkPoint pts1[] = {{0.f, 0.f}, {(SkScalar)w, (SkScalar)h}};
    auto grad = SkGradientShader::MakeLinear(pts1, colors, nullptr, 2, SkTileMode::kClamp);
    paint.setShader(std::move(grad));
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(kStripeW);
    SkPoint stripePts[] = {{-w - kStripeW, -kStripeW}, {kStripeW, h + kStripeW}};
    while (stripePts[0].fX <= w) {
        surf->getCanvas()->drawPoints(SkCanvas::kLines_PointMode, 2, stripePts, paint);
        stripePts[0].fX += kStripeSpacing;
        stripePts[1].fX += kStripeSpacing;
    }

    SkPoint pts2[] = {{0.f, (SkScalar)h}, {(SkScalar)w, 0.f}};
    grad = SkGradientShader::MakeLinear(pts2, colors + 2, nullptr, 2, SkTileMode::kClamp);
    paint.setShader(std::move(grad));
    paint.setBlendMode(SkBlendMode::kMultiply);
    stripePts[0] = {-w - kStripeW, h + kStripeW};
    stripePts[1] = {kStripeW, -kStripeW};
    while (stripePts[0].fX <= w) {
        surf->getCanvas()->drawPoints(SkCanvas::kLines_PointMode, 2, stripePts, paint);
        stripePts[0].fX += kStripeSpacing;
        stripePts[1].fX += kStripeSpacing;
    }
    auto fullImage = surf->makeImageSnapshot();
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < m; ++x) {
            // Images will have 1 pixel of overlap at interior seams for filtering continuity.
            SkIRect subset = SkIRect::MakeXYWH(x * tileW - 1, y * tileH - 1, tileW + 2, tileH + 2);
            set[y * m + x].fAAFlags = SkCanvas::kNone_QuadAAFlags;
            if (x == 0) {
                subset.fLeft = 0;
                set[y * m + x].fAAFlags |= SkCanvas::kLeft_QuadAAFlag;
            }
            if (x == m - 1) {
                subset.fRight = w;
                set[y * m + x].fAAFlags |= SkCanvas::kRight_QuadAAFlag;
            }
            if (y == 0) {
                subset.fTop = 0;
                set[y * m + x].fAAFlags |= SkCanvas::kTop_QuadAAFlag;
            }
            if (y == n - 1) {
                subset.fBottom = h;
                set[y * m + x].fAAFlags |= SkCanvas::kBottom_QuadAAFlag;
            }
            set[y * m + x].fImage = fullImage->makeSubset(subset);
            set[y * m + x].fSrcRect =
                    SkRect::MakeXYWH(x == 0 ? 0 : 1, y == 0 ? 0 : 1, tileW, tileH);
            set[y * m + x].fDstRect = SkRect::MakeXYWH(x * tileW, y * tileH, tileW, tileH);
            set[y * m + x].fAlpha = 1.f;
            SkASSERT(set[y * m + x].fImage);
        }
    }
}

namespace skiagm {

class DrawImageSetGM : public GM {
private:
    SkString onShortName() final { return SkString("draw_image_set"); }
    SkISize onISize() override { return SkISize::Make(1000, 725); }
    void onOnceBeforeDraw() override {
        static constexpr SkColor kColors[] = {SK_ColorCYAN,    SK_ColorBLACK,
                                              SK_ColorMAGENTA, SK_ColorBLACK};
        make_image_tiles(kTileW, kTileH, kM, kN, kColors, fSet);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar d = SkVector{kM * kTileW, kN * kTileH}.length();
        SkMatrix matrices[4];
        // rotation
        matrices[0].setRotate(30);
        matrices[0].postTranslate(d / 3, 0);
        // perespective
        SkPoint src[4];
        SkRect::MakeWH(kM * kTileW, kN * kTileH).toQuad(src);
        SkPoint dst[4] = {{0, 0},
                          {kM * kTileW + 10.f, -5.f},
                          {kM * kTileW - 28.f, kN * kTileH + 40.f},
                          {45.f, kN * kTileH - 25.f}};
        SkAssertResult(matrices[1].setPolyToPoly(src, dst, 4));
        matrices[1].postTranslate(d, 50.f);
        // skew
        matrices[2].setRotate(-60.f);
        matrices[2].postSkew(0.5f, -1.15f);
        matrices[2].postScale(0.6f, 1.05f);
        matrices[2].postTranslate(d, 2.6f * d);
        // perspective + mirror in x.
        dst[1] = {-.25 * kM * kTileW, 0};
        dst[0] = {5.f / 4.f * kM * kTileW, 0};
        dst[3] = {2.f / 3.f * kM * kTileW, 1 / 2.f * kN * kTileH};
        dst[2] = {1.f / 3.f * kM * kTileW, 1 / 2.f * kN * kTileH - 0.1f * kTileH};
        SkAssertResult(matrices[3].setPolyToPoly(src, dst, 4));
        matrices[3].postTranslate(100.f, d);
        for (auto fm : {kNone_SkFilterQuality, kLow_SkFilterQuality}) {
            SkPaint setPaint;
            setPaint.setFilterQuality(fm);
            setPaint.setBlendMode(SkBlendMode::kSrcOver);

            for (size_t m = 0; m < SK_ARRAY_COUNT(matrices); ++m) {
                // Draw grid of red lines at interior tile boundaries.
                static constexpr SkScalar kLineOutset = 10.f;
                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(SK_ColorRED);
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeWidth(0.f);
                for (int x = 1; x < kM; ++x) {
                    SkPoint pts[] = {{x * kTileW, 0}, {x * kTileW, kN * kTileH}};
                    matrices[m].mapPoints(pts, 2);
                    SkVector v = pts[1] - pts[0];
                    v.setLength(v.length() + kLineOutset);
                    canvas->drawLine(pts[1] - v, pts[0] + v, paint);
                }
                for (int y = 1; y < kN; ++y) {
                    SkPoint pts[] = {{0, y * kTileH}, {kTileW * kM, y * kTileH}};
                    matrices[m].mapPoints(pts, 2);
                    SkVector v = pts[1] - pts[0];
                    v.setLength(v.length() + kLineOutset);
                    canvas->drawLine(pts[1] - v, pts[0] + v, paint);
                }
                canvas->save();
                canvas->concat(matrices[m]);
                canvas->experimental_DrawEdgeAAImageSet(fSet, kM * kN, nullptr, nullptr, &setPaint,
                                                        SkCanvas::kFast_SrcRectConstraint);
                canvas->restore();
            }
            // A more exotic case with an unusual blend mode, mixed aa flags set, and alpha,
            // subsets the image
            SkCanvas::ImageSetEntry entry;
            entry.fSrcRect = SkRect::MakeWH(kTileW, kTileH).makeInset(kTileW / 4.f, kTileH / 4.f);
            entry.fDstRect = SkRect::MakeWH(2 * kTileW, 2 * kTileH).makeOffset(d / 4, 2 * d);
            entry.fImage = fSet[0].fImage;
            entry.fAlpha = 0.7f;
            entry.fAAFlags = SkCanvas::kLeft_QuadAAFlag | SkCanvas::kTop_QuadAAFlag;
            canvas->save();
            canvas->rotate(3.f);

            setPaint.setBlendMode(SkBlendMode::kExclusion);
            canvas->experimental_DrawEdgeAAImageSet(&entry, 1, nullptr, nullptr, &setPaint,
                                                    SkCanvas::kFast_SrcRectConstraint);
            canvas->restore();
            canvas->translate(2 * d, 0);
        }
    }
    static constexpr int kM = 4;
    static constexpr int kN = 3;
    static constexpr SkScalar kTileW = 30;
    static constexpr SkScalar kTileH = 60;
    SkCanvas::ImageSetEntry fSet[kM * kN];
};

// This GM exercises rect-stays-rect type matrices to test that filtering and antialiasing are not
// incorrectly disabled.
class DrawImageSetRectToRectGM : public GM {
private:
    SkString onShortName() final { return SkString("draw_image_set_rect_to_rect"); }
    SkISize onISize() override { return SkISize::Make(1250, 850); }
    void onOnceBeforeDraw() override {
        static constexpr SkColor kColors[] = {SK_ColorBLUE, SK_ColorWHITE,
                                              SK_ColorRED,  SK_ColorWHITE};
        make_image_tiles(kTileW, kTileH, kM, kN, kColors, fSet);
    }

    void onDraw(SkCanvas* canvas) override {
        ToolUtils::draw_checkerboard(canvas, SK_ColorBLACK, SK_ColorWHITE, 50);
        static constexpr SkScalar kW = kM * kTileW;
        static constexpr SkScalar kH = kN * kTileH;
        SkMatrix matrices[5];
        // Identity
        matrices[0].reset();
        // 90 degree rotation
        matrices[1].setRotate(90, kW / 2.f, kH / 2.f);
        // Scaling
        matrices[2].setScale(2.f, 0.5f);
        // Mirror in x and y
        matrices[3].setScale(-1.f, -1.f);
        matrices[3].postTranslate(kW, kH);
        // Mirror in y, rotate, and scale.
        matrices[4].setScale(1.f, -1.f);
        matrices[4].postTranslate(0, kH);
        matrices[4].postRotate(90, kW / 2.f, kH / 2.f);
        matrices[4].postScale(2.f, 0.5f);

        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setBlendMode(SkBlendMode::kSrcOver);

        static constexpr SkScalar kTranslate = SkTMax(kW, kH) * 2.f + 10.f;
        canvas->translate(5.f, 5.f);
        canvas->save();
        for (SkScalar frac : {0.f, 0.5f}) {
            canvas->save();
            canvas->translate(frac, frac);
            for (size_t m = 0; m < SK_ARRAY_COUNT(matrices); ++m) {
                canvas->save();
                canvas->concat(matrices[m]);
                canvas->experimental_DrawEdgeAAImageSet(fSet, kM * kN, nullptr, nullptr, &paint,
                                                        SkCanvas::kFast_SrcRectConstraint);
                canvas->restore();
                canvas->translate(kTranslate, 0);
            }
            canvas->restore();
            canvas->restore();
            canvas->translate(0, kTranslate);
            canvas->save();
        }
        for (SkVector scale : {SkVector{2.f, 0.5f}, SkVector{0.5, 2.f}}) {
            SkCanvas::ImageSetEntry scaledSet[kM * kN];
            std::copy_n(fSet, kM * kN, scaledSet);
            for (int i = 0; i < kM * kN; ++i) {
                scaledSet[i].fDstRect.fLeft *= scale.fX;
                scaledSet[i].fDstRect.fTop *= scale.fY;
                scaledSet[i].fDstRect.fRight *= scale.fX;
                scaledSet[i].fDstRect.fBottom *= scale.fY;
                scaledSet[i].fAlpha = 0 == (i % 3) ? 0.4f : 1.f;
            }
            for (size_t m = 0; m < SK_ARRAY_COUNT(matrices); ++m) {
                canvas->save();
                canvas->concat(matrices[m]);
                canvas->experimental_DrawEdgeAAImageSet(scaledSet, kM * kN, nullptr, nullptr,
                                                        &paint, SkCanvas::kFast_SrcRectConstraint);
                canvas->restore();
                canvas->translate(kTranslate, 0);
            }
            canvas->restore();
            canvas->translate(0, kTranslate);
            canvas->save();
        }
    }
    static constexpr int kM = 2;
    static constexpr int kN = 2;
    static constexpr int kTileW = 40;
    static constexpr int kTileH = 50;
    SkCanvas::ImageSetEntry fSet[kM * kN];
};

DEF_GM(return new DrawImageSetGM();)
DEF_GM(return new DrawImageSetRectToRectGM();)

}  // namespace skiagm
