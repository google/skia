/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/DashOp.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDefaultGeoProcFactory.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"

using namespace skia_private;

using AAMode = skgpu::ganesh::DashOp::AAMode;

#if defined(GR_TEST_UTILS)
constexpr int kAAModeCnt = static_cast<int>(skgpu::ganesh::DashOp::AAMode::kCoverageWithMSAA) + 1;
#endif

namespace skgpu::ganesh::DashOp {

namespace {

void calc_dash_scaling(SkScalar* parallelScale, SkScalar* perpScale,
                       const SkMatrix& viewMatrix, const SkPoint pts[2]) {
    SkVector vecSrc = pts[1] - pts[0];
    if (pts[1] == pts[0]) {
        vecSrc.set(1.0, 0.0);
    }
    SkScalar magSrc = vecSrc.length();
    SkScalar invSrc = magSrc ? SkScalarInvert(magSrc) : 0;
    vecSrc.scale(invSrc);

    SkVector vecSrcPerp;
    SkPointPriv::RotateCW(vecSrc, &vecSrcPerp);
    viewMatrix.mapVectors(&vecSrc, 1);
    viewMatrix.mapVectors(&vecSrcPerp, 1);

    // parallelScale tells how much to scale along the line parallel to the dash line
    // perpScale tells how much to scale in the direction perpendicular to the dash line
    *parallelScale = vecSrc.length();
    *perpScale = vecSrcPerp.length();
}

// calculates the rotation needed to aligned pts to the x axis with pts[0] < pts[1]
// Stores the rotation matrix in rotMatrix, and the mapped points in ptsRot
void align_to_x_axis(const SkPoint pts[2], SkMatrix* rotMatrix, SkPoint ptsRot[2] = nullptr) {
    SkVector vec = pts[1] - pts[0];
    if (pts[1] == pts[0]) {
        vec.set(1.0, 0.0);
    }
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
SkScalar calc_start_adjustment(const SkScalar intervals[2], SkScalar phase) {
    SkASSERT(phase < intervals[0] + intervals[1]);
    if (phase >= intervals[0] && phase != 0) {
        SkScalar srcIntervalLen = intervals[0] + intervals[1];
        return srcIntervalLen - phase;
    }
    return 0;
}

SkScalar calc_end_adjustment(const SkScalar intervals[2], const SkPoint pts[2],
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
        return *endingInt - intervals[0];
    }
    return 0;
}

enum DashCap {
    kRound_DashCap,
    kNonRound_DashCap,
};

void setup_dashed_rect(const SkRect& rect,
                       VertexWriter& vertices,
                       const SkMatrix& matrix,
                       SkScalar offset,
                       SkScalar bloatX,
                       SkScalar len,
                       SkScalar startInterval,
                       SkScalar endInterval,
                       SkScalar strokeWidth,
                       SkScalar perpScale,
                       DashCap cap) {
    SkScalar intervalLength = startInterval + endInterval;
    // 'dashRect' gets interpolated over the rendered 'rect'. For y we want the perpendicular signed
    // distance from the stroke center line in device space. 'perpScale' is the scale factor applied
    // to the y dimension of 'rect' isolated from 'matrix'.
    SkScalar halfDevRectHeight = rect.height() * perpScale / 2.f;
    SkRect dashRect = { offset       - bloatX, -halfDevRectHeight,
                        offset + len + bloatX,  halfDevRectHeight };

    if (kRound_DashCap == cap) {
        SkScalar radius = SkScalarHalf(strokeWidth) - 0.5f;
        SkScalar centerX = SkScalarHalf(endInterval);

        vertices.writeQuad(GrQuad::MakeFromRect(rect, matrix),
                           VertexWriter::TriStripFromRect(dashRect),
                           intervalLength,
                           radius,
                           centerX);
    } else {
        SkASSERT(kNonRound_DashCap == cap);
        SkScalar halfOffLen = SkScalarHalf(endInterval);
        SkScalar halfStroke = SkScalarHalf(strokeWidth);
        SkRect rectParam;
        rectParam.setLTRB(halfOffLen                 + 0.5f, -halfStroke + 0.5f,
                          halfOffLen + startInterval - 0.5f,  halfStroke - 0.5f);

        vertices.writeQuad(GrQuad::MakeFromRect(rect, matrix),
                           VertexWriter::TriStripFromRect(dashRect),
                           intervalLength,
                           rectParam);
    }
}

/**
 * An GrGeometryProcessor that renders a dashed line.
 * This GrGeometryProcessor is meant for dashed lines that only have a single on/off interval pair.
 * Bounding geometry is rendered and the effect computes coverage based on the fragment's
 * position relative to the dashed line.
 */
GrGeometryProcessor* make_dash_gp(SkArenaAlloc* arena,
                                  const SkPMColor4f&,
                                  AAMode aaMode,
                                  DashCap cap,
                                  const SkMatrix& localMatrix,
                                  bool usesLocalCoords);

class DashOpImpl final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    struct LineData {
        SkMatrix fViewMatrix;
        SkMatrix fSrcRotInv;
        SkPoint fPtsRot[2];
        SkScalar fSrcStrokeWidth;
        SkScalar fPhase;
        SkScalar fIntervals[2];
        SkScalar fParallelScale;
        SkScalar fPerpendicularScale;
    };

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const LineData& geometry,
                            SkPaint::Cap cap,
                            AAMode aaMode, bool fullDash,
                            const GrUserStencilSettings* stencilSettings) {
        return GrOp::Make<DashOpImpl>(context, std::move(paint), geometry, cap,
                                      aaMode, fullDash, stencilSettings);
    }

