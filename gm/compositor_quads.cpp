/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrClip.h"
#include "GrRect.h"
#include "GrRenderTargetContextPriv.h"

#include "Resources.h"
#include "SkFont.h"
#include "SkGr.h"
#include "SkGradientShader.h"
#include "SkImage_Base.h"
#include "SkLineClipper.h"

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

class TileRenderer : public SkRefCntBase {
public:
    virtual ~TileRenderer() {}

    // Draw the base rect, possibly clipped by 'clip' if that is not null. The edges to antialias
    // are specified in 'edgeAA' (to make manipulation easier than an unsigned bitfield). 'tileID'
    // represents the location of rect within the tile grid, 'quadID' is the unique ID of the clip
    // region within the tile (reset for each tile).
    //
    // The edgeAA order matches that of clip, so it refers to top, right, bottom, left.
    virtual void drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4],
                          const bool edgeAA[4], int tileID, int quadID) = 0;

    virtual void drawBanner(SkCanvas* canvas) = 0;

    virtual void drawTiles(SkCanvas* canvas, GrContext* context, GrRenderTargetContext* rtc) {
        // TODO (michaelludwig) - once the quad APIs are in SkCanvas, drop these
        // cached fields, which drawTile() needs
        fContext = context;
        fRTC = rtc;

        // All three lines in a list
        SkPoint lines[6];
        clipping_line_segment(kClipP1, kClipP2, lines);
        clipping_line_segment(kClipP2, kClipP3, lines + 2);
        clipping_line_segment(kClipP3, kClipP1, lines + 4);

        bool edgeAA[4];
        int tileID = 0;
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
                this->clipTile(canvas, tileID, tile, nullptr, edgeAA, lines, 3, &quadCount);
                tileID++;
            }
        }
    }

protected:
    // Remembered for convenience in drawTile, set by drawTiles()
    GrContext* fContext;
    GrRenderTargetContext* fRTC;

    GrQuadAAFlags maskToFlags(const bool edgeAA[4]) const {
        GrQuadAAFlags flags = GrQuadAAFlags::kNone;
        flags |= edgeAA[0] ? GrQuadAAFlags::kTop    : GrQuadAAFlags::kNone;
        flags |= edgeAA[1] ? GrQuadAAFlags::kRight  : GrQuadAAFlags::kNone;
        flags |= edgeAA[2] ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;
        flags |= edgeAA[3] ? GrQuadAAFlags::kLeft   : GrQuadAAFlags::kNone;
        return flags;
    }

    // Recursively splits the quadrilateral against the segments stored in 'lines', which must be
    // 2 * lineCount long. Increments 'quadCount' for each split quadrilateral, and invokes the
    // drawTile at leaves.
    void clipTile(SkCanvas* canvas, int tileID, const SkRect& baseRect, const SkPoint quad[4],
                  const bool edgeAA[4], const SkPoint lines[], int lineCount, int* quadCount) {
        if (lineCount == 0) {
            // No lines, so end recursion by drawing the tile. If the tile was never split then
            // 'quad' remains null so that drawTile() can differentiate how it should draw.
            this->drawTile(canvas, baseRect, quad, edgeAA, tileID, *quadCount);
            *quadCount = *quadCount + 1;
            return;
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
                    return;
            }
        }

        SkPoint sub[4];
        bool subAA[4];
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
            this->clipTile(canvas, tileID, baseRect, sub, subAA, lines + 2, lineCount - 1,
                           quadCount);
        }
    }
};

class CompositorGM : public skiagm::GpuGM {
public:
    CompositorGM(const char* name, sk_sp<TileRenderer> renderer)
            : fName(name) {
        fRenderers.push_back(std::move(renderer));
    }
    CompositorGM(const char* name, sk_sp<TileRenderer> r1, sk_sp<TileRenderer> r2)
            : fName(name) {
        fRenderers.push_back(std::move(r1));
        fRenderers.push_back(std::move(r2));
    }
    CompositorGM(const char* name, sk_sp<TileRenderer> r1, sk_sp<TileRenderer> r2,
                 sk_sp<TileRenderer> r3)
            : fName(name) {
        fRenderers.push_back(std::move(r1));
        fRenderers.push_back(std::move(r2));
        fRenderers.push_back(std::move(r3));
    }
    // 3 renderer modes is the max any GM needs right now

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

