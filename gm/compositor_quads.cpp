/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrClip.h"
#include "GrContext.h"
#include "GrRect.h"
#include "GrRenderTargetContextPriv.h"
#include "Resources.h"
#include "SkColorMatrixFilter.h"
#include "SkFont.h"
#include "SkGpuDevice.h"
#include "SkGradientShader.h"
#include "SkImage_Base.h"
#include "SkLineClipper.h"
#include "SkMorphologyImageFilter.h"
#include "SkPaintFilterCanvas.h"
#include "SkShaderMaskFilter.h"

#include <array>

// This GM mimics the draw calls used by complex compositors that focus on drawing rectangles
// and quadrilaterals with per-edge AA, with complex images, effects, and seamless tiling.
// It will be updated to reflect the patterns seen in Chromium's SkiaRenderer. It is currently
// restricted to adding draw ops directly in Ganesh since there is no fully-specified public API.

static constexpr SkScalar kTileWidth = 40;
static constexpr SkScalar kTileHeight = 30;

static constexpr int kRowCount = 4;
static constexpr int kColCount = 3;

// To mimic Chromium's BSP clipping strategy, a set of three lines formed by triangle edges
// of the below points are used to clip against the regular tile grid. The tile grid occupies
// a 120 x 120 rectangle (40px * 3 cols by 30px * 4 rows).
static constexpr SkPoint kClipP1 = {1.75f * kTileWidth, 0.8f * kTileHeight};
static constexpr SkPoint kClipP2 = {0.6f * kTileWidth, 2.f * kTileHeight};
static constexpr SkPoint kClipP3 = {2.9f * kTileWidth, 3.5f * kTileHeight};

///////////////////////////////////////////////////////////////////////////////////////////////
// Utilities for operating on lines and tiles
///////////////////////////////////////////////////////////////////////////////////////////////

// p0 and p1 form a segment contained the tile grid, so extends them by a large enough margin
// that the output points stored in 'line' are outside the tile grid (thus effectively infinite).
static void clipping_line_segment(const SkPoint& p0, const SkPoint& p1, SkPoint line[2]) {
    SkVector v = p1 - p0;
    // 10f was chosen as a balance between large enough to scale the currently set clip
    // points outside of the tile grid, but small enough to preserve precision.
    line[0] = p0 - v * 10.f;
    line[1] = p1 + v * 10.f;
}

// Returns true if line segment (p0-p1) intersects with line segment (l0-l1); if true is returned,
// the intersection point is stored in 'intersect'.
static bool intersect_line_segments(const SkPoint& p0, const SkPoint& p1,
                                    const SkPoint& l0, const SkPoint& l1, SkPoint* intersect) {
    static constexpr SkScalar kHorizontalTolerance = 0.01f; // Pretty conservative

    // Use doubles for accuracy, since the clipping strategy used below can create T
    // junctions, and lower precision could artificially create gaps
    double pY = (double) p1.fY - (double) p0.fY;
    double pX = (double) p1.fX - (double) p0.fX;
    double lY = (double) l1.fY - (double) l0.fY;
    double lX = (double) l1.fX - (double) l0.fX;
    double plY = (double) p0.fY - (double) l0.fY;
    double plX = (double) p0.fX - (double) l0.fX;
    if (SkScalarNearlyZero(pY, kHorizontalTolerance)) {
        if (SkScalarNearlyZero(lY, kHorizontalTolerance)) {
            // Two horizontal lines
            return false;
        } else {
            // Recalculate but swap p and l
            return intersect_line_segments(l0, l1, p0, p1, intersect);
        }
    }

    // Up to now, the line segments do not form an invalid intersection
    double lNumerator = plX * pY - plY * pX;
    double lDenom = lX * pY - lY * pX;
    if (SkScalarNearlyZero(lDenom)) {
        // Parallel or identical
        return false;
    }

    // Calculate alphaL that provides the intersection point along (l0-l1), e.g. l0+alphaL*(l1-l0)
    double alphaL = lNumerator / lDenom;
    if (alphaL < 0.0 || alphaL > 1.0) {
        // Outside of the l segment
        return false;
    }

    // Calculate alphaP from the valid alphaL (since it could be outside p segment)
    // double alphaP = (alphaL * l.fY - pl.fY) / p.fY;
    double alphaP = (alphaL * lY - plY) / pY;
    if (alphaP < 0.0 || alphaP > 1.0) {
        // Outside of p segment
        return false;
    }

    // Is valid, so calculate the actual intersection point
    *intersect = l1 * SkScalar(alphaL) + l0 * SkScalar(1.0 - alphaL);
    return true;
}

// Draw a line through the two points, outset by a fixed length in screen space
static void draw_outset_line(SkCanvas* canvas, const SkMatrix& local, const SkPoint pts[2],
                             const SkPaint& paint) {
    static constexpr SkScalar kLineOutset = 10.f;
    SkPoint mapped[2];
    local.mapPoints(mapped, pts, 2);
    SkVector v = mapped[1] - mapped[0];
    v.setLength(v.length() + kLineOutset);
    canvas->drawLine(mapped[1] - v, mapped[0] + v, paint);
}