    const char* name() const override { return "DashOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fProcessorSet.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        FixedFunctionFlags flags = FixedFunctionFlags::kNone;
        if (AAMode::kCoverageWithMSAA == fAAMode) {
            flags |= FixedFunctionFlags::kUsesHWAA;
        }
        if (fStencilSettings != &GrUserStencilSettings::kUnused) {
            flags |= FixedFunctionFlags::kUsesStencil;
        }
        return flags;
    }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        GrProcessorAnalysisCoverage coverage = GrProcessorAnalysisCoverage::kSingleChannel;
        auto analysis = fProcessorSet.finalize(fColor, coverage, clip, fStencilSettings, caps,
                                               clampType, &fColor);
        fUsesLocalCoords = analysis.usesLocalCoords();
        return analysis;
    }

private:
    friend class GrOp; // for ctor

    DashOpImpl(GrPaint&& paint, const LineData& geometry, SkPaint::Cap cap, AAMode aaMode,
               bool fullDash, const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fColor(paint.getColor4f())
            , fFullDash(fullDash)
            , fCap(cap)
            , fAAMode(aaMode)
            , fProcessorSet(std::move(paint))
            , fStencilSettings(stencilSettings) {
        fLines.push_back(geometry);

        // compute bounds
        SkScalar halfStrokeWidth = 0.5f * geometry.fSrcStrokeWidth;
        SkScalar xBloat = SkPaint::kButt_Cap == cap ? 0 : halfStrokeWidth;
        SkRect bounds;
        bounds.set(geometry.fPtsRot[0], geometry.fPtsRot[1]);
        bounds.outset(xBloat, halfStrokeWidth);

        // Note, we actually create the combined matrix here, and save the work
        SkMatrix& combinedMatrix = fLines[0].fSrcRotInv;
        combinedMatrix.postConcat(geometry.fViewMatrix);

        IsHairline zeroArea = geometry.fSrcStrokeWidth ? IsHairline::kNo : IsHairline::kYes;
        HasAABloat aaBloat = (aaMode == AAMode::kNone) ? HasAABloat::kNo : HasAABloat::kYes;
        this->setTransformedBounds(bounds, combinedMatrix, aaBloat, zeroArea);
    }

    struct DashDraw {
        DashDraw(const LineData& geo) {
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
        SkScalar fDevBloatX;
        SkScalar fPerpendicularScale;
        bool fLineDone;
        bool fHasStartRect;
        bool fHasEndRect;
    };

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {

        DashCap capType = (this->cap() == SkPaint::kRound_Cap) ? kRound_DashCap : kNonRound_DashCap;

        GrGeometryProcessor* gp;
        if (this->fullDash()) {
            gp = make_dash_gp(arena, this->color(), this->aaMode(), capType,
                              this->viewMatrix(), fUsesLocalCoords);
        } else {
            // Set up the vertex data for the line and start/end dashes
            using namespace GrDefaultGeoProcFactory;
            Color color(this->color());
            LocalCoords::Type localCoordsType =
                    fUsesLocalCoords ? LocalCoords::kUsePosition_Type : LocalCoords::kUnused_Type;
            gp = MakeForDeviceSpace(arena,
                                    color,
                                    Coverage::kSolid_Type,
                                    localCoordsType,
                                    this->viewMatrix());
        }

        if (!gp) {
            SkDebugf("Could not create GrGeometryProcessor\n");
            return;
        }

        fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps,
                                                                   arena,
                                                                   writeView,
                                                                   usesMSAASurface,
                                                                   std::move(appliedClip),
                                                                   dstProxyView,
                                                                   gp,
                                                                   std::move(fProcessorSet),
                                                                   GrPrimitiveType::kTriangles,
                                                                   renderPassXferBarriers,
                                                                   colorLoadOp,
                                                                   GrPipeline::InputFlags::kNone,
                                                                   fStencilSettings);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        int instanceCount = fLines.size();
        SkPaint::Cap cap = this->cap();
        DashCap capType = (SkPaint::kRound_Cap == cap) ? kRound_DashCap : kNonRound_DashCap;

        if (!fProgramInfo) {
            this->createProgramInfo(target);
            if (!fProgramInfo) {
                return;
            }
        }

        // useAA here means Edge AA or MSAA
        bool useAA = this->aaMode() != AAMode::kNone;
        bool fullDash = this->fullDash();

        // We do two passes over all of the dashes.  First we setup the start, end, and bounds,
        // rectangles.  We preserve all of this work in the rects / draws arrays below.  Then we
        // iterate again over these decomposed dashes to generate vertices
        static const int kNumStackDashes = 128;
        STArray<kNumStackDashes, SkRect, true> rects;
        STArray<kNumStackDashes, DashDraw, true> draws;

        int totalRectCount = 0;
        int rectOffset = 0;
        rects.push_back_n(3 * instanceCount);
        for (int i = 0; i < instanceCount; i++) {
            const LineData& args = fLines[i];

            DashDraw& draw = draws.push_back(args);

            bool hasCap = SkPaint::kButt_Cap != cap;

            SkScalar halfSrcStroke = args.fSrcStrokeWidth * 0.5f;
            if (halfSrcStroke == 0.0f || this->aaMode() != AAMode::kCoverageWithMSAA) {
                // In the non-MSAA case, we always want to at least stroke out half a pixel on each
                // side in device space. 0.5f / fPerpendicularScale gives us this min in src space.
                // This is also necessary when the stroke width is zero, to allow hairlines to draw.
                halfSrcStroke = std::max(halfSrcStroke, 0.5f / args.fPerpendicularScale);
            }

            SkScalar strokeAdj = hasCap ? halfSrcStroke : 0.0f;
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
                    startPts[1].fX = std::min(startPts[0].fX + draw.fIntervals[0] - draw.fPhase,
                                              draw.fPtsRot[1].fX);
                    startRect.setBounds(startPts, 2);
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

                    endRect.setBounds(endPts, 2);
                    endRect.outset(strokeAdj, halfSrcStroke);

                    hasEndRect = true;
                    endAdj = endingInterval + draw.fIntervals[1];

                    draw.fPtsRot[1].fX -= endAdj;
                    if (draw.fPtsRot[0].fX >= draw.fPtsRot[1].fX) {
                        lineDone = true;
                    }
                }
            }

            if (draw.fPtsRot[0].fX == draw.fPtsRot[1].fX &&
                (0 != endAdj || 0 == startAdj) &&
                hasCap) {
                // At this point the fPtsRot[0]/[1] represent the start and end of the inner rect of
                // dashes that we want to draw. The only way they can be equal is if the on interval
                // is zero (or an edge case if the end of line ends at a full off interval, but this
                // is handled as well). Thus if the on interval is zero then we need to draw a cap
                // at this position if the stroke has caps. The spec says we only draw this point if
                // point lies between [start of line, end of line). Thus we check if we are at the
                // end (but not the start), and if so we don't draw the cap.
                lineDone = false;
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

            if ((strokeWidth < 1.f && !useAA) || 0.f == strokeWidth) {
                strokeWidth = 1.f;
            }

            SkScalar halfDevStroke = strokeWidth * 0.5f;

            if (SkPaint::kSquare_Cap == cap) {
                // add cap to on interval and remove from off interval
                devIntervals[0] += strokeWidth;
                devIntervals[1] -= strokeWidth;
            }
            SkScalar startOffset = devIntervals[1] * 0.5f + devPhase;

            SkScalar devBloatX = 0.0f;
            SkScalar devBloatY = 0.0f;
            switch (this->aaMode()) {
                case AAMode::kNone:
                    break;
                case AAMode::kCoverage:
                    // For EdgeAA, we bloat in X & Y for both square and round caps.
                    devBloatX = 0.5f;
                    devBloatY = 0.5f;
                    break;
                case AAMode::kCoverageWithMSAA:
                    // For MSAA, we only bloat in Y for round caps.
                    devBloatY = (cap == SkPaint::kRound_Cap) ? 0.5f : 0.0f;
                    break;
            }

            SkScalar bloatX = devBloatX / args.fParallelScale;
            SkScalar bloatY = devBloatY / args.fPerpendicularScale;

            if (devIntervals[1] <= 0.f && useAA) {
                // Case when we end up drawing a solid AA rect
                // Reset the start rect to draw this single solid rect
                // but it requires to upload a new intervals uniform so we can mimic
                // one giant dash
                draw.fPtsRot[0].fX -= hasStartRect ? startAdj : 0;
                draw.fPtsRot[1].fX += hasEndRect ? endAdj : 0;
                startRect.setBounds(draw.fPtsRot, 2);
                startRect.outset(strokeAdj, halfSrcStroke);
                hasStartRect = true;
                hasEndRect = false;
                lineDone = true;

                SkPoint devicePts[2];
                args.fSrcRotInv.mapPoints(devicePts, draw.fPtsRot, 2);
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
                args.fSrcRotInv.mapPoints(devicePts, draw.fPtsRot, 2);
                draw.fLineLength = SkPoint::Distance(devicePts[0], devicePts[1]);
                if (hasCap) {
                    draw.fLineLength += 2.f * halfDevStroke;
                }

                bounds.setLTRB(draw.fPtsRot[0].fX, draw.fPtsRot[0].fY,
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
            draw.fPerpendicularScale = args.fPerpendicularScale;
            draw.fStrokeWidth = strokeWidth;
            draw.fHasStartRect = hasStartRect;
            draw.fLineDone = lineDone;
            draw.fHasEndRect = hasEndRect;
        }

        if (!totalRectCount) {
            return;
        }

        QuadHelper helper(target, fProgramInfo->geomProc().vertexStride(), totalRectCount);
        VertexWriter vertices{ helper.vertices() };
        if (!vertices) {
            return;
        }

        int rectIndex = 0;
        for (int i = 0; i < instanceCount; i++) {
            const LineData& geom = fLines[i];

            if (!draws[i].fLineDone) {
                if (fullDash) {
                    setup_dashed_rect(rects[rectIndex], vertices, geom.fSrcRotInv,
                                      draws[i].fStartOffset, draws[i].fDevBloatX,
                                      draws[i].fLineLength, draws[i].fIntervals[0],
                                      draws[i].fIntervals[1], draws[i].fStrokeWidth,
                                      draws[i].fPerpendicularScale,
                                      capType);
                } else {
                    vertices.writeQuad(GrQuad::MakeFromRect(rects[rectIndex], geom.fSrcRotInv));
                }
            }
            rectIndex++;

            if (draws[i].fHasStartRect) {
                if (fullDash) {
                    setup_dashed_rect(rects[rectIndex], vertices, geom.fSrcRotInv,
                                      draws[i].fStartOffset, draws[i].fDevBloatX,
                                      draws[i].fIntervals[0], draws[i].fIntervals[0],
                                      draws[i].fIntervals[1], draws[i].fStrokeWidth,
                                      draws[i].fPerpendicularScale, capType);
                } else {
                    vertices.writeQuad(GrQuad::MakeFromRect(rects[rectIndex], geom.fSrcRotInv));
                }
            }
            rectIndex++;

            if (draws[i].fHasEndRect) {
                if (fullDash) {
                    setup_dashed_rect(rects[rectIndex], vertices, geom.fSrcRotInv,
                                      draws[i].fStartOffset, draws[i].fDevBloatX,
                                      draws[i].fIntervals[0], draws[i].fIntervals[0],
                                      draws[i].fIntervals[1], draws[i].fStrokeWidth,
                                      draws[i].fPerpendicularScale, capType);
                } else {
                    vertices.writeQuad(GrQuad::MakeFromRect(rects[rectIndex], geom.fSrcRotInv));
                }
            }
            rectIndex++;
        }

        fMesh = helper.mesh();
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        auto that = t->cast<DashOpImpl>();
        if (fProcessorSet != that->fProcessorSet) {
            return CombineResult::kCannotCombine;
        }

        if (this->aaMode() != that->aaMode()) {
            return CombineResult::kCannotCombine;
        }

        if (this->fullDash() != that->fullDash()) {
            return CombineResult::kCannotCombine;
        }

        if (this->cap() != that->cap()) {
            return CombineResult::kCannotCombine;
        }

        // TODO vertex color
        if (this->color() != that->color()) {
            return CombineResult::kCannotCombine;
        }

        if (fUsesLocalCoords && !SkMatrixPriv::CheapEqual(this->viewMatrix(), that->viewMatrix())) {
            return CombineResult::kCannotCombine;
        }

        fLines.push_back_n(that->fLines.size(), that->fLines.begin());
        return CombineResult::kMerged;
    }

#if defined(GR_TEST_UTILS)
    SkString onDumpInfo() const override {
        SkString string;
        for (const auto& geo : fLines) {
            string.appendf("Pt0: [%.2f, %.2f], Pt1: [%.2f, %.2f], Width: %.2f, Ival0: %.2f, "
                           "Ival1 : %.2f, Phase: %.2f\n",
                           geo.fPtsRot[0].fX, geo.fPtsRot[0].fY,
                           geo.fPtsRot[1].fX, geo.fPtsRot[1].fY,
                           geo.fSrcStrokeWidth,
                           geo.fIntervals[0],
                           geo.fIntervals[1],
                           geo.fPhase);
        }
        string += fProcessorSet.dumpProcessors();
        return string;
    }
#endif

    const SkPMColor4f& color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fLines[0].fViewMatrix; }
    AAMode aaMode() const { return fAAMode; }
    bool fullDash() const { return fFullDash; }
    SkPaint::Cap cap() const { return fCap; }

    STArray<1, LineData, true> fLines;
    SkPMColor4f fColor;
    bool fUsesLocalCoords : 1;
    bool fFullDash : 1;
    // We use 3 bits for this 3-value enum because MSVS makes the underlying types signed.
    SkPaint::Cap fCap : 3;
    AAMode fAAMode;
    GrProcessorSet fProcessorSet;
    const GrUserStencilSettings* fStencilSettings;

    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

/*
 * This effect will draw a dotted line (defined as a dashed lined with round caps and no on
 * interval). The radius of the dots is given by the strokeWidth and the spacing by the DashInfo.
 * Both of the previous two parameters are in device space. This effect also requires the setting of
 * a float2 vertex attribute for the the four corners of the bounding rect. This attribute is the
 * "dash position" of each vertex. In other words it is the vertex coords (in device space) if we
 * transform the line to be horizontal, with the start of line at the origin then shifted to the
 * right by half the off interval. The line then goes in the positive x direction.
 */
class DashingCircleEffect : public GrGeometryProcessor {
public:
    typedef SkPathEffect::DashInfo DashInfo;

    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const SkPMColor4f&,
                                     AAMode aaMode,
                                     const SkMatrix& localMatrix,
                                     bool usesLocalCoords);

    const char* name() const override { return "DashingCircleEffect"; }

    void addToKey(const GrShaderCaps&, KeyBuilder*) const override;

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    class Impl;

    DashingCircleEffect(const SkPMColor4f&, AAMode aaMode, const SkMatrix& localMatrix,
                        bool usesLocalCoords);

    SkPMColor4f fColor;
    SkMatrix    fLocalMatrix;
    bool        fUsesLocalCoords;
    AAMode      fAAMode;

    Attribute   fInPosition;
    Attribute   fInDashParams;
    Attribute   fInCircleParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

//////////////////////////////////////////////////////////////////////////////

class DashingCircleEffect::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager&,
                 const GrShaderCaps&,
                 const GrGeometryProcessor&) override;

