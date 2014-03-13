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

class GLCircularRRectEffect;

class CircularRRectEffect : public GrEffect {
public:
    // This effect only supports circular corner rrects where the radius is >= kRadiusMin.
    static const SkScalar kRadiusMin;

    enum CornerFlags {
        kTopLeft_CornerFlag     = (1 << SkRRect::kUpperLeft_Corner),
        kTopRight_CornerFlag    = (1 << SkRRect::kUpperRight_Corner),
        kBottomRight_CornerFlag = (1 << SkRRect::kLowerRight_Corner),
        kBottomLeft_CornerFlag  = (1 << SkRRect::kLowerLeft_Corner),

        kLeft_CornerFlags   = kTopLeft_CornerFlag    | kBottomLeft_CornerFlag,
        kTop_CornerFlags    = kTopLeft_CornerFlag    | kTopRight_CornerFlag,
        kRight_CornerFlags  = kTopRight_CornerFlag   | kBottomRight_CornerFlag,
        kBottom_CornerFlags = kBottomLeft_CornerFlag | kBottomRight_CornerFlag,

        kAll_CornerFlags = kTopLeft_CornerFlag    | kTopRight_CornerFlag |
                           kBottomLeft_CornerFlag | kBottomRight_CornerFlag,

    };

    // The flags are used to indicate which corners are circluar (unflagged corners are assumed to
    // be square).
    static GrEffectRef* Create(GrEffectEdgeType, uint32_t circularCornerFlags, const SkRRect&);

    virtual ~CircularRRectEffect() {};
    static const char* Name() { return "CircularRRect"; }

    const SkRRect& getRRect() const { return fRRect; }

    uint32_t getCircularCornerFlags() const { return fCircularCornerFlags; }

    GrEffectEdgeType getEdgeType() const { return fEdgeType; }

    typedef GLCircularRRectEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    CircularRRectEffect(GrEffectEdgeType, uint32_t circularCornerFlags, const SkRRect&);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    SkRRect             fRRect;
    GrEffectEdgeType    fEdgeType;
    uint32_t            fCircularCornerFlags;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

const SkScalar CircularRRectEffect::kRadiusMin = 0.5f;

GrEffectRef* CircularRRectEffect::Create(GrEffectEdgeType edgeType,
                                 uint32_t circularCornerFlags,
                                 const SkRRect& rrect) {
    SkASSERT(kFillAA_GrEffectEdgeType == edgeType || kInverseFillAA_GrEffectEdgeType == edgeType);
    return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(CircularRRectEffect,
                                                      (edgeType, circularCornerFlags, rrect))));
}

void CircularRRectEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& CircularRRectEffect::getFactory() const {
    return GrTBackendEffectFactory<CircularRRectEffect>::getInstance();
}

CircularRRectEffect::CircularRRectEffect(GrEffectEdgeType edgeType, uint32_t circularCornerFlags,
                         const SkRRect& rrect)
    : fRRect(rrect)
    , fEdgeType(edgeType)
    , fCircularCornerFlags(circularCornerFlags) {
    this->setWillReadFragmentPosition();
}

bool CircularRRectEffect::onIsEqual(const GrEffect& other) const {
    const CircularRRectEffect& crre = CastEffect<CircularRRectEffect>(other);
    // The corner flags are derived from fRRect, so no need to check them.
    return fEdgeType == crre.fEdgeType && fRRect == crre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(CircularRRectEffect);

GrEffectRef* CircularRRectEffect::TestCreate(SkRandom* random,
                                     GrContext*,
                                     const GrDrawTargetCaps& caps,
                                     GrTexture*[]) {
    SkScalar w = random->nextRangeScalar(20.f, 1000.f);
    SkScalar h = random->nextRangeScalar(20.f, 1000.f);
    SkScalar r = random->nextRangeF(kRadiusMin, 9.f);
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);
    GrEffectRef* effect;
    do {
        GrEffectEdgeType et = (GrEffectEdgeType)random->nextULessThan(kGrEffectEdgeTypeCnt);
        effect = GrRRectEffect::Create(et, rrect);
    } while (NULL == effect);
    return effect;
}

//////////////////////////////////////////////////////////////////////////////

