/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashingEffect.h"

#include "../GrAARectRenderer.h"

#include "GrGeometryProcessor.h"
#include "gl/builders/GrGLFullProgramBuilder.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/GrGLSL.h"
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrDrawTarget.h"
#include "GrDrawTargetCaps.h"
#include "GrProcessor.h"
#include "GrGpu.h"
#include "GrStrokeInfo.h"
#include "GrTBackendProcessorFactory.h"
#include "SkGr.h"

///////////////////////////////////////////////////////////////////////////////

// Returns whether or not the gpu can fast path the dash line effect.
static bool can_fast_path_dash(const SkPoint pts[2], const GrStrokeInfo& strokeInfo,
                               const GrDrawTarget& target, const SkMatrix& viewMatrix) {
    if (target.getDrawState().getRenderTarget()->isMultisampled()) {
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
};

extern const GrVertexAttrib gDashLineNoAAVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding }
};

extern const GrVertexAttrib gDashLineVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, sizeof(SkPoint),   kGeometryProcessor_GrVertexAttribBinding },
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

static void setup_dashed_rect(const SkRect& rect, DashLineVertex* verts, int idx, const SkMatrix& matrix,
                       SkScalar offset, SkScalar bloat, SkScalar len, SkScalar stroke) {

        SkScalar startDashX = offset - bloat;
        SkScalar endDashX = offset + len + bloat;
        SkScalar startDashY = -stroke - bloat;
        SkScalar endDashY = stroke + bloat;
        verts[idx].fDashPos = SkPoint::Make(startDashX , startDashY);
        verts[idx + 1].fDashPos = SkPoint::Make(startDashX, endDashY);
        verts[idx + 2].fDashPos = SkPoint::Make(endDashX, endDashY);
        verts[idx + 3].fDashPos = SkPoint::Make(endDashX, startDashY);

        verts[idx].fPos = SkPoint::Make(rect.fLeft, rect.fTop);
        verts[idx + 1].fPos = SkPoint::Make(rect.fLeft, rect.fBottom);
        verts[idx + 2].fPos = SkPoint::Make(rect.fRight, rect.fBottom);
        verts[idx + 3].fPos = SkPoint::Make(rect.fRight, rect.fTop);

        matrix.mapPointsWithStride(&verts[idx].fPos, sizeof(DashLineVertex), 4);
}