private:
    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    SkMatrix    fLocalMatrix         = SkMatrix::InvalidMatrix();
    SkPMColor4f fColor               = SK_PMColor4fILLEGAL;

    UniformHandle fParamUniform;
    UniformHandle fColorUniform;
    UniformHandle fLocalMatrixUniform;
};

void DashingCircleEffect::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const DashingCircleEffect& dce = args.fGeomProc.cast<DashingCircleEffect>();
    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    // emit attributes
    varyingHandler->emitAttributes(dce);

    // XY are dashPos, Z is dashInterval
    GrGLSLVarying dashParams(SkSLType::kHalf3);
    varyingHandler->addVarying("DashParam", &dashParams);
    vertBuilder->codeAppendf("%s = %s;", dashParams.vsOut(), dce.fInDashParams.name());

    // x refers to circle radius - 0.5, y refers to cicle's center x coord
    GrGLSLVarying circleParams(SkSLType::kHalf2);
    varyingHandler->addVarying("CircleParams", &circleParams);
    vertBuilder->codeAppendf("%s = %s;", circleParams.vsOut(), dce.fInCircleParams.name());

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
    this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);

    // Setup position
    WriteOutputPosition(vertBuilder, gpArgs, dce.fInPosition.name());
    if (dce.fUsesLocalCoords) {
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        dce.fInPosition.asShaderVar(),
                        dce.fLocalMatrix,
                        &fLocalMatrixUniform);
    }

    // transforms all points so that we can compare them to our test circle
    fragBuilder->codeAppendf("half xShifted = half(%s.x - floor(%s.x / %s.z) * %s.z);",
                             dashParams.fsIn(), dashParams.fsIn(), dashParams.fsIn(),
                             dashParams.fsIn());
    fragBuilder->codeAppendf("half2 fragPosShifted = half2(xShifted, half(%s.y));",
                             dashParams.fsIn());
    fragBuilder->codeAppendf("half2 center = half2(%s.y, 0.0);", circleParams.fsIn());
    fragBuilder->codeAppend("half dist = length(center - fragPosShifted);");
    if (dce.fAAMode != AAMode::kNone) {
        fragBuilder->codeAppendf("half diff = dist - %s.x;", circleParams.fsIn());
        fragBuilder->codeAppend("diff = 1.0 - diff;");
        fragBuilder->codeAppend("half alpha = saturate(diff);");
    } else {
        fragBuilder->codeAppendf("half alpha = 1.0;");
        fragBuilder->codeAppendf("alpha *=  dist < %s.x + 0.5 ? 1.0 : 0.0;", circleParams.fsIn());
    }
    fragBuilder->codeAppendf("half4 %s = half4(alpha);", args.fOutputCoverage);
}

