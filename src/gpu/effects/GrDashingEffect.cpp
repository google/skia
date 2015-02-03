/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashingEffect.h"

#include "../GrAARectRenderer.h"

#include "GrGeometryProcessor.h"
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawTarget.h"
#include "GrDrawTargetCaps.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "GrStrokeInfo.h"
#include "SkGr.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/builders/GrGLProgramBuilder.h"

///////////////////////////////////////////////////////////////////////////////

// Returns whether or not the gpu can fast path the dash line effect.
static bool can_fast_path_dash(const SkPoint pts[2], const GrStrokeInfo& strokeInfo,
                               const GrDrawTarget& target, const GrPipelineBuilder& pipelineBuilder,
                               const SkMatrix& viewMatrix) {
    if (pipelineBuilder.getRenderTarget()->isMultisampled()) {
        return false;
    }

    // Pts must be either horizontal or vertical in src space
    if (pts[0].fX != pts[1].fX && pts[0].fY != pts[1].fY) {
        return false;
    }

    // May be able to relax this to include skew. As of now cannot do perspective
    // because of the non uniform scaling of bloating a rect
    if (!viewMatrix.preservesRightAngles()) {
        return false;
    }

    if (!strokeInfo.isDashed() || 2 != strokeInfo.dashCount()) {
        return false;
    }

    const SkPathEffect::DashInfo& info = strokeInfo.getDashInfo();
    if (0 == info.fIntervals[0] && 0 == info.fIntervals[1]) {
        return false;
    }

    SkPaint::Cap cap = strokeInfo.getStrokeRec().getCap();
    // Current we do don't handle Round or Square cap dashes
    if (SkPaint::kRound_Cap == cap && info.fIntervals[0] != 0.f) {
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
static void align_to_x_axis(const SkPoint pts[2], SkMatrix* rotMatrix, SkPoint ptsRot[2] = NULL) {
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
static SkScalar calc_start_adjustment(const SkPathEffect::DashInfo& info) {
    SkASSERT(info.fPhase < info.fIntervals[0] + info.fIntervals[1]);
    if (info.fPhase >= info.fIntervals[0] && info.fPhase != 0) {
        SkScalar srcIntervalLen = info.fIntervals[0] + info.fIntervals[1];
        return srcIntervalLen - info.fPhase;
    }
    return 0;
}

static SkScalar calc_end_adjustment(const SkPathEffect::DashInfo& info, const SkPoint pts[2],
                                    SkScalar phase, SkScalar* endingInt) {
    if (pts[1].fX <= pts[0].fX) {
        return 0;
    }
    SkScalar srcIntervalLen = info.fIntervals[0] + info.fIntervals[1];
    SkScalar totalLen = pts[1].fX - pts[0].fX;
    SkScalar temp = SkScalarDiv(totalLen, srcIntervalLen);
    SkScalar numFullIntervals = SkScalarFloorToScalar(temp);
    *endingInt = totalLen - numFullIntervals * srcIntervalLen + phase;
    temp = SkScalarDiv(*endingInt, srcIntervalLen);
    *endingInt = *endingInt - SkScalarFloorToScalar(temp) * srcIntervalLen;
    if (0 == *endingInt) {
        *endingInt = srcIntervalLen;
    }
    if (*endingInt > info.fIntervals[0]) {
        if (0 == info.fIntervals[0]) {
            *endingInt -= 0.01f; // make sure we capture the last zero size pnt (used if has caps)
        }
        return *endingInt - info.fIntervals[0];
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
                              SkScalar offset, SkScalar bloat, SkScalar len, SkScalar stroke) {
    SkScalar startDashX = offset - bloat;
    SkScalar endDashX = offset + len + bloat;
    SkScalar startDashY = -stroke - bloat;
    SkScalar endDashY = stroke + bloat;
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
                              const SkMatrix& matrix, SkScalar offset, SkScalar bloat,
                              SkScalar len, SkScalar stroke, SkScalar startInterval,
                              SkScalar endInterval, SkScalar strokeWidth, DashCap cap,
                              const size_t vertexStride) {
    SkScalar intervalLength = startInterval + endInterval;

    if (kRound_DashCap == cap) {
        SkASSERT(vertexStride == sizeof(DashCircleVertex));
        DashCircleVertex* verts = reinterpret_cast<DashCircleVertex*>(vertices);

        setup_dashed_rect_common<DashCircleVertex>(rect, matrix, verts, idx, offset, bloat, len,
                                                   stroke);

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

        setup_dashed_rect_common<DashLineVertex>(rect, matrix, verts, idx, offset, bloat, len,
                                                 stroke);

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
                                           GrPrimitiveEdgeType edgeType,
                                           DashCap cap,
                                           const SkMatrix& localMatrix);

bool GrDashingEffect::DrawDashLine(GrGpu* gpu, GrDrawTarget* target,
                                   GrPipelineBuilder* pipelineBuilder, GrColor color,
                                   const SkMatrix& viewMatrix, const SkPoint pts[2],
                                   const GrPaint& paint, const GrStrokeInfo& strokeInfo) {
    if (!can_fast_path_dash(pts, strokeInfo, *target, *pipelineBuilder, viewMatrix)) {
        return false;
    }

    const SkPathEffect::DashInfo& info = strokeInfo.getDashInfo();

    SkPaint::Cap cap = strokeInfo.getStrokeRec().getCap();

    SkScalar srcStrokeWidth = strokeInfo.getStrokeRec().getWidth();

    // the phase should be normalized to be [0, sum of all intervals)
    SkASSERT(info.fPhase >= 0 && info.fPhase < info.fIntervals[0] + info.fIntervals[1]);

    SkScalar srcPhase = info.fPhase;

    // Rotate the src pts so they are aligned horizontally with pts[0].fX < pts[1].fX
    SkMatrix srcRotInv;
    SkPoint ptsRot[2];
    if (pts[0].fY != pts[1].fY || pts[0].fX > pts[1].fX) {
        SkMatrix rotMatrix;
        align_to_x_axis(pts, &rotMatrix, ptsRot);
        if(!rotMatrix.invert(&srcRotInv)) {
            SkDebugf("Failed to create invertible rotation matrix!\n");
            return false;
        }
    } else {
        srcRotInv.reset();
        memcpy(ptsRot, pts, 2 * sizeof(SkPoint));
    }

    bool useAA = paint.isAntiAlias();

    // Scale corrections of intervals and stroke from view matrix
    SkScalar parallelScale;
    SkScalar perpScale;
    calc_dash_scaling(&parallelScale, &perpScale, viewMatrix, ptsRot);

    bool hasCap = SkPaint::kButt_Cap != cap && 0 != srcStrokeWidth;

    // We always want to at least stroke out half a pixel on each side in device space
    // so 0.5f / perpScale gives us this min in src space
    SkScalar halfSrcStroke = SkMaxScalar(srcStrokeWidth * 0.5f, 0.5f / perpScale);

    SkScalar strokeAdj;
    if (!hasCap) {
        strokeAdj = 0.f;
    } else {
        strokeAdj = halfSrcStroke;
    }

    SkScalar startAdj = 0;

    SkMatrix combinedMatrix = srcRotInv;
    combinedMatrix.postConcat(viewMatrix);

    bool lineDone = false;
    SkRect startRect;
    bool hasStartRect = false;
    // If we are using AA, check to see if we are drawing a partial dash at the start. If so
    // draw it separately here and adjust our start point accordingly
    if (useAA) {
        if (srcPhase > 0 && srcPhase < info.fIntervals[0]) {
            SkPoint startPts[2];
            startPts[0] = ptsRot[0];
            startPts[1].fY = startPts[0].fY;
            startPts[1].fX = SkMinScalar(startPts[0].fX + info.fIntervals[0] - srcPhase,
                                         ptsRot[1].fX);
            startRect.set(startPts, 2);
            startRect.outset(strokeAdj, halfSrcStroke);

            hasStartRect = true;
            startAdj = info.fIntervals[0] + info.fIntervals[1] - srcPhase;
        }
    }

    // adjustments for start and end of bounding rect so we only draw dash intervals
    // contained in the original line segment.
    startAdj += calc_start_adjustment(info);
    if (startAdj != 0) {
        ptsRot[0].fX += startAdj;
        srcPhase = 0;
    }
    SkScalar endingInterval = 0;
    SkScalar endAdj = calc_end_adjustment(info, ptsRot, srcPhase, &endingInterval);
    ptsRot[1].fX -= endAdj;
    if (ptsRot[0].fX >= ptsRot[1].fX) {
        lineDone = true;
    }

    SkRect endRect;
    bool hasEndRect = false;
    // If we are using AA, check to see if we are drawing a partial dash at then end. If so
    // draw it separately here and adjust our end point accordingly
    if (useAA && !lineDone) {
        // If we adjusted the end then we will not be drawing a partial dash at the end.
        // If we didn't adjust the end point then we just need to make sure the ending
        // dash isn't a full dash
        if (0 == endAdj && endingInterval != info.fIntervals[0]) {
            SkPoint endPts[2];
            endPts[1] = ptsRot[1];
            endPts[0].fY = endPts[1].fY;
            endPts[0].fX = endPts[1].fX - endingInterval;

            endRect.set(endPts, 2);
            endRect.outset(strokeAdj, halfSrcStroke);

            hasEndRect = true;
            endAdj = endingInterval + info.fIntervals[1];

            ptsRot[1].fX -= endAdj;
            if (ptsRot[0].fX >= ptsRot[1].fX) {
                lineDone = true;
            }
        }
    }

    if (startAdj != 0) {
        srcPhase = 0;
    }

    // Change the dashing info from src space into device space
    SkScalar devIntervals[2];
    devIntervals[0] = info.fIntervals[0] * parallelScale;
    devIntervals[1] = info.fIntervals[1] * parallelScale;
    SkScalar devPhase = srcPhase * parallelScale;
    SkScalar strokeWidth = srcStrokeWidth * perpScale;

    if ((strokeWidth < 1.f && !useAA) || 0.f == strokeWidth) {
        strokeWidth = 1.f;
    }

    SkScalar halfDevStroke = strokeWidth * 0.5f;

    if (SkPaint::kSquare_Cap == cap && 0 != srcStrokeWidth) {
        // add cap to on interveal and remove from off interval
        devIntervals[0] += strokeWidth;
        devIntervals[1] -= strokeWidth;
    }
    SkScalar startOffset = devIntervals[1] * 0.5f + devPhase;

    SkScalar bloatX = useAA ? 0.5f / parallelScale : 0.f;
    SkScalar bloatY = useAA ? 0.5f / perpScale : 0.f;

    SkScalar devBloat = useAA ? 0.5f : 0.f;

    if (devIntervals[1] <= 0.f && useAA) {
        // Case when we end up drawing a solid AA rect
        // Reset the start rect to draw this single solid rect
        // but it requires to upload a new intervals uniform so we can mimic
        // one giant dash
        ptsRot[0].fX -= hasStartRect ? startAdj : 0;
        ptsRot[1].fX += hasEndRect ? endAdj : 0;
        startRect.set(ptsRot, 2);
        startRect.outset(strokeAdj, halfSrcStroke);
        hasStartRect = true;
        hasEndRect = false;
        lineDone = true;

        SkPoint devicePts[2];
        viewMatrix.mapPoints(devicePts, ptsRot, 2);
        SkScalar lineLength = SkPoint::Distance(devicePts[0], devicePts[1]);
        if (hasCap) {
            lineLength += 2.f * halfDevStroke;
        }
        devIntervals[0] = lineLength;
    }

    // reset to device coordinates
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        SkDebugf("Failed to invert\n");
        return false;
    }

    bool isRoundCap = SkPaint::kRound_Cap == cap;
    DashCap capType = isRoundCap ? kRound_DashCap : kNonRound_DashCap;

    SkAutoTUnref<const GrGeometryProcessor> gp;
    bool fullDash = devIntervals[1] > 0.f || useAA;
    if (fullDash) {
        SkPathEffect::DashInfo devInfo;
        devInfo.fPhase = devPhase;
        devInfo.fCount = 2;
        devInfo.fIntervals = devIntervals;
        GrPrimitiveEdgeType edgeType = useAA ? kFillAA_GrProcessorEdgeType :
            kFillBW_GrProcessorEdgeType;
        gp.reset(create_dash_gp(color, edgeType, capType, invert));
    } else {
        // Set up the vertex data for the line and start/end dashes
        gp.reset(GrDefaultGeoProcFactory::Create(GrDefaultGeoProcFactory::kPosition_GPType,
                                                 color,
                                                 SkMatrix::I(),
                                                 invert));
    }

    int totalRectCnt = 0;

    totalRectCnt += !lineDone ? 1 : 0;
    totalRectCnt += hasStartRect ? 1 : 0;
    totalRectCnt += hasEndRect ? 1 : 0;

    GrDrawTarget::AutoReleaseGeometry geo(target,
                                          totalRectCnt * 4,
                                          gp->getVertexStride(), 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return false;
    }

    int curVIdx = 0;

    if (SkPaint::kRound_Cap == cap && 0 != srcStrokeWidth) {
        // need to adjust this for round caps to correctly set the dashPos attrib on vertices
        startOffset -= halfDevStroke;
    }

    // Draw interior part of dashed line
    if (!lineDone) {
        SkPoint devicePts[2];
        viewMatrix.mapPoints(devicePts, ptsRot, 2);
        SkScalar lineLength = SkPoint::Distance(devicePts[0], devicePts[1]);
        if (hasCap) {
            lineLength += 2.f * halfDevStroke;
        }

        SkRect bounds;
        bounds.set(ptsRot[0].fX, ptsRot[0].fY, ptsRot[1].fX, ptsRot[1].fY);
        bounds.outset(bloatX + strokeAdj, bloatY + halfSrcStroke);
        if (fullDash) {
            setup_dashed_rect(bounds, geo.vertices(), curVIdx, combinedMatrix, startOffset,
                              devBloat, lineLength, halfDevStroke, devIntervals[0], devIntervals[1],
                              strokeWidth, capType, gp->getVertexStride());
        } else {
            SkPoint* verts = reinterpret_cast<SkPoint*>(geo.vertices());
            SkASSERT(gp->getVertexStride() == sizeof(SkPoint));
            setup_dashed_rect_pos(bounds, curVIdx, combinedMatrix, verts);
        }
        curVIdx += 4;
    }

    if (hasStartRect) {
        SkASSERT(useAA);  // so that we know bloatX and bloatY have been set
        startRect.outset(bloatX, bloatY);
        if (fullDash) {
            setup_dashed_rect(startRect, geo.vertices(), curVIdx, combinedMatrix, startOffset,
                              devBloat, devIntervals[0], halfDevStroke, devIntervals[0],
                              devIntervals[1], strokeWidth, capType, gp->getVertexStride());
        } else {
            SkPoint* verts = reinterpret_cast<SkPoint*>(geo.vertices());
            SkASSERT(gp->getVertexStride() == sizeof(SkPoint));
            setup_dashed_rect_pos(startRect, curVIdx, combinedMatrix, verts);
        }

        curVIdx += 4;
    }

    if (hasEndRect) {
        SkASSERT(useAA);  // so that we know bloatX and bloatY have been set
        endRect.outset(bloatX, bloatY);
        if (fullDash) {
            setup_dashed_rect(endRect, geo.vertices(), curVIdx, combinedMatrix, startOffset,
                              devBloat, devIntervals[0], halfDevStroke, devIntervals[0],
                              devIntervals[1], strokeWidth, capType, gp->getVertexStride());
        } else {
            SkPoint* verts = reinterpret_cast<SkPoint*>(geo.vertices());
            SkASSERT(gp->getVertexStride() == sizeof(SkPoint));
            setup_dashed_rect_pos(endRect, curVIdx, combinedMatrix, verts);
        }

    }

    target->setIndexSourceToBuffer(gpu->getContext()->getQuadIndexBuffer());
    target->drawIndexedInstances(pipelineBuilder, gp, kTriangles_GrPrimitiveType,
                                 totalRectCnt, 4, 6);
    target->resetIndexSource();
    return true;
}

//////////////////////////////////////////////////////////////////////////////

class GLDashingCircleEffect;

struct DashingCircleBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

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
                                       GrPrimitiveEdgeType edgeType,
                                       const SkMatrix& localMatrix);

    virtual ~DashingCircleEffect();

    const char* name() const SK_OVERRIDE { return "DashingCircleEffect"; }

    const Attribute* inPosition() const { return fInPosition; }

    const Attribute* inDashParams() const { return fInDashParams; }

    const Attribute* inCircleParams() const { return fInCircleParams; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    virtual void getGLProcessorKey(const GrBatchTracker&,
                                   const GrGLCaps&,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker&,
                                                     const GrGLCaps&) const SK_OVERRIDE;

    void initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const SK_OVERRIDE;

    bool onCanMakeEqual(const GrBatchTracker&,
                        const GrGeometryProcessor&,
                        const GrBatchTracker&) const SK_OVERRIDE;

private:
    DashingCircleEffect(GrColor, GrPrimitiveEdgeType edgeType, const SkMatrix& localMatrix);

    bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE;

    void onGetInvariantOutputCoverage(GrInitInvariantOutput*) const SK_OVERRIDE;

    GrPrimitiveEdgeType fEdgeType;
    const Attribute*    fInPosition;
    const Attribute*    fInDashParams;
    const Attribute*    fInCircleParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLDashingCircleEffect : public GrGLGeometryProcessor {
public:
    GLDashingCircleEffect(const GrGeometryProcessor&, const GrBatchTracker&);

    void onEmitCode(EmitArgs&, GrGPArgs*) SK_OVERRIDE;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrBatchTracker&,
                              const GrGLCaps&,
                              GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&,
                         const GrPrimitiveProcessor&,
                         const GrBatchTracker&) SK_OVERRIDE;

private:
    UniformHandle fParamUniform;
    UniformHandle fColorUniform;
    GrColor       fColor;
    SkScalar      fPrevRadius;
    SkScalar      fPrevCenterX;
    SkScalar      fPrevIntervalLength;
    typedef GrGLGeometryProcessor INHERITED;
};

GLDashingCircleEffect::GLDashingCircleEffect(const GrGeometryProcessor&,
                                             const GrBatchTracker&) {
    fColor = GrColor_ILLEGAL;
    fPrevRadius = SK_ScalarMin;
    fPrevCenterX = SK_ScalarMin;
    fPrevIntervalLength = SK_ScalarMax;
}

void GLDashingCircleEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const DashingCircleEffect& dce = args.fGP.cast<DashingCircleEffect>();
    const DashingCircleBatchTracker local = args.fBT.cast<DashingCircleBatchTracker>();
    GrGLGPBuilder* pb = args.fPB;
    GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

    // emit attributes
    vsBuilder->emitAttributes(dce);

    // XY are dashPos, Z is dashInterval
    GrGLVertToFrag dashParams(kVec3f_GrSLType);
    args.fPB->addVarying("DashParam", &dashParams);
    vsBuilder->codeAppendf("%s = %s;", dashParams.vsOut(), dce.inDashParams()->fName);

    // xy, refer to circle radius - 0.5, z refers to cicles center x coord
    GrGLVertToFrag circleParams(kVec2f_GrSLType);
    args.fPB->addVarying("CircleParams", &circleParams);
    vsBuilder->codeAppendf("%s = %s;", circleParams.vsOut(), dce.inCircleParams()->fName);

    // Setup pass through color
    this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL, &fColorUniform);

    // setup uniform viewMatrix
    this->addUniformViewMatrix(pb);

    // Setup position
    SetupPosition(vsBuilder, gpArgs, dce.inPosition()->fName, dce.viewMatrix(), this->uViewM());

    // emit transforms
    this->emitTransforms(args.fPB, gpArgs->fPositionVar, dce.inPosition()->fName, dce.localMatrix(),
                         args.fTransformsIn, args.fTransformsOut);

    // transforms all points so that we can compare them to our test circle
    GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("float xShifted = %s.x - floor(%s.x / %s.z) * %s.z;",
                           dashParams.fsIn(), dashParams.fsIn(), dashParams.fsIn(),
                           dashParams.fsIn());
    fsBuilder->codeAppendf("vec2 fragPosShifted = vec2(xShifted, %s.y);", dashParams.fsIn());
    fsBuilder->codeAppendf("vec2 center = vec2(%s.y, 0.0);", circleParams.fsIn());
    fsBuilder->codeAppend("float dist = length(center - fragPosShifted);");
    if (GrProcessorEdgeTypeIsAA(dce.getEdgeType())) {
        fsBuilder->codeAppendf("float diff = dist - %s.x;", circleParams.fsIn());
        fsBuilder->codeAppend("diff = 1.0 - diff;");
        fsBuilder->codeAppend("float alpha = clamp(diff, 0.0, 1.0);");
    } else {
        fsBuilder->codeAppendf("float alpha = 1.0;");
        fsBuilder->codeAppendf("alpha *=  dist < %s.x + 0.5 ? 1.0 : 0.0;", circleParams.fsIn());
    }
    fsBuilder->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
}

void GLDashingCircleEffect::setData(const GrGLProgramDataManager& pdman,
                                    const GrPrimitiveProcessor& processor,
                                    const GrBatchTracker& bt) {
    this->setUniformViewMatrix(pdman, processor.viewMatrix());

    const DashingCircleBatchTracker& local = bt.cast<DashingCircleBatchTracker>();
    if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
        GrGLfloat c[4];
        GrColorToRGBAFloat(local.fColor, c);
        pdman.set4fv(fColorUniform, 1, c);
        fColor = local.fColor;
    }
}

