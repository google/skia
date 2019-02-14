/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkFont.h"
#include "SkGradientShader.h"
#include "SkLineClipper.h"

#if SK_SUPPORT_GPU

#include "ops/GrFillRectOp.h"
#include "ops/GrTextureOp.h"
#include "GrRenderTargetContextPriv.h"

#include "SkImage_Base.h"
#include "SkGr.h"

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
    line[0] = p0 - v * 100.f;
    line[1] = p1 + v * 100.f;
}

// Returns true if line segment (p0-p1) intersects with line segment (l0-l1); if true is returned,
// the intersection point is stored in 'intersect'.
static bool intersect_line_segments(const SkPoint& p0, const SkPoint& p1,
                                    const SkPoint& l0, const SkPoint& l1, SkPoint* intersect,
                                    bool verbose) {
    if (verbose) {
        SkDebugf("p0 [%.3f %.3f] - p1 [%.3f %.3f]\n", p0.fX, p0.fY, p1.fX, p1.fY);
        SkDebugf("l0 [%.3f %.3f] - l1 [%.3f %.3f]\n", l0.fX, l0.fY, l1.fX, l1.fY);
    }
    static constexpr SkScalar kHorizontalTolerance = 0.01f; // Pretty conservative
    SkVector p = p1 - p0;
    SkVector l = l1 - l0;
    SkVector pl = p0 - l0;
    if (SkScalarNearlyZero(p.fY, kHorizontalTolerance)) {
        if (SkScalarNearlyZero(l.fY, kHorizontalTolerance)) {
            // Two horizontal lines
            if (verbose) {
                SkDebugf("two horizontal lines\n");
            }
            return false;
        } else {
            // Recalculate but swap p and l
            if (verbose) {
                SkDebugf("horizontal first segment, swapping\n");
            }
            return intersect_line_segments(l0, l1, p0, p1, intersect, verbose);
        }
    }

    if (verbose) {
        SkDebugf("p [%.4f %.4f]\n", p.fX, p.fY);
        SkDebugf("l [%.4f %.4f]\n", l.fX, l.fY);
        SkDebugf("pl [%.4f %.4f]\n", pl.fX, pl.fY);
    }

    // Up to now, the line segments do not form an invalid intersection
    SkScalar lNumerator = pl.fX * p.fY - pl.fY * p.fX;
    SkScalar lDenom = l.fX * p.fY - l.fY * p.fX;
    if (verbose) {
        SkDebugf("alphaL %.4f = %.4f / %.4f\n", lNumerator / lDenom, lNumerator, lDenom);
    }
    if (SkScalarNearlyZero(lDenom)) {
        // Parallel or identical
        if (verbose) {
            SkDebugf("zero denominator, failing\n");
        }
        return false;
    }

    // Calculate alphaL that provides the intersection point along (l0-l1), e.g. l0+alphaL*(l1-l0)
    SkScalar alphaL = lNumerator / lDenom;
    if (alphaL < 0.f || alphaL > 1.f) {
        // Outside of the l segment
        if (verbose) {
            SkDebugf("outside of second line: %.4f\n", alphaL);
        }
        return false;
    }
    SkPoint lI = l0 + (l1 - l0) * alphaL;


    // Calculate alphaP from the valid alphaL (since it could be outside p segment)
    SkScalar alphaP = (alphaL * l.fY - pl.fY) / p.fY;
    if (alphaP < 0.f || alphaP > 1.f) {
        if (verbose) {
            SkDebugf("Outside of first line: %.4f, point: [%.4f %.4f]\n", alphaP, lI.fX, lI.fY);
        }
        return false;
    }

    // Is valid, so calculate the actual intersection point
    *intersect = p0 + (p1 - p0) * alphaP;
    if (verbose) {
        SkDebugf("intersection found: [%.4f %.4f]\n", intersect->fX, intersect->fY);

        SkPoint lI = l0 + (l1 - l0) * alphaL;
        SkDebugf("L point: [%.4f %.4f]\n", lI.fX, lI.fY);
        SkDebugf("alphaP = %f\n", alphaP);
    }
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
// the Chromium quad types.
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
    virtual void drawTile(const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                          int tileID, int quadID) = 0;

    virtual void drawBanner() = 0;

    void drawTiles() {
        // All three lines in a list
        SkPoint lines[6];
        clipping_line_segment(kClipP1, kClipP2, lines);
        clipping_line_segment(kClipP2, kClipP3, lines + 2);
        clipping_line_segment(kClipP3, kClipP1, lines + 4);

        // Storage for the base unwrapped tile rectangle
        SkPoint quad[4];
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
                tile.toQuad(quad);
                int quadCount = 0;
                if (!this->clipTile(tileID, tile, quad, edgeAA, lines, 3, &quadCount)) {
                    // Wasn't clipped so draw it directly as a rectangle
                    this->drawTile(tile, nullptr, edgeAA, tileID, quadCount);
                    quadCount++;
                }
                tileID++;
            }
        }
    }

