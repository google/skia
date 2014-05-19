/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashingEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrDrawTargetCaps.h"
#include "GrEffect.h"
#include "GrTBackendEffectFactory.h"
#include "SkGr.h"

///////////////////////////////////////////////////////////////////////////////

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

static SkScalar calc_end_adjustment(const SkPathEffect::DashInfo& info, const SkPoint pts[2], SkScalar* endingInt) {
    if (pts[1].fX <= pts[0].fX) {
        return 0;
    }
    SkScalar srcIntervalLen = info.fIntervals[0] + info.fIntervals[1];
    SkScalar totalLen = pts[1].fX - pts[0].fX;
    SkScalar temp = SkScalarDiv(totalLen, srcIntervalLen);
    SkScalar numFullIntervals = SkScalarFloorToScalar(temp);
    *endingInt = totalLen - numFullIntervals * srcIntervalLen + info.fPhase;
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


bool GrDashingEffect::DrawDashLine(const SkPoint pts[2], const SkPaint& paint, GrContext* context) {
    if (context->getRenderTarget()->isMultisampled()) {
        return false;
    }

    const SkMatrix& viewMatrix = context->getMatrix();
    if (!viewMatrix.preservesRightAngles()) {
        return false;
    }

    const SkPathEffect* pe = paint.getPathEffect();
    SkPathEffect::DashInfo info;
    SkPathEffect::DashType dashType = pe->asADash(&info);
    // Must be a dash effect with 2 intervals (1 on and 1 off)
    if (SkPathEffect::kDash_DashType != dashType || 2 != info.fCount) {
        return false;
    }

    SkPaint::Cap cap = paint.getStrokeCap();
    // Current we do don't handle Round or Square cap dashes
    if (SkPaint::kRound_Cap == cap) {
        return false;
    }

    SkScalar srcStrokeWidth = paint.getStrokeWidth();

    // Get all info about the dash effect
    SkAutoTArray<SkScalar> intervals(info.fCount);
    info.fIntervals = intervals.get();
    pe->asADash(&info);

    // the phase should be normalized to be [0, sum of all intervals)
    SkASSERT(info.fPhase >= 0 && info.fPhase < info.fIntervals[0] + info.fIntervals[1]);

    SkMatrix coordTrans;

    // Rotate the src pts so they are aligned horizontally with pts[0].fX < pts[1].fX
    SkMatrix srcRotInv;
    SkPoint ptsRot[2];
    if (pts[0].fY != pts[1].fY || pts[0].fX > pts[1].fX) {
        align_to_x_axis(pts, &coordTrans, ptsRot);
        if(!coordTrans.invert(&srcRotInv)) {
            return false;
        }
    } else {
        coordTrans.reset();
        srcRotInv.reset();
        memcpy(ptsRot, pts, 2 * sizeof(SkPoint));
    }

    GrPaint grPaint;
    SkPaint2GrPaintShader(context, paint, true, &grPaint);

    bool useAA = paint.isAntiAlias();
    
    // Scale corrections of intervals and stroke from view matrix
    SkScalar parallelScale;
    SkScalar perpScale;
    calc_dash_scaling(&parallelScale, &perpScale, viewMatrix, ptsRot);

    bool hasCap = SkPaint::kSquare_Cap == cap && 0 != srcStrokeWidth;

    // We always want to at least stroke out half a pixel on each side in device space
    // so 0.5f / perpScale gives us this min in src space
    SkScalar halfStroke = SkMaxScalar(srcStrokeWidth * 0.5f, 0.5f / perpScale);

    SkScalar xStroke;
    if (!hasCap) {
        xStroke = 0.f;
    } else {
        xStroke = halfStroke;
    }

    // If we are using AA, check to see if we are drawing a partial dash at the start. If so
    // draw it separately here and adjust our start point accordingly
    if (useAA) {
        if (info.fPhase > 0 && info.fPhase < info.fIntervals[0]) {
            SkPoint startPts[2];
            startPts[0] = ptsRot[0];
            startPts[1].fY = startPts[0].fY;
            startPts[1].fX = SkMinScalar(startPts[0].fX + info.fIntervals[0] - info.fPhase,
                                         ptsRot[1].fX);
            SkRect startRect;
            startRect.set(startPts, 2);
            startRect.outset(xStroke, halfStroke);
            context->drawRect(grPaint, startRect, NULL, &srcRotInv);

            ptsRot[0].fX += info.fIntervals[0] + info.fIntervals[1] - info.fPhase;
            info.fPhase = 0; 
        }
    }

    // adjustments for start and end of bounding rect so we only draw dash intervals
    // contained in the original line segment.
    SkScalar startAdj = calc_start_adjustment(info);
    SkScalar endingInterval = 0;
    SkScalar endAdj = calc_end_adjustment(info, ptsRot, &endingInterval);
    if (ptsRot[0].fX + startAdj >= ptsRot[1].fX - endAdj) {
        // Nothing left to draw so just return
        return true;
    }

    // If we are using AA, check to see if we are drawing a partial dash at then end. If so
    // draw it separately here and adjust our end point accordingly
    if (useAA) {
        // If we adjusted the end then we will not be drawing a partial dash at the end.
        // If we didn't adjust the end point then we just need to make sure the ending
        // dash isn't a full dash
        if (0 == endAdj && endingInterval != info.fIntervals[0]) {
            
            SkPoint endPts[2];
            endPts[1] = ptsRot[1];
            endPts[0].fY = endPts[1].fY;
            endPts[0].fX = endPts[1].fX - endingInterval; 

            SkRect endRect;
            endRect.set(endPts, 2);
            endRect.outset(xStroke, halfStroke);
            context->drawRect(grPaint, endRect, NULL, &srcRotInv);
            
            ptsRot[1].fX -= endingInterval + info.fIntervals[1];
            if (ptsRot[0].fX >= ptsRot[1].fX) {
                // Nothing left to draw so just return
                return true;
            }
        }
    }
    coordTrans.postConcat(viewMatrix);

    SkPoint devicePts[2];
    viewMatrix.mapPoints(devicePts, ptsRot, 2);

    info.fIntervals[0] *= parallelScale;
    info.fIntervals[1] *= parallelScale;
    info.fPhase *= parallelScale;
    SkScalar strokeWidth = srcStrokeWidth * perpScale;

    if ((strokeWidth < 1.f && !useAA) || 0.f == strokeWidth) {
        strokeWidth = 1.f;
    }

    // Set up coordTransform for device space transforms
    // We rotate the dashed line such that it is horizontal with the start point at smaller x
    // then we translate the start point to the origin
    if (devicePts[0].fY != devicePts[1].fY || devicePts[0].fX > devicePts[1].fX) {
        SkMatrix rot;
        align_to_x_axis(devicePts, &rot);
        coordTrans.postConcat(rot);
    }
    coordTrans.postTranslate(-devicePts[0].fX, -devicePts[0].fY);
    coordTrans.postTranslate(info.fIntervals[1] * 0.5f + info.fPhase, 0);

    if (SkPaint::kSquare_Cap == cap && 0 != srcStrokeWidth) {
        // add cap to on interveal and remove from off interval
        info.fIntervals[0] += strokeWidth;
        info.fIntervals[1] -= strokeWidth;
    }

    if (info.fIntervals[1] > 0.f) {
        GrEffectEdgeType edgeType= useAA ? kFillAA_GrEffectEdgeType :
            kFillBW_GrEffectEdgeType;
        grPaint.addCoverageEffect(
            GrDashingEffect::Create(edgeType, info, coordTrans, strokeWidth))->unref();
        grPaint.setAntiAlias(false);
    }

    SkRect rect;
    bool bloat = useAA && info.fIntervals[1] > 0.f;
    SkScalar bloatX = bloat ? 0.5f / parallelScale : 0.f;
    SkScalar bloatY = bloat ? 0.5f / perpScale : 0.f;
    ptsRot[0].fX += startAdj;
    ptsRot[1].fX -= endAdj;
    if (!hasCap) {
        xStroke = 0.f;
    } else {
        xStroke = halfStroke;
    }
    rect.set(ptsRot, 2);
    rect.outset(bloatX + xStroke, bloatY + halfStroke);
    context->drawRect(grPaint, rect, NULL, &srcRotInv);

    return true;
}

//////////////////////////////////////////////////////////////////////////////

class GLDashingLineEffect;

class DashingLineEffect : public GrEffect {
public:
    typedef SkPathEffect::DashInfo DashInfo;

    /**
     * The effect calculates the coverage for the case of a horizontal line in device space.
     * The matrix that is passed in should be able to convert a line in source space to a
     * horizontal line in device space. Additionally, the coord transform matrix should translate
     * the the start of line to origin, and the shift it along the positive x-axis by the phase
     * and half the off interval.
     */
    static GrEffectRef* Create(GrEffectEdgeType edgeType, const DashInfo& info,
                               const SkMatrix& matrix, SkScalar strokeWidth);

    virtual ~DashingLineEffect();

    static const char* Name() { return "DashingEffect"; }

    GrEffectEdgeType getEdgeType() const { return fEdgeType; }

    const SkRect& getRect() const { return fRect; }

    SkScalar getIntervalLength() const { return fIntervalLength; }

    typedef GLDashingLineEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    DashingLineEffect(GrEffectEdgeType edgeType, const DashInfo& info, const SkMatrix& matrix,
                      SkScalar strokeWidth);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    GrEffectEdgeType    fEdgeType;
    GrCoordTransform    fCoordTransform;
    SkRect              fRect;
    SkScalar            fIntervalLength;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class GLDashingLineEffect : public GrGLEffect {
public:
    GLDashingLineEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    GrGLUniformManager::UniformHandle   fRectUniform;
    GrGLUniformManager::UniformHandle   fIntervalUniform;
    SkRect                              fPrevRect;
    SkScalar                            fPrevIntervalLength;
    typedef GrGLEffect INHERITED;
};

GLDashingLineEffect::GLDashingLineEffect(const GrBackendEffectFactory& factory, 
                                     const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevRect.fLeft = SK_ScalarNaN;
    fPrevIntervalLength = SK_ScalarMax;

}

void GLDashingLineEffect::emitCode(GrGLShaderBuilder* builder,
                                    const GrDrawEffect& drawEffect,
                                    EffectKey key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TransformedCoordsArray& coords,
                                    const TextureSamplerArray& samplers) {
    const DashingLineEffect& de = drawEffect.castEffect<DashingLineEffect>();
    const char *rectName;
    // The rect uniform's xyzw refer to (left + 0.5, top + 0.5, right - 0.5, bottom - 0.5),
    // respectively.
    fRectUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                       kVec4f_GrSLType,
                                       "rect",
                                       &rectName);
    const char *intervalName;
    // The interval uniform's refers to the total length of the interval (on + off)
    fIntervalUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                       kFloat_GrSLType,
                                       "interval",
                                       &intervalName);
    // transforms all points so that we can compare them to our test rect
    builder->fsCodeAppendf("\t\tfloat xShifted = %s.x - floor(%s.x / %s) * %s;\n",
                           coords[0].c_str(), coords[0].c_str(), intervalName, intervalName);
    builder->fsCodeAppendf("\t\tvec2 fragPosShifted = vec2(xShifted, %s.y);\n", coords[0].c_str());
    if (GrEffectEdgeTypeIsAA(de.getEdgeType())) {
        // The amount of coverage removed in x and y by the edges is computed as a pair of negative
        // numbers, xSub and ySub.
        builder->fsCodeAppend("\t\tfloat xSub, ySub;\n");
        builder->fsCodeAppendf("\t\txSub = min(fragPosShifted.x - %s.x, 0.0);\n", rectName);
        builder->fsCodeAppendf("\t\txSub += min(%s.z - fragPosShifted.x, 0.0);\n", rectName);
        builder->fsCodeAppendf("\t\tySub = min(fragPosShifted.y - %s.y, 0.0);\n", rectName);
        builder->fsCodeAppendf("\t\tySub += min(%s.w - fragPosShifted.y, 0.0);\n", rectName);
        // Now compute coverage in x and y and multiply them to get the fraction of the pixel
        // covered.
        builder->fsCodeAppendf("\t\tfloat alpha = (1.0 + max(xSub, -1.0)) * (1.0 + max(ySub, -1.0));\n");
    } else {
        // Assuming the bounding geometry is tight so no need to check y values
        builder->fsCodeAppendf("\t\tfloat alpha = 1.0;\n");
        builder->fsCodeAppendf("\t\talpha *= (fragPosShifted.x - %s.x) > -0.5 ? 1.0 : 0.0;\n", rectName);
        builder->fsCodeAppendf("\t\talpha *= (%s.z - fragPosShifted.x) >= -0.5 ? 1.0 : 0.0;\n", rectName);
    }
    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLDashingLineEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const DashingLineEffect& de = drawEffect.castEffect<DashingLineEffect>();
    const SkRect& rect = de.getRect();
    SkScalar intervalLength = de.getIntervalLength();
    if (rect != fPrevRect || intervalLength != fPrevIntervalLength) {
        uman.set4f(fRectUniform, rect.fLeft + 0.5f, rect.fTop + 0.5f,
                   rect.fRight - 0.5f, rect.fBottom - 0.5f);
        uman.set1f(fIntervalUniform, intervalLength);
        fPrevRect = rect;
        fPrevIntervalLength = intervalLength;
    }
}