void GLDashingCircleEffect::GenKey(const GrGeometryProcessor& gp,
                                   const GrBatchTracker& bt,
                                   const GrGLCaps&,
                                   GrProcessorKeyBuilder* b) {
    const DashingCircleBatchTracker& local = bt.cast<DashingCircleBatchTracker>();
    const DashingCircleEffect& dce = gp.cast<DashingCircleEffect>();
    uint32_t key = 0;
    key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x1 : 0x0;
    key |= ComputePosKey(gp.viewMatrix()) << 1;
    key |= dce.getEdgeType() << 8;
    b->add32(key << 16 | local.fInputColorType);
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingCircleEffect::Create(GrColor color,
                                                 GrPrimitiveEdgeType edgeType,
                                                 const SkMatrix& localMatrix) {
    return SkNEW_ARGS(DashingCircleEffect, (color, edgeType, localMatrix));
}

DashingCircleEffect::~DashingCircleEffect() {}

void DashingCircleEffect::onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setUnknownSingleComponent();
}

void DashingCircleEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                            const GrGLCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GLDashingCircleEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor* DashingCircleEffect::createGLInstance(const GrBatchTracker& bt,
                                                              const GrGLCaps&) const {
    return SkNEW_ARGS(GLDashingCircleEffect, (*this, bt));
}

