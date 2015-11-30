/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashingEffect.h"

#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrCaps.h"
#include "GrGeometryProcessor.h"
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawTarget.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "GrStrokeInfo.h"
#include "GrVertexBuffer.h"
#include "SkGr.h"
#include "batches/GrVertexBatch.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

///////////////////////////////////////////////////////////////////////////////

// Returns whether or not the gpu can fast path the dash line effect.
bool GrDashingEffect::CanDrawDashLine(const SkPoint pts[2], const GrStrokeInfo& strokeInfo,
                                      const SkMatrix& viewMatrix) {
    // Pts must be either horizontal or vertical in src space
    if (pts[0].fX != pts[1].fX && pts[0].fY != pts[1].fY) {
        return false;
    }

    // May be able to relax this to include skew. As of now cannot do perspective
    // because of the non uniform scaling of bloating a rect
    if (!viewMatrix.preservesRightAngles()) {
        return false;
    }

    if (!strokeInfo.isDashed() || 2 != strokeInfo.getDashCount()) {
        return false;
    }

    const SkScalar* intervals = strokeInfo.getDashIntervals();
    if (0 == intervals[0] && 0 == intervals[1]) {
        return false;
    }

    SkPaint::Cap cap = strokeInfo.getCap();
    // Current we do don't handle Round or Square cap dashes
    if (SkPaint::kRound_Cap == cap && intervals[0] != 0.f) {
        return false;
    }

    return true;
}

namespace {
struct DashLineVertex {
    SkPoint fPos;
    SkPoint fDashPos;
    SkScalar fIntervalLength;
    SkRect fRect;
};
struct DashCircleVertex {
    SkPoint fPos;
    SkPoint fDashPos;
    SkScalar fIntervalLength;
    SkScalar fRadius;
    SkScalar fCenterX;
};

enum DashAAMode {
    kBW_DashAAMode,
    kEdgeAA_DashAAMode,
    kMSAA_DashAAMode,

    kDashAAModeCount,
};
};

static void calc_dash_scaling(SkScalar* parallelScale, SkScalar* perpScale,
                            const SkMatrix& viewMatrix, const SkPoint pts[2]) {
    SkVector vecSrc = pts[1] - pts[0];
    SkScalar magSrc = vecSrc.length();
    SkScalar invSrc = magSrc ? SkScalarInvert(magSrc) : 0;
    vecSrc.scale(invSrc);

    SkVector vecSrcPerp;
    vecSrc.rotateCW(&vecSrcPerp);
    viewMatrix.mapVectors(&vecSrc, 1);
    viewMatrix.mapVectors(&vecSrcPerp, 1);

    // parallelScale tells how much to scale along the line parallel to the dash line
    // perpScale tells how much to scale in the direction perpendicular to the dash line
    *parallelScale = vecSrc.length();
    *perpScale = vecSrcPerp.length();
}

// calculates the rotation needed to aligned pts to the x axis with pts[0] < pts[1]
// Stores the rotation matrix in rotMatrix, and the mapped points in ptsRot
static void align_to_x_axis(const SkPoint pts[2], SkMatrix* rotMatrix, SkPoint ptsRot[2] = nullptr) {
    SkVector vec = pts[1] - pts[0];
    SkScalar mag = vec.length();
    SkScalar inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    rotMatrix->setSinCos(-vec.fY, vec.fX, pts[0].fX, pts[0].fY);
    if (ptsRot) {
        rotMatrix->mapPoints(ptsRot, pts, 2);
        // correction for numerical issues if map doesn't make ptsRot exactly horizontal
        ptsRot[1].fY = pts[0].fY;
    }
}

// Assumes phase < sum of all intervals
static SkScalar calc_start_adjustment(const SkScalar intervals[2], SkScalar phase) {
    SkASSERT(phase < intervals[0] + intervals[1]);
    if (phase >= intervals[0] && phase != 0) {
        SkScalar srcIntervalLen = intervals[0] + intervals[1];
        return srcIntervalLen - phase;
    }
    return 0;
}

static SkScalar calc_end_adjustment(const SkScalar intervals[2], const SkPoint pts[2],
                                    SkScalar phase, SkScalar* endingInt) {
    if (pts[1].fX <= pts[0].fX) {
        return 0;
    }
    SkScalar srcIntervalLen = intervals[0] + intervals[1];
    SkScalar totalLen = pts[1].fX - pts[0].fX;
    SkScalar temp = totalLen / srcIntervalLen;
    SkScalar numFullIntervals = SkScalarFloorToScalar(temp);
    *endingInt = totalLen - numFullIntervals * srcIntervalLen + phase;
    temp = *endingInt / srcIntervalLen;
    *endingInt = *endingInt - SkScalarFloorToScalar(temp) * srcIntervalLen;
    if (0 == *endingInt) {
        *endingInt = srcIntervalLen;
    }
    if (*endingInt > intervals[0]) {
        if (0 == intervals[0]) {
            *endingInt -= 0.01f; // make sure we capture the last zero size pnt (used if has caps)
        }
        return *endingInt - intervals[0];
    }
    return 0;
}

enum DashCap {
    kRound_DashCap,
    kNonRound_DashCap,
};

static int kDashVertices = 4;

template <typename T>
void setup_dashed_rect_common(const SkRect& rect, const SkMatrix& matrix, T* vertices, int idx,
                              SkScalar offset, SkScalar bloatX, SkScalar bloatY, SkScalar len,
                              SkScalar stroke) {
    SkScalar startDashX = offset - bloatX;
    SkScalar endDashX = offset + len + bloatX;
    SkScalar startDashY = -stroke - bloatY;
    SkScalar endDashY = stroke + bloatY;
    vertices[idx].fDashPos = SkPoint::Make(startDashX , startDashY);
    vertices[idx + 1].fDashPos = SkPoint::Make(startDashX, endDashY);
    vertices[idx + 2].fDashPos = SkPoint::Make(endDashX, endDashY);
    vertices[idx + 3].fDashPos = SkPoint::Make(endDashX, startDashY);

    vertices[idx].fPos = SkPoint::Make(rect.fLeft, rect.fTop);
    vertices[idx + 1].fPos = SkPoint::Make(rect.fLeft, rect.fBottom);
    vertices[idx + 2].fPos = SkPoint::Make(rect.fRight, rect.fBottom);
    vertices[idx + 3].fPos = SkPoint::Make(rect.fRight, rect.fTop);

    matrix.mapPointsWithStride(&vertices[idx].fPos, sizeof(T), 4);
}