        // Print a row header
        canvas->save();
        canvas->translate(kOffset, kGap + 0.5f * kRowCount * kTileHeight);
        for (int j = 0; j < fRenderers.count(); ++j) {
            fRenderers[j]->drawBanner(canvas);
            canvas->translate(0.f, kGap + kRowCount * kTileHeight);
        }
        canvas->restore();

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
                fRenderers[j]->drawTiles(canvas, ctx, rtc);

                canvas->restore();
                // And advance to the next row
                canvas->translate(0.f, kGap + kRowCount * kTileHeight);
            }
            // Reset back to the left edge
            canvas->restore();
            // And advance to the next column
            canvas->translate(kGap + kColCount * kTileWidth, 0.f);
        }
    }

private:
    static constexpr int kMatrixCount = 5;

    SkTArray<sk_sp<TileRenderer>> fRenderers;
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

class DebugTileRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make() {
        // Since aa override is disabled, the quad flags arg doesn't matter.
        return sk_sp<TileRenderer>(new DebugTileRenderer(GrQuadAAFlags::kAll, false));
    }

    static sk_sp<TileRenderer> MakeAA() {
        return sk_sp<TileRenderer>(new DebugTileRenderer(GrQuadAAFlags::kAll, true));
    }

    static sk_sp<TileRenderer> MakeNonAA() {
        return sk_sp<TileRenderer>(new DebugTileRenderer(GrQuadAAFlags::kNone, true));
    }

    void drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        // Colorize the tile based on its grid position and quad ID
        int i = tileID / kColCount;
        int j = tileID % kColCount;

        SkPMColor4f c = {(i + 1.f) / kRowCount, (j + 1.f) / kColCount, .4f, 1.f};
        float alpha = quadID / 10.f;
        c.fR = c.fR * (1 - alpha) + alpha;
        c.fG = c.fG * (1 - alpha) + alpha;
        c.fB = c.fB * (1 - alpha) + alpha;
        c.fA = c.fA * (1 - alpha) + alpha;

        GrPaint paint;
        paint.setColor4f(c);

        GrQuadAAFlags aaFlags = fEnableAAOverride ? fAAOverride : this->maskToFlags(edgeAA);
        if (clip) {
            fRTC->fillQuadWithEdgeAA(GrNoClip(), std::move(paint), GrAA::kYes, aaFlags,
                                     canvas->getTotalMatrix(), clip, nullptr);
        } else {
            fRTC->fillRectWithEdgeAA(GrNoClip(), std::move(paint), GrAA::kYes, aaFlags,
                                     canvas->getTotalMatrix(), rect);
        }
    }

    void drawBanner(SkCanvas* canvas) override {
        canvas->save();
        draw_text(canvas, "Edge AA");
        canvas->translate(0.f, 15.f);

        SkString config;
        static const char* kFormat = "Ext(%s) - Int(%s)";
        if (fEnableAAOverride) {
            SkASSERT(fAAOverride == GrQuadAAFlags::kAll || fAAOverride == GrQuadAAFlags::kNone);
            if (fAAOverride == GrQuadAAFlags::kAll) {
                config.appendf(kFormat, "yes", "yes");
            } else {
                config.appendf(kFormat, "no", "no");
            }
        } else {
            config.appendf(kFormat, "yes", "no");
        }
        canvas->translate(0.f, 6.f);
        draw_text(canvas, config.c_str());
        canvas->restore();
    }

private:
    GrQuadAAFlags fAAOverride;
    bool fEnableAAOverride;

    DebugTileRenderer(GrQuadAAFlags aa, bool enableAAOverrde)
            : fAAOverride(aa)
            , fEnableAAOverride(enableAAOverrde) {}

    typedef TileRenderer INHERITED;
};

class SolidColorRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make(const SkPMColor4f& color) {
        return sk_sp<TileRenderer>(new SolidColorRenderer(color));
    }

    void drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        GrPaint paint;
        paint.setColor4f(fColor);

        if (clip) {
            fRTC->fillQuadWithEdgeAA(GrNoClip(), std::move(paint), GrAA::kYes,
                    this->maskToFlags(edgeAA), canvas->getTotalMatrix(), clip, nullptr);
        } else {
            fRTC->fillRectWithEdgeAA(GrNoClip(), std::move(paint), GrAA::kYes,
                    this->maskToFlags(edgeAA), canvas->getTotalMatrix(), rect);
        }
    }

    void drawBanner(SkCanvas* canvas) override {
        draw_text(canvas, "Solid Color");
    }