// Draw grid of red lines at interior tile boundaries.
static void draw_tile_boundaries(SkCanvas* canvas, const SkMatrix& local) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0.f);
    for (int x = 1; x < kColCount; ++x) {
        SkPoint pts[] = {{x * kTileWidth, 0}, {x * kTileWidth, kRowCount * kTileHeight}};
        draw_outset_line(canvas, local, pts, paint);
    }
    for (int y = 1; y < kRowCount; ++y) {
        SkPoint pts[] = {{0, y * kTileHeight}, {kTileWidth * kColCount, y * kTileHeight}};
        draw_outset_line(canvas, local, pts, paint);
    }
}

// Draw the arbitrary clipping/split boundaries that intersect the tile grid as green lines
static void draw_clipping_boundaries(SkCanvas* canvas, const SkMatrix& local) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorGREEN);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0.f);

    // Clip the "infinite" line segments to a rectangular region outside the tile grid
    SkRect border = SkRect::MakeWH(kTileWidth * kColCount, kTileHeight * kRowCount);

    // Draw p1 to p2
    SkPoint line[2];
    SkPoint clippedLine[2];
    clipping_line_segment(kClipP1, kClipP2, line);
    SkAssertResult(SkLineClipper::IntersectLine(line, border, clippedLine));
    draw_outset_line(canvas, local, clippedLine, paint);

    // Draw p2 to p3
    clipping_line_segment(kClipP2, kClipP3, line);
    SkAssertResult(SkLineClipper::IntersectLine(line, border, clippedLine));
    draw_outset_line(canvas, local, clippedLine, paint);

    // Draw p3 to p1
    clipping_line_segment(kClipP3, kClipP1, line);
    SkAssertResult(SkLineClipper::IntersectLine(line, border, clippedLine));
    draw_outset_line(canvas, local, clippedLine, paint);
}

static void draw_text(SkCanvas* canvas, const char* text) {
    canvas->drawString(text, 0, 0, SkFont(nullptr, 12), SkPaint());
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Abstraction for rendering a possibly clipped tile, that can apply different effects to mimic
// the Chromium quad types, and a generic GM template to arrange renderers x transforms in a grid
/////////////////////////////////////////////////////////////////////////////////////////////////

class ClipTileRenderer : public SkRefCntBase {
public:
    virtual ~ClipTileRenderer() {}

    // Draw the base rect, possibly clipped by 'clip' if that is not null. The edges to antialias
    // are specified in 'edgeAA' (to make manipulation easier than an unsigned bitfield). 'tileID'
    // represents the location of rect within the tile grid, 'quadID' is the unique ID of the clip
    // region within the tile (reset for each tile).
    //
    // The edgeAA order matches that of clip, so it refers to top, right, bottom, left.
    // Return draw count
    virtual int drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4],
                          const bool edgeAA[4], int tileID, int quadID) = 0;

    virtual void drawBanner(SkCanvas* canvas) = 0;

    // Return draw count
    virtual int drawTiles(SkCanvas* canvas, GrContext* context, GrRenderTargetContext* rtc) {
        // TODO (michaelludwig) - once the quad APIs are in SkCanvas, drop these
        // cached fields, which drawTile() needs
        fContext = context;

        SkBaseDevice* device = canvas->getDevice();
        if (device->context()) {
            // Pretty sure it's a SkGpuDevice since this is a run as a GPU GM, unfortunately we
            // don't have RTTI for dynamic_cast
            fDevice = static_cast<SkGpuDevice*>(device);
        } else {
            // This is either Viewer passing an SkPaintFilterCanvas (in which case we could get
            // it's wrapped proxy to get the SkGpuDevice), or it is an SkColorSpaceXformCanvas
            // that doesn't expose any access to original device. Unfortunately without RTTI
            // there is no way to distinguish these cases so just avoid drawing. Once the API
            // is in SkCanvas, this is a non-issue. Code that works for viewer can be uncommented
            // to test locally (and must add ClipTileRenderer as a friend in SkPaintFilterCanvas)
            // SkPaintFilterCanvas* filteredCanvas = static_cast<SkPaintFilterCanvas*>(canvas);
            // fDevice = static_cast<SkGpuDevice*>(filteredCanvas->proxy()->getDevice());
            return 0;
        }

        // All three lines in a list
        SkPoint lines[6];
        clipping_line_segment(kClipP1, kClipP2, lines);
        clipping_line_segment(kClipP2, kClipP3, lines + 2);
        clipping_line_segment(kClipP3, kClipP1, lines + 4);

        bool edgeAA[4];
        int tileID = 0;
        int drawCount = 0;
        for (int i = 0; i < kRowCount; ++i) {
            for (int j = 0; j < kColCount; ++j) {
                // The unclipped tile geometry
                SkRect tile = SkRect::MakeXYWH(j * kTileWidth, i * kTileHeight,
                                               kTileWidth, kTileHeight);
                // Base edge AA flags if there are no clips; clipped lines will only turn off edges
                edgeAA[0] = i == 0;             // Top
                edgeAA[1] = j == kColCount - 1; // Right
                edgeAA[2] = i == kRowCount - 1; // Bottom
                edgeAA[3] = j == 0;             // Left

                // Now clip against the 3 lines formed by kClipPx and split into general purpose
                // quads as needed.
                int quadCount = 0;
                drawCount += this->clipTile(canvas, tileID, tile, nullptr, edgeAA, lines, 3,
                                            &quadCount);
                tileID++;
            }
        }

        return drawCount;
    }