static void setup_dashed_rect(const SkRect& rect, void* vertices, int idx,
                              const SkMatrix& matrix, SkScalar offset, SkScalar bloatX,
                              SkScalar bloatY, SkScalar len, SkScalar stroke,
                              SkScalar startInterval, SkScalar endInterval, SkScalar strokeWidth,
                              DashCap cap, const size_t vertexStride) {
    SkScalar intervalLength = startInterval + endInterval;

    if (kRound_DashCap == cap) {
        SkASSERT(vertexStride == sizeof(DashCircleVertex));
        DashCircleVertex* verts = reinterpret_cast<DashCircleVertex*>(vertices);

        setup_dashed_rect_common<DashCircleVertex>(rect, matrix, verts, idx, offset, bloatX,
                                                   bloatY, len, stroke);

        SkScalar radius = SkScalarHalf(strokeWidth) - 0.5f;
        SkScalar centerX = SkScalarHalf(endInterval);

        for (int i = 0; i < kDashVertices; i++) {
            verts[idx + i].fIntervalLength = intervalLength;
            verts[idx + i].fRadius = radius;
            verts[idx + i].fCenterX = centerX;
        }

    } else {
        SkASSERT(kNonRound_DashCap == cap && vertexStride == sizeof(DashLineVertex));
        DashLineVertex* verts = reinterpret_cast<DashLineVertex*>(vertices);

        setup_dashed_rect_common<DashLineVertex>(rect, matrix, verts, idx, offset, bloatX,
                                                 bloatY, len, stroke);

        SkScalar halfOffLen = SkScalarHalf(endInterval);
        SkScalar halfStroke = SkScalarHalf(strokeWidth);
        SkRect rectParam;
        rectParam.set(halfOffLen + 0.5f, -halfStroke + 0.5f,
                      halfOffLen + startInterval - 0.5f, halfStroke - 0.5f);
        for (int i = 0; i < kDashVertices; i++) {
            verts[idx + i].fIntervalLength = intervalLength;
            verts[idx + i].fRect = rectParam;
        }
    }
}

static void setup_dashed_rect_pos(const SkRect& rect, int idx, const SkMatrix& matrix,
                                  SkPoint* verts) {
    verts[idx] = SkPoint::Make(rect.fLeft, rect.fTop);
    verts[idx + 1] = SkPoint::Make(rect.fLeft, rect.fBottom);
    verts[idx + 2] = SkPoint::Make(rect.fRight, rect.fBottom);
    verts[idx + 3] = SkPoint::Make(rect.fRight, rect.fTop);
    matrix.mapPoints(&verts[idx], 4);
}


/**
 * An GrGeometryProcessor that renders a dashed line.
 * This GrGeometryProcessor is meant for dashed lines that only have a single on/off interval pair.
 * Bounding geometry is rendered and the effect computes coverage based on the fragment's
 * position relative to the dashed line.
 */
static GrGeometryProcessor* create_dash_gp(GrColor,
                                           DashAAMode aaMode,
                                           DashCap cap,
                                           const SkMatrix& localMatrix,
                                           bool usesLocalCoords);

class DashBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkMatrix fViewMatrix;
        SkMatrix fSrcRotInv;
        SkPoint fPtsRot[2];
        SkScalar fSrcStrokeWidth;
        SkScalar fPhase;
        SkScalar fIntervals[2];
        SkScalar fParallelScale;
        SkScalar fPerpendicularScale;
        GrColor fColor;
    };

    static GrDrawBatch* Create(const Geometry& geometry, SkPaint::Cap cap, DashAAMode aaMode,
                               bool fullDash) {
        return new DashBatch(geometry, cap, aaMode, fullDash);
    }

    const char* name() const override { return "DashBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color, 
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
        overrides->fUsePLSDstRead = false;
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    DashBatch(const Geometry& geometry, SkPaint::Cap cap, DashAAMode aaMode, bool fullDash)
        : INHERITED(ClassID()) {
        fGeoData.push_back(geometry);

        fBatch.fAAMode = aaMode;
        fBatch.fCap = cap;
        fBatch.fFullDash = fullDash;

        // compute bounds
        SkScalar halfStrokeWidth = 0.5f * geometry.fSrcStrokeWidth;
        SkScalar xBloat = SkPaint::kButt_Cap == cap ? 0 : halfStrokeWidth;
        fBounds.set(geometry.fPtsRot[0], geometry.fPtsRot[1]);
        fBounds.outset(xBloat, halfStrokeWidth);

        // Note, we actually create the combined matrix here, and save the work
        SkMatrix& combinedMatrix = fGeoData[0].fSrcRotInv;
        combinedMatrix.postConcat(geometry.fViewMatrix);
        combinedMatrix.mapRect(&fBounds);
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);

        // setup batch properties
        fBatch.fColorIgnored = !overrides.readsColor();
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = overrides.readsLocalCoords();
        fBatch.fCoverageIgnored = !overrides.readsCoverage();
    }

    struct DashDraw {
        DashDraw(const Geometry& geo) {
            memcpy(fPtsRot, geo.fPtsRot, sizeof(geo.fPtsRot));
            memcpy(fIntervals, geo.fIntervals, sizeof(geo.fIntervals));
            fPhase = geo.fPhase;
        }
        SkPoint fPtsRot[2];
        SkScalar fIntervals[2];
        SkScalar fPhase;
        SkScalar fStartOffset;
        SkScalar fStrokeWidth;
        SkScalar fLineLength;
        SkScalar fHalfDevStroke;
        SkScalar fDevBloatX;
        SkScalar fDevBloatY;
        bool fLineDone;
        bool fHasStartRect;
        bool fHasEndRect;
    };

    void onPrepareDraws(Target* target) const override {
        int instanceCount = fGeoData.count();
        SkPaint::Cap cap = this->cap();
        bool isRoundCap = SkPaint::kRound_Cap == cap;
        DashCap capType = isRoundCap ? kRound_DashCap : kNonRound_DashCap;

        SkAutoTUnref<const GrGeometryProcessor> gp;
        if (this->fullDash()) {
            gp.reset(create_dash_gp(this->color(), this->aaMode(), capType, this->viewMatrix(),
                                    this->usesLocalCoords()));
        } else {
            // Set up the vertex data for the line and start/end dashes
            using namespace GrDefaultGeoProcFactory;
            Color color(this->color());
            Coverage coverage(this->coverageIgnored() ? Coverage::kNone_Type :
                                                        Coverage::kSolid_Type);
            LocalCoords localCoords(this->usesLocalCoords() ? LocalCoords::kUsePosition_Type :
                                                              LocalCoords::kUnused_Type);
            gp.reset(CreateForDeviceSpace(color, coverage, localCoords, this->viewMatrix()));
        }

        if (!gp) {
            SkDebugf("Could not create GrGeometryProcessor\n");
            return;
        }

        target->initDraw(gp, this->pipeline());

        // useAA here means Edge AA or MSAA
        bool useAA = this->aaMode() != kBW_DashAAMode;
        bool fullDash = this->fullDash();

        // We do two passes over all of the dashes.  First we setup the start, end, and bounds,
        // rectangles.  We preserve all of this work in the rects / draws arrays below.  Then we
        // iterate again over these decomposed dashes to generate vertices
        static const int kNumStackDashes = 128;
        SkSTArray<kNumStackDashes, SkRect, true> rects;
        SkSTArray<kNumStackDashes, DashDraw, true> draws;

        int totalRectCount = 0;
        int rectOffset = 0;
        rects.push_back_n(3 * instanceCount);
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            DashDraw& draw = draws.push_back(args);

            bool hasCap = SkPaint::kButt_Cap != cap && 0 != args.fSrcStrokeWidth;

            // We always want to at least stroke out half a pixel on each side in device space
            // so 0.5f / perpScale gives us this min in src space
            SkScalar halfSrcStroke =
                    SkMaxScalar(args.fSrcStrokeWidth * 0.5f, 0.5f / args.fPerpendicularScale);

            SkScalar strokeAdj;
            if (!hasCap) {
                strokeAdj = 0.f;
            } else {
                strokeAdj = halfSrcStroke;
            }

            SkScalar startAdj = 0;

            bool lineDone = false;

            // Too simplify the algorithm, we always push back rects for start and end rect.
            // Otherwise we'd have to track start / end rects for each individual geometry
            SkRect& bounds = rects[rectOffset++];
            SkRect& startRect = rects[rectOffset++];
            SkRect& endRect = rects[rectOffset++];

            bool hasStartRect = false;
            // If we are using AA, check to see if we are drawing a partial dash at the start. If so
            // draw it separately here and adjust our start point accordingly
            if (useAA) {
                if (draw.fPhase > 0 && draw.fPhase < draw.fIntervals[0]) {
                    SkPoint startPts[2];
                    startPts[0] = draw.fPtsRot[0];
                    startPts[1].fY = startPts[0].fY;
                    startPts[1].fX = SkMinScalar(startPts[0].fX + draw.fIntervals[0] - draw.fPhase,
                                                 draw.fPtsRot[1].fX);
                    startRect.set(startPts, 2);
                    startRect.outset(strokeAdj, halfSrcStroke);

                    hasStartRect = true;
                    startAdj = draw.fIntervals[0] + draw.fIntervals[1] - draw.fPhase;
                }
            }

            // adjustments for start and end of bounding rect so we only draw dash intervals
            // contained in the original line segment.
            startAdj += calc_start_adjustment(draw.fIntervals, draw.fPhase);
            if (startAdj != 0) {
                draw.fPtsRot[0].fX += startAdj;
                draw.fPhase = 0;
            }
            SkScalar endingInterval = 0;
            SkScalar endAdj = calc_end_adjustment(draw.fIntervals, draw.fPtsRot, draw.fPhase,
                                                  &endingInterval);
            draw.fPtsRot[1].fX -= endAdj;
            if (draw.fPtsRot[0].fX >= draw.fPtsRot[1].fX) {
                lineDone = true;
            }

            bool hasEndRect = false;
            // If we are using AA, check to see if we are drawing a partial dash at then end. If so
            // draw it separately here and adjust our end point accordingly
            if (useAA && !lineDone) {
                // If we adjusted the end then we will not be drawing a partial dash at the end.
                // If we didn't adjust the end point then we just need to make sure the ending
                // dash isn't a full dash
                if (0 == endAdj && endingInterval != draw.fIntervals[0]) {
                    SkPoint endPts[2];
                    endPts[1] = draw.fPtsRot[1];
                    endPts[0].fY = endPts[1].fY;
                    endPts[0].fX = endPts[1].fX - endingInterval;

                    endRect.set(endPts, 2);
                    endRect.outset(strokeAdj, halfSrcStroke);

                    hasEndRect = true;
                    endAdj = endingInterval + draw.fIntervals[1];

                    draw.fPtsRot[1].fX -= endAdj;
                    if (draw.fPtsRot[0].fX >= draw.fPtsRot[1].fX) {
                        lineDone = true;
                    }
                }
            }

            if (startAdj != 0) {
                draw.fPhase = 0;
            }

            // Change the dashing info from src space into device space
            SkScalar* devIntervals = draw.fIntervals;
            devIntervals[0] = draw.fIntervals[0] * args.fParallelScale;
            devIntervals[1] = draw.fIntervals[1] * args.fParallelScale;
            SkScalar devPhase = draw.fPhase * args.fParallelScale;
            SkScalar strokeWidth = args.fSrcStrokeWidth * args.fPerpendicularScale;

            if ((strokeWidth < 1.f && useAA) || 0.f == strokeWidth) {
                strokeWidth = 1.f;
            }

            SkScalar halfDevStroke = strokeWidth * 0.5f;

            if (SkPaint::kSquare_Cap == cap && 0 != args.fSrcStrokeWidth) {
                // add cap to on interval and remove from off interval
                devIntervals[0] += strokeWidth;
                devIntervals[1] -= strokeWidth;
            }
            SkScalar startOffset = devIntervals[1] * 0.5f + devPhase;

            // For EdgeAA, we bloat in X & Y for both square and round caps.
            // For MSAA, we don't bloat at all for square caps, and bloat in Y only for round caps.
            SkScalar devBloatX = this->aaMode() == kEdgeAA_DashAAMode ? 0.5f : 0.0f;
            SkScalar devBloatY = (SkPaint::kRound_Cap == cap && this->aaMode() == kMSAA_DashAAMode)
                                 ? 0.5f : devBloatX;

            SkScalar bloatX = devBloatX / args.fParallelScale;
            SkScalar bloatY = devBloatY / args.fPerpendicularScale;

            if (devIntervals[1] <= 0.f && useAA) {
                // Case when we end up drawing a solid AA rect
                // Reset the start rect to draw this single solid rect
                // but it requires to upload a new intervals uniform so we can mimic
                // one giant dash
                draw.fPtsRot[0].fX -= hasStartRect ? startAdj : 0;
                draw.fPtsRot[1].fX += hasEndRect ? endAdj : 0;
                startRect.set(draw.fPtsRot, 2);
                startRect.outset(strokeAdj, halfSrcStroke);
                hasStartRect = true;
                hasEndRect = false;
                lineDone = true;

                SkPoint devicePts[2];
                args.fViewMatrix.mapPoints(devicePts, draw.fPtsRot, 2);
                SkScalar lineLength = SkPoint::Distance(devicePts[0], devicePts[1]);
                if (hasCap) {
                    lineLength += 2.f * halfDevStroke;
                }
                devIntervals[0] = lineLength;
            }

            totalRectCount += !lineDone ? 1 : 0;
            totalRectCount += hasStartRect ? 1 : 0;
            totalRectCount += hasEndRect ? 1 : 0;

            if (SkPaint::kRound_Cap == cap && 0 != args.fSrcStrokeWidth) {
                // need to adjust this for round caps to correctly set the dashPos attrib on
                // vertices
                startOffset -= halfDevStroke;
            }

            if (!lineDone) {
                SkPoint devicePts[2];
                args.fViewMatrix.mapPoints(devicePts, draw.fPtsRot, 2);
                draw.fLineLength = SkPoint::Distance(devicePts[0], devicePts[1]);
                if (hasCap) {
                    draw.fLineLength += 2.f * halfDevStroke;
                }

                bounds.set(draw.fPtsRot[0].fX, draw.fPtsRot[0].fY,
                           draw.fPtsRot[1].fX, draw.fPtsRot[1].fY);
                bounds.outset(bloatX + strokeAdj, bloatY + halfSrcStroke);
            }

            if (hasStartRect) {
                SkASSERT(useAA);  // so that we know bloatX and bloatY have been set
                startRect.outset(bloatX, bloatY);
            }

            if (hasEndRect) {
                SkASSERT(useAA);  // so that we know bloatX and bloatY have been set
                endRect.outset(bloatX, bloatY);
            }

            draw.fStartOffset = startOffset;
            draw.fDevBloatX = devBloatX;
            draw.fDevBloatY = devBloatY;
            draw.fHalfDevStroke = halfDevStroke;
            draw.fStrokeWidth = strokeWidth;
            draw.fHasStartRect = hasStartRect;
            draw.fLineDone = lineDone;
            draw.fHasEndRect = hasEndRect;
        }

        if (!totalRectCount) {
            return;
        }

        QuadHelper helper;
        void* vertices = helper.init(target, gp->getVertexStride(), totalRectCount);
        if (!vertices) {
            return;
        }

        int curVIdx = 0;
        int rectIndex = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& geom = fGeoData[i];

            if (!draws[i].fLineDone) {
                if (fullDash) {
                    setup_dashed_rect(rects[rectIndex], vertices, curVIdx, geom.fSrcRotInv,
                                      draws[i].fStartOffset, draws[i].fDevBloatX,
                                      draws[i].fDevBloatY, draws[i].fLineLength,
                                      draws[i].fHalfDevStroke, draws[i].fIntervals[0],
                                      draws[i].fIntervals[1], draws[i].fStrokeWidth,
                                      capType, gp->getVertexStride());
                } else {
                    SkPoint* verts = reinterpret_cast<SkPoint*>(vertices);
                    SkASSERT(gp->getVertexStride() == sizeof(SkPoint));
                    setup_dashed_rect_pos(rects[rectIndex], curVIdx, geom.fSrcRotInv, verts);
                }
                curVIdx += 4;
            }
            rectIndex++;

            if (draws[i].fHasStartRect) {
                if (fullDash) {
                    setup_dashed_rect(rects[rectIndex], vertices, curVIdx, geom.fSrcRotInv,
                                      draws[i].fStartOffset, draws[i].fDevBloatX,
                                      draws[i].fDevBloatY, draws[i].fIntervals[0],
                                      draws[i].fHalfDevStroke, draws[i].fIntervals[0],
                                      draws[i].fIntervals[1], draws[i].fStrokeWidth, capType,
                                      gp->getVertexStride());
                } else {
                    SkPoint* verts = reinterpret_cast<SkPoint*>(vertices);
                    SkASSERT(gp->getVertexStride() == sizeof(SkPoint));
                    setup_dashed_rect_pos(rects[rectIndex], curVIdx, geom.fSrcRotInv, verts);
                }
                curVIdx += 4;
            }
            rectIndex++;

            if (draws[i].fHasEndRect) {
                if (fullDash) {
                    setup_dashed_rect(rects[rectIndex], vertices, curVIdx, geom.fSrcRotInv,
                                      draws[i].fStartOffset, draws[i].fDevBloatX,
                                      draws[i].fDevBloatY, draws[i].fIntervals[0],
                                      draws[i].fHalfDevStroke, draws[i].fIntervals[0],
                                      draws[i].fIntervals[1], draws[i].fStrokeWidth, capType,
                                      gp->getVertexStride());
                } else {
                    SkPoint* verts = reinterpret_cast<SkPoint*>(vertices);
                    SkASSERT(gp->getVertexStride() == sizeof(SkPoint));
                    setup_dashed_rect_pos(rects[rectIndex], curVIdx, geom.fSrcRotInv, verts);
                }
                curVIdx += 4;
            }
            rectIndex++;
        }
        SkASSERT(0 == (curVIdx % 4) && (curVIdx / 4) == totalRectCount);
        helper.recordDraw(target);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        DashBatch* that = t->cast<DashBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (this->aaMode() != that->aaMode()) {
            return false;
        }

        if (this->fullDash() != that->fullDash()) {
            return false;
        }

        if (this->cap() != that->cap()) {
            return false;
        }

        // TODO vertex color
        if (this->color() != that->color()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    DashAAMode aaMode() const { return fBatch.fAAMode; }
    bool fullDash() const { return fBatch.fFullDash; }
    SkPaint::Cap cap() const { return fBatch.fCap; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        SkPaint::Cap fCap;
        DashAAMode fAAMode;
        bool fFullDash;
    };

    static const int kVertsPerDash = 4;
    static const int kIndicesPerDash = 6;

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

static GrDrawBatch* create_batch(GrColor color, const SkMatrix& viewMatrix, const SkPoint pts[2],
                                 bool useAA, const GrStrokeInfo& strokeInfo, bool msaaRT) {
    const SkScalar* intervals = strokeInfo.getDashIntervals();
    SkScalar phase = strokeInfo.getDashPhase();

    SkPaint::Cap cap = strokeInfo.getCap();

    DashBatch::Geometry geometry;
    geometry.fSrcStrokeWidth = strokeInfo.getWidth();

    // the phase should be normalized to be [0, sum of all intervals)
    SkASSERT(phase >= 0 && phase < intervals[0] + intervals[1]);

    // Rotate the src pts so they are aligned horizontally with pts[0].fX < pts[1].fX
    if (pts[0].fY != pts[1].fY || pts[0].fX > pts[1].fX) {
        SkMatrix rotMatrix;
        align_to_x_axis(pts, &rotMatrix, geometry.fPtsRot);
        if(!rotMatrix.invert(&geometry.fSrcRotInv)) {
            SkDebugf("Failed to create invertible rotation matrix!\n");
            return nullptr;
        }
    } else {
        geometry.fSrcRotInv.reset();
        memcpy(geometry.fPtsRot, pts, 2 * sizeof(SkPoint));
    }

    // Scale corrections of intervals and stroke from view matrix
    calc_dash_scaling(&geometry.fParallelScale, &geometry.fPerpendicularScale, viewMatrix,
                      geometry.fPtsRot);

    SkScalar offInterval = intervals[1] * geometry.fParallelScale;
    SkScalar strokeWidth = geometry.fSrcStrokeWidth * geometry.fPerpendicularScale;

    if (SkPaint::kSquare_Cap == cap && 0 != geometry.fSrcStrokeWidth) {
        // add cap to on interveal and remove from off interval
        offInterval -= strokeWidth;
    }

    DashAAMode aaMode = msaaRT ? kMSAA_DashAAMode :
                                 useAA ? kEdgeAA_DashAAMode : kBW_DashAAMode;

    // TODO we can do a real rect call if not using fulldash(ie no off interval, not using AA)
    bool fullDash = offInterval > 0.f || aaMode != kBW_DashAAMode;

    geometry.fColor = color;
    geometry.fViewMatrix = viewMatrix;
    geometry.fPhase = phase;
    geometry.fIntervals[0] = intervals[0];
    geometry.fIntervals[1] = intervals[1];

    return DashBatch::Create(geometry, cap, aaMode, fullDash);
}

bool GrDashingEffect::DrawDashLine(GrDrawTarget* target,
                                   const GrPipelineBuilder& pipelineBuilder, GrColor color,
                                   const SkMatrix& viewMatrix, const SkPoint pts[2],
                                   bool useAA, const GrStrokeInfo& strokeInfo) {
    SkAutoTUnref<GrDrawBatch> batch(
            create_batch(color, viewMatrix, pts, useAA, strokeInfo,
                         pipelineBuilder.getRenderTarget()->isUnifiedMultisampled()));
    if (!batch) {
        return false;
    }

    target->drawBatch(pipelineBuilder, batch);
    return true;
}

//////////////////////////////////////////////////////////////////////////////

class GLDashingCircleEffect;

/*
 * This effect will draw a dotted line (defined as a dashed lined with round caps and no on
 * interval). The radius of the dots is given by the strokeWidth and the spacing by the DashInfo.
 * Both of the previous two parameters are in device space. This effect also requires the setting of
 * a vec2 vertex attribute for the the four corners of the bounding rect. This attribute is the
 * "dash position" of each vertex. In other words it is the vertex coords (in device space) if we
 * transform the line to be horizontal, with the start of line at the origin then shifted to the
 * right by half the off interval. The line then goes in the positive x direction.
 */
class DashingCircleEffect : public GrGeometryProcessor {
public:
    typedef SkPathEffect::DashInfo DashInfo;

    static GrGeometryProcessor* Create(GrColor,
                                       DashAAMode aaMode,
                                       const SkMatrix& localMatrix,
                                       bool usesLocalCoords);

    const char* name() const override { return "DashingCircleEffect"; }

    const Attribute* inPosition() const { return fInPosition; }

    const Attribute* inDashParams() const { return fInDashParams; }

    const Attribute* inCircleParams() const { return fInCircleParams; }

    DashAAMode aaMode() const { return fAAMode; }

    GrColor color() const { return fColor; }

    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }

    const SkMatrix& localMatrix() const { return fLocalMatrix; }

    bool usesLocalCoords() const { return fUsesLocalCoords; }

    void getGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override;

private:
    DashingCircleEffect(GrColor, DashAAMode aaMode, const SkMatrix& localMatrix,
                        bool usesLocalCoords);

    GrColor             fColor;
    SkMatrix            fLocalMatrix;
    bool                fUsesLocalCoords;
    DashAAMode          fAAMode;
    const Attribute*    fInPosition;
    const Attribute*    fInDashParams;
    const Attribute*    fInCircleParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLDashingCircleEffect : public GrGLSLGeometryProcessor {
public:
    GLDashingCircleEffect();

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&) override;

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLSLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<DashingCircleEffect>(primProc, pdman, index, transforms);
    }