private:
    SkPMColor4f fColor;

    SolidColorRenderer(const SkPMColor4f& color) : fColor(color) {}

    typedef TileRenderer INHERITED;
};

class GradientRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> MakeSeamless() {
        return sk_sp<TileRenderer>(new GradientRenderer(false));
    }

    static sk_sp<TileRenderer> MakeLocal() {
        return sk_sp<TileRenderer>(new GradientRenderer(true));
    }

    void drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        GrPaint paint;
        SkPaintToGrPaint(fContext, fRTC->colorSpaceInfo(), fGradient, canvas->getTotalMatrix(),
                         &paint);

        SkRect localRect = SkRect::MakeWH(kTileWidth, kTileHeight);
        SkPoint localQuad[4];
        if (fLocal && clip) {
            GrMapRectPoints(rect, localRect, clip, localQuad, 4);
        }

        if (clip) {
            fRTC->fillQuadWithEdgeAA(GrNoClip(), std::move(paint), GrAA::kYes,
                    this->maskToFlags(edgeAA), canvas->getTotalMatrix(), clip,
                    fLocal ? localQuad : nullptr);
        } else {
            fRTC->fillRectWithEdgeAA(GrNoClip(), std::move(paint), GrAA::kYes,
                    this->maskToFlags(edgeAA), canvas->getTotalMatrix(), rect,
                    fLocal ? &localRect : nullptr);
        }
    }

    void drawBanner(SkCanvas* canvas) override {
        canvas->save();
        draw_text(canvas, "Gradient");
        canvas->translate(0.f, 15.f);
        if (fLocal) {
            draw_text(canvas, "Local");
        } else {
            draw_text(canvas, "Seamless");
        }
        canvas->restore();
    }

private:
    SkPaint fGradient;
    bool fLocal;

    GradientRenderer(bool local) : fLocal(local) {
        static constexpr SkPoint pts[] = { {0.f, 0.f}, {0.25f * kTileWidth, 0.25f * kTileHeight} };
        static constexpr SkColor colors[] = { SK_ColorBLUE, SK_ColorWHITE };
        auto gradient = SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                     SkShader::kMirror_TileMode);
        fGradient.setShader(gradient);
    }

    typedef TileRenderer INHERITED;
};

static SkRect get_image_local_rect(const sk_sp<SkImage> image, const SkRect& rect) {
    // This acts like the whole image is rendered over the entire tile grid, so derive local
    // coordinates from 'rect', based on the grid to image transform.
    SkMatrix gridToImage = SkMatrix::MakeRectToRect(SkRect::MakeWH(kColCount * kTileWidth,
                                                                   kRowCount * kTileHeight),
                                                    SkRect::MakeWH(image->width(),
                                                                   image->height()),
                                                    SkMatrix::kFill_ScaleToFit);
    return gridToImage.mapRect(rect);
}

class TextureRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make(sk_sp<SkImage> image) {
        return sk_sp<TileRenderer>(new TextureRenderer(image));
    }

    void drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        SkPMColor4f color = {1.f, 1.f, 1.f, 1.f};
        SkRect localRect = get_image_local_rect(fImage, rect);

        fImage = fImage->makeTextureImage(fContext, nullptr);
        sk_sp<GrTextureProxy> proxy = as_IB(fImage)->asTextureProxyRef();
        SkASSERT(proxy);
        if (clip) {
            SkPoint localQuad[4];
            GrMapRectPoints(rect, localRect, clip, localQuad, 4);
            fRTC->drawTextureQuad(GrNoClip(), std::move(proxy), GrSamplerState::Filter::kBilerp,
                    SkBlendMode::kSrcOver, color, localQuad, clip, GrAA::kYes,
                    this->maskToFlags(edgeAA), nullptr, canvas->getTotalMatrix(), nullptr);
        } else {
            fRTC->drawTexture(GrNoClip(), std::move(proxy), GrSamplerState::Filter::kBilerp,
                    SkBlendMode::kSrcOver, color, localRect, rect, GrAA::kYes,
                    this->maskToFlags(edgeAA), SkCanvas::kFast_SrcRectConstraint,
                    canvas->getTotalMatrix(), nullptr);
        }
    }

    void drawBanner(SkCanvas* canvas) override {
        draw_text(canvas, "Texture");
    }

private:
    sk_sp<SkImage> fImage;

    TextureRenderer(sk_sp<SkImage> image)
            : fImage(image) {}

    typedef TileRenderer INHERITED;
};

