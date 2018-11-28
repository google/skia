/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "GrClip.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrSurfaceContextPriv.h"
#include "SkGr.h"
#include "SkGradientShader.h"

static constexpr SkScalar kTileWidth = 40;
static constexpr SkScalar kTileHeight = 30;

static constexpr int kRowCount = 4;
static constexpr int kColCount = 3;

static void draw_text(SkCanvas* canvas, const char* text) {
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setTextSize(12.0f);
    paint.setAntiAlias(true);

    canvas->drawString(text, 0, 0, paint);
}

static void draw_gradient_tiles(GrRenderTargetContext* rtc, const SkMatrix& view,
                                bool adjustLocal) {
    GrRenderTargetContext::QuadSetEntry quads[kRowCount * kColCount];

    for (int i = 0; i < kRowCount; ++i) {
        for (int j = 0; j < kColCount; ++j) {
            SkRect tile = SkRect::MakeXYWH(j * kTileWidth, i * kTileHeight, kTileWidth, kTileHeight);

            int q = i * kColCount + j;
            quads[q].fRect = tile;
            quads[q].fColor = {1.f, 1.f, 1.f, 1.f};

            if (adjustLocal) {
                quads[q].fLocalMatrix.setTranslate(-tile.fLeft, -tile.fTop);
            } else {
                quads[q].fLocalMatrix.setIdentity();
            }

            quads[q].fAAFlags = GrQuadAAFlags::kNone;
            if (i == 0) {
                quads[q].fAAFlags |= GrQuadAAFlags::kTop;
            }
            if (i == kRowCount - 1) {
                quads[q].fAAFlags |= GrQuadAAFlags::kBottom;
            }
            if (j == 0) {
                quads[q].fAAFlags |= GrQuadAAFlags::kLeft;
            }
            if (j == kColCount - 1) {
                quads[q].fAAFlags |= GrQuadAAFlags::kRight;
            }
        }
    }

    // Make a shared gradient paint
    static constexpr SkPoint pts[] = { {0.f, 0.f}, {0.25f * kTileWidth, 0.25f * kTileHeight} };
    static constexpr SkColor colors[] = { SK_ColorBLUE, SK_ColorWHITE };

    auto gradient = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kMirror_TileMode);
    SkPaint paint;
    paint.setShader(gradient);
    GrPaint grPaint;
    SkPaintToGrPaint(rtc->surfPriv().getContext(), rtc->colorSpaceInfo(), paint, view, &grPaint);
    // And use private API to use GrFillRectOp
    rtc->drawQuadSet(GrNoClip(), std::move(grPaint), GrAA::kYes, view, quads, SK_ARRAY_COUNT(quads));
}

static void draw_color_tiles(GrRenderTargetContext* rtc, const SkMatrix& view, bool multicolor) {
    GrRenderTargetContext::QuadSetEntry quads[kRowCount * kColCount];

    for (int i = 0; i < kRowCount; ++i) {
        for (int j = 0; j < kColCount; ++j) {
            SkRect tile = SkRect::MakeXYWH(j * kTileWidth, i * kTileHeight, kTileWidth, kTileHeight);

            int q = i * kColCount + j;
            quads[q].fRect = tile;
            quads[q].fLocalMatrix.setIdentity();

            if (multicolor) {
                quads[q].fColor = {(i + 1.f) / kRowCount, (j + 1.f) / kColCount, .4f, 1.f};
            } else {
                quads[q].fColor = {.2f, .8f, .3f, 1.f};
            }

            quads[q].fAAFlags = GrQuadAAFlags::kNone;
            if (i == 0) {
                quads[q].fAAFlags |= GrQuadAAFlags::kTop;
            }
            if (i == kRowCount - 1) {
                quads[q].fAAFlags |= GrQuadAAFlags::kBottom;
            }
            if (j == 0) {
                quads[q].fAAFlags |= GrQuadAAFlags::kLeft;
            }
            if (j == kColCount - 1) {
                quads[q].fAAFlags |= GrQuadAAFlags::kRight;
            }
        }
    }

    GrPaint grPaint;
    // And use private API to use GrFillRectOp
    rtc->drawQuadSet(GrNoClip(), std::move(grPaint), GrAA::kYes, view, quads, SK_ARRAY_COUNT(quads));
}

static void draw_tile_boundaries(SkCanvas* canvas, const SkMatrix& local) {
    // Draw grid of red lines at interior tile boundaries.
    static constexpr SkScalar kLineOutset = 10.f;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0.f);
    for (int x = 1; x < kColCount; ++x) {
        SkPoint pts[] = {{x * kTileWidth, 0}, {x * kTileWidth, kRowCount * kTileHeight}};
        local.mapPoints(pts, 2);
        SkVector v = pts[1] - pts[0];
        v.setLength(v.length() + kLineOutset);
        canvas->drawLine(pts[1] - v, pts[0] + v, paint);
    }
    for (int y = 1; y < kRowCount; ++y) {
        SkPoint pts[] = {{0, y * kTileHeight}, {kTileWidth * kColCount, y * kTileHeight}};
        local.mapPoints(pts, 2);
        SkVector v = pts[1] - pts[0];
        v.setLength(v.length() + kLineOutset);
        canvas->drawLine(pts[1] - v, pts[0] + v, paint);
    }
}