private:
    UniformHandle fParamUniform;
    UniformHandle fColorUniform;
    GrColor       fColor;
    SkScalar      fPrevRadius;
    SkScalar      fPrevCenterX;
    SkScalar      fPrevIntervalLength;
    typedef GrGLSLGeometryProcessor INHERITED;
};

GLDashingCircleEffect::GLDashingCircleEffect() {
    fColor = GrColor_ILLEGAL;
    fPrevRadius = SK_ScalarMin;
    fPrevCenterX = SK_ScalarMin;
    fPrevIntervalLength = SK_ScalarMax;
}

void GLDashingCircleEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const DashingCircleEffect& dce = args.fGP.cast<DashingCircleEffect>();
    GrGLSLGPBuilder* pb = args.fPB;
    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;

    // emit attributes
    varyingHandler->emitAttributes(dce);

    // XY are dashPos, Z is dashInterval
    GrGLSLVertToFrag dashParams(kVec3f_GrSLType);
    varyingHandler->addVarying("DashParam", &dashParams);
    vertBuilder->codeAppendf("%s = %s;", dashParams.vsOut(), dce.inDashParams()->fName);

    // x refers to circle radius - 0.5, y refers to cicle's center x coord
    GrGLSLVertToFrag circleParams(kVec2f_GrSLType);
    varyingHandler->addVarying("CircleParams", &circleParams);
    vertBuilder->codeAppendf("%s = %s;", circleParams.vsOut(), dce.inCircleParams()->fName);

    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    if (!dce.colorIgnored()) {
        this->setupUniformColor(pb, fragBuilder, args.fOutputColor, &fColorUniform);
    }

    // Setup position
    this->setupPosition(pb, vertBuilder, gpArgs, dce.inPosition()->fName);

    // emit transforms
    this->emitTransforms(args.fPB,
                         vertBuilder,
                         varyingHandler,
                         gpArgs->fPositionVar,
                         dce.inPosition()->fName,
                         dce.localMatrix(),
                         args.fTransformsIn,
                         args.fTransformsOut);

    // transforms all points so that we can compare them to our test circle
    fragBuilder->codeAppendf("float xShifted = %s.x - floor(%s.x / %s.z) * %s.z;",
                             dashParams.fsIn(), dashParams.fsIn(), dashParams.fsIn(),
                             dashParams.fsIn());
    fragBuilder->codeAppendf("vec2 fragPosShifted = vec2(xShifted, %s.y);", dashParams.fsIn());
    fragBuilder->codeAppendf("vec2 center = vec2(%s.y, 0.0);", circleParams.fsIn());
    fragBuilder->codeAppend("float dist = length(center - fragPosShifted);");
    if (dce.aaMode() != kBW_DashAAMode) {
        fragBuilder->codeAppendf("float diff = dist - %s.x;", circleParams.fsIn());
        fragBuilder->codeAppend("diff = 1.0 - diff;");
        fragBuilder->codeAppend("float alpha = clamp(diff, 0.0, 1.0);");
    } else {
        fragBuilder->codeAppendf("float alpha = 1.0;");
        fragBuilder->codeAppendf("alpha *=  dist < %s.x + 0.5 ? 1.0 : 0.0;", circleParams.fsIn());
    }
    fragBuilder->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
}

