/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRRectEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

#include "SkRRect.h"

using namespace GrRRectEffect;

class GLRRectEffect;

class RRectEffect : public GrEffect {
public:
    // This effect only supports circular corner rrects where the radius is >= kRadiusMin.
    static const SkScalar kRadiusMin;
    
    /// The types of circular corner rrects supported
    enum RRectType {
        kCircleCorner_RRectType,     //<! All four corners have the same circular radius.
        kLeftCircleTab_RRectType,    //<! The left side has circular corners, the right is a rect.
        kTopCircleTab_RRectType,     //<! etc
        kRightCircleTab_RRectType,
        kBottomCircleTab_RRectType,
    };

    static GrEffectRef* Create(EdgeType, const SkRRect&, RRectType);

    virtual ~RRectEffect() {};
    static const char* Name() { return "RRect"; }

    const SkRRect& getRRect() const { return fRRect; }

    RRectType getType() const { return fRRectType; }
    
    EdgeType getEdgeType() const { return fEdgeType; }

    typedef GLRRectEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    RRectEffect(EdgeType, const SkRRect&, RRectType);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    SkRRect     fRRect;
    EdgeType    fEdgeType;
    RRectType   fRRectType;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

const SkScalar RRectEffect::kRadiusMin = 0.5f;

GrEffectRef* RRectEffect::Create(EdgeType edgeType, const SkRRect& rrect, RRectType rrtype) {
    return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(RRectEffect, (edgeType, rrect, rrtype))));
}

void RRectEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& RRectEffect::getFactory() const {
    return GrTBackendEffectFactory<RRectEffect>::getInstance();
}

RRectEffect::RRectEffect(EdgeType edgeType, const SkRRect& rrect, RRectType rrtype)
    : fRRect(rrect)
    , fEdgeType(edgeType)
    , fRRectType(rrtype) {
    this->setWillReadFragmentPosition();
}

bool RRectEffect::onIsEqual(const GrEffect& other) const {
    const RRectEffect& rre = CastEffect<RRectEffect>(other);
    // type is derived from fRRect, so no need to check it.
    return fEdgeType == rre.fEdgeType && fRRect == rre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(RRectEffect);

GrEffectRef* RRectEffect::TestCreate(SkRandom* random,
                                     GrContext*,
                                     const GrDrawTargetCaps& caps,
                                     GrTexture*[]) {
    SkScalar w = random->nextRangeScalar(20.f, 1000.f);
    SkScalar h = random->nextRangeScalar(20.f, 1000.f);
    SkScalar r = random->nextRangeF(kRadiusMin, 9.f);
    EdgeType et = (EdgeType) random->nextULessThan(kEdgeTypeCnt);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);

    return GrRRectEffect::Create(et, rrect);
}

//////////////////////////////////////////////////////////////////////////////

class GLRRectEffect : public GrGLEffect {
public:
    GLRRectEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

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
    GrGLUniformManager::UniformHandle   fInnerRectUniform;
    GrGLUniformManager::UniformHandle   fRadiusPlusHalfUniform;
    SkRRect                             fPrevRRect;
    typedef GrGLEffect INHERITED;
};

GLRRectEffect::GLRRectEffect(const GrBackendEffectFactory& factory,
                             const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevRRect.setEmpty();
}

