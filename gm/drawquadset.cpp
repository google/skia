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
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tools/ToolUtils.h"

#include <utility>

static constexpr SkScalar kTileWidth = 40;
static constexpr SkScalar kTileHeight = 30;

static constexpr int kRowCount = 4;
static constexpr int kColCount = 3;

static void draw_text(SkCanvas* canvas, const char* text) {
    SkFont font(ToolUtils::create_portable_typeface(), 12);
    canvas->drawString(text, 0, 0, font, SkPaint());
}

static void draw_gradient_tiles(SkCanvas* canvas, bool alignGradients) {
    // Always draw the same gradient
    static constexpr SkPoint pts[] = { {0.f, 0.f}, {0.25f * kTileWidth, 0.25f * kTileHeight} };
    static constexpr SkColor colors[] = { SK_ColorBLUE, SK_ColorWHITE };

    auto sdc = skgpu::ganesh::TopDeviceSurfaceDrawContext(canvas);

    auto rContext = canvas->recordingContext();

    auto gradient = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kMirror);
    SkPaint paint;
    paint.setShader(gradient);

    for (int i = 0; i < kRowCount; ++i) {
        for (int j = 0; j < kColCount; ++j) {
            SkRect tile = SkRect::MakeWH(kTileWidth, kTileHeight);
            if (alignGradients) {
                tile.offset(j * kTileWidth, i * kTileHeight);
            } else {
                canvas->save();
                canvas->translate(j * kTileWidth, i * kTileHeight);
            }

            unsigned aa = SkCanvas::kNone_QuadAAFlags;
            if (i == 0) {
                aa |= SkCanvas::kTop_QuadAAFlag;
            }
            if (i == kRowCount - 1) {
                aa |= SkCanvas::kBottom_QuadAAFlag;
            }
            if (j == 0) {
                aa |= SkCanvas::kLeft_QuadAAFlag;
            }
            if (j == kColCount - 1) {
                aa |= SkCanvas::kRight_QuadAAFlag;
            }

            if (sdc) {
                // Use non-public API to leverage general GrPaint capabilities
                const SkMatrix& view = canvas->getTotalMatrix();
                SkSurfaceProps props;
                GrPaint grPaint;
                SkPaintToGrPaint(rContext, sdc->colorInfo(), paint, view, props, &grPaint);
                sdc->fillRectWithEdgeAA(nullptr, std::move(grPaint),
                                        static_cast<GrQuadAAFlags>(aa), view, tile);
            } else {
                // Fallback to solid color on raster backend since the public API only has color
                SkColor color = alignGradients ? SK_ColorBLUE
                                               : (i * kColCount + j) % 2 == 0 ? SK_ColorBLUE
                                                                              : SK_ColorWHITE;
                canvas->experimental_DrawEdgeAAQuad(
                        tile, nullptr, static_cast<SkCanvas::QuadAAFlags>(aa), color,
                        SkBlendMode::kSrcOver);
            }

            if (!alignGradients) {
                // Pop off the matrix translation when drawing unaligned
                canvas->restore();
            }
        }
    }
}

static void draw_color_tiles(SkCanvas* canvas, bool multicolor) {
    for (int i = 0; i < kRowCount; ++i) {
        for (int j = 0; j < kColCount; ++j) {
            SkRect tile = SkRect::MakeXYWH(j * kTileWidth, i * kTileHeight, kTileWidth, kTileHeight);

            SkColor4f color;
            if (multicolor) {
                color = {(i + 1.f) / kRowCount, (j + 1.f) / kColCount, .4f, 1.f};
            } else {
                color = {.2f, .8f, .3f, 1.f};
            }

            unsigned aa = SkCanvas::kNone_QuadAAFlags;
            if (i == 0) {
                aa |= SkCanvas::kTop_QuadAAFlag;
            }
            if (i == kRowCount - 1) {
                aa |= SkCanvas::kBottom_QuadAAFlag;
            }
            if (j == 0) {
                aa |= SkCanvas::kLeft_QuadAAFlag;
            }
            if (j == kColCount - 1) {
                aa |= SkCanvas::kRight_QuadAAFlag;
            }

            canvas->experimental_DrawEdgeAAQuad(
                    tile, nullptr, static_cast<SkCanvas::QuadAAFlags>(aa), color.toSkColor(),
                    SkBlendMode::kSrcOver);
        }
    }
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
typedef void (*TileRenderer)(SkCanvas*);
static TileRenderer kTileSets[] = {
    [](SkCanvas* canvas) { draw_gradient_tiles(canvas, /* aligned */ false); },
    [](SkCanvas* canvas) { draw_gradient_tiles(canvas, /* aligned */ true); },
    [](SkCanvas* canvas) { draw_color_tiles(canvas, /* multicolor */ false); },
    [](SkCanvas* canvas) { draw_color_tiles(canvas, /* multicolor */true); },
};
static const char* kTileSetNames[] = { "Local", "Aligned", "Green", "Multicolor" };
static_assert(std::size(kTileSets) == std::size(kTileSetNames), "Count mismatch");

namespace skiagm {

class DrawQuadSetGM : public GM {
private:
    SkString getName() const override { return SkString("draw_quad_set"); }
    SkISize getISize() override { return SkISize::Make(800, 800); }

    void onDraw(SkCanvas* canvas) override {
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
        static_assert(std::size(matrixNames) == std::size(rowMatrices), "Count mismatch");

        // Print a column header
        canvas->save();
        canvas->translate(110.f, 20.f);
        for (size_t j = 0; j < std::size(kTileSetNames); ++j) {
            draw_text(canvas, kTileSetNames[j]);
            canvas->translate(kColCount * kTileWidth + 30.f, 0.f);
        }
        canvas->restore();
        canvas->translate(0.f, 40.f);

        // Render all tile variations
        for (size_t i = 0; i < std::size(rowMatrices); ++i) {
            canvas->save();
            canvas->translate(10.f, 0.5f * kRowCount * kTileHeight);
            draw_text(canvas, matrixNames[i]);

            canvas->translate(100.f, -0.5f * kRowCount * kTileHeight);
            for (size_t j = 0; j < std::size(kTileSets); ++j) {
                canvas->save();
                draw_tile_boundaries(canvas, rowMatrices[i]);

                canvas->concat(rowMatrices[i]);
                kTileSets[j](canvas);
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