void DashingCircleEffect::Impl::setData(const GrGLSLProgramDataManager& pdman,
                                        const GrShaderCaps& shaderCaps,
                                        const GrGeometryProcessor& geomProc) {
    const DashingCircleEffect& dce = geomProc.cast<DashingCircleEffect>();
    if (dce.fColor != fColor) {
        pdman.set4fv(fColorUniform, 1, dce.fColor.vec());
        fColor = dce.fColor;
    }
    SetTransform(pdman, shaderCaps, fLocalMatrixUniform, dce.fLocalMatrix, &fLocalMatrix);
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingCircleEffect::Make(SkArenaAlloc* arena,
                                               const SkPMColor4f& color,
                                               AAMode aaMode,
                                               const SkMatrix& localMatrix,
                                               bool usesLocalCoords) {
    return arena->make([&](void* ptr) {
        return new (ptr) DashingCircleEffect(color, aaMode, localMatrix, usesLocalCoords);
    });
}

void DashingCircleEffect::addToKey(const GrShaderCaps& caps, KeyBuilder* b) const {
    uint32_t key = 0;
    key |= fUsesLocalCoords ? 0x1 : 0x0;
    key |= static_cast<uint32_t>(fAAMode) << 1;
    key |= ProgramImpl::ComputeMatrixKey(caps, fLocalMatrix) << 3;
    b->add32(key);
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> DashingCircleEffect::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

DashingCircleEffect::DashingCircleEffect(const SkPMColor4f& color,
                                         AAMode aaMode,
                                         const SkMatrix& localMatrix,
                                         bool usesLocalCoords)
        : INHERITED(kDashingCircleEffect_ClassID)
        , fColor(color)
        , fLocalMatrix(localMatrix)
        , fUsesLocalCoords(usesLocalCoords)
        , fAAMode(aaMode) {
    fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
    fInDashParams = {"inDashParams", kFloat3_GrVertexAttribType, SkSLType::kHalf3};
    fInCircleParams = {"inCircleParams", kFloat2_GrVertexAttribType, SkSLType::kHalf2};
    this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingCircleEffect)

#if defined(GR_TEST_UTILS)
GrGeometryProcessor* DashingCircleEffect::TestCreate(GrProcessorTestData* d) {
    AAMode aaMode = static_cast<AAMode>(d->fRandom->nextULessThan(kAAModeCnt));
    GrColor color = GrTest::RandomColor(d->fRandom);
    SkMatrix matrix = GrTest::TestMatrix(d->fRandom);
    return DashingCircleEffect::Make(d->allocator(),
                                     SkPMColor4f::FromBytes_RGBA(color),
                                     aaMode,
                                     matrix,
                                     d->fRandom->nextBool());
}
#endif

//////////////////////////////////////////////////////////////////////////////

/*
 * This effect will draw a dashed line. The width of the dash is given by the strokeWidth and the
 * length and spacing by the DashInfo. Both of the previous two parameters are in device space.
 * This effect also requires the setting of a float2 vertex attribute for the the four corners of the
 * bounding rect. This attribute is the "dash position" of each vertex. In other words it is the
 * vertex coords (in device space) if we transform the line to be horizontal, with the start of
 * line at the origin then shifted to the right by half the off interval. The line then goes in the
 * positive x direction.
 */
class DashingLineEffect : public GrGeometryProcessor {
public:
    typedef SkPathEffect::DashInfo DashInfo;

    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const SkPMColor4f&,
                                     AAMode aaMode,
                                     const SkMatrix& localMatrix,
                                     bool usesLocalCoords);

    const char* name() const override { return "DashingEffect"; }

    bool usesLocalCoords() const { return fUsesLocalCoords; }

    void addToKey(const GrShaderCaps&, KeyBuilder*) const override;

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    class Impl;

    DashingLineEffect(const SkPMColor4f&, AAMode aaMode, const SkMatrix& localMatrix,
                      bool usesLocalCoords);

    SkPMColor4f fColor;
    SkMatrix    fLocalMatrix;
    bool        fUsesLocalCoords;
    AAMode      fAAMode;

    Attribute   fInPosition;
    Attribute   fInDashParams;
    Attribute   fInRect;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

//////////////////////////////////////////////////////////////////////////////

class DashingLineEffect::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager&,
                 const GrShaderCaps&,
                 const GrGeometryProcessor&) override;

private:
    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    SkPMColor4f fColor       = SK_PMColor4fILLEGAL;
    SkMatrix    fLocalMatrix = SkMatrix::InvalidMatrix();

    UniformHandle fLocalMatrixUniform;
    UniformHandle fColorUniform;
};