protected:
    // Remembered for convenience in drawTile, set by drawTiles()
    GrContext* fContext;
    SkGpuDevice* fDevice;

    SkCanvas::QuadAAFlags maskToFlags(const bool edgeAA[4]) const {
        unsigned flags = (edgeAA[0] * SkCanvas::kTop_QuadAAFlag) |
                         (edgeAA[1] * SkCanvas::kRight_QuadAAFlag) |
                         (edgeAA[2] * SkCanvas::kBottom_QuadAAFlag) |
                         (edgeAA[3] * SkCanvas::kLeft_QuadAAFlag);
        return static_cast<SkCanvas::QuadAAFlags>(flags);
    }

    // Recursively splits the quadrilateral against the segments stored in 'lines', which must be
    // 2 * lineCount long. Increments 'quadCount' for each split quadrilateral, and invokes the
    // drawTile at leaves.
    int clipTile(SkCanvas* canvas, int tileID, const SkRect& baseRect, const SkPoint quad[4],
                  const bool edgeAA[4], const SkPoint lines[], int lineCount, int* quadCount) {
        if (lineCount == 0) {
            // No lines, so end recursion by drawing the tile. If the tile was never split then
            // 'quad' remains null so that drawTile() can differentiate how it should draw.
            int draws = this->drawTile(canvas, baseRect, quad, edgeAA, tileID, *quadCount);
            *quadCount = *quadCount + 1;
            return draws;
        }

        static constexpr int kTL = 0; // Top-left point index in points array
        static constexpr int kTR = 1; // Top-right point index in points array
        static constexpr int kBR = 2; // Bottom-right point index in points array
        static constexpr int kBL = 3; // Bottom-left point index in points array
        static constexpr int kS0 = 4; // First split point index in points array
        static constexpr int kS1 = 5; // Second split point index in points array

        SkPoint points[6];
        if (quad) {
            // Copy the original 4 points into set of points to consider
            for (int i = 0; i < 4; ++i) {
                points[i] = quad[i];
            }
        } else {
            //  Haven't been split yet, so fill in based on the rect
            baseRect.toQuad(points);
        }

        // Consider the first line against the 4 quad edges in tile, which should have 0,1, or 2
        // intersection points since the tile is convex.
        int splitIndices[2]; // Edge that was intersected
        int intersectionCount = 0;
        for (int i = 0; i < 4; ++i) {
            SkPoint intersect;
            if (intersect_line_segments(points[i], points[i == 3 ? 0 : i + 1],
                                        lines[0], lines[1], &intersect)) {
                // If the intersected point is the same as the last found intersection, the line
                // runs through a vertex, so don't double count it
                bool duplicate = false;
                for (int j = 0; j < intersectionCount; ++j) {
                    if (SkScalarNearlyZero((intersect - points[kS0 + j]).length())) {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate) {
                    points[kS0 + intersectionCount] = intersect;
                    splitIndices[intersectionCount] = i;
                    intersectionCount++;
                }
            }
        }

        if (intersectionCount < 2) {
            // Either the first line never intersected the quad (count == 0), or it intersected at a
            // single vertex without going through quad area (count == 1), so check next line
            return this->clipTile(
                    canvas, tileID, baseRect, quad, edgeAA, lines + 2, lineCount - 1, quadCount);
        }

        SkASSERT(intersectionCount == 2);
        // Split the tile points into 2+ sub quads and recurse to the next lines, which may or may
        // not further split the tile. Since the configurations are relatively simple, the possible
        // splits are hardcoded below; subtile quad orderings are such that the sub tiles remain in
        // clockwise order and match expected edges for QuadAAFlags. subtile indices refer to the
        // 6-element 'points' array.
        SkSTArray<3, std::array<int, 4>> subtiles;
        int s2 = -1; // Index of an original vertex chosen for a artificial split
        if (splitIndices[1] - splitIndices[0] == 2) {
            // Opposite edges, so the split trivially forms 2 sub quads
            if (splitIndices[0] == 0) {
                subtiles.push_back({{kTL, kS0, kS1, kBL}});
                subtiles.push_back({{kS0, kTR, kBR, kS1}});
            } else {
                subtiles.push_back({{kTL, kTR, kS0, kS1}});
                subtiles.push_back({{kS1, kS0, kBR, kBL}});
            }
        } else {
            // Adjacent edges, which makes for a more complicated split, since it forms a degenerate
            // quad (triangle) and a pentagon that must be artificially split. The pentagon is split
            // using one of the original vertices (remembered in 's2'), which adds an additional
            // degenerate quad, but ensures there are no T-junctions.
            switch(splitIndices[0]) {
                case 0:
                    // Could be connected to edge 1 or edge 3
                    if (splitIndices[1] == 1) {
                        s2 = kBL;
                        subtiles.push_back({{kS0, kTR, kS1, kS1}}); // degenerate
                        subtiles.push_back({{kTL, kS0, kBL, kBL}}); // degenerate
                        subtiles.push_back({{kS0, kS1, kBR, kBL}});
                    } else {
                        SkASSERT(splitIndices[1] == 3);
                        s2 = kBR;
                        subtiles.push_back({{kTL, kS0, kS1, kS1}}); // degenerate
                        subtiles.push_back({{kS1, kS1, kBR, kBL}}); // degenerate
                        subtiles.push_back({{kS0, kTR, kBR, kS1}});
                    }
                    break;
                case 1:
                    // Edge 0 handled above, should only be connected to edge 2
                    SkASSERT(splitIndices[1] == 2);
                    s2 = kTL;
                    subtiles.push_back({{kS0, kS0, kBR, kS1}}); // degenerate
                    subtiles.push_back({{kTL, kTR, kS0, kS0}}); // degenerate
                    subtiles.push_back({{kTL, kS0, kS1, kBL}});
                    break;
                case 2:
                    // Edge 1 handled above, should only be connected to edge 3
                    SkASSERT(splitIndices[1] == 3);
                    s2 = kTR;
                    subtiles.push_back({{kS1, kS1, kS0, kBL}}); // degenerate
                    subtiles.push_back({{kTR, kTR, kBR, kS0}}); // degenerate
                    subtiles.push_back({{kTL, kTR, kS0, kS1}});
                    break;
                case 3:
                    // Fall through, an adjacent edge split that hits edge 3 should have first found
                    // been found with edge 0 or edge 2 for the other end
                default:
                    SkASSERT(false);
                    return 0;
            }
        }

        SkPoint sub[4];
        bool subAA[4];
        int draws = 0;
        for (int i = 0; i < subtiles.count(); ++i) {
            // Fill in the quad points and update edge AA rules for new interior edges
            for (int j = 0; j < 4; ++j) {
                int p = subtiles[i][j];
                sub[j] = points[p];

                int np = j == 3 ? subtiles[i][0] : subtiles[i][j + 1];
                // The "new" edges are the edges that connect between the two split points or
                // between a split point and the chosen s2 point. Otherwise the edge remains aligned
                // with the original shape, so should preserve the AA setting.
                if ((p == s2 || p >= kS0) && (np == s2 || np >= kS0)) {
                    // New edge
                    subAA[j] = false;
                } else {
                    // The subtiles indices were arranged so that their edge ordering was still top,
                    // right, bottom, left so 'j' can be used to access edgeAA
                    subAA[j] = edgeAA[j];
                }
            }

            // Split the sub quad with the next line
            draws += this->clipTile(canvas, tileID, baseRect, sub, subAA, lines + 2, lineCount - 1,
                                    quadCount);
        }
        return draws;
    }
};

