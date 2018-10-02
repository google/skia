/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkGradientShader.h"
#include "SkSurface.h"
#include "GrTypesPriv.h"

namespace skiagm {

class AAFlagsGM : public GM {
private:
    SkString onShortName() final { return SkString("aaflags"); }
    SkISize onISize() override { return SkISize::Make(1000, 1450); }
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
        matrices[1].setPolyToPoly(src, dst, 4);
        matrices[1].postTranslate(d, 50.f);
        // skew
        matrices[2].setRotate(-60.f);
        matrices[2].postSkew(0.5f, -1.35f);
        matrices[2].postScale(0.6f, 1.05f);
        matrices[2].postTranslate(d, 2.5f * d);
        // perspective + mirror in x.
        dst[1] = {0, 0};
        dst[0] = {3.f / 4.f * kM * kTileW, 0};
        dst[3] = {2.f / 3.f * kM * kTileW, 1 / 4.f * kN * kTileH};
        dst[2] = {1.f / 3.f * kM * kTileW, 1 / 4.f * kN * kTileH - 25.f};
        matrices[3].setPolyToPoly(src, dst, 4);
        matrices[3].postTranslate(100.f, d);
        SkCanvas::ImageSetEntry set[kM * kN];
        for (int x = 0; x < kM; ++x) {
            for (int y = 0; y < kN; ++y) {
                set[y * kM + x].fAAFlags = GrQuadAAFlags::kNone;
                if (x == 0) {
                    set[y * kM + x].fAAFlags |= GrQuadAAFlags::kLeft;
                }
                if (x == kM - 1) {
                    set[y * kM + x].fAAFlags |= GrQuadAAFlags::kRight;
                }
                if (y == 0) {
                    set[y * kM + x].fAAFlags |= GrQuadAAFlags::kTop;
                }
                if (y == kN - 1) {
                    set[y * kM + x].fAAFlags |= GrQuadAAFlags::kBottom;
                }
                set[y * kM + x].fSrcRect = SkRect::MakeWH(kTileW, kTileH);;
                set[y * kM + x].fDstRect = SkRect::MakeXYWH(x * kTileW, y * kTileH, kTileW, kTileH);
                set[y * kM + x].fImage = fImage[x][y].get();
            }
        }
        for (size_t m = 0; m < SK_ARRAY_COUNT(matrices); ++m) {
            canvas->save();
            canvas->concat(matrices[m]);
#if 0
            // Draw grid of green lines at interior tile boundaries.
            static constexpr SkScalar kLineOutset = 10.f;
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SK_ColorGREEN);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(8.f);
            for (int x = 1; x < kM; ++x) {
                canvas->drawLine(x * kTileW, -kLineOutset, x * kTileW, kN * kTileH + kLineOutset, paint);
            }
            for (int y = 1; y < kN; ++y) {
                canvas->drawLine(-kLineOutset, y * kTileH, kTileW * kM + kLineOutset, y * kTileH, paint);
            }
#endif
            canvas->experimentalDrawImageSet(set, kM * kN);
            canvas->restore();
        }
    }
    static constexpr int kM = 4;
    static constexpr int kN = 4;
    static constexpr SkScalar kTileW = 100;
    static constexpr SkScalar kTileH = 100;
    sk_sp<SkImage> fImage[kM][kN];
};

DEF_GM(return new AAFlagsGM();)

}  // namespace skiagm