void DashingLineEffect::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const DashingLineEffect& de = args.fGeomProc.cast<DashingLineEffect>();

    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    // emit attributes
    varyingHandler->emitAttributes(de);

    // XY refers to dashPos, Z is the dash interval length
    GrGLSLVarying inDashParams(SkSLType::kFloat3);
    varyingHandler->addVarying("DashParams", &inDashParams);
    vertBuilder->codeAppendf("%s = %s;", inDashParams.vsOut(), de.fInDashParams.name());

    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    GrGLSLVarying inRectParams(SkSLType::kFloat4);
    varyingHandler->addVarying("RectParams", &inRectParams);
    vertBuilder->codeAppendf("%s = %s;", inRectParams.vsOut(), de.fInRect.name());

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
    this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);

    // Setup position
    WriteOutputPosition(vertBuilder, gpArgs, de.fInPosition.name());
    if (de.usesLocalCoords()) {
        WriteLocalCoord(vertBuilder,
                        uniformHandler,
                        *args.fShaderCaps,
                        gpArgs,
                        de.fInPosition.asShaderVar(),
                        de.fLocalMatrix,
                        &fLocalMatrixUniform);
    }

    // transforms all points so that we can compare them to our test rect
    fragBuilder->codeAppendf("half xShifted = half(%s.x - floor(%s.x / %s.z) * %s.z);",
                             inDashParams.fsIn(), inDashParams.fsIn(), inDashParams.fsIn(),
                             inDashParams.fsIn());
    fragBuilder->codeAppendf("half2 fragPosShifted = half2(xShifted, half(%s.y));",
                             inDashParams.fsIn());
    if (de.fAAMode == AAMode::kCoverage) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        fragBuilder->codeAppend("half xSub, ySub;");
        fragBuilder->codeAppendf("xSub = half(min(fragPosShifted.x - %s.x, 0.0));",
                                 inRectParams.fsIn());
        fragBuilder->codeAppendf("xSub += half(min(%s.z - fragPosShifted.x, 0.0));",
                                 inRectParams.fsIn());
        fragBuilder->codeAppendf("ySub = half(min(fragPosShifted.y - %s.y, 0.0));",
                                 inRectParams.fsIn());
        fragBuilder->codeAppendf("ySub += half(min(%s.w - fragPosShifted.y, 0.0));",
                                 inRectParams.fsIn());
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        fragBuilder->codeAppendf(
            "half alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));");
    } else if (de.fAAMode == AAMode::kCoverageWithMSAA) {
        // For MSAA, we don't modulate the alpha by the Y distance, since MSAA coverage will handle
        // AA on the the top and bottom edges. The shader is only responsible for intra-dash alpha.
        fragBuilder->codeAppend("half xSub;");
        fragBuilder->codeAppendf("xSub = half(min(fragPosShifted.x - %s.x, 0.0));",
                                 inRectParams.fsIn());
        fragBuilder->codeAppendf("xSub += half(min(%s.z - fragPosShifted.x, 0.0));",
                                 inRectParams.fsIn());
        // Now compute coverage in x to get the fraction of the pixel covered.
        fragBuilder->codeAppendf("half alpha = (1.0 + max(xSub, -1.0));");
    } else {
        // Assuming the bounding geometry is tight so no need to check y values
        fragBuilder->codeAppendf("half alpha = 1.0;");
        fragBuilder->codeAppendf("alpha *= (fragPosShifted.x - %s.x) > -0.5 ? 1.0 : 0.0;",
                                 inRectParams.fsIn());
        fragBuilder->codeAppendf("alpha *= (%s.z - fragPosShifted.x) >= -0.5 ? 1.0 : 0.0;",
                                 inRectParams.fsIn());
    }
    fragBuilder->codeAppendf("half4 %s = half4(alpha);", args.fOutputCoverage);
}