protected:
    SkCanvas* fCanvas;
    GrContext* fContext;
    GrRenderTargetContext* fRTC;

    // Drawing API pointers must live longer than the tile renderers
    TileRenderer(SkCanvas* canvas, GrContext* context, GrRenderTargetContext* rtc)
            : fCanvas(canvas)
            , fContext(context)
            , fRTC(rtc) {}

    GrQuadAAFlags maskToFlags(const bool edgeAA[4]) const {
        GrQuadAAFlags flags = GrQuadAAFlags::kNone;
        flags |= edgeAA[0] ? GrQuadAAFlags::kTop    : GrQuadAAFlags::kNone;
        flags |= edgeAA[1] ? GrQuadAAFlags::kRight  : GrQuadAAFlags::kNone;
        flags |= edgeAA[2] ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;
        flags |= edgeAA[3] ? GrQuadAAFlags::kLeft   : GrQuadAAFlags::kNone;
        return flags;
    }

    // NOTE: This will go away when the quad APIs exist on the RTC and we don't have to add
    // draw ops manually.
    GrAAType getDefaultAAType(GrAA aa = GrAA::kYes) const {
        return GrChooseAAType(aa, fRTC->fsaaType(), GrAllowMixedSamples::kNo, *fRTC->caps());
    }

    // Recursively splits the quadrilateral against the segments stored in 'lines', which must be
    // 2 * lineCount long. Increments 'quadCount' for each split quadrilateral, and invokes the
    // drawTile at leafs. Returns false if the quad was not split.
    bool clipTile(int tileID, const SkRect& baseRect, const SkPoint tile[4], const bool edgeAA[4],
                  const SkPoint lines[], int lineCount, int* quadCount) {
        // No lines, so end recursion
        if (lineCount == 0) {
            return false;
        }

        // Consider the first line against the 4 quad edges in tile, which should have 0,1, or 2
        // intersection points since the tile is convex.
        SkPoint newPoints[2];
        int splitIndices[2]; // Edge that was intersected
        int intersectionCount = 0;
        for (int i = 0; i < 4; ++i) {
            SkPoint intersect;
            if (intersect_line_segments(tile[i], tile[i == 3 ? 0 : i + 1],
                                        lines[0], lines[1], &intersect, false)) {
                // If the intersected point is the same as the last found intersection, the line
                // runs through a vertex, so don't double count it
                bool duplicate = false;
                for (int j = 0; j < intersectionCount; ++j) {
                    if (SkScalarNearlyZero((intersect - newPoints[j]).length())) {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate) {
                    newPoints[intersectionCount] = intersect;
                    splitIndices[intersectionCount] = i;
                    intersectionCount++;
                }
            }
        }

        SkASSERT(intersectionCount <= 2);

        if (intersectionCount == 2) {
            // Split the tile points into 2+ sub quads and recurse to the next lines, which may or
            // may not further split the tile. Since the configurations are relatively simple, the
            // possible splits are hardcoded below; subtile quad orderings are such that the sub
            // tiles remain in counter clockwise order and match expected edges for QuadAAFlags.

            // Holds original 4 points, the new intersection points, and possibly an arbitrary point
            // along an edge used to further subdivide a polygon (for a max of 7 points).
            SkSTArray<7, SkPoint> points(tile, 4);
            points.push_back(newPoints[0]);
            points.push_back(newPoints[1]);
            int splitP0 = points.count() - 2;
            int splitP1 = points.count() - 1;

            SkSTArray<3, SkIRect> subtiles; // Abuse SkIRect for an int4 tuple instead of int[4].
            if (splitIndices[1] - splitIndices[0] == 2) {
                // Opposite edges, so the split trivially forms 2 sub quads
                if (splitIndices[0] == 0) {
                    SkASSERT(splitIndices[1] == 2);
                    subtiles.push_back({0, splitP0, splitP1, 3});
                    subtiles.push_back({splitP0, 1, 2, splitP1});
                } else {
                    SkASSERT(splitIndices[0] == 1 && splitIndices[1] == 3);
                    subtiles.push_back({0, 1, splitP0, splitP1});
                    subtiles.push_back({splitP1, splitP0, 2, 3});
                }
            } else {
                // Adjacent edges, which makes for a more complicated split, since it forms a
                // degenerate quad (triangle) and a pentagon that must be artificially split. They
                // all add a point halfway along an opposite edge, whose index is stored in
                // halfwayEdge
                int halfwayEdge = points.count();
                switch(splitIndices[0]) {
                case 0:
                    // Could be connected to edge 1 or edge 3
                    if (splitIndices[1] == 1) {
                        points.push_back((tile[0] + tile[3]) * 0.5f);
                        subtiles.push_back({0, splitP0, splitP1, halfwayEdge});
                        subtiles.push_back({halfwayEdge, splitP1, 2, 3});
                        subtiles.push_back({splitP0, 1, splitP1, splitP1}); // degenerate
                    } else {
                        SkASSERT(splitIndices[1] == 3);
                        points.push_back((tile[2] + tile[3]) * 0.5f);
                        subtiles.push_back({splitP0, 1, 2, halfwayEdge});
                        subtiles.push_back({splitP1, splitP0, halfwayEdge, 3});
                        subtiles.push_back({0, splitP0, splitP1, splitP1}); // degenerate
                    }
                    break;
                case 1:
                    // Edge 0 handled above, should only be connected to edge 2
                    SkASSERT(splitIndices[1] == 2);
                    points.push_back((tile[0] + tile[1]) * 0.5f);
                    subtiles.push_back({halfwayEdge, 1, splitP0, splitP1});
                    subtiles.push_back({0, halfwayEdge, splitP1, 3});
                    subtiles.push_back({splitP1, splitP0, 2, splitP1}); // degenerate
                    break;
                case 2:
                    // Edge 1 handled above, should only be connected to edge 3
                    SkASSERT(splitIndices[1] == 3);
                    points.push_back((tile[1] + tile[2]) * 0.5f);
                    subtiles.push_back({0, 1, halfwayEdge, splitP1});
                    subtiles.push_back({splitP1, halfwayEdge, 2, splitP0});
                    subtiles.push_back({splitP1, splitP0, splitP0, 3}); // degenerate
                    break;
                case 3:
                    // Fall through, an adjacent edge split that hits edge 3 should have first found
                    // been found with edge 0 or edge 2 for the other end
                default:
                    SkASSERT(false);
                    return false;
                }
            }

            SkPoint sub[4];
            bool subAA[4];
            for (int i = 0; i < subtiles.count(); ++i) {
                for (int j = 0; j < 4; ++j) {
                    int* tileIndices = (int*) &subtiles[i]; // Pretend SkIRect is an int[4]
                    int p = tileIndices[j];
                    sub[j] = points[p];

                    int np = j == 3 ? tileIndices[0] : tileIndices[j + 1];
                    // The "new" edges are the edges that connect between the two intersection points
                    // or between an intersection point and the halfway point. If the edge includes
                    // one of the first 4 points, it is still aligned with the original shape so
                    // keep that edge's AA setting.
                    if (p >= 4 && np >= 4) {
                        // New edge
                        subAA[j] = false;
                    } else {
                        // The subtiles indices were arranged so that their edge ordering was
                        // still top, right, bottom, left so 'j' can be used to access edgeAA
                        subAA[j] = edgeAA[j];
                    }
                }

                // Split the sub quad with the next line
                bool split = this->clipTile(
                        tileID, baseRect, sub, subAA, lines + 2, lineCount - 1, quadCount);
                if (!split) {
                    // The quad was not further split, which means we're responsible for sending it
                    // to the renderer.
                    this->drawTile(baseRect, sub, subAA, tileID, *quadCount);
                    *quadCount = *quadCount + 1;
                }
            }
            return true;
        } else {
            // Either the first line never intersected the quad (count == 0), or it intersected at a
            // single vertex without going through quad area (count == 1), so advance to the next
            // line
            return this->clipTile(
                        tileID, baseRect, tile, edgeAA, lines + 2, lineCount - 1, quadCount);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Implementations of TileRenderer that color the clipped tiles in various ways
////////////////////////////////////////////////////////////////////////////////////////////////

class DebugTileRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make(SkCanvas* canvas, GrContext* context,
                                    GrRenderTargetContext* rtc) {
        // Since aa override is disabled, the quad flags arg doesn't matter.
        return sk_sp<TileRenderer>(new DebugTileRenderer(
                canvas, context, rtc, GrQuadAAFlags::kAll, false));
    }

    static sk_sp<TileRenderer> MakeAA(SkCanvas* canvas, GrContext* context,
                                      GrRenderTargetContext* rtc) {
        return sk_sp<TileRenderer>(new DebugTileRenderer(
                canvas, context, rtc, GrQuadAAFlags::kAll, true));
    }

    static sk_sp<TileRenderer> MakeNonAA(SkCanvas* canvas, GrContext* context,
                                         GrRenderTargetContext* rtc) {
        return sk_sp<TileRenderer>(new DebugTileRenderer(
                canvas, context, rtc, GrQuadAAFlags::kNone, true));
    }

    void drawTile(const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
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
        std::unique_ptr<GrDrawOp> op;
        if (clip) {
            op = GrFillRectOp::MakePerEdgeQuad(fContext, std::move(paint), this->getDefaultAAType(),
                                               aaFlags, fCanvas->getTotalMatrix(), clip, nullptr);
        } else {
            op = GrFillRectOp::MakePerEdge(fContext, std::move(paint), this->getDefaultAAType(),
                                           aaFlags, fCanvas->getTotalMatrix(), rect);
        }

        fRTC->priv().testingOnly_addDrawOp(std::move(op));
    }

    void drawBanner() override {
        fCanvas->save();
        draw_text(fCanvas, "Debug View");
        fCanvas->translate(0.f, 15.f);

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
        fCanvas->translate(0.f, 6.f);
        draw_text(fCanvas, config.c_str());
        fCanvas->restore();
    }

private:
    GrQuadAAFlags fAAOverride;
    bool fEnableAAOverride;

    DebugTileRenderer(SkCanvas* canvas, GrContext* context, GrRenderTargetContext* rtc,
                      GrQuadAAFlags aa, bool enableAAOverrde)
            : INHERITED(canvas, context, rtc)
            , fAAOverride(aa)
            , fEnableAAOverride(enableAAOverrde) {}

    typedef TileRenderer INHERITED;
};

class SolidColorRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make(SkCanvas* canvas, GrContext* context,
                                    GrRenderTargetContext* rtc, const SkPMColor4f& color) {
        return sk_sp<TileRenderer>(new SolidColorRenderer(canvas, context, rtc, color));
    }

    void drawTile(const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        GrPaint paint;
        paint.setColor4f(fColor);

        std::unique_ptr<GrDrawOp> op;
        if (clip) {
            op = GrFillRectOp::MakePerEdgeQuad(fContext, std::move(paint), this->getDefaultAAType(),
                                               this->maskToFlags(edgeAA), fCanvas->getTotalMatrix(),
                                               clip, nullptr);
        } else {
            op = GrFillRectOp::MakePerEdge(fContext, std::move(paint), this->getDefaultAAType(),
                                           this->maskToFlags(edgeAA), fCanvas->getTotalMatrix(),
                                           rect);
        }

        fRTC->priv().testingOnly_addDrawOp(std::move(op));
    }

    void drawBanner() override {
        draw_text(fCanvas, "Solid Color");
    }

private:
    SkPMColor4f fColor;

    SolidColorRenderer(SkCanvas* canvas, GrContext* context, GrRenderTargetContext* rtc,
                       const SkPMColor4f& color)
            : INHERITED(canvas, context, rtc)
            , fColor(color) {}

    typedef TileRenderer INHERITED;
};

class GradientRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make(SkCanvas* canvas, GrContext* context,
                                    GrRenderTargetContext* rtc, bool local) {
        return sk_sp<TileRenderer>(new GradientRenderer(canvas, context, rtc, local));
    }

    void drawTile(const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        GrPaint paint;
        SkPaintToGrPaint(fContext, fRTC->colorSpaceInfo(), fGradient, fCanvas->getTotalMatrix(),
                         &paint);

        std::unique_ptr<GrDrawOp> op;
        if (fLocal) {
            SkRect localRect = SkRect::MakeWH(kTileWidth, kTileHeight);
            if (clip) {
                SkMatrix toLocal = SkMatrix::MakeRectToRect(rect, localRect,
                                                            SkMatrix::kFill_ScaleToFit);
                SkPoint localQuad[4];
                toLocal.mapPoints(localQuad, clip, 4);

                op = GrFillRectOp::MakePerEdgeQuad(
                        fContext, std::move(paint), this->getDefaultAAType(),
                        this->maskToFlags(edgeAA), fCanvas->getTotalMatrix(),
                        clip, localQuad);
            } else {
                op = GrFillRectOp::MakePerEdgeWithLocalRect(
                        fContext, std::move(paint), this->getDefaultAAType(),
                        this->maskToFlags(edgeAA), fCanvas->getTotalMatrix(), rect, localRect);
            }
        } else {
            if (clip) {
                op = GrFillRectOp::MakePerEdgeQuad(
                        fContext, std::move(paint), this->getDefaultAAType(),
                        this->maskToFlags(edgeAA), fCanvas->getTotalMatrix(),
                        clip, nullptr);
            } else {
                op = GrFillRectOp::MakePerEdge(
                        fContext, std::move(paint), this->getDefaultAAType(),
                        this->maskToFlags(edgeAA), fCanvas->getTotalMatrix(), rect);
            }
        }

        fRTC->priv().testingOnly_addDrawOp(std::move(op));
    }

    void drawBanner() override {
        fCanvas->save();
        draw_text(fCanvas, "Gradient");
        fCanvas->translate(0.f, 15.f);
        if (fLocal) {
            draw_text(fCanvas, "Local");
        } else {
            draw_text(fCanvas, "Seamless");
        }
        fCanvas->restore();
    }

private:
    SkPaint fGradient;
    bool fLocal;

    GradientRenderer(SkCanvas* canvas, GrContext* context, GrRenderTargetContext* rtc, bool local)
            : INHERITED(canvas, context, rtc)
            , fLocal(local) {
        static constexpr SkPoint pts[] = { {0.f, 0.f}, {0.25f * kTileWidth, 0.25f * kTileHeight} };
        static constexpr SkColor colors[] = { SK_ColorBLUE, SK_ColorWHITE };
        auto gradient = SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                     SkShader::kMirror_TileMode);
        fGradient.setShader(gradient);
    }

    typedef TileRenderer INHERITED;
};