static constexpr int kMatrixCount = 5;

class CompositorGM : public skiagm::GpuGM {
public:
    CompositorGM(const char* name, sk_sp<ClipTileRenderer> renderer)
            : fName(name) {
        fRenderers.push_back(std::move(renderer));
    }
    CompositorGM(const char* name, const SkTArray<sk_sp<ClipTileRenderer>> renderers)
            : fRenderers(renderers)
            , fName(name) {}

protected:
    SkISize onISize() override {
        // The GM draws a grid of renderers (rows) x transforms (col). Within each cell, the
        // renderer draws the transformed tile grid, which is approximately
        // (kColCount*kTileWidth, kRowCount*kTileHeight), although it has additional line
        // visualizations and can be transformed outside of those rectangular bounds (i.e. persp),
        // so pad the cell dimensions to be conservative. Must also account for the banner text.
        static constexpr SkScalar kCellWidth = 1.3f * kColCount * kTileWidth;
        static constexpr SkScalar kCellHeight = 1.3f * kRowCount * kTileHeight;
        return SkISize::Make(SkScalarRoundToInt(kCellWidth * kMatrixCount + 175.f),
                             SkScalarRoundToInt(kCellHeight * fRenderers.count() + 75.f));
    }

    SkString onShortName() override {
        SkString fullName;
        fullName.appendf("compositor_quads_%s", fName.c_str());
        return fullName;
    }

    void onOnceBeforeDraw() override {
        this->configureMatrices();
    }

    void onDraw(GrContext* ctx, GrRenderTargetContext* rtc, SkCanvas* canvas) override {
        static constexpr SkScalar kGap = 40.f;
        static constexpr SkScalar kBannerWidth = 120.f;
        static constexpr SkScalar kOffset = 15.f;

        SkTArray<int> drawCounts(fRenderers.count());
        drawCounts.push_back_n(fRenderers.count(), 0);

        canvas->save();
        canvas->translate(kOffset + kBannerWidth, kOffset);
        for (int i = 0; i < fMatrices.count(); ++i) {
            canvas->save();
            draw_text(canvas, fMatrixNames[i].c_str());

            canvas->translate(0.f, kGap);
            for (int j = 0; j < fRenderers.count(); ++j) {
                canvas->save();
                draw_tile_boundaries(canvas, fMatrices[i]);
                draw_clipping_boundaries(canvas, fMatrices[i]);

                canvas->concat(fMatrices[i]);
                drawCounts[j] += fRenderers[j]->drawTiles(canvas, ctx, rtc);

                canvas->restore();
                // And advance to the next row
                canvas->translate(0.f, kGap + kRowCount * kTileHeight);
            }
            // Reset back to the left edge
            canvas->restore();
            // And advance to the next column
            canvas->translate(kGap + kColCount * kTileWidth, 0.f);
        }
        canvas->restore();

        // Print a row header, with total draw counts
        canvas->save();
        canvas->translate(kOffset, kGap + 0.5f * kRowCount * kTileHeight);
        for (int j = 0; j < fRenderers.count(); ++j) {
            fRenderers[j]->drawBanner(canvas);
            canvas->translate(0.f, 15.f);
            draw_text(canvas, SkStringPrintf("Draws = %d", drawCounts[j]).c_str());
            canvas->translate(0.f, kGap + kRowCount * kTileHeight);
        }
        canvas->restore();
    }

private:
    SkTArray<sk_sp<ClipTileRenderer>> fRenderers;
    SkTArray<SkMatrix> fMatrices;
    SkTArray<SkString> fMatrixNames;