bool GrDashingEffect::DrawDashLine(const SkPoint pts[2], const GrPaint& paint,
                                   const GrStrokeInfo& strokeInfo, GrGpu* gpu,
                                   GrDrawTarget* target, const SkMatrix& vm) {

    if (!can_fast_path_dash(pts, strokeInfo, *target, vm)) {
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
            GrPrintf("Failed to create invertible rotation matrix!\n");
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
    calc_dash_scaling(&parallelScale, &perpScale, vm, ptsRot);

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
    combinedMatrix.postConcat(vm);

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

    GrDrawState* drawState = target->drawState();
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
        vm.mapPoints(devicePts, ptsRot, 2);
        SkScalar lineLength = SkPoint::Distance(devicePts[0], devicePts[1]);
        if (hasCap) {
            lineLength += 2.f * halfDevStroke;
        }
        devIntervals[0] = lineLength;
    }
    if (devIntervals[1] > 0.f || useAA) {
        SkPathEffect::DashInfo devInfo;
        devInfo.fPhase = devPhase;
        devInfo.fCount = 2;
        devInfo.fIntervals = devIntervals;
        GrPrimitiveEdgeType edgeType= useAA ? kFillAA_GrProcessorEdgeType :
            kFillBW_GrProcessorEdgeType;
        bool isRoundCap = SkPaint::kRound_Cap == cap;
        GrDashingEffect::DashCap capType = isRoundCap ? GrDashingEffect::kRound_DashCap :
                                                        GrDashingEffect::kNonRound_DashCap;
        drawState->setGeometryProcessor(
                GrDashingEffect::Create(edgeType, devInfo, strokeWidth, capType))->unref();

        // Set up the vertex data for the line and start/end dashes
        drawState->setVertexAttribs<gDashLineVertexAttribs>(SK_ARRAY_COUNT(gDashLineVertexAttribs),
                                                            sizeof(DashLineVertex));
    } else {
        // Set up the vertex data for the line and start/end dashes
        drawState->setVertexAttribs<gDashLineNoAAVertexAttribs>(
                SK_ARRAY_COUNT(gDashLineNoAAVertexAttribs), sizeof(DashLineVertex));
    }

    int totalRectCnt = 0;

    totalRectCnt += !lineDone ? 1 : 0;
    totalRectCnt += hasStartRect ? 1 : 0;
    totalRectCnt += hasEndRect ? 1 : 0;

    GrDrawTarget::AutoReleaseGeometry geo(target, totalRectCnt * 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return false;
    }

    DashLineVertex* verts = reinterpret_cast<DashLineVertex*>(geo.vertices());

    int curVIdx = 0;

    if (SkPaint::kRound_Cap == cap && 0 != srcStrokeWidth) {
        // need to adjust this for round caps to correctly set the dashPos attrib on vertices
        startOffset -= halfDevStroke;
    }

    // Draw interior part of dashed line
    if (!lineDone) {
        SkPoint devicePts[2];
        vm.mapPoints(devicePts, ptsRot, 2);
        SkScalar lineLength = SkPoint::Distance(devicePts[0], devicePts[1]);
        if (hasCap) {
            lineLength += 2.f * halfDevStroke;
        }

        SkRect bounds;
        bounds.set(ptsRot[0].fX, ptsRot[0].fY, ptsRot[1].fX, ptsRot[1].fY);
        bounds.outset(bloatX + strokeAdj, bloatY + halfSrcStroke);
        setup_dashed_rect(bounds, verts, curVIdx, combinedMatrix, startOffset, devBloat,
                          lineLength, halfDevStroke);
        curVIdx += 4;
    }

    if (hasStartRect) {
        SkASSERT(useAA);  // so that we know bloatX and bloatY have been set
        startRect.outset(bloatX, bloatY);
        setup_dashed_rect(startRect, verts, curVIdx, combinedMatrix, startOffset, devBloat,
                          devIntervals[0], halfDevStroke);
        curVIdx += 4;
    }

    if (hasEndRect) {
        SkASSERT(useAA);  // so that we know bloatX and bloatY have been set
        endRect.outset(bloatX, bloatY);
        setup_dashed_rect(endRect, verts, curVIdx, combinedMatrix, startOffset, devBloat,
                          devIntervals[0], halfDevStroke);
    }

    target->setIndexSourceToBuffer(gpu->getContext()->getQuadIndexBuffer());
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, totalRectCnt, 4, 6);
    target->resetIndexSource();
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

    static GrGeometryProcessor* Create(GrPrimitiveEdgeType edgeType,
                                       const DashInfo& info,
                                       SkScalar radius);

    virtual ~DashingCircleEffect();

    static const char* Name() { return "DashingCircleEffect"; }

    const GrShaderVar& inCoord() const { return fInCoord; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    SkScalar getRadius() const { return fRadius; }

    SkScalar getCenterX() const { return fCenterX; }

    SkScalar getIntervalLength() const { return fIntervalLength; }

    typedef GLDashingCircleEffect GLProcessor;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE;

private:
    DashingCircleEffect(GrPrimitiveEdgeType edgeType, const DashInfo& info, SkScalar radius);

    virtual bool onIsEqual(const GrProcessor& other) const SK_OVERRIDE;

    GrPrimitiveEdgeType    fEdgeType;
    const GrShaderVar&  fInCoord;
    SkScalar            fIntervalLength;
    SkScalar            fRadius;
    SkScalar            fCenterX;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLDashingCircleEffect : public GrGLGeometryProcessor {
public:
    GLDashingCircleEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

private:
    GrGLProgramDataManager::UniformHandle fParamUniform;
    SkScalar                              fPrevRadius;
    SkScalar                              fPrevCenterX;
    SkScalar                              fPrevIntervalLength;
    typedef GrGLGeometryProcessor INHERITED;
};

GLDashingCircleEffect::GLDashingCircleEffect(const GrBackendProcessorFactory& factory,
                                             const GrProcessor&)
    : INHERITED (factory) {
    fPrevRadius = SK_ScalarMin;
    fPrevCenterX = SK_ScalarMin;
    fPrevIntervalLength = SK_ScalarMax;
}

void GLDashingCircleEffect::emitCode(GrGLFullProgramBuilder* builder,
                                    const GrGeometryProcessor& geometryProcessor,
                                    const GrProcessorKey& key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TransformedCoordsArray&,
                                    const TextureSamplerArray& samplers) {
    const DashingCircleEffect& dce = geometryProcessor.cast<DashingCircleEffect>();
    const char *paramName;
    // The param uniforms, xyz, refer to circle radius - 0.5, cicles center x coord, and
    // the total interval length of the dash.
    fParamUniform = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec3f_GrSLType,
                                       "params",
                                       &paramName);

    const char *vsCoordName, *fsCoordName;
    builder->addVarying(kVec2f_GrSLType, "Coord", &vsCoordName, &fsCoordName);

    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    vsBuilder->codeAppendf("\t%s = %s;\n", vsCoordName, dce.inCoord().c_str());

    // transforms all points so that we can compare them to our test circle
    GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("\t\tfloat xShifted = %s.x - floor(%s.x / %s.z) * %s.z;\n",
                           fsCoordName, fsCoordName, paramName, paramName);
    fsBuilder->codeAppendf("\t\tvec2 fragPosShifted = vec2(xShifted, %s.y);\n", fsCoordName);
    fsBuilder->codeAppendf("\t\tvec2 center = vec2(%s.y, 0.0);\n", paramName);
    fsBuilder->codeAppend("\t\tfloat dist = length(center - fragPosShifted);\n");
    if (GrProcessorEdgeTypeIsAA(dce.getEdgeType())) {
        fsBuilder->codeAppendf("\t\tfloat diff = dist - %s.x;\n", paramName);
        fsBuilder->codeAppend("\t\tdiff = 1.0 - diff;\n");
        fsBuilder->codeAppend("\t\tfloat alpha = clamp(diff, 0.0, 1.0);\n");
    } else {
        fsBuilder->codeAppendf("\t\tfloat alpha = 1.0;\n");
        fsBuilder->codeAppendf("\t\talpha *=  dist < %s.x + 0.5 ? 1.0 : 0.0;\n", paramName);
    }
    fsBuilder->codeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLDashingCircleEffect::setData(const GrGLProgramDataManager& pdman
                                    , const GrProcessor& processor) {
    const DashingCircleEffect& dce = processor.cast<DashingCircleEffect>();
    SkScalar radius = dce.getRadius();
    SkScalar centerX = dce.getCenterX();
    SkScalar intervalLength = dce.getIntervalLength();
    if (radius != fPrevRadius || centerX != fPrevCenterX || intervalLength != fPrevIntervalLength) {
        pdman.set3f(fParamUniform, radius - 0.5f, centerX, intervalLength);
        fPrevRadius = radius;
        fPrevCenterX = centerX;
        fPrevIntervalLength = intervalLength;
    }
}

void GLDashingCircleEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                                   GrProcessorKeyBuilder* b) {
    const DashingCircleEffect& dce = processor.cast<DashingCircleEffect>();
    b->add32(dce.getEdgeType());
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingCircleEffect::Create(GrPrimitiveEdgeType edgeType, const DashInfo& info,
                                                 SkScalar radius) {
    if (info.fCount != 2 || info.fIntervals[0] != 0) {
        return NULL;
    }

    return SkNEW_ARGS(DashingCircleEffect, (edgeType, info, radius));
}

DashingCircleEffect::~DashingCircleEffect() {}

void DashingCircleEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendGeometryProcessorFactory& DashingCircleEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<DashingCircleEffect>::getInstance();
}