void GLDashingCircleEffect::setData(const GrGLSLProgramDataManager& pdman,
                                    const GrPrimitiveProcessor& processor) {
    const DashingCircleEffect& dce = processor.cast<DashingCircleEffect>();
    if (dce.color() != fColor) {
        float c[4];
        GrColorToRGBAFloat(dce.color(), c);
        pdman.set4fv(fColorUniform, 1, c);
        fColor = dce.color();
    }
}

void GLDashingCircleEffect::GenKey(const GrGeometryProcessor& gp,
                                   const GrGLSLCaps&,
                                   GrProcessorKeyBuilder* b) {
    const DashingCircleEffect& dce = gp.cast<DashingCircleEffect>();
    uint32_t key = 0;
    key |= dce.usesLocalCoords() && dce.localMatrix().hasPerspective() ? 0x1 : 0x0;
    key |= dce.colorIgnored() ? 0x2 : 0x0;
    key |= dce.aaMode() << 8;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingCircleEffect::Create(GrColor color,
                                                 DashAAMode aaMode,
                                                 const SkMatrix& localMatrix,
                                                 bool usesLocalCoords) {
    return new DashingCircleEffect(color, aaMode, localMatrix, usesLocalCoords);
}

void DashingCircleEffect::getGLSLProcessorKey(const GrGLSLCaps& caps,
                                              GrProcessorKeyBuilder* b) const {
    GLDashingCircleEffect::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* DashingCircleEffect::createGLSLInstance(const GrGLSLCaps&) const {
    return new GLDashingCircleEffect();
}

DashingCircleEffect::DashingCircleEffect(GrColor color,
                                         DashAAMode aaMode,
                                         const SkMatrix& localMatrix,
                                         bool usesLocalCoords)
    : fColor(color)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fAAMode(aaMode) {
    this->initClassID<DashingCircleEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
    fInDashParams = &this->addVertexAttrib(Attribute("inDashParams", kVec3f_GrVertexAttribType));
    fInCircleParams = &this->addVertexAttrib(Attribute("inCircleParams",
                                                       kVec2f_GrVertexAttribType));
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingCircleEffect);

const GrGeometryProcessor* DashingCircleEffect::TestCreate(GrProcessorTestData* d) {
    DashAAMode aaMode = static_cast<DashAAMode>(d->fRandom->nextULessThan(kDashAAModeCount));
    return DashingCircleEffect::Create(GrRandomColor(d->fRandom),
                                      aaMode, GrTest::TestMatrix(d->fRandom),
                                      d->fRandom->nextBool());
}

//////////////////////////////////////////////////////////////////////////////

class GLDashingLineEffect;

/*
 * This effect will draw a dashed line. The width of the dash is given by the strokeWidth and the
 * length and spacing by the DashInfo. Both of the previous two parameters are in device space.
 * This effect also requires the setting of a vec2 vertex attribute for the the four corners of the
 * bounding rect. This attribute is the "dash position" of each vertex. In other words it is the
 * vertex coords (in device space) if we transform the line to be horizontal, with the start of
 * line at the origin then shifted to the right by half the off interval. The line then goes in the
 * positive x direction.
 */
class DashingLineEffect : public GrGeometryProcessor {
public:
    typedef SkPathEffect::DashInfo DashInfo;

    static GrGeometryProcessor* Create(GrColor,
                                       DashAAMode aaMode,
                                       const SkMatrix& localMatrix,
                                       bool usesLocalCoords);

    const char* name() const override { return "DashingEffect"; }

    const Attribute* inPosition() const { return fInPosition; }

    const Attribute* inDashParams() const { return fInDashParams; }

    const Attribute* inRectParams() const { return fInRectParams; }

    DashAAMode aaMode() const { return fAAMode; }

    GrColor color() const { return fColor; }

    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }

    const SkMatrix& localMatrix() const { return fLocalMatrix; }

    bool usesLocalCoords() const { return fUsesLocalCoords; }

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override;

private:
    DashingLineEffect(GrColor, DashAAMode aaMode, const SkMatrix& localMatrix,
                      bool usesLocalCoords);

    GrColor             fColor;
    SkMatrix            fLocalMatrix;
    bool                fUsesLocalCoords;
    DashAAMode          fAAMode;
    const Attribute*    fInPosition;
    const Attribute*    fInDashParams;
    const Attribute*    fInRectParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLDashingLineEffect : public GrGLSLGeometryProcessor {
public:
    GLDashingLineEffect();

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&) override;

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLSLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<DashingLineEffect>(primProc, pdman, index, transforms);
    }

private:
    GrColor       fColor;
    UniformHandle fColorUniform;
    typedef GrGLSLGeometryProcessor INHERITED;
};