    SkString fName;

    void configureMatrices() {
        fMatrices.reset();
        fMatrixNames.reset();
        fMatrices.push_back_n(kMatrixCount);

        // Identity
        fMatrices[0].setIdentity();
        fMatrixNames.push_back(SkString("Identity"));

        // Translate/scale
        fMatrices[1].setTranslate(5.5f, 20.25f);
        fMatrices[1].postScale(.9f, .7f);
        fMatrixNames.push_back(SkString("T+S"));

        // Rotation
        fMatrices[2].setRotate(20.0f);
        fMatrices[2].preTranslate(15.f, -20.f);
        fMatrixNames.push_back(SkString("Rotate"));

        // Skew
        fMatrices[3].setSkew(.5f, .25f);
        fMatrices[3].preTranslate(-30.f, 0.f);
        fMatrixNames.push_back(SkString("Skew"));

        // Perspective
        SkPoint src[4];
        SkRect::MakeWH(kColCount * kTileWidth, kRowCount * kTileHeight).toQuad(src);
        SkPoint dst[4] = {{0, 0},
                          {kColCount * kTileWidth + 10.f, 15.f},
                          {kColCount * kTileWidth - 28.f, kRowCount * kTileHeight + 40.f},
                          {25.f, kRowCount * kTileHeight - 15.f}};
        SkAssertResult(fMatrices[4].setPolyToPoly(src, dst, 4));
        fMatrices[4].preTranslate(0.f, 10.f);
        fMatrixNames.push_back(SkString("Perspective"));

        SkASSERT(fMatrices.count() == fMatrixNames.count());
    }

    typedef skiagm::GM INHERITED;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Implementations of TileRenderer that color the clipped tiles in various ways
////////////////////////////////////////////////////////////////////////////////////////////////

class DebugTileRenderer : public ClipTileRenderer {
public:

    static sk_sp<ClipTileRenderer> Make() {
        // Since aa override is disabled, the quad flags arg doesn't matter.
        return sk_sp<ClipTileRenderer>(new DebugTileRenderer(SkCanvas::kAll_QuadAAFlags, false));
    }

    static sk_sp<ClipTileRenderer> MakeAA() {
        return sk_sp<ClipTileRenderer>(new DebugTileRenderer(SkCanvas::kAll_QuadAAFlags, true));
    }

    static sk_sp<ClipTileRenderer> MakeNonAA() {
        return sk_sp<ClipTileRenderer>(new DebugTileRenderer(SkCanvas::kNone_QuadAAFlags, true));
    }

    int drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        // Colorize the tile based on its grid position and quad ID
        int i = tileID / kColCount;
        int j = tileID % kColCount;

        SkColor4f c = {(i + 1.f) / kRowCount, (j + 1.f) / kColCount, .4f, 1.f};
        float alpha = quadID / 10.f;
        c.fR = c.fR * (1 - alpha) + alpha;
        c.fG = c.fG * (1 - alpha) + alpha;
        c.fB = c.fB * (1 - alpha) + alpha;
        c.fA = c.fA * (1 - alpha) + alpha;

        SkCanvas::QuadAAFlags aaFlags = fEnableAAOverride ? fAAOverride : this->maskToFlags(edgeAA);
        fDevice->tmp_drawEdgeAAQuad(
                rect, clip, clip ? 4 : 0, aaFlags, c.toSkColor(), SkBlendMode::kSrcOver);
        return 1;
    }

    void drawBanner(SkCanvas* canvas) override {
        draw_text(canvas, "Edge AA");
        canvas->translate(0.f, 15.f);

        SkString config;
        static const char* kFormat = "Ext(%s) - Int(%s)";
        if (fEnableAAOverride) {
            SkASSERT(fAAOverride == SkCanvas::kAll_QuadAAFlags ||
                     fAAOverride == SkCanvas::kNone_QuadAAFlags);
            if (fAAOverride == SkCanvas::kAll_QuadAAFlags) {
                config.appendf(kFormat, "yes", "yes");
            } else {
                config.appendf(kFormat, "no", "no");
            }
        } else {
            config.appendf(kFormat, "yes", "no");
        }
        draw_text(canvas, config.c_str());
    }

private:
    SkCanvas::QuadAAFlags fAAOverride;
    bool fEnableAAOverride;

    DebugTileRenderer(SkCanvas::QuadAAFlags aa, bool enableAAOverrde)
            : fAAOverride(aa)
            , fEnableAAOverride(enableAAOverrde) {}

    typedef ClipTileRenderer INHERITED;
};

// Tests tmp_drawEdgeAAQuad
class SolidColorRenderer : public ClipTileRenderer {
public:

    static sk_sp<ClipTileRenderer> Make(const SkColor4f& color) {
        return sk_sp<ClipTileRenderer>(new SolidColorRenderer(color));
    }