DashingCircleEffect::DashingCircleEffect(GrPrimitiveEdgeType edgeType, const DashInfo& info,
                                         SkScalar radius)
    : fEdgeType(edgeType)
    , fInCoord(this->addVertexAttrib(GrShaderVar("inCoord",
                                                 kVec2f_GrSLType,
                                                 GrShaderVar::kAttribute_TypeModifier))) {
    SkScalar onLen = info.fIntervals[0];
    SkScalar offLen = info.fIntervals[1];
    fIntervalLength = onLen + offLen;
    fRadius = radius;
    fCenterX = SkScalarHalf(offLen);
}

bool DashingCircleEffect::onIsEqual(const GrProcessor& other) const {
    const DashingCircleEffect& dce = other.cast<DashingCircleEffect>();
    return (fEdgeType == dce.fEdgeType &&
            fIntervalLength == dce.fIntervalLength &&
            fRadius == dce.fRadius &&
            fCenterX == dce.fCenterX);
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingCircleEffect);

GrGeometryProcessor* DashingCircleEffect::TestCreate(SkRandom* random,
                                                     GrContext*,
                                                     const GrDrawTargetCaps& caps,
                                                     GrTexture*[]) {
    GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(random->nextULessThan(
            kGrProcessorEdgeTypeCnt));
    SkScalar strokeWidth = random->nextRangeScalar(0, 100.f);
    DashInfo info;
    info.fCount = 2;
    SkAutoTArray<SkScalar> intervals(info.fCount);
    info.fIntervals = intervals.get();
    info.fIntervals[0] = 0; 
    info.fIntervals[1] = random->nextRangeScalar(0, 10.f);
    info.fPhase = random->nextRangeScalar(0, info.fIntervals[1]);

    return DashingCircleEffect::Create(edgeType, info, strokeWidth);
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

    static GrGeometryProcessor* Create(GrPrimitiveEdgeType edgeType,
                                       const DashInfo& info,
                                       SkScalar strokeWidth);

    virtual ~DashingLineEffect();

    static const char* Name() { return "DashingEffect"; }

    const GrShaderVar& inCoord() const { return fInCoord; }

    GrPrimitiveEdgeType getEdgeType() const { return fEdgeType; }

    const SkRect& getRect() const { return fRect; }

    SkScalar getIntervalLength() const { return fIntervalLength; }

    typedef GLDashingLineEffect GLProcessor;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE;

private:
    DashingLineEffect(GrPrimitiveEdgeType edgeType, const DashInfo& info, SkScalar strokeWidth);

    virtual bool onIsEqual(const GrProcessor& other) const SK_OVERRIDE;

    GrPrimitiveEdgeType    fEdgeType;
    const GrShaderVar&  fInCoord;
    SkRect              fRect;
    SkScalar            fIntervalLength;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLDashingLineEffect : public GrGLGeometryProcessor {
public:
    GLDashingLineEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

private:
    GrGLProgramDataManager::UniformHandle fRectUniform;
    GrGLProgramDataManager::UniformHandle fIntervalUniform;
    SkRect                                fPrevRect;
    SkScalar                              fPrevIntervalLength;
    typedef GrGLGeometryProcessor INHERITED;
};

GLDashingLineEffect::GLDashingLineEffect(const GrBackendProcessorFactory& factory,
                                         const GrProcessor&)
    : INHERITED (factory) {
    fPrevRect.fLeft = SK_ScalarNaN;
    fPrevIntervalLength = SK_ScalarMax;
}

void GLDashingLineEffect::emitCode(GrGLFullProgramBuilder* builder,
                                   const GrGeometryProcessor& geometryProcessor,
                                   const GrProcessorKey& key,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const TransformedCoordsArray&,
                                   const TextureSamplerArray& samplers) {
    const DashingLineEffect& de = geometryProcessor.cast<DashingLineEffect>();
    const char *rectName;
    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    fRectUniform = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       "rect",
                                       &rectName);
    const char *intervalName;
    // The interval uniform's refers to the total length of the interval (on + off)
    fIntervalUniform = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                       kFloat_GrSLType,
                                       "interval",
                                       &intervalName);

    const char *vsCoordName, *fsCoordName;
    builder->addVarying(kVec2f_GrSLType, "Coord", &vsCoordName, &fsCoordName);
    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    vsBuilder->codeAppendf("\t%s = %s;\n", vsCoordName, de.inCoord().c_str());

    // transforms all points so that we can compare them to our test rect
    GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("\t\tfloat xShifted = %s.x - floor(%s.x / %s) * %s;\n",
                           fsCoordName, fsCoordName, intervalName, intervalName);
    fsBuilder->codeAppendf("\t\tvec2 fragPosShifted = vec2(xShifted, %s.y);\n", fsCoordName);
    if (GrProcessorEdgeTypeIsAA(de.getEdgeType())) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        fsBuilder->codeAppend("\t\tfloat xSub, ySub;\n");
        fsBuilder->codeAppendf("\t\txSub = min(fragPosShifted.x - %s.x, 0.0);\n", rectName);
        fsBuilder->codeAppendf("\t\txSub += min(%s.z - fragPosShifted.x, 0.0);\n", rectName);
        fsBuilder->codeAppendf("\t\tySub = min(fragPosShifted.y - %s.y, 0.0);\n", rectName);
        fsBuilder->codeAppendf("\t\tySub += min(%s.w - fragPosShifted.y, 0.0);\n", rectName);
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        fsBuilder->codeAppendf("\t\tfloat alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));\n");
    } else {
        // Assuming the bounding geometry is tight so no need to check y values
        fsBuilder->codeAppendf("\t\tfloat alpha = 1.0;\n");
        fsBuilder->codeAppendf("\t\talpha *= (fragPosShifted.x - %s.x) > -0.5 ? 1.0 : 0.0;\n", rectName);
        fsBuilder->codeAppendf("\t\talpha *= (%s.z - fragPosShifted.x) >= -0.5 ? 1.0 : 0.0;\n", rectName);
    }
    fsBuilder->codeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLDashingLineEffect::setData(const GrGLProgramDataManager& pdman,
                                  const GrProcessor& processor) {
    const DashingLineEffect& de = processor.cast<DashingLineEffect>();
    const SkRect& rect = de.getRect();
    SkScalar intervalLength = de.getIntervalLength();
    if (rect != fPrevRect || intervalLength != fPrevIntervalLength) {
        pdman.set4f(fRectUniform, rect.fLeft + 0.5f, rect.fTop + 0.5f,
                    rect.fRight - 0.5f, rect.fBottom - 0.5f);
        pdman.set1f(fIntervalUniform, intervalLength);
        fPrevRect = rect;
        fPrevIntervalLength = intervalLength;
    }
}

void GLDashingLineEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                                 GrProcessorKeyBuilder* b) {
    const DashingLineEffect& de = processor.cast<DashingLineEffect>();
    b->add32(de.getEdgeType());
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* DashingLineEffect::Create(GrPrimitiveEdgeType edgeType,
                                               const DashInfo& info,
                                               SkScalar strokeWidth) {
    if (info.fCount != 2) {
        return NULL;
    }

    return SkNEW_ARGS(DashingLineEffect, (edgeType, info, strokeWidth));
}

DashingLineEffect::~DashingLineEffect() {}

void DashingLineEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendGeometryProcessorFactory& DashingLineEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<DashingLineEffect>::getInstance();
}

DashingLineEffect::DashingLineEffect(GrPrimitiveEdgeType edgeType, const DashInfo& info,
                                     SkScalar strokeWidth)
    : fEdgeType(edgeType)
    , fInCoord(this->addVertexAttrib(GrShaderVar("inCoord",
                                                 kVec2f_GrSLType,
                                                 GrShaderVar::kAttribute_TypeModifier))) {
    SkScalar onLen = info.fIntervals[0];
    SkScalar offLen = info.fIntervals[1];
    SkScalar halfOffLen = SkScalarHalf(offLen);
    SkScalar halfStroke = SkScalarHalf(strokeWidth);
    fIntervalLength = onLen + offLen;
    fRect.set(halfOffLen, -halfStroke, halfOffLen + onLen, halfStroke);
}