GLDashingLineEffect::GLDashingLineEffect() {
    fColor = GrColor_ILLEGAL;
}

void GLDashingLineEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const DashingLineEffect& de = args.fGP.cast<DashingLineEffect>();
    GrGLSLGPBuilder* pb = args.fPB;

    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;

    // emit attributes
    varyingHandler->emitAttributes(de);

    // XY refers to dashPos, Z is the dash interval length
    GrGLSLVertToFrag inDashParams(kVec3f_GrSLType);
    varyingHandler->addVarying("DashParams", &inDashParams, GrSLPrecision::kHigh_GrSLPrecision);
    vertBuilder->codeAppendf("%s = %s;", inDashParams.vsOut(), de.inDashParams()->fName);

    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    GrGLSLVertToFrag inRectParams(kVec4f_GrSLType);
    varyingHandler->addVarying("RectParams", &inRectParams, GrSLPrecision::kHigh_GrSLPrecision);
    vertBuilder->codeAppendf("%s = %s;", inRectParams.vsOut(), de.inRectParams()->fName);

    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    if (!de.colorIgnored()) {
        this->setupUniformColor(pb, fragBuilder, args.fOutputColor, &fColorUniform);
    }

    // Setup position
    this->setupPosition(pb, vertBuilder, gpArgs, de.inPosition()->fName);

    // emit transforms
    this->emitTransforms(args.fPB,
                         vertBuilder,
                         varyingHandler,
                         gpArgs->fPositionVar,
                         de.inPosition()->fName,
                         de.localMatrix(),
                         args.fTransformsIn,
                         args.fTransformsOut);

    // transforms all points so that we can compare them to our test rect
    fragBuilder->codeAppendf("float xShifted = %s.x - floor(%s.x / %s.z) * %s.z;",
                             inDashParams.fsIn(), inDashParams.fsIn(), inDashParams.fsIn(),
                             inDashParams.fsIn());
    fragBuilder->codeAppendf("vec2 fragPosShifted = vec2(xShifted, %s.y);", inDashParams.fsIn());
    if (de.aaMode() == kEdgeAA_DashAAMode) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        fragBuilder->codeAppend("float xSub, ySub;");
        fragBuilder->codeAppendf("xSub = min(fragPosShifted.x - %s.x, 0.0);", inRectParams.fsIn());
        fragBuilder->codeAppendf("xSub += min(%s.z - fragPosShifted.x, 0.0);", inRectParams.fsIn());
        fragBuilder->codeAppendf("ySub = min(fragPosShifted.y - %s.y, 0.0);", inRectParams.fsIn());
        fragBuilder->codeAppendf("ySub += min(%s.w - fragPosShifted.y, 0.0);", inRectParams.fsIn());
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        fragBuilder->codeAppendf(
            "float alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));");
    } else if (de.aaMode() == kMSAA_DashAAMode) {
        // For MSAA, we don't modulate the alpha by the Y distance, since MSAA coverage will handle
        // AA on the the top and bottom edges. The shader is only responsible for intra-dash alpha.
        fragBuilder->codeAppend("float xSub;");
        fragBuilder->codeAppendf("xSub = min(fragPosShifted.x - %s.x, 0.0);", inRectParams.fsIn());
        fragBuilder->codeAppendf("xSub += min(%s.z - fragPosShifted.x, 0.0);", inRectParams.fsIn());
        // Now compute coverage in x to get the fraction of the pixel covered.
        fragBuilder->codeAppendf("float alpha = (1.0 + max(xSub, -1.0));");
    } else {
        // Assuming the bounding geometry is tight so no need to check y values
        fragBuilder->codeAppendf("float alpha = 1.0;");
        fragBuilder->codeAppendf("alpha *= (fragPosShifted.x - %s.x) > -0.5 ? 1.0 : 0.0;",
                                 inRectParams.fsIn());
        fragBuilder->codeAppendf("alpha *= (%s.z - fragPosShifted.x) >= -0.5 ? 1.0 : 0.0;",
                                 inRectParams.fsIn());
    }
    fragBuilder->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
}