class TextureRenderer : public TileRenderer {
public:

    static sk_sp<TileRenderer> Make(SkCanvas* canvas, GrContext* context,
                                    GrRenderTargetContext* rtc, sk_sp<SkImage> image) {
        return sk_sp<TileRenderer>(new TextureRenderer(canvas, context, rtc, image));
    }

    void drawTile(const SkRect& rect, const SkPoint clip[4], const bool edgeAA[4],
                  int tileID, int quadID) override {
        // This acts like the whole image is rendered over the entire tile grid, so derive local
        // coordinates from 'rect', based on the grid to image transform.
        SkMatrix gridToImage = SkMatrix::MakeRectToRect(SkRect::MakeWH(kColCount * kTileWidth,
                                                                       kRowCount * kTileHeight),
                                                        SkRect::MakeWH(fImage->width(),
                                                                       fImage->height()),
                                                        SkMatrix::kFill_ScaleToFit);
        SkRect localRect = gridToImage.mapRect(rect);

        SkPMColor4f color = {1.f, 1.f, 1.f, 1.f};

        // uint32_t pinnedUniqueID;
        // sk_sp<GrTextureProxy> proxy = as_IB(fImage)->refPinnedTextureProxy(&pinnedUniqueID);
        fImage = fImage->makeTextureImage(fContext, nullptr);
        sk_sp<GrTextureProxy> proxy = as_IB(fImage)->asTextureProxyRef();
        SkASSERT(proxy);

        std::unique_ptr<GrDrawOp> op;
        if (clip) {
            SkMatrix toLocal = SkMatrix::MakeRectToRect(rect, localRect,
                                                        SkMatrix::kFill_ScaleToFit);
            SkPoint localQuad[4];
            toLocal.mapPoints(localQuad, clip, 4);

            op = GrTextureOp::MakeQuad(fContext, std::move(proxy), GrSamplerState::Filter::kBilerp,
                    color, localQuad, clip, this->getDefaultAAType(), this->maskToFlags(edgeAA),
                    nullptr, fCanvas->getTotalMatrix(), nullptr);
        } else {
            op = GrTextureOp::Make(fContext, std::move(proxy), GrSamplerState::Filter::kBilerp,
                    color, localRect, rect, this->getDefaultAAType(), this->maskToFlags(edgeAA),
                    SkCanvas::kFast_SrcRectConstraint, fCanvas->getTotalMatrix(), nullptr);
        }

        fRTC->priv().testingOnly_addDrawOp(std::move(op));
    }