    int drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        fDevice->tmp_drawEdgeAAQuad(rect, clip, clip ? 4 : 0, this->maskToFlags(edgeAA),
                                    fColor.toSkColor(), SkBlendMode::kSrcOver);
        return 1;
    }

    void drawBanner(SkCanvas* canvas) override {
        draw_text(canvas, "Solid Color");
    }

private:
    SkColor4f fColor;

    SolidColorRenderer(const SkColor4f& color) : fColor(color) {}

    typedef ClipTileRenderer INHERITED;
};

// Tests tmp_drawImageSet(), but can batch the entries together in different ways
// TODO(michaelludwig) - add transform batching
class TextureSetRenderer : public ClipTileRenderer {
public:

    static sk_sp<ClipTileRenderer> MakeUnbatched(sk_sp<SkImage> image) {
        return Make("Texture", "", std::move(image), nullptr, nullptr, nullptr, nullptr,
                    1.f, true, 0);
    }

    static sk_sp<ClipTileRenderer> MakeBatched(sk_sp<SkImage> image, int transformCount) {
        const char* subtitle = transformCount == 0 ? "" : "w/ xforms";
        return Make("Texture Set", subtitle, std::move(image), nullptr, nullptr, nullptr, nullptr,
                    1.f, false, transformCount);
    }

    static sk_sp<ClipTileRenderer> MakeShader(const char* name, sk_sp<SkImage> image,
                                              sk_sp<SkShader> shader, bool local) {
        return Make("Shader", name, std::move(image), std::move(shader),
                    nullptr, nullptr, nullptr, 1.f, local, 0);
    }

    static sk_sp<ClipTileRenderer> MakeColorFilter(const char* name, sk_sp<SkImage> image,
                                                   sk_sp<SkColorFilter> filter) {
        return Make("Color Filter", name, std::move(image), nullptr, std::move(filter), nullptr,
                    nullptr, 1.f, false, 0);
    }

    static sk_sp<ClipTileRenderer> MakeImageFilter(const char* name, sk_sp<SkImage> image,
                                                   sk_sp<SkImageFilter> filter) {
        return Make("Image Filter", name, std::move(image), nullptr, nullptr, std::move(filter),
                    nullptr, 1.f, false, 0);
    }

    static sk_sp<ClipTileRenderer> MakeMaskFilter(const char* name, sk_sp<SkImage> image,
                                                  sk_sp<SkMaskFilter> filter) {
        return Make("Mask Filter", name, std::move(image), nullptr, nullptr, nullptr,
                    std::move(filter), 1.f, false, 0);
    }

    static sk_sp<ClipTileRenderer> MakeAlpha(sk_sp<SkImage> image, SkScalar alpha) {
        return Make("Alpha", SkStringPrintf("a = %.2f", alpha).c_str(), std::move(image), nullptr,
                    nullptr, nullptr, nullptr, alpha, false, 0);
    }

    static sk_sp<ClipTileRenderer> Make(const char* topBanner, const char* bottomBanner,
                                        sk_sp<SkImage> image, sk_sp<SkShader> shader,
                                        sk_sp<SkColorFilter> colorFilter,
                                        sk_sp<SkImageFilter> imageFilter,
                                        sk_sp<SkMaskFilter> maskFilter, SkScalar paintAlpha,
                                        bool resetAfterEachQuad, int transformCount) {
        return sk_sp<ClipTileRenderer>(new TextureSetRenderer(topBanner, bottomBanner,
                std::move(image), std::move(shader), std::move(colorFilter), std::move(imageFilter),
                std::move(maskFilter), paintAlpha, resetAfterEachQuad, transformCount));
    }

    int drawTiles(SkCanvas* canvas, GrContext* ctx, GrRenderTargetContext* rtc) override {
        SkASSERT(fImage); // initImage should be called before any drawing
        int draws = this->INHERITED::drawTiles(canvas, ctx, rtc);
        // Push the last tile set
        draws += this->drawAndReset(canvas);
        return draws;
    }

    int drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        // Now don't actually draw the tile, accumulate it in the growing entry set
        int clipCount = 0;
        if (clip) {
            // Record the four points into fDstClips
            clipCount = 4;
            fDstClips.push_back_n(4, clip);
        }

        int preViewIdx = -1;
        if (!fResetEachQuad && fTransformBatchCount > 0) {
            // Handle transform batching. This works by capturing the CTM of the first tile draw,
            // and then calculate the difference between that and future CTMs for later tiles.
            if (fPreViewXforms.count() == 0) {
                fBaseCTM = canvas->getTotalMatrix();
                fPreViewXforms.push_back(SkMatrix::I());
                preViewIdx = 0;
            } else {
                // Calculate matrix s.t. getTotalMatrix() = fBaseCTM * M
                SkMatrix invBase;
                if (!fBaseCTM.invert(&invBase)) {
                    SkDebugf("Cannot invert CTM, transform batching will not be correct.\n");
                } else {
                    SkMatrix preView = SkMatrix::Concat(invBase, canvas->getTotalMatrix());
                    if (preView != fPreViewXforms[fPreViewXforms.count() - 1]) {
                        // Add the new matrix
                        fPreViewXforms.push_back(preView);
                    } // else re-use the last matrix
                    preViewIdx = fPreViewXforms.count() - 1;
                }
            }
        }