// Looks like TextureRenderer, but bundles tiles into drawTextureSet calls
class TextureSetRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make(sk_sp<SkImage> image) {
        return sk_sp<TileRenderer>(new TextureSetRenderer(image));
    }

    void drawTiles(SkCanvas* canvas, GrContext* ctx, GrRenderTargetContext* rtc) override {
        this->INHERITED::drawTiles(canvas, ctx, rtc);
        // Push the last tile set
        this->drawAndReset(canvas);
    }

    void drawTile(SkCanvas* canvas, const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        // Submit the last batch if we've moved on to a new tile
        if (tileID != fCurrentTileID) {
            this->drawAndReset(canvas);
        }
        SkASSERT((fCurrentTileID < 0 && fDstClips.count() == 0 && fDstClipIndices.count() == 0 &&
                  fSetEntries.count() == 0) ||
                 (fCurrentTileID == tileID && fSetEntries.count() > 0));

        // Now don't actually draw the tile, accumulate it in the growing entry set
        fCurrentTileID = tileID;

        int clipIndex = -1;
        if (clip) {
            // Record the four points into fDstClips and get the pointer to the first in the array
            clipIndex = fDstClips.count();
            fDstClips.push_back_n(4, clip);
        }

        SkRect localRect = get_image_local_rect(fImage, rect);

        fImage = fImage->makeTextureImage(fContext, nullptr);
        sk_sp<GrTextureProxy> proxy = as_IB(fImage)->asTextureProxyRef();
        // drawTextureSet automatically derives appropriate local quad from localRect if clipPtr
        // is not null.
        fSetEntries.push_back({proxy, localRect, rect, nullptr, 1.f, this->maskToFlags(edgeAA)});
        fDstClipIndices.push_back(clipIndex);
    }

    void drawBanner(SkCanvas* canvas) override {
        draw_text(canvas, "Texture Set");
    }

private:
    sk_sp<SkImage> fImage;

    SkTArray<SkPoint> fDstClips;
    // Since fDstClips will reallocate as needed, can't get the final pointer for the entries'
    // fDstClip values until submitting the entire set
    SkTArray<int> fDstClipIndices;
    SkTArray<GrRenderTargetContext::TextureSetEntry> fSetEntries;
    int fCurrentTileID;

    TextureSetRenderer(sk_sp<SkImage> image)
            : fImage(image)
            , fCurrentTileID(-1) {}

    void drawAndReset(SkCanvas* canvas) {
        // Early out if there's nothing to draw
        if (fSetEntries.count() == 0) {
            SkASSERT(fCurrentTileID < 0 && fDstClips.count() == 0 && fDstClipIndices.count() == 0);
            return;
        }

        // Fill in fDstClip in the entries now that fDstClips' storage won't change until after the
        // draw is finished.
        // NOTE: The eventual API in SkGpuDevice will make easier to collect
        // SkCanvas::ImageSetEntries and dst clips without this extra work, but also internally maps
        // very cleanly on to the TextureSetEntry fDstClip approach.
        SkASSERT(fDstClipIndices.count() == fSetEntries.count());
        for (int i = 0; i < fSetEntries.count(); ++i) {
            if (fDstClipIndices[i] >= 0) {
                fSetEntries[i].fDstClip = &fDstClips[fDstClipIndices[i]];
            }
        }

        // Send to GPU
        fRTC->drawTextureSet(GrNoClip(), fSetEntries.begin(), fSetEntries.count(),
                             GrSamplerState::Filter::kBilerp, SkBlendMode::kSrcOver, GrAA::kYes,
                             canvas->getTotalMatrix(), nullptr);
        // Reset for next tile
        fCurrentTileID = -1;
        fDstClips.reset();
        fDstClipIndices.reset();
        fSetEntries.reset();
    }

    typedef TileRenderer INHERITED;
};

DEF_GM(return new CompositorGM("debug",
        DebugTileRenderer::Make(), DebugTileRenderer::MakeAA(),
        DebugTileRenderer::MakeNonAA());)
DEF_GM(return new CompositorGM("color", SolidColorRenderer::Make({.2f, .8f, .3f, 1.f})));
DEF_GM(return new CompositorGM("shader",
        GradientRenderer::MakeSeamless(), GradientRenderer::MakeLocal()));
DEF_GM(return new CompositorGM("image",
        TextureRenderer::Make(GetResourceAsImage("images/mandrill_512.png")),
        TextureSetRenderer::Make(GetResourceAsImage("images/mandrill_512.png"))));

#endif // SK_SUPPORT_GPU