// Tile renderers (column variation)
typedef void (*TileRenderer)(GrRenderTargetContext*, const SkMatrix&);
static TileRenderer kTileSets[] = {
    [](GrRenderTargetContext* rtc, const SkMatrix& view) { draw_gradient_tiles(rtc, view, true); },
    [](GrRenderTargetContext* rtc, const SkMatrix& view) { draw_gradient_tiles(rtc, view, false); },
    [](GrRenderTargetContext* rtc, const SkMatrix& view) { draw_color_tiles(rtc, view, false); },
    [](GrRenderTargetContext* rtc, const SkMatrix& view) { draw_color_tiles(rtc, view, true); },
};
static const char* kTileSetNames[] = { "Local", "Aligned", "Green", "Multicolor" };
static_assert(SK_ARRAY_COUNT(kTileSets) == SK_ARRAY_COUNT(kTileSetNames), "Count mismatch");

namespace skiagm {

class DrawQuadSetGM : public GM {
private:
    SkString onShortName() final { return SkString("draw_quad_set"); }
    SkISize onISize() override { return SkISize::Make(800, 800); }

    void onDraw(SkCanvas* canvas) override {
        GrContext* ctx = canvas->getGrContext();
        if (!ctx) {
            DrawGpuOnlyMessage(canvas);
            return;
        }
        GrRenderTargetContext* rtc = canvas->internal_private_accessTopLayerRenderTargetContext();
        SkASSERT(rtc);

        SkMatrix rowMatrices[5];
        // Identity
        rowMatrices[0].setIdentity();
        // Translate/scale
        rowMatrices[1].setTranslate(5.5f, 20.25f);
        rowMatrices[1].postScale(.9f, .7f);
        // Rotation
        rowMatrices[2].setRotate(20.0f);
        rowMatrices[2].preTranslate(15.f, -20.f);
        // Skew
        rowMatrices[3].setSkew(.5f, .25f);
        rowMatrices[3].preTranslate(-30.f, 0.f);
        // Perspective
        SkPoint src[4];
        SkRect::MakeWH(kColCount * kTileWidth, kRowCount * kTileHeight).toQuad(src);
        SkPoint dst[4] = {{0, 0},
                          {kColCount * kTileWidth + 10.f, 15.f},
                          {kColCount * kTileWidth - 28.f, kRowCount * kTileHeight + 40.f},
                          {25.f, kRowCount * kTileHeight - 15.f}};
        SkAssertResult(rowMatrices[4].setPolyToPoly(src, dst, 4));
        rowMatrices[4].preTranslate(0.f, +10.f);
        static const char* matrixNames[] = { "Identity", "T+S", "Rotate", "Skew", "Perspective" };
        static_assert(SK_ARRAY_COUNT(matrixNames) == SK_ARRAY_COUNT(rowMatrices), "Count mismatch");

        // Print a column header
        canvas->save();
        canvas->translate(110.f, 20.f);
        for (size_t j = 0; j < SK_ARRAY_COUNT(kTileSetNames); ++j) {
            draw_text(canvas, kTileSetNames[j]);
            canvas->translate(kColCount * kTileWidth + 30.f, 0.f);
        }
        canvas->restore();
        canvas->translate(0.f, 40.f);

        // Render all tile variations
        for (size_t i = 0; i < SK_ARRAY_COUNT(rowMatrices); ++i) {
            canvas->save();
            canvas->translate(10.f, 0.5f * kRowCount * kTileHeight);
            draw_text(canvas, matrixNames[i]);

            canvas->translate(100.f, -0.5f * kRowCount * kTileHeight);
            for (size_t j = 0; j < SK_ARRAY_COUNT(kTileSets); ++j) {
                canvas->save();
                draw_tile_boundaries(canvas, rowMatrices[i]);

                canvas->concat(rowMatrices[i]);
                kTileSets[j](rtc, canvas->getTotalMatrix());
                // Undo the local transformation
                canvas->restore();
                // And advance to the next column
                canvas->translate(kColCount * kTileWidth + 30.f, 0.f);
            }
            // Reset back to the left edge
            canvas->restore();
            // And advance to the next row
            canvas->translate(0.f, kRowCount * kTileHeight + 20.f);
        }
    }
};

DEF_GM(return new DrawQuadSetGM();)

} // namespace skiagm