void GLRRectEffect::emitCode(GrGLShaderBuilder* builder,
                             const GrDrawEffect& drawEffect,
                             EffectKey key,
                             const char* outputColor,
                             const char* inputColor,
                             const TransformedCoordsArray&,
                             const TextureSamplerArray& samplers) {
    const RRectEffect& rre = drawEffect.castEffect<RRectEffect>();
    const char *rectName;
    const char *radiusPlusHalfName;
    // The inner rect is the rrect bounds inset by the radius. Its top, left, right, and bottom
    // edges correspond to components x, y, z, and w, respectively. When one side of the rrect has
    // rectangular corners, that side's value corresponds to the rect edge's value outset by half a
    // pixel.
    fInnerRectUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kVec4f_GrSLType,
                                            "innerRect",
                                            &rectName);
    fRadiusPlusHalfUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                 kFloat_GrSLType,
                                                 "radiusPlusHalf",
                                                 &radiusPlusHalfName);
    const char* fragmentPos = builder->fragmentPosition();
    // At each quarter-circle corner we compute a vector that is the offset of the fragment position
    // from the circle center. The vector is pinned in x and y to be in the quarter-plane relevant
    // to that corner. This means that points near the interior near the rrect top edge will have
    // a vector that points straight up for both the TL left and TR corners. Computing an
    // alpha from this vector at either the TR or TL corner will give the correct result. Similarly,
    // fragments near the other three edges will get the correct AA. Fragments in the interior of
    // the rrect will have a (0,0) vector at all four corners. So long as the radius > 0.5 they will
    // correctly produce an alpha value of 1 at all four corners. We take the min of all the alphas.
    // The code below is a simplified version of the above that performs maxs on the vector
    // components before computing distances and alpha values so that only one distance computation
    // need be computed to determine the min alpha.
    //
    // For the cases where one half of the rrect is rectangular we drop one of the x or y
    // computations, compute a separate rect edge alpha for the rect side, and mul the two computed
    // alphas together.
    switch (rre.getType()) {
        case RRectEffect::kCircleCorner_RRectType:
            builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(max(dxy0, dxy1), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat alpha = clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case RRectEffect::kLeftCircleTab_RRectType:
            builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat dy1 = %s.y - %s.w;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(dxy0.x, max(dxy0.y, dy1)), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat rightAlpha = clamp(%s.z - %s.x, 0.0, 1.0);\n",
                                    rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat alpha = rightAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case RRectEffect::kTopCircleTab_RRectType:
            builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat dx1 = %s.x - %s.z;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(max(dxy0.x, dx1), dxy0.y), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat bottomAlpha = clamp(%s.w - %s.y, 0.0, 1.0);\n",
                                   rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat alpha = bottomAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case RRectEffect::kRightCircleTab_RRectType:
            builder->fsCodeAppendf("\t\tfloat dy0 = %s.y - %s.y;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(dxy1.x, max(dy0, dxy1.y)), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat leftAlpha = clamp(%s.x - %s.x, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat alpha = leftAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case RRectEffect::kBottomCircleTab_RRectType:
            builder->fsCodeAppendf("\t\tfloat dx0 = %s.x - %s.x;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(max(dx0, dxy1.x), dxy1.y), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat topAlpha = clamp(%s.y - %s.y, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat alpha = topAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
    }
    
    if (kInverseFillAA_EdgeType == rre.getEdgeType()) {
        builder->fsCodeAppend("\t\talpha = 1.0 - alpha;\n");
    }

    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

GrGLEffect::EffectKey GLRRectEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
    const RRectEffect& rre = drawEffect.castEffect<RRectEffect>();
    GR_STATIC_ASSERT(kEdgeTypeCnt <= 4);
    return (rre.getType() << 2) | rre.getEdgeType();
}

void GLRRectEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const RRectEffect& rre = drawEffect.castEffect<RRectEffect>();
    const SkRRect& rrect = rre.getRRect();
    if (rrect != fPrevRRect) {
        SkRect rect = rrect.getBounds();
        SkScalar radius = 0;
        switch (rre.getType()) {
            case RRectEffect::kCircleCorner_RRectType:
                SkASSERT(rrect.isSimpleCircular());
                radius = rrect.getSimpleRadii().fX;
                SkASSERT(radius >= RRectEffect::kRadiusMin);
                rect.inset(radius, radius);
                break;
            case RRectEffect::kLeftCircleTab_RRectType:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight += 0.5f;
                rect.fBottom -= radius;
                break;
            case RRectEffect::kTopCircleTab_RRectType:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom += 0.5f;
                break;
            case RRectEffect::kRightCircleTab_RRectType:
                radius = rrect.radii(SkRRect::kUpperRight_Corner).fX;
                rect.fLeft -= 0.5f;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
            case RRectEffect::kBottomCircleTab_RRectType:
                radius = rrect.radii(SkRRect::kLowerLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop -= 0.5f;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
        }
        uman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        uman.set1f(fRadiusPlusHalfUniform, radius + 0.5f);
        fPrevRRect = rrect;
    }
}

//////////////////////////////////////////////////////////////////////////////

GrEffectRef* GrRRectEffect::Create(EdgeType edgeType, const SkRRect& rrect) {
    RRectEffect::RRectType rrtype;
    if (rrect.isSimpleCircular()) {
        if (rrect.getSimpleRadii().fX < RRectEffect::kRadiusMin) {
            return NULL;
        }
        rrtype = RRectEffect::kCircleCorner_RRectType;
    } else if (rrect.isComplex()) {
        // Check for the "tab" cases - two adjacent circular corners and two square corners.
        SkScalar radius = 0;
        int circleCornerBitfield = 0;
        for (int c = 0; c < 4; ++c) {
            const SkVector& r = rrect.radii((SkRRect::Corner)c);
            SkASSERT((0 == r.fX) == (0 == r.fY));
            if (0 == r.fX) {
                continue;
            }
            if (r.fX != r.fY) {
                return NULL;
            }
            if (!circleCornerBitfield) {
                radius = r.fX;
                if (radius < RRectEffect::kRadiusMin) {
                    return NULL;
                }
                circleCornerBitfield = 1 << c;
            } else {
                if (r.fX != radius) {
                    return NULL;
                }
                circleCornerBitfield |= 1 << c;
            }
        }

        GR_STATIC_ASSERT(SkRRect::kUpperLeft_Corner  == 0);
        GR_STATIC_ASSERT(SkRRect::kUpperRight_Corner == 1);
        GR_STATIC_ASSERT(SkRRect::kLowerRight_Corner == 2);
        GR_STATIC_ASSERT(SkRRect::kLowerLeft_Corner  == 3);
        switch (circleCornerBitfield) {
            case 3:
                rrtype = RRectEffect::kTopCircleTab_RRectType;
                break;
            case 6:
                rrtype = RRectEffect::kRightCircleTab_RRectType;
                break;
            case 9:
                rrtype = RRectEffect::kLeftCircleTab_RRectType;
                break;
            case 12:
                rrtype = RRectEffect::kBottomCircleTab_RRectType;
                break;
            default:
                return NULL;
        }
    } else {
        return NULL;
    }
    return RRectEffect::Create(edgeType, rrect, rrtype);
}