DashingCircleEffect::DashingCircleEffect(GrColor color,
                                         GrPrimitiveEdgeType edgeType,
                                         const SkMatrix& localMatrix)
    : INHERITED(color, SkMatrix::I(), localMatrix), fEdgeType(edgeType) {
    this->initClassID<DashingCircleEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
    fInDashParams = &this->addVertexAttrib(Attribute("inDashParams", kVec3f_GrVertexAttribType));
    fInCircleParams = &this->addVertexAttrib(Attribute("inCircleParams",
                                                       kVec2f_GrVertexAttribType));
}

bool DashingCircleEffect::onIsEqual(const GrGeometryProcessor& other) const {
    const DashingCircleEffect& dce = other.cast<DashingCircleEffect>();
    return fEdgeType == dce.fEdgeType;
}

void DashingCircleEffect::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
    DashingCircleBatchTracker* local = bt->cast<DashingCircleBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool DashingCircleEffect::onCanMakeEqual(const GrBatchTracker& m,
                                         const GrGeometryProcessor& that,
                                         const GrBatchTracker& t) const {
    const DashingCircleBatchTracker& mine = m.cast<DashingCircleBatchTracker>();
    const DashingCircleBatchTracker& theirs = t.cast<DashingCircleBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                   that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor);
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingCircleEffect);