GrGLEffect::EffectKey GLDashingLineEffect::GenKey(const GrDrawEffect& drawEffect,
                                                const GrGLCaps&) {
    const DashingLineEffect& de = drawEffect.castEffect<DashingLineEffect>();
    return de.getEdgeType();
}

//////////////////////////////////////////////////////////////////////////////
    
GrEffectRef* DashingLineEffect::Create(GrEffectEdgeType edgeType, const DashInfo& info,
                                     const SkMatrix& matrix, SkScalar strokeWidth) {
    if (info.fCount != 2) {
        return NULL;
    }

    return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(DashingLineEffect,
                                                      (edgeType, info, matrix, strokeWidth))));
}

DashingLineEffect::~DashingLineEffect() {}

void DashingLineEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& DashingLineEffect::getFactory() const {
    return GrTBackendEffectFactory<DashingLineEffect>::getInstance();
}

DashingLineEffect::DashingLineEffect(GrEffectEdgeType edgeType, const DashInfo& info,
                                 const SkMatrix& matrix, SkScalar strokeWidth)
    : fEdgeType(edgeType)
    , fCoordTransform(kLocal_GrCoordSet, matrix) {
    SkScalar onLen = info.fIntervals[0];
    SkScalar offLen = info.fIntervals[1];
    SkScalar halfOffLen = SkScalarHalf(offLen);
    SkScalar halfStroke = SkScalarHalf(strokeWidth);
    fIntervalLength = onLen + offLen;
    fRect.set(halfOffLen, -halfStroke, halfOffLen + onLen, halfStroke);

    addCoordTransform(&fCoordTransform);
}