void DashingLineEffect::Impl::setData(const GrGLSLProgramDataManager& pdman,
                                      const GrShaderCaps& shaderCaps,
                                      const GrGeometryProcessor& geomProc) {
    const DashingLineEffect& de = geomProc.cast<DashingLineEffect>();
    if (de.fColor != fColor) {
        pdman.set4fv(fColorUniform, 1, de.fColor.vec());
        fColor = de.fColor;
    }
    SetTransform(pdman, shaderCaps, fLocalMatrixUniform, de.fLocalMatrix, &fLocalMatrix);
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingLineEffect::Make(SkArenaAlloc* arena,
                                             const SkPMColor4f& color,
                                             AAMode aaMode,
                                             const SkMatrix& localMatrix,
                                             bool usesLocalCoords) {
    return arena->make([&](void* ptr) {
        return new (ptr) DashingLineEffect(color, aaMode, localMatrix, usesLocalCoords);
    });
}

void DashingLineEffect::addToKey(const GrShaderCaps& caps, KeyBuilder* b) const {
    uint32_t key = 0;
    key |= fUsesLocalCoords ? 0x1 : 0x0;
    key |= static_cast<int>(fAAMode) << 1;
    key |= ProgramImpl::ComputeMatrixKey(caps, fLocalMatrix) << 3;
    b->add32(key);
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> DashingLineEffect::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

DashingLineEffect::DashingLineEffect(const SkPMColor4f& color,
                                     AAMode aaMode,
                                     const SkMatrix& localMatrix,
                                     bool usesLocalCoords)
        : INHERITED(kDashingLineEffect_ClassID)
        , fColor(color)
        , fLocalMatrix(localMatrix)
        , fUsesLocalCoords(usesLocalCoords)
        , fAAMode(aaMode) {
    fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
    fInDashParams = {"inDashParams", kFloat3_GrVertexAttribType, SkSLType::kHalf3};
    fInRect = {"inRect", kFloat4_GrVertexAttribType, SkSLType::kHalf4};
    this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingLineEffect)

#if defined(GR_TEST_UTILS)
GrGeometryProcessor* DashingLineEffect::TestCreate(GrProcessorTestData* d) {
    AAMode aaMode = static_cast<AAMode>(d->fRandom->nextULessThan(kAAModeCnt));
    GrColor color = GrTest::RandomColor(d->fRandom);
    SkMatrix matrix = GrTest::TestMatrix(d->fRandom);
    return DashingLineEffect::Make(d->allocator(),
                                   SkPMColor4f::FromBytes_RGBA(color),
                                   aaMode,
                                   matrix,
                                   d->fRandom->nextBool());
}

#endif
//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* make_dash_gp(SkArenaAlloc* arena,
                                  const SkPMColor4f& color,
                                  AAMode aaMode,
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
            return DashingCircleEffect::Make(arena, color, aaMode, invert, usesLocalCoords);
        case kNonRound_DashCap:
            return DashingLineEffect::Make(arena, color, aaMode, invert, usesLocalCoords);
    }
    return nullptr;
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////////////////////////////////////

GrOp::Owner MakeDashLineOp(GrRecordingContext* context,
                           GrPaint&& paint,
                           const SkMatrix& viewMatrix,
                           const SkPoint pts[2],
                           AAMode aaMode,
                           const GrStyle& style,
                           const GrUserStencilSettings* stencilSettings) {
    SkASSERT(CanDrawDashLine(pts, style, viewMatrix));
    const SkScalar* intervals = style.dashIntervals();
    SkScalar phase = style.dashPhase();

    SkPaint::Cap cap = style.strokeRec().getCap();

    DashOpImpl::LineData lineData;
    lineData.fSrcStrokeWidth = style.strokeRec().getWidth();

    // the phase should be normalized to be [0, sum of all intervals)
    SkASSERT(phase >= 0 && phase < intervals[0] + intervals[1]);

    // Rotate the src pts so they are aligned horizontally with pts[0].fX < pts[1].fX
    if (pts[0].fY != pts[1].fY || pts[0].fX > pts[1].fX) {
        SkMatrix rotMatrix;
        align_to_x_axis(pts, &rotMatrix, lineData.fPtsRot);
        if (!rotMatrix.invert(&lineData.fSrcRotInv)) {
            SkDebugf("Failed to create invertible rotation matrix!\n");
            return nullptr;
        }
    } else {
        lineData.fSrcRotInv.reset();
        memcpy(lineData.fPtsRot, pts, 2 * sizeof(SkPoint));
    }

    // Scale corrections of intervals and stroke from view matrix
    calc_dash_scaling(&lineData.fParallelScale, &lineData.fPerpendicularScale, viewMatrix, pts);
    if (SkScalarNearlyZero(lineData.fParallelScale) ||
        SkScalarNearlyZero(lineData.fPerpendicularScale)) {
        return nullptr;
    }

    SkScalar offInterval = intervals[1] * lineData.fParallelScale;
    SkScalar strokeWidth = lineData.fSrcStrokeWidth * lineData.fPerpendicularScale;

    if (SkPaint::kSquare_Cap == cap && 0 != lineData.fSrcStrokeWidth) {
        // add cap to on interval and remove from off interval
        offInterval -= strokeWidth;
    }

    // TODO we can do a real rect call if not using fulldash(ie no off interval, not using AA)
    bool fullDash = offInterval > 0.f || aaMode != AAMode::kNone;

    lineData.fViewMatrix = viewMatrix;
    lineData.fPhase = phase;
    lineData.fIntervals[0] = intervals[0];
    lineData.fIntervals[1] = intervals[1];

    return DashOpImpl::Make(context, std::move(paint), lineData, cap, aaMode, fullDash,
                            stencilSettings);
}

// Returns whether or not the gpu can fast path the dash line effect.
bool CanDrawDashLine(const SkPoint pts[2], const GrStyle& style, const SkMatrix& viewMatrix) {
    // Pts must be either horizontal or vertical in src space
    if (pts[0].fX != pts[1].fX && pts[0].fY != pts[1].fY) {
        return false;
    }

    // May be able to relax this to include skew. As of now cannot do perspective
    // because of the non uniform scaling of bloating a rect
    if (!viewMatrix.preservesRightAngles()) {
        return false;
    }

    if (!style.isDashed() || 2 != style.dashIntervalCnt()) {
        return false;
    }

    const SkScalar* intervals = style.dashIntervals();
    if (0 == intervals[0] && 0 == intervals[1]) {
        return false;
    }

    SkPaint::Cap cap = style.strokeRec().getCap();
    if (SkPaint::kRound_Cap == cap) {
        // Current we don't support round caps unless the on interval is zero
        if (intervals[0] != 0.f) {
            return false;
        }
        // If the width of the circle caps in greater than the off interval we will pick up unwanted
        // segments of circles at the start and end of the dash line.
        if (style.strokeRec().getWidth() > intervals[1]) {
            return false;
        }
    }

    return true;
}

} // namespace skgpu::ganesh::DashOp