GrGeometryProcessor* DashingCircleEffect::TestCreate(SkRandom* random,
                                                     GrContext*,
                                                     const GrDrawTargetCaps& caps,
                                                     GrTexture*[]) {
    GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(random->nextULessThan(
            kGrProcessorEdgeTypeCnt));
    return DashingCircleEffect::Create(GrRandomColor(random),
                                       edgeType, GrProcessorUnitTest::TestMatrix(random));
}

//////////////////////////////////////////////////////////////////////////////

class GLDashingLineEffect;

struct DashingLineBatchTracker {
    GrGPInput fInputColorType;
    GrColor  fColor;
    bool fUsesLocalCoords;
};

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
                                       GrPrimitiveEdgeType edgeType,
                                       const SkMatrix& localMatrix);

    virtual ~DashingLineEffect();

    const char* name() const SK_OVERRIDE { return "DashingEffect"; }

    const Attribute* inPosition() const { return fInPosition; }

    const Attribute* inDashParams() const { return fInDashParams; }

    const Attribute* inRectParams() const { return fInRectParams; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLCaps&) const SK_OVERRIDE;

    void initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const SK_OVERRIDE;

    bool onCanMakeEqual(const GrBatchTracker&,
                        const GrGeometryProcessor&,
                        const GrBatchTracker&) const SK_OVERRIDE;