        // This acts like the whole image is rendered over the entire tile grid, so derive local
        // coordinates from 'rect', based on the grid to image transform.
        SkMatrix gridToImage = SkMatrix::MakeRectToRect(SkRect::MakeWH(kColCount * kTileWidth,
                                                                       kRowCount * kTileHeight),
                                                        SkRect::MakeWH(fImage->width(),
                                                                       fImage->height()),
                                                        SkMatrix::kFill_ScaleToFit);
        SkRect localRect = gridToImage.mapRect(rect);

        // drawTextureSet automatically derives appropriate local quad from localRect if clipPtr
        // is not null.
        fSetEntries.push_back({fImage, localRect, rect, 1.f, this->maskToFlags(edgeAA)});
        fDstClipCounts.push_back(clipCount);
        fPreViewIdx.push_back(preViewIdx);

        if (fResetEachQuad) {
            // Only ever draw one entry at a time
            return this->drawAndReset(canvas);
        } else {
            return 0;
        }
    }

    void drawBanner(SkCanvas* canvas) override {
        if (fTopBanner.size() > 0) {
            draw_text(canvas, fTopBanner.c_str());
        }
        canvas->translate(0.f, 15.f);
        if (fBottomBanner.size() > 0) {
            draw_text(canvas, fBottomBanner.c_str());
        }
    }

private:
    SkString fTopBanner;
    SkString fBottomBanner;

    sk_sp<SkImage> fImage;
    sk_sp<SkShader> fShader;
    sk_sp<SkColorFilter> fColorFilter;
    sk_sp<SkImageFilter> fImageFilter;
    sk_sp<SkMaskFilter> fMaskFilter;
    SkScalar fPaintAlpha;

    // Batching rules
    bool fResetEachQuad;
    int fTransformBatchCount;

    SkTArray<SkPoint> fDstClips;
    SkTArray<SkMatrix> fPreViewXforms;
    // ImageSetEntry does not yet have a fDstClipCount or fPreViewIdx field
    SkTArray<int> fDstClipCounts;
    SkTArray<int> fPreViewIdx;
    SkTArray<SkCanvas::ImageSetEntry> fSetEntries;

    SkMatrix fBaseCTM;
    int fBatchCount;

    TextureSetRenderer(const char* topBanner,
                       const char* bottomBanner,
                       sk_sp<SkImage> image,
                       sk_sp<SkShader> shader,
                       sk_sp<SkColorFilter> colorFilter,
                       sk_sp<SkImageFilter> imageFilter,
                       sk_sp<SkMaskFilter> maskFilter,
                       SkScalar paintAlpha,
                       bool resetEachQuad,
                       int transformBatchCount)
            : fTopBanner(topBanner)
            , fBottomBanner(bottomBanner)
            , fImage(std::move(image))
            , fShader(std::move(shader))
            , fColorFilter(std::move(colorFilter))
            , fImageFilter(std::move(imageFilter))
            , fMaskFilter(std::move(maskFilter))
            , fPaintAlpha(paintAlpha)
            , fResetEachQuad(resetEachQuad)
            , fTransformBatchCount(transformBatchCount)
            , fBatchCount(0) {
        SkASSERT(transformBatchCount >= 0 && (!resetEachQuad || transformBatchCount == 0));
    }

    void configureTilePaint(const SkRect& rect, SkPaint* paint) const {
        paint->setAntiAlias(true);
        paint->setFilterQuality(kLow_SkFilterQuality);
        paint->setBlendMode(SkBlendMode::kSrcOver);

        // Send non-white RGB, that should be ignored
        paint->setColor4f({1.f, 0.4f, 0.25f, fPaintAlpha}, nullptr);


        if (fShader) {
            if (fResetEachQuad) {
                // Apply a local transform in the shader to map from the tile rectangle to (0,0,w,h)
                static const SkRect kTarget = SkRect::MakeWH(kTileWidth, kTileHeight);
                SkMatrix local = SkMatrix::MakeRectToRect(kTarget, rect,
                                                          SkMatrix::kFill_ScaleToFit);
                paint->setShader(fShader->makeWithLocalMatrix(local));
            } else {
                paint->setShader(fShader);
            }
        }

        paint->setColorFilter(fColorFilter);
        paint->setImageFilter(fImageFilter);
        paint->setMaskFilter(fMaskFilter);
    }

    int drawAndReset(SkCanvas* canvas) {
        // Early out if there's nothing to draw
        if (fSetEntries.count() == 0) {
            SkASSERT(fDstClips.count() == 0 && fPreViewXforms.count() == 0 &&
                     fDstClipCounts.count() == 0 && fPreViewIdx.count() == 0);
            return 0;
        }

        if (!fResetEachQuad && fTransformBatchCount > 0) {
            // A batch is completed
            fBatchCount++;
            if (fBatchCount < fTransformBatchCount) {
                // Haven't hit the point to submit yet, but end the current tile
                return 0;
            }

            // Submitting all tiles back to where fBaseCTM was the canvas' matrix, while the
            // canvas currently has the CTM of the last tile batch, so reset it.
            canvas->setMatrix(fBaseCTM);
        }

        // NOTE: Eventually these will just be stored as a field on each entry
        SkASSERT(fDstClipCounts.count() == fSetEntries.count());
        SkASSERT(fPreViewIdx.count() == fSetEntries.count());

#ifdef SK_DEBUG
        int expectedDstClipCount = 0;
        for (int i = 0; i < fDstClipCounts.count(); ++i) {
            expectedDstClipCount += fDstClipCounts[i];
            SkASSERT(fPreViewIdx[i] < 0 || fPreViewIdx[i] < fPreViewXforms.count());
        }
        SkASSERT(expectedDstClipCount == fDstClips.count());
#endif

        SkPaint paint;
        SkRect lastTileRect = fSetEntries[fSetEntries.count() - 1].fDstRect;
        this->configureTilePaint(lastTileRect, &paint);

        fDevice->tmp_drawImageSetV3(fSetEntries.begin(), fDstClipCounts.begin(),
                                    fPreViewIdx.begin(), fSetEntries.count(),
                                    fDstClips.begin(), fPreViewXforms.begin(),
                                    paint, SkCanvas::kFast_SrcRectConstraint);

        // Reset for next tile
        fDstClips.reset();
        fDstClipCounts.reset();
        fPreViewXforms.reset();
        fPreViewIdx.reset();
        fSetEntries.reset();
        fBatchCount = 0;

        return 1;
    }

    typedef ClipTileRenderer INHERITED;
};