    void drawBanner() override {
        draw_text(fCanvas, "Texture");
    }

private:
    sk_sp<SkImage> fImage;

    TextureRenderer(SkCanvas* canvas, GrContext* context, GrRenderTargetContext* rtc,
                    sk_sp<SkImage> image)
            : INHERITED(canvas, context, rtc)
            , fImage(image) {}

    typedef TileRenderer INHERITED;
};

DEF_SIMPLE_GPU_GM(compositor_quads, ctx, rtc, canvas, 800, 800) {
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

    SkTArray<sk_sp<TileRenderer>> renderers;
    // Should see smooth edges on the outer edge of tile grid, non-AA edges on the interior
    // without any seams
    renderers.push_back(DebugTileRenderer::Make(canvas, ctx, rtc));
    // Every edge is AA'ed, so should see small seams at interior quad edges, for explicit and
    // implicit splits
    renderers.push_back(DebugTileRenderer::MakeAA(canvas, ctx, rtc));
    // No edge is AA'ed, so hard outer edge and no interior seams or gaps
    renderers.push_back(DebugTileRenderer::MakeNonAA(canvas, ctx, rtc));
    // Solid color that should have smooth outer edge and uniform color interior
    renderers.push_back(SolidColorRenderer::Make(canvas, ctx, rtc, {.2f, .8f, .3f, 1.f}));
    // Seamless gradient across the entire tile grid
    renderers.push_back(GradientRenderer::Make(canvas, ctx, rtc, /* local */ false));
    // Gradient per tile, but seamless within the arbitrary splits in a tile
    renderers.push_back(GradientRenderer::Make(canvas, ctx, rtc, /* local */ true));
    // Leverage GrTextureOp too
    renderers.push_back(TextureRenderer::Make(canvas, ctx, rtc,
                                              GetResourceAsImage("images/mandrill_512.png")));

    // Print a column header
    canvas->save();
    canvas->translate(110.f, 20.f);
    for (int j = 0; j < renderers.count(); ++j) {
        renderers[j]->drawBanner();
        canvas->translate(kColCount * kTileWidth + 30.f, 0.f);
    }
    canvas->restore();
    canvas->translate(0.f, 80.f);

    for (size_t i = 0; i < SK_ARRAY_COUNT(rowMatrices); ++i) {
        canvas->save();
        canvas->translate(10.f, 0.5f * kRowCount * kTileHeight);
        draw_text(canvas, matrixNames[i]);

        canvas->translate(100.f, -0.5f * kRowCount * kTileHeight);
        for (int j = 0; j < renderers.count(); ++j) {
            canvas->save();
            draw_tile_boundaries(canvas, rowMatrices[i]);
            draw_clipping_boundaries(canvas, rowMatrices[i]);

            canvas->concat(rowMatrices[i]);
            renderers[j]->drawTiles();

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

#endif // SK_SUPPORT_GPU