bool DashingLineEffect::onIsEqual(const GrProcessor& other) const {
    const DashingLineEffect& de = other.cast<DashingLineEffect>();
    return (fEdgeType == de.fEdgeType &&
            fRect == de.fRect &&
            fIntervalLength == de.fIntervalLength);
}

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DashingLineEffect);

GrGeometryProcessor* DashingLineEffect::TestCreate(SkRandom* random,
                                                   GrContext*,
                                                   const GrDrawTargetCaps& caps,
                                                   GrTexture*[]) {
    GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(random->nextULessThan(
            kGrProcessorEdgeTypeCnt));
    SkScalar strokeWidth = random->nextRangeScalar(0, 100.f);
    DashInfo info;
    info.fCount = 2;
    SkAutoTArray<SkScalar> intervals(info.fCount);
    info.fIntervals = intervals.get();
    info.fIntervals[0] = random->nextRangeScalar(0, 10.f);
    info.fIntervals[1] = random->nextRangeScalar(0, 10.f);
    info.fPhase = random->nextRangeScalar(0, info.fIntervals[0] + info.fIntervals[1]);

    return DashingLineEffect::Create(edgeType, info, strokeWidth);
}

//////////////////////////////////////////////////////////////////////////////

GrGeometryProcessor* GrDashingEffect::Create(GrPrimitiveEdgeType edgeType,
                                             const SkPathEffect::DashInfo& info,
                                             SkScalar strokeWidth,
                                             GrDashingEffect::DashCap cap) {
    switch (cap) {
        case GrDashingEffect::kRound_DashCap:
            return DashingCircleEffect::Create(edgeType, info, SkScalarHalf(strokeWidth));
        case GrDashingEffect::kNonRound_DashCap:
            return DashingLineEffect::Create(edgeType, info, strokeWidth);
        default:
            SkFAIL("Unexpected dashed cap.");
    }
    return NULL;
}