static SkTArray<sk_sp<ClipTileRenderer>> make_debug_renderers() {
    SkTArray<sk_sp<ClipTileRenderer>> renderers;
    renderers.push_back(DebugTileRenderer::Make());
    renderers.push_back(DebugTileRenderer::MakeAA());
    renderers.push_back(DebugTileRenderer::MakeNonAA());
    return renderers;
}

static SkTArray<sk_sp<ClipTileRenderer>> make_shader_renderers() {
    static constexpr SkPoint kPts[] = { {0.f, 0.f}, {0.25f * kTileWidth, 0.25f * kTileHeight} };
    static constexpr SkColor kColors[] = { SK_ColorBLUE, SK_ColorWHITE };
    auto gradient = SkGradientShader::MakeLinear(kPts, kColors, nullptr, 2,
                                                 SkShader::kMirror_TileMode);

    auto info = SkImageInfo::Make(1, 1, kAlpha_8_SkColorType, kOpaque_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(info);
    bm.eraseColor(SK_ColorWHITE);
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bm);

    SkTArray<sk_sp<ClipTileRenderer>> renderers;
    renderers.push_back(TextureSetRenderer::MakeShader("Gradient", image, gradient, false));
    renderers.push_back(TextureSetRenderer::MakeShader("Local Gradient", image, gradient, true));
    return renderers;
}

static SkTArray<sk_sp<ClipTileRenderer>> make_image_renderers() {
    sk_sp<SkImage> mandrill = GetResourceAsImage("images/mandrill_512.png");
    SkTArray<sk_sp<ClipTileRenderer>> renderers;
    renderers.push_back(TextureSetRenderer::MakeUnbatched(mandrill));
    renderers.push_back(TextureSetRenderer::MakeBatched(mandrill, 0));
    renderers.push_back(TextureSetRenderer::MakeBatched(mandrill, kMatrixCount));
    return renderers;
}

static SkTArray<sk_sp<ClipTileRenderer>> make_filtered_renderers() {
    sk_sp<SkImage> mandrill = GetResourceAsImage("images/mandrill_512.png");

    SkColorMatrix cm;
    cm.setSaturation(10);
    sk_sp<SkColorFilter> colorFilter = SkColorFilter::MakeMatrixFilterRowMajor255(cm.fMat);
    sk_sp<SkImageFilter> imageFilter = SkDilateImageFilter::Make(8, 8, nullptr);

    static constexpr SkColor kAlphas[] = { SK_ColorTRANSPARENT, SK_ColorBLACK };
    auto alphaGradient = SkGradientShader::MakeRadial(
            {0.5f * kTileWidth * kColCount, 0.5f * kTileHeight * kRowCount},
            0.25f * kTileWidth * kColCount, kAlphas, nullptr, 2, SkShader::kClamp_TileMode);
    sk_sp<SkMaskFilter> maskFilter = SkShaderMaskFilter::Make(std::move(alphaGradient));

    SkTArray<sk_sp<ClipTileRenderer>> renderers;
    renderers.push_back(TextureSetRenderer::MakeAlpha(mandrill, 0.5f));
    renderers.push_back(TextureSetRenderer::MakeColorFilter("Saturation", mandrill,
                                                            std::move(colorFilter)));
    // NOTE: won't draw correctly until SkCanvas' AutoLoopers are used to handle image filters
    renderers.push_back(TextureSetRenderer::MakeImageFilter("Dilate", mandrill,
                                                            std::move(imageFilter)));

    renderers.push_back(TextureSetRenderer::MakeMaskFilter("Shader", mandrill,
                                                           std::move(maskFilter)));
    // NOTE: blur mask filters do work (tested locally), but visually they don't make much
    // sense, since each quad is blurred independently
    return renderers;
}

DEF_GM(return new CompositorGM("debug", make_debug_renderers());)
DEF_GM(return new CompositorGM("color", SolidColorRenderer::Make({.2f, .8f, .3f, 1.f}));)
DEF_GM(return new CompositorGM("shader", make_shader_renderers());)
DEF_GM(return new CompositorGM("image", make_image_renderers());)
DEF_GM(return new CompositorGM("filter", make_filtered_renderers());)

#endif // SK_SUPPORT_GPU