private:
    DashingLineEffect(GrColor, GrPrimitiveEdgeType edgeType, const SkMatrix& localMatrix);

    bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE;

    void onGetInvariantOutputCoverage(GrInitInvariantOutput*) const SK_OVERRIDE;

    GrPrimitiveEdgeType fEdgeType;
    const Attribute*    fInPosition;
    const Attribute*    fInDashParams;
    const Attribute*    fInRectParams;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLDashingLineEffect : public GrGLGeometryProcessor {
public:
    GLDashingLineEffect(const GrGeometryProcessor&, const GrBatchTracker&);

    void onEmitCode(EmitArgs&, GrGPArgs*) SK_OVERRIDE;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrBatchTracker&,
                              const GrGLCaps&,
                              GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&,
                         const GrPrimitiveProcessor&,
                         const GrBatchTracker&) SK_OVERRIDE;

private:
    GrColor       fColor;
    UniformHandle fColorUniform;
    typedef GrGLGeometryProcessor INHERITED;
};

GLDashingLineEffect::GLDashingLineEffect(const GrGeometryProcessor&,
                                         const GrBatchTracker&) {
    fColor = GrColor_ILLEGAL;
}

void GLDashingLineEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const DashingLineEffect& de = args.fGP.cast<DashingLineEffect>();
    const DashingLineBatchTracker& local = args.fBT.cast<DashingLineBatchTracker>();
    GrGLGPBuilder* pb = args.fPB;

    GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

    // emit attributes
    vsBuilder->emitAttributes(de);

    // XY refers to dashPos, Z is the dash interval length
    GrGLVertToFrag inDashParams(kVec3f_GrSLType);
    args.fPB->addVarying("DashParams", &inDashParams);
    vsBuilder->codeAppendf("%s = %s;", inDashParams.vsOut(), de.inDashParams()->fName);

    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    GrGLVertToFrag inRectParams(kVec4f_GrSLType);
    args.fPB->addVarying("RectParams", &inRectParams);
    vsBuilder->codeAppendf("%s = %s;", inRectParams.vsOut(), de.inRectParams()->fName);

    // Setup pass through color
    this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL, &fColorUniform);

    // setup uniform viewMatrix
    this->addUniformViewMatrix(pb);

    // Setup position
    SetupPosition(vsBuilder, gpArgs, de.inPosition()->fName, de.viewMatrix(), this->uViewM());

    // emit transforms
    this->emitTransforms(args.fPB, gpArgs->fPositionVar, de.inPosition()->fName, de.localMatrix(),
                         args.fTransformsIn, args.fTransformsOut);

    // transforms all points so that we can compare them to our test rect
    GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("float xShifted = %s.x - floor(%s.x / %s.z) * %s.z;",
                           inDashParams.fsIn(), inDashParams.fsIn(), inDashParams.fsIn(),
                           inDashParams.fsIn());
    fsBuilder->codeAppendf("vec2 fragPosShifted = vec2(xShifted, %s.y);", inDashParams.fsIn());
    if (GrProcessorEdgeTypeIsAA(de.getEdgeType())) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        fsBuilder->codeAppend("float xSub, ySub;");
        fsBuilder->codeAppendf("xSub = min(fragPosShifted.x - %s.x, 0.0);", inRectParams.fsIn());
        fsBuilder->codeAppendf("xSub += min(%s.z - fragPosShifted.x, 0.0);", inRectParams.fsIn());
        fsBuilder->codeAppendf("ySub = min(fragPosShifted.y - %s.y, 0.0);", inRectParams.fsIn());
        fsBuilder->codeAppendf("ySub += min(%s.w - fragPosShifted.y, 0.0);", inRectParams.fsIn());
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        fsBuilder->codeAppendf("float alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));");
    } else {
        // Assuming the bounding geometry is tight so no need to check y values
        fsBuilder->codeAppendf("float alpha = 1.0;");
        fsBuilder->codeAppendf("alpha *= (fragPosShifted.x - %s.x) > -0.5 ? 1.0 : 0.0;",
                               inRectParams.fsIn());
        fsBuilder->codeAppendf("alpha *= (%s.z - fragPosShifted.x) >= -0.5 ? 1.0 : 0.0;",
                               inRectParams.fsIn());
    }
    fsBuilder->codeAppendf("%s = vec4(alpha);", args.fOutputCoverage);
}