#if defined(GR_TEST_UTILS)

#include "src/gpu/ganesh/GrDrawOpTest.h"

GR_DRAW_OP_TEST_DEFINE(DashOpImpl) {
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    AAMode aaMode;
    do {
        aaMode = static_cast<AAMode>(random->nextULessThan(kAAModeCnt));
    } while (AAMode::kCoverageWithMSAA == aaMode && numSamples <= 1);

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
    SkPaint::Cap cap = SkPaint::Cap(random->nextULessThan(SkPaint::kCapCount));

    SkScalar intervals[2];

    // We can only dash with the following intervals
    enum Intervals {
        kOpenOpen_Intervals ,
        kOpenClose_Intervals,
        kCloseOpen_Intervals,
    };

    Intervals intervalType = SkPaint::kRound_Cap == cap ?
                             kOpenClose_Intervals :
                             Intervals(random->nextULessThan(kCloseOpen_Intervals + 1));
    static const SkScalar kIntervalMin = 0.1f;
    static const SkScalar kIntervalMinCircles = 1.f; // Must be >= to stroke width
    static const SkScalar kIntervalMax = 10.f;
    switch (intervalType) {
        case kOpenOpen_Intervals:
            intervals[0] = random->nextRangeScalar(kIntervalMin, kIntervalMax);
            intervals[1] = random->nextRangeScalar(kIntervalMin, kIntervalMax);
            break;
        case kOpenClose_Intervals: {
            intervals[0] = 0.f;
            SkScalar min = SkPaint::kRound_Cap == cap ? kIntervalMinCircles : kIntervalMin;
            intervals[1] = random->nextRangeScalar(min, kIntervalMax);
            break;
        }
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
    p.setPathEffect(GrTest::TestDashPathEffect::Make(intervals, 2, phase));

    GrStyle style(p);

    return skgpu::ganesh::DashOp::MakeDashLineOp(context, std::move(paint), viewMatrix, pts, aaMode,
                                                 style, GrGetRandomStencil(random, context));
}

#endif // defined(GR_TEST_UTILS)