bool DashingLineEffect::onIsEqual(const GrEffect& other) const {
    const DashingLineEffect& de = CastEffect<DashingLineEffect>(other);
    return (fEdgeType == de.fEdgeType &&
            fCoordTransform == de.fCoordTransform &&
            fRect == de.fRect &&
            fIntervalLength == de.fIntervalLength);
}

GR_DEFINE_EFFECT_TEST(DashingLineEffect);

GrEffectRef* DashingLineEffect::TestCreate(SkRandom* random,
                                         GrContext*,
                                         const GrDrawTargetCaps& caps,
                                         GrTexture*[]) {
    GrEffectRef* effect;
    SkMatrix m;
    m.reset();
    GrEffectEdgeType edgeType = static_cast<GrEffectEdgeType>(random->nextULessThan(
            kGrEffectEdgeTypeCnt));
    SkScalar strokeWidth = random->nextRangeScalar(0, 100.f);
    DashInfo info;
    info.fCount = 2;
    SkAutoTArray<SkScalar> intervals(info.fCount);
    info.fIntervals = intervals.get();
    info.fIntervals[0] = random->nextRangeScalar(0, 10.f);
    info.fIntervals[1] = random->nextRangeScalar(0, 10.f);
    info.fPhase = random->nextRangeScalar(0, info.fIntervals[0] + info.fIntervals[1]);

    effect = DashingLineEffect::Create(edgeType, info, m, strokeWidth);
    return effect;
}

//////////////////////////////////////////////////////////////////////////////

GrEffectRef* GrDashingEffect::Create(GrEffectEdgeType edgeType, const SkPathEffect::DashInfo& info,
                                     const SkMatrix& matrix, SkScalar strokeWidth) {
    return DashingLineEffect::Create(edgeType, info, matrix, strokeWidth);
}