class GLCircularRRectEffect : public GrGLEffect {
public:
    GLCircularRRectEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

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

GLCircularRRectEffect::GLCircularRRectEffect(const GrBackendEffectFactory& factory,
                             const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevRRect.setEmpty();
}

void GLCircularRRectEffect::emitCode(GrGLShaderBuilder* builder,
                             const GrDrawEffect& drawEffect,
                             EffectKey key,
                             const char* outputColor,
                             const char* inputColor,
                             const TransformedCoordsArray&,
                             const TextureSamplerArray& samplers) {
    const CircularRRectEffect& crre = drawEffect.castEffect<CircularRRectEffect>();
    const char *rectName;
    const char *radiusPlusHalfName;
    // The inner rect is the rrect bounds inset by the radius. Its left, top, right, and bottom
    // edges correspond to components x, y, z, and w, respectively. When a side of the rrect has
    // only rectangular corners, that side's value corresponds to the rect edge's value outset by
    // half a pixel.
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
    switch (crre.getCircularCornerFlags()) {
        case CircularRRectEffect::kAll_CornerFlags:
            builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(max(dxy0, dxy1), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat alpha = clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kTopLeft_CornerFlag:
            builder->fsCodeAppendf("\t\tvec2 dxy = max(%s.xy - %s.xy, 0.0);\n",
                                   rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat rightAlpha = clamp(%s.z - %s.x, 0.0, 1.0);\n",
                                    rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat bottomAlpha = clamp(%s.w - %s.y, 0.0, 1.0);\n",
                                    rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat alpha = bottomAlpha * rightAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kTopRight_CornerFlag:
            builder->fsCodeAppendf("\t\tvec2 dxy = max(vec2(%s.x - %s.z, %s.y - %s.y), 0.0);\n",
                                   fragmentPos, rectName, rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat leftAlpha = clamp(%s.x - %s.x, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat bottomAlpha = clamp(%s.w - %s.y, 0.0, 1.0);\n",
                                    rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat alpha = bottomAlpha * leftAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kBottomRight_CornerFlag:
            builder->fsCodeAppendf("\t\tvec2 dxy = max(%s.xy - %s.zw, 0.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat leftAlpha = clamp(%s.x - %s.x, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat topAlpha = clamp(%s.y - %s.y, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat alpha = topAlpha * leftAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kBottomLeft_CornerFlag:
            builder->fsCodeAppendf("\t\tvec2 dxy = max(vec2(%s.x - %s.x, %s.y - %s.w), 0.0);\n",
                                   rectName, fragmentPos, fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat rightAlpha = clamp(%s.z - %s.x, 0.0, 1.0);\n",
                                    rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat topAlpha = clamp(%s.y - %s.y, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat alpha = topAlpha * rightAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kLeft_CornerFlags:
            builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat dy1 = %s.y - %s.w;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(dxy0.x, max(dxy0.y, dy1)), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat rightAlpha = clamp(%s.z - %s.x, 0.0, 1.0);\n",
                                    rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat alpha = rightAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kTop_CornerFlags:
            builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat dx1 = %s.x - %s.z;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(max(dxy0.x, dx1), dxy0.y), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat bottomAlpha = clamp(%s.w - %s.y, 0.0, 1.0);\n",
                                   rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tfloat alpha = bottomAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kRight_CornerFlags:
            builder->fsCodeAppendf("\t\tfloat dy0 = %s.y - %s.y;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(dxy1.x, max(dy0, dxy1.y)), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat leftAlpha = clamp(%s.x - %s.x, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat alpha = leftAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
        case CircularRRectEffect::kBottom_CornerFlags:
            builder->fsCodeAppendf("\t\tfloat dx0 = %s.x - %s.x;\n", rectName, fragmentPos);
            builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(vec2(max(dx0, dxy1.x), dxy1.y), 0.0);\n");
            builder->fsCodeAppendf("\t\tfloat topAlpha = clamp(%s.y - %s.y, 0.0, 1.0);\n",
                                   fragmentPos, rectName);
            builder->fsCodeAppendf("\t\tfloat alpha = topAlpha * clamp(%s - length(dxy), 0.0, 1.0);\n",
                                   radiusPlusHalfName);
            break;
    }

    if (kInverseFillAA_GrEffectEdgeType == crre.getEdgeType()) {
        builder->fsCodeAppend("\t\talpha = 1.0 - alpha;\n");
    }

    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

GrGLEffect::EffectKey GLCircularRRectEffect::GenKey(const GrDrawEffect& drawEffect,
                                                    const GrGLCaps&) {
    const CircularRRectEffect& crre = drawEffect.castEffect<CircularRRectEffect>();
    GR_STATIC_ASSERT(kGrEffectEdgeTypeCnt <= 8);
    return (crre.getCircularCornerFlags() << 3) | crre.getEdgeType();
}

void GLCircularRRectEffect::setData(const GrGLUniformManager& uman,
                                    const GrDrawEffect& drawEffect) {
    const CircularRRectEffect& crre = drawEffect.castEffect<CircularRRectEffect>();
    const SkRRect& rrect = crre.getRRect();
    if (rrect != fPrevRRect) {
        SkRect rect = rrect.getBounds();
        SkScalar radius = 0;
        switch (crre.getCircularCornerFlags()) {
            case CircularRRectEffect::kAll_CornerFlags:
                SkASSERT(rrect.isSimpleCircular());
                radius = rrect.getSimpleRadii().fX;
                SkASSERT(radius >= CircularRRectEffect::kRadiusMin);
                rect.inset(radius, radius);
                break;
            case CircularRRectEffect::kTopLeft_CornerFlag:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight += 0.5f;
                rect.fBottom += 0.5f;
                break;
            case CircularRRectEffect::kTopRight_CornerFlag:
                radius = rrect.radii(SkRRect::kUpperRight_Corner).fX;
                rect.fLeft -= 0.5f;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom += 0.5f;
                break;
            case CircularRRectEffect::kBottomRight_CornerFlag:
                radius = rrect.radii(SkRRect::kLowerRight_Corner).fX;
                rect.fLeft -= 0.5f;
                rect.fTop -= 0.5f;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kBottomLeft_CornerFlag:
                radius = rrect.radii(SkRRect::kLowerLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop -= 0.5f;
                rect.fRight += 0.5f;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kLeft_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight += 0.5f;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kTop_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom += 0.5f;
                break;
            case CircularRRectEffect::kRight_CornerFlags:
                radius = rrect.radii(SkRRect::kUpperRight_Corner).fX;
                rect.fLeft -= 0.5f;
                rect.fTop += radius;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
            case CircularRRectEffect::kBottom_CornerFlags:
                radius = rrect.radii(SkRRect::kLowerLeft_Corner).fX;
                rect.fLeft += radius;
                rect.fTop -= 0.5f;
                rect.fRight -= radius;
                rect.fBottom -= radius;
                break;
            default:
                GrCrash("Should have been one of the above cases.");
        }
        uman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        uman.set1f(fRadiusPlusHalfUniform, radius + 0.5f);
        fPrevRRect = rrect;
    }
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipticalRRectEffect;

class EllipticalRRectEffect : public GrEffect {
public:
    // This effect currently works for these two classifications of SkRRects
    enum RRectType {
        kSimple_RRectType,       // SkRRect::kSimple_Type
        kNinePatch_RRectType,    // The two left x radii are the same, the two
                                 // top y radii are the same, etc.
    };

    // This effect only supports rrects where the radii are >= kRadiusMin.
    static const SkScalar kRadiusMin;

    static GrEffectRef* Create(GrEffectEdgeType, RRectType, const SkRRect&);

    virtual ~EllipticalRRectEffect() {};
    static const char* Name() { return "EllipticalRRect"; }

    const SkRRect& getRRect() const { return fRRect; }

    RRectType getRRectType() const { return fRRectType; }

    GrEffectEdgeType getEdgeType() const { return fEdgeType; }

    typedef GLEllipticalRRectEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    EllipticalRRectEffect(GrEffectEdgeType, RRectType, const SkRRect&);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    SkRRect             fRRect;
    RRectType           fRRectType;
    GrEffectEdgeType    fEdgeType;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

const SkScalar EllipticalRRectEffect::kRadiusMin = 0.5f;

GrEffectRef* EllipticalRRectEffect::Create(GrEffectEdgeType edgeType,
                                           RRectType rrType,
                                           const SkRRect& rrect) {
    SkASSERT(kFillAA_GrEffectEdgeType == edgeType || kInverseFillAA_GrEffectEdgeType == edgeType);
    return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(EllipticalRRectEffect, (edgeType, rrType,
                                                                              rrect))));
}

void EllipticalRRectEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& EllipticalRRectEffect::getFactory() const {
    return GrTBackendEffectFactory<EllipticalRRectEffect>::getInstance();
}

EllipticalRRectEffect::EllipticalRRectEffect(GrEffectEdgeType edgeType, RRectType rrType,
                                             const SkRRect& rrect)
    : fRRect(rrect)
    , fRRectType(rrType)
    , fEdgeType(edgeType){
    this->setWillReadFragmentPosition();
}

bool EllipticalRRectEffect::onIsEqual(const GrEffect& other) const {
    const EllipticalRRectEffect& erre = CastEffect<EllipticalRRectEffect>(other);
    // No need to check fRRectType as it is derived from fRRect.
    return fEdgeType == erre.fEdgeType && fRRect == erre.fRRect;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(EllipticalRRectEffect);

GrEffectRef* EllipticalRRectEffect::TestCreate(SkRandom* random,
                                               GrContext*,
                                               const GrDrawTargetCaps& caps,
                                               GrTexture*[]) {
    SkScalar w = random->nextRangeScalar(20.f, 1000.f);
    SkScalar h = random->nextRangeScalar(20.f, 1000.f);
    SkVector r[4];
    r[SkRRect::kUpperLeft_Corner].fX = random->nextRangeF(kRadiusMin, 9.f);
    // ensure at least one corner really is elliptical
    do {
        r[SkRRect::kUpperLeft_Corner].fY = random->nextRangeF(kRadiusMin, 9.f);
    } while (r[SkRRect::kUpperLeft_Corner].fY == r[SkRRect::kUpperLeft_Corner].fX);

    SkRRect rrect;
    if (random->nextBool()) {
        // half the time create a four-radii rrect.
        r[SkRRect::kLowerRight_Corner].fX = random->nextRangeF(kRadiusMin, 9.f);
        r[SkRRect::kLowerRight_Corner].fY = random->nextRangeF(kRadiusMin, 9.f);

        r[SkRRect::kUpperRight_Corner].fX = r[SkRRect::kLowerRight_Corner].fX;
        r[SkRRect::kUpperRight_Corner].fY = r[SkRRect::kUpperLeft_Corner].fY;

        r[SkRRect::kLowerLeft_Corner].fX = r[SkRRect::kUpperLeft_Corner].fX;
        r[SkRRect::kLowerLeft_Corner].fY = r[SkRRect::kLowerRight_Corner].fY;

        rrect.setRectRadii(SkRect::MakeWH(w, h), r);
    } else {
        rrect.setRectXY(SkRect::MakeWH(w, h), r[SkRRect::kUpperLeft_Corner].fX,
                                              r[SkRRect::kUpperLeft_Corner].fY);
    }
    GrEffectRef* effect;
    do {
        GrEffectEdgeType et = (GrEffectEdgeType)random->nextULessThan(kGrEffectEdgeTypeCnt);
        effect = GrRRectEffect::Create(et, rrect);
    } while (NULL == effect);
    return effect;
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipticalRRectEffect : public GrGLEffect {
public:
    GLEllipticalRRectEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

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
    GrGLUniformManager::UniformHandle   fInvRadiiSqdUniform;
    SkRRect                             fPrevRRect;
    typedef GrGLEffect INHERITED;
};

GLEllipticalRRectEffect::GLEllipticalRRectEffect(const GrBackendEffectFactory& factory,
                             const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevRRect.setEmpty();
}

void GLEllipticalRRectEffect::emitCode(GrGLShaderBuilder* builder,
                                       const GrDrawEffect& drawEffect,
                                       EffectKey key,
                                       const char* outputColor,
                                       const char* inputColor,
                                       const TransformedCoordsArray&,
                                       const TextureSamplerArray& samplers) {
    const EllipticalRRectEffect& erre = drawEffect.castEffect<EllipticalRRectEffect>();
    const char *rectName;
    // The inner rect is the rrect bounds inset by the x/y radii
    fInnerRectUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kVec4f_GrSLType,
                                            "innerRect",
                                            &rectName);
    const char* fragmentPos = builder->fragmentPosition();
    // At each quarter-ellipse corner we compute a vector that is the offset of the fragment pos
    // to the ellipse center. The vector is pinned in x and y to be in the quarter-plane relevant
    // to that corner. This means that points near the interior near the rrect top edge will have
    // a vector that points straight up for both the TL left and TR corners. Computing an
    // alpha from this vector at either the TR or TL corner will give the correct result. Similarly,
    // fragments near the other three edges will get the correct AA. Fragments in the interior of
    // the rrect will have a (0,0) vector at all four corners. So long as the radii > 0.5 they will
    // correctly produce an alpha value of 1 at all four corners. We take the min of all the alphas.
    // The code below is a simplified version of the above that performs maxs on the vector
    // components before computing distances and alpha values so that only one distance computation
    // need be computed to determine the min alpha.
    builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
    builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
    switch (erre.getRRectType()) {
        case EllipticalRRectEffect::kSimple_RRectType: {
            const char *invRadiiXYSqdName;
            fInvRadiiSqdUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                      kVec2f_GrSLType,
                                                      "invRadiiXY",
                                                      &invRadiiXYSqdName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(max(dxy0, dxy1), 0.0);\n");
            // Z is the x/y offsets divided by squared radii.
            builder->fsCodeAppendf("\t\tvec2 Z = dxy * %s;\n", invRadiiXYSqdName);
            break;
        }
        case EllipticalRRectEffect::kNinePatch_RRectType: {
            const char *invRadiiLTRBSqdName;
            fInvRadiiSqdUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                                      kVec4f_GrSLType,
                                                      "invRadiiLTRB",
                                                      &invRadiiLTRBSqdName);
            builder->fsCodeAppend("\t\tvec2 dxy = max(max(dxy0, dxy1), 0.0);\n");
            // Z is the x/y offsets divided by squared radii. We only care about the (at most) one
            // corner where both the x and y offsets are positive, hence the maxes. (The inverse
            // squared radii will always be positive.)
            builder->fsCodeAppendf("\t\tvec2 Z = max(max(dxy0 * %s.xy, dxy1 * %s.zw), 0.0);\n",
                                   invRadiiLTRBSqdName, invRadiiLTRBSqdName);
            break;
       }
    }
    // implicit is the evaluation of (x/a)^2 + (y/b)^2 - 1.
    builder->fsCodeAppend("\t\tfloat implicit = dot(Z, dxy) - 1.0;\n");
    // grad_dot is the squared length of the gradient of the implicit.
    builder->fsCodeAppendf("\t\tfloat grad_dot = 4.0 * dot(Z, Z);\n");
    builder->fsCodeAppend("\t\tgrad_dot = max(grad_dot, 1.0e-4);\n");
    builder->fsCodeAppendf("\t\tfloat approx_dist = implicit * inversesqrt(grad_dot);\n");

    if (kFillAA_GrEffectEdgeType == erre.getEdgeType()) {
        builder->fsCodeAppend("\t\tfloat alpha = clamp(0.5 - approx_dist, 0.0, 1.0);\n");
    } else {
        builder->fsCodeAppend("\t\tfloat alpha = clamp(0.5 + approx_dist, 0.0, 1.0);\n");
    }

    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

GrGLEffect::EffectKey GLEllipticalRRectEffect::GenKey(const GrDrawEffect& drawEffect,
                                                      const GrGLCaps&) {
    const EllipticalRRectEffect& erre = drawEffect.castEffect<EllipticalRRectEffect>();
    GR_STATIC_ASSERT(kLast_GrEffectEdgeType < (1 << 3));
    return erre.getRRectType() | erre.getEdgeType() << 3;
}

void GLEllipticalRRectEffect::setData(const GrGLUniformManager& uman,
                                      const GrDrawEffect& drawEffect) {
    const EllipticalRRectEffect& erre = drawEffect.castEffect<EllipticalRRectEffect>();
    const SkRRect& rrect = erre.getRRect();
    if (rrect != fPrevRRect) {
        SkRect rect = rrect.getBounds();
        const SkVector& r0 = rrect.radii(SkRRect::kUpperLeft_Corner);
        SkASSERT(r0.fX >= EllipticalRRectEffect::kRadiusMin);
        SkASSERT(r0.fY >= EllipticalRRectEffect::kRadiusMin);
        switch (erre.getRRectType()) {
            case EllipticalRRectEffect::kSimple_RRectType:
                rect.inset(r0.fX, r0.fY);
                uman.set2f(fInvRadiiSqdUniform, 1.f / (r0.fX * r0.fX),
                                                1.f / (r0.fY * r0.fY));
                break;
            case EllipticalRRectEffect::kNinePatch_RRectType: {
                const SkVector& r1 = rrect.radii(SkRRect::kLowerRight_Corner);
                SkASSERT(r1.fX >= EllipticalRRectEffect::kRadiusMin);
                SkASSERT(r1.fY >= EllipticalRRectEffect::kRadiusMin);
                rect.fLeft += r0.fX;
                rect.fTop += r0.fY;
                rect.fRight -= r1.fX;
                rect.fBottom -= r1.fY;
                uman.set4f(fInvRadiiSqdUniform, 1.f / (r0.fX * r0.fX),
                                                1.f / (r0.fY * r0.fY),
                                                1.f / (r1.fX * r1.fX),
                                                1.f / (r1.fY * r1.fY));
                break;
            }
        }
        uman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        fPrevRRect = rrect;
    }
}

//////////////////////////////////////////////////////////////////////////////

GrEffectRef* GrRRectEffect::Create(GrEffectEdgeType edgeType, const SkRRect& rrect) {
    if (kFillAA_GrEffectEdgeType != edgeType && kInverseFillAA_GrEffectEdgeType != edgeType) {
        return NULL;
    }
    uint32_t cornerFlags;
    if (rrect.isSimple()) {
        if (rrect.getSimpleRadii().fX == rrect.getSimpleRadii().fY) {
            if (rrect.getSimpleRadii().fX < CircularRRectEffect::kRadiusMin) {
                return NULL;
            }
            cornerFlags = CircularRRectEffect::kAll_CornerFlags;
        } else {
            if (rrect.getSimpleRadii().fX < EllipticalRRectEffect::kRadiusMin ||
                rrect.getSimpleRadii().fY < EllipticalRRectEffect::kRadiusMin) {
                return NULL;
            }
            return EllipticalRRectEffect::Create(edgeType,
                                                 EllipticalRRectEffect::kSimple_RRectType, rrect);
        }
    } else if (rrect.isComplex()) {
        // Check for the "tab" cases - two adjacent circular corners and two square corners.
        SkScalar radius = 0;
        cornerFlags = 0;
        for (int c = 0; c < 4; ++c) {
            const SkVector& r = rrect.radii((SkRRect::Corner)c);
            SkASSERT((0 == r.fX) == (0 == r.fY));
            if (0 == r.fX) {
                continue;
            }
            if (r.fX != r.fY) {
                cornerFlags = ~0U;
                break;
            }
            if (!cornerFlags) {
                radius = r.fX;
                if (radius < CircularRRectEffect::kRadiusMin) {
                    cornerFlags = ~0U;
                    break;
                }
                cornerFlags = 1 << c;
            } else {
                if (r.fX != radius) {
                   cornerFlags = ~0U;
                   break;
                }
                cornerFlags |= 1 << c;
            }
        }

        switch (cornerFlags) {
            case CircularRRectEffect::kTopLeft_CornerFlag:
            case CircularRRectEffect::kTopRight_CornerFlag:
            case CircularRRectEffect::kBottomRight_CornerFlag:
            case CircularRRectEffect::kBottomLeft_CornerFlag:
            case CircularRRectEffect::kLeft_CornerFlags:
            case CircularRRectEffect::kTop_CornerFlags:
            case CircularRRectEffect::kRight_CornerFlags:
            case CircularRRectEffect::kBottom_CornerFlags:
            case CircularRRectEffect::kAll_CornerFlags:
                break;
            default:
                if (rrect.isNinePatch()) {
                    const SkVector& r0 = rrect.radii(SkRRect::kUpperLeft_Corner);
                    const SkVector& r1 = rrect.radii(SkRRect::kLowerRight_Corner);
                    if (r0.fX >= EllipticalRRectEffect::kRadiusMin &&
                        r0.fY >= EllipticalRRectEffect::kRadiusMin &&
                        r1.fX >= EllipticalRRectEffect::kRadiusMin &&
                        r1.fY >= EllipticalRRectEffect::kRadiusMin) {
                        return EllipticalRRectEffect::Create(edgeType,
                                                        EllipticalRRectEffect::kNinePatch_RRectType,
                                                        rrect);
                    }
                }
                return NULL;
        }
    } else {
        return NULL;
    }
    return CircularRRectEffect::Create(edgeType, cornerFlags, rrect);
}