void GLDashingLineEffect::setData(const GrGLProgramDataManager& pdman,
                                  const GrPrimitiveProcessor& processor,
                                  const GrBatchTracker& bt) {
    this->setUniformViewMatrix(pdman, processor.viewMatrix());

    const DashingLineBatchTracker& local = bt.cast<DashingLineBatchTracker>();
    if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
        GrGLfloat c[4];
        GrColorToRGBAFloat(local.fColor, c);
        pdman.set4fv(fColorUniform, 1, c);
        fColor = local.fColor;
    }
}

void GLDashingLineEffect::GenKey(const GrGeometryProcessor& gp,
                                 const GrBatchTracker& bt,
                                 const GrGLCaps&,
                                 GrProcessorKeyBuilder* b) {
    const DashingLineBatchTracker& local = bt.cast<DashingLineBatchTracker>();
    const DashingLineEffect& de = gp.cast<DashingLineEffect>();
    uint32_t key = 0;
    key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x1 : 0x0;
    key |= ComputePosKey(gp.viewMatrix()) << 1;
    key |= de.getEdgeType() << 8;
    b->add32(key << 16 | local.fInputColorType);
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingLineEffect::Create(GrColor color,
                                               GrPrimitiveEdgeType edgeType,
                                               const SkMatrix& localMatrix) {
    return SkNEW_ARGS(DashingLineEffect, (color, edgeType, localMatrix));
}

DashingLineEffect::~DashingLineEffect() {}

void DashingLineEffect::onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setUnknownSingleComponent();
}

void DashingLineEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                          const GrGLCaps& caps,
                                          GrProcessorKeyBuilder* b) const {
    GLDashingLineEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor* DashingLineEffect::createGLInstance(const GrBatchTracker& bt,
                                                            const GrGLCaps&) const {
    return SkNEW_ARGS(GLDashingLineEffect, (*this, bt));
}

DashingLineEffect::DashingLineEffect(GrColor color,
                                     GrPrimitiveEdgeType edgeType,
                                     const SkMatrix& localMatrix)
    : INHERITED(color, SkMatrix::I(), localMatrix), fEdgeType(edgeType) {
    this->initClassID<DashingLineEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
    fInDashParams = &this->addVertexAttrib(Attribute("inDashParams", kVec3f_GrVertexAttribType));
    fInRectParams = &this->addVertexAttrib(Attribute("inRect", kVec4f_GrVertexAttribType));
}

bool DashingLineEffect::onIsEqual(const GrGeometryProcessor& other) const {
    const DashingLineEffect& de = other.cast<DashingLineEffect>();
    return fEdgeType == de.fEdgeType;
}

void DashingLineEffect::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
    DashingLineBatchTracker* local = bt->cast<DashingLineBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

bool DashingLineEffect::onCanMakeEqual(const GrBatchTracker& m,
                                       const GrGeometryProcessor& that,
                                       const GrBatchTracker& t) const {
    const DashingLineBatchTracker& mine = m.cast<DashingLineBatchTracker>();
    const DashingLineBatchTracker& theirs = t.cast<DashingLineBatchTracker>();
    return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                  that, theirs.fUsesLocalCoords) &&
           CanCombineOutput(mine.fInputColorType, mine.fColor,
                            theirs.fInputColorType, theirs.fColor);
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingLineEffect);

GrGeometryProcessor* DashingLineEffect::TestCreate(SkRandom* random,
                                                   GrContext*,
                                                   const GrDrawTargetCaps& caps,
                                                   GrTexture*[]) {
    GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(random->nextULessThan(
            kGrProcessorEdgeTypeCnt));

    return DashingLineEffect::Create(GrRandomColor(random),
                                     edgeType, GrProcessorUnitTest::TestMatrix(random));
}

//////////////////////////////////////////////////////////////////////////////

static GrGeometryProcessor* create_dash_gp(GrColor color,
                                           GrPrimitiveEdgeType edgeType,
                                           DashCap cap,
                                           const SkMatrix& localMatrix) {
    switch (cap) {
        case kRound_DashCap:
            return DashingCircleEffect::Create(color, edgeType, localMatrix);
        case kNonRound_DashCap:
            return DashingLineEffect::Create(color, edgeType, localMatrix);
        default:
            SkFAIL("Unexpected dashed cap.");
    }
    return NULL;
}
