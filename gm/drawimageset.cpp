/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkGradientShader.h"
#include "SkSurface.h"

namespace skiagm {

class DrawImageSetGM : public GM {
private:
    SkString onShortName() final { return SkString("draw_image_set"); }
    SkISize onISize() override { return SkISize::Make(1000, 725); }
    void onOnceBeforeDraw() override {
        static constexpr SkScalar kW = SkIntToScalar(kTileW * kM);
        static constexpr SkScalar kH = SkIntToScalar(kTileH * kN);

        auto surf = SkSurface::MakeRaster(
                SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
        surf->getCanvas()->clear(SK_ColorLTGRAY);

        static constexpr SkScalar kStripeW = 10;
        static constexpr SkScalar kStripeSpacing = 30;
        SkPaint paint;

        static constexpr SkPoint pts1[] = {{0.f, 0.f}, {kW, kH}};
        static constexpr SkColor kColors1[] = {SK_ColorCYAN, SK_ColorBLACK};
        auto grad =
                SkGradientShader::MakeLinear(pts1, kColors1, nullptr, 2, SkShader::kClamp_TileMode);
        paint.setShader(std::move(grad));
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(kStripeW);
        SkPoint stripePts[] = {{-kW - kStripeW, -kStripeW}, {kStripeW, kH + kStripeW}};
        while (stripePts[0].fX <= kW) {
            surf->getCanvas()->drawPoints(SkCanvas::kLines_PointMode, 2, stripePts, paint);
            stripePts[0].fX += kStripeSpacing;
            stripePts[1].fX += kStripeSpacing;
        }

        static constexpr SkPoint pts2[] = {{0.f, kH}, {kW, 0.f}};
        static constexpr SkColor kColors2[] = {SK_ColorMAGENTA, SK_ColorBLACK};
        grad = SkGradientShader::MakeLinear(pts2, kColors2, nullptr, 2, SkShader::kClamp_TileMode);
        paint.setShader(std::move(grad));
        paint.setBlendMode(SkBlendMode::kMultiply);
        stripePts[0] = {-kW - kStripeW, kH + kStripeW};
        stripePts[1] = {kStripeW, -kStripeW};
        while (stripePts[0].fX <= kW) {
            surf->getCanvas()->drawPoints(SkCanvas::kLines_PointMode, 2, stripePts, paint);
            stripePts[0].fX += kStripeSpacing;
            stripePts[1].fX += kStripeSpacing;
        }

        auto img = surf->makeImageSnapshot();
        for (int y = 0; y < kN; ++y) {
            for (int x = 0; x < kM; ++x) {
                fImage[x][y] =
                        img->makeSubset(SkIRect::MakeXYWH(x * kTileW, y * kTileH, kTileW, kTileH));
            }
        }
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
        SkCanvas::ImageSetEntry set[kM * kN];
        for (int x = 0; x < kM; ++x) {
            for (int y = 0; y < kN; ++y) {
                set[y * kM + x].fAAFlags = SkCanvas::kNone_QuadAAFlags;
                if (x == 0) {
                    set[y * kM + x].fAAFlags |= SkCanvas::kLeft_QuadAAFlag;
                }
                if (x == kM - 1) {
                    set[y * kM + x].fAAFlags |= SkCanvas::kRight_QuadAAFlag;
                }
                if (y == 0) {
                    set[y * kM + x].fAAFlags |= SkCanvas::kTop_QuadAAFlag;
                }
                if (y == kN - 1) {
                    set[y * kM + x].fAAFlags |= SkCanvas::kBottom_QuadAAFlag;
                }
                set[y * kM + x].fSrcRect = SkRect::MakeWH(kTileW, kTileH);
                set[y * kM + x].fDstRect = SkRect::MakeXYWH(x * kTileW, y * kTileH, kTileW, kTileH);
                set[y * kM + x].fImage = fImage[x][y];
            }
        }
        for (auto fm : {kNone_SkFilterQuality, kLow_SkFilterQuality}) {
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
                canvas->experimental_DrawImageSetV0(set, kM * kN, 1.f, fm, SkBlendMode::kSrcOver);
                canvas->restore();
            }
            // A more exotic case with an unusual blend mode, all aa flags set, and alpha, and
            // subsets the image
            SkCanvas::ImageSetEntry entry;
            entry.fSrcRect = SkRect::MakeWH(kTileW, kTileH).makeInset(kTileW / 4.f, kTileH / 4.f);
            entry.fDstRect = SkRect::MakeWH(2 * kTileW, 2 * kTileH).makeOffset(d / 4, 2 * d);
            entry.fImage = fImage[0][0];
            entry.fAAFlags = SkCanvas::kAll_QuadAAFlags;
            canvas->save();
            canvas->rotate(3.f);
            canvas->experimental_DrawImageSetV0(&entry, 1, 0.7f, fm, SkBlendMode::kLuminosity);
            canvas->restore();
            canvas->translate(2 * d, 0);
        }
    }
    static constexpr int kM = 4;
    static constexpr int kN = 4;
    static constexpr SkScalar kTileW = 50;
    static constexpr SkScalar kTileH = 50;
    sk_sp<SkImage> fImage[kM][kN];
};

DEF_GM(return new DrawImageSetGM();)

}  // namespace skiagm