void GLDashingLineEffect::setData(const GrGLSLProgramDataManager& pdman,
                                  const GrPrimitiveProcessor& processor) {
    const DashingLineEffect& de = processor.cast<DashingLineEffect>();
    if (de.color() != fColor) {
        float c[4];
        GrColorToRGBAFloat(de.color(), c);
        pdman.set4fv(fColorUniform, 1, c);
        fColor = de.color();
    }
}

void GLDashingLineEffect::GenKey(const GrGeometryProcessor& gp,
                                 const GrGLSLCaps&,
                                 GrProcessorKeyBuilder* b) {
    const DashingLineEffect& de = gp.cast<DashingLineEffect>();
    uint32_t key = 0;
    key |= de.usesLocalCoords() && de.localMatrix().hasPerspective() ? 0x1 : 0x0;
    key |= de.colorIgnored() ? 0x2 : 0x0;
    key |= de.aaMode() << 8;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingLineEffect::Create(GrColor color,
                                               DashAAMode aaMode,
                                               const SkMatrix& localMatrix,
                                               bool usesLocalCoords) {
    return new DashingLineEffect(color, aaMode, localMatrix, usesLocalCoords);
}

void DashingLineEffect::getGLSLProcessorKey(const GrGLSLCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GLDashingLineEffect::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* DashingLineEffect::createGLSLInstance(const GrGLSLCaps&) const {
    return new GLDashingLineEffect();
}

DashingLineEffect::DashingLineEffect(GrColor color,
                                     DashAAMode aaMode,
                                     const SkMatrix& localMatrix,
                                     bool usesLocalCoords)
    : fColor(color)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fAAMode(aaMode) {
    this->initClassID<DashingLineEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
    fInDashParams = &this->addVertexAttrib(Attribute("inDashParams", kVec3f_GrVertexAttribType));
    fInRectParams = &this->addVertexAttrib(Attribute("inRect", kVec4f_GrVertexAttribType));
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingLineEffect);

const GrGeometryProcessor* DashingLineEffect::TestCreate(GrProcessorTestData* d) {
    DashAAMode aaMode = static_cast<DashAAMode>(d->fRandom->nextULessThan(kDashAAModeCount));
    return DashingLineEffect::Create(GrRandomColor(d->fRandom),
                                     aaMode, GrTest::TestMatrix(d->fRandom),
                                     d->fRandom->nextBool());
}

//////////////////////////////////////////////////////////////////////////////

static GrGeometryProcessor* create_dash_gp(GrColor color,
                                           DashAAMode dashAAMode,
                                           DashCap cap,
                                           const SkMatrix& viewMatrix,
                                           bool usesLocalCoords) {
    SkMatrix invert;
    if (usesLocalCoords && !viewMatrix.invert(&invert)) {
        SkDebugf("Failed to invert\n");
        return nullptr;
    }

    switch (cap) {
        case kRound_DashCap:
            return DashingCircleEffect::Create(color, dashAAMode, invert, usesLocalCoords);
        case kNonRound_DashCap:
            return DashingLineEffect::Create(color, dashAAMode, invert, usesLocalCoords);
    }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(DashBatch) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    bool useAA = random->nextBool();
    bool msaaRT = random->nextBool();

    // We can only dash either horizontal or vertical lines
    SkPoint pts[2];
    if (random->nextBool()) {
        // vertical
        pts[0].fX = 1.f;
        pts[0].fY = random->nextF() * 10.f;
        pts[1].fX = 1.f;
        pts[1].fY = random->nextF() * 10.f;
    } else {
        // horizontal
        pts[0].fX = random->nextF() * 10.f;
        pts[0].fY = 1.f;
        pts[1].fX = random->nextF() * 10.f;
        pts[1].fY = 1.f;
    }

    // pick random cap
    SkPaint::Cap cap = SkPaint::Cap(random->nextULessThan(SkPaint::Cap::kCapCount));

    SkScalar intervals[2];

    // We can only dash with the following intervals
    enum Intervals {
        kOpenOpen_Intervals ,
        kOpenClose_Intervals,
        kCloseOpen_Intervals,
    };

    Intervals intervalType = SkPaint::kRound_Cap ?
                             kOpenClose_Intervals :
                             Intervals(random->nextULessThan(kCloseOpen_Intervals + 1));
    static const SkScalar kIntervalMin = 0.1f;
    static const SkScalar kIntervalMax = 10.f;
    switch (intervalType) {
        case kOpenOpen_Intervals:
            intervals[0] = random->nextRangeScalar(kIntervalMin, kIntervalMax);
            intervals[1] = random->nextRangeScalar(kIntervalMin, kIntervalMax);
            break;
        case kOpenClose_Intervals:
            intervals[0] = 0.f;
            intervals[1] = random->nextRangeScalar(kIntervalMin, kIntervalMax);
            break;
        case kCloseOpen_Intervals:
            intervals[0] = random->nextRangeScalar(kIntervalMin, kIntervalMax);
            intervals[1] = 0.f;
            break;

    }

    // phase is 0 < sum (i0, i1)
    SkScalar phase = random->nextRangeScalar(0, intervals[0] + intervals[1]);

    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SkIntToScalar(1));
    p.setStrokeCap(cap);

    GrStrokeInfo strokeInfo(p);

    SkPathEffect::DashInfo info;
    info.fIntervals = intervals;
    info.fCount = 2;
    info.fPhase = phase;
    SkDEBUGCODE(bool success = ) strokeInfo.setDashInfo(info);
    SkASSERT(success);

    return create_batch(color, viewMatrix, pts, useAA, strokeInfo, msaaRT);
}

#endif
