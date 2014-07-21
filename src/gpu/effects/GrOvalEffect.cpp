/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

#include "SkRect.h"

//////////////////////////////////////////////////////////////////////////////

class GLCircleEffect;

class CircleEffect : public GrEffect {
public:
    static GrEffect* Create(GrEffectEdgeType, const SkPoint& center, SkScalar radius);

    virtual ~CircleEffect() {};
    static const char* Name() { return "Circle"; }

    const SkPoint& getCenter() const { return fCenter; }
    SkScalar getRadius() const { return fRadius; }

    GrEffectEdgeType getEdgeType() const { return fEdgeType; }

    typedef GLCircleEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    CircleEffect(GrEffectEdgeType, const SkPoint& center, SkScalar radius);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    SkPoint             fCenter;
    SkScalar            fRadius;
    GrEffectEdgeType    fEdgeType;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

GrEffect* CircleEffect::Create(GrEffectEdgeType edgeType, const SkPoint& center, SkScalar radius) {
    SkASSERT(radius >= 0);
    return SkNEW_ARGS(CircleEffect, (edgeType, center, radius));
}

void CircleEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& CircleEffect::getFactory() const {
    return GrTBackendEffectFactory<CircleEffect>::getInstance();
}

CircleEffect::CircleEffect(GrEffectEdgeType edgeType, const SkPoint& c, SkScalar r)
    : fCenter(c)
    , fRadius(r)
    , fEdgeType(edgeType) {
    this->setWillReadFragmentPosition();
}

bool CircleEffect::onIsEqual(const GrEffect& other) const {
    const CircleEffect& ce = CastEffect<CircleEffect>(other);
    return fEdgeType == ce.fEdgeType && fCenter == ce.fCenter && fRadius == ce.fRadius;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(CircleEffect);

GrEffect* CircleEffect::TestCreate(SkRandom* random,
                                   GrContext*,
                                   const GrDrawTargetCaps& caps,
                                   GrTexture*[]) {
    SkPoint center;
    center.fX = random->nextRangeScalar(0.f, 1000.f);
    center.fY = random->nextRangeScalar(0.f, 1000.f);
    SkScalar radius = random->nextRangeF(0.f, 1000.f);
    GrEffectEdgeType et;
    do {
        et = (GrEffectEdgeType)random->nextULessThan(kGrEffectEdgeTypeCnt);
    } while (kHairlineAA_GrEffectEdgeType == et);
    return CircleEffect::Create(et, center, radius);
}

//////////////////////////////////////////////////////////////////////////////

class GLCircleEffect : public GrGLEffect {
public:
    GLCircleEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    GrGLUniformManager::UniformHandle   fCircleUniform;
    SkPoint                             fPrevCenter;
    SkScalar                            fPrevRadius;

    typedef GrGLEffect INHERITED;
};

GLCircleEffect::GLCircleEffect(const GrBackendEffectFactory& factory,
                               const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevRadius = -1.f;
}

void GLCircleEffect::emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              const GrEffectKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const CircleEffect& ce = drawEffect.castEffect<CircleEffect>();
    const char *circleName;
    // The circle uniform is (center.x, center.y, radius + 0.5) for regular fills and
    // (... ,radius - 0.5) for inverse fills.
    fCircleUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                         kVec3f_GrSLType,
                                         "circle",
                                         &circleName);
    const char* fragmentPos = builder->fragmentPosition();

    SkASSERT(kHairlineAA_GrEffectEdgeType != ce.getEdgeType());
    if (GrEffectEdgeTypeIsInverseFill(ce.getEdgeType())) {
        builder->fsCodeAppendf("\t\tfloat d = length(%s.xy - %s.xy) - %s.z;\n",
                                circleName, fragmentPos, circleName);
    } else {
        builder->fsCodeAppendf("\t\tfloat d = %s.z - length(%s.xy - %s.xy);\n",
                               circleName, fragmentPos, circleName);
    }
    if (GrEffectEdgeTypeIsAA(ce.getEdgeType())) {
        builder->fsCodeAppend("\t\td = clamp(d, 0.0, 1.0);\n");
    } else {
        builder->fsCodeAppend("\t\td = d > 0.5 ? 1.0 : 0.0;\n");
    }

    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("d")).c_str());
}

void GLCircleEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                            GrEffectKeyBuilder* b) {
    const CircleEffect& ce = drawEffect.castEffect<CircleEffect>();
    b->add32(ce.getEdgeType());
}

void GLCircleEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const CircleEffect& ce = drawEffect.castEffect<CircleEffect>();
    if (ce.getRadius() != fPrevRadius || ce.getCenter() != fPrevCenter) {
        SkScalar radius = ce.getRadius();
        if (GrEffectEdgeTypeIsInverseFill(ce.getEdgeType())) {
            radius -= 0.5f;
        } else {
            radius += 0.5f;
        }
        uman.set3f(fCircleUniform, ce.getCenter().fX, ce.getCenter().fY, radius);
        fPrevCenter = ce.getCenter();
        fPrevRadius = ce.getRadius();
    }
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipseEffect;

class EllipseEffect : public GrEffect {
public:
    static GrEffect* Create(GrEffectEdgeType, const SkPoint& center, SkScalar rx, SkScalar ry);

    virtual ~EllipseEffect() {};
    static const char* Name() { return "Ellipse"; }

    const SkPoint& getCenter() const { return fCenter; }
    SkVector getRadii() const { return fRadii; }

    GrEffectEdgeType getEdgeType() const { return fEdgeType; }

    typedef GLEllipseEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    EllipseEffect(GrEffectEdgeType, const SkPoint& center, SkScalar rx, SkScalar ry);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    SkPoint             fCenter;
    SkVector            fRadii;
    GrEffectEdgeType    fEdgeType;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

GrEffect* EllipseEffect::Create(GrEffectEdgeType edgeType,
                                const SkPoint& center,
                                SkScalar rx,
                                SkScalar ry) {
    SkASSERT(rx >= 0 && ry >= 0);
    return SkNEW_ARGS(EllipseEffect, (edgeType, center, rx, ry));
}

void EllipseEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& EllipseEffect::getFactory() const {
    return GrTBackendEffectFactory<EllipseEffect>::getInstance();
}

EllipseEffect::EllipseEffect(GrEffectEdgeType edgeType, const SkPoint& c, SkScalar rx, SkScalar ry)
    : fCenter(c)
    , fRadii(SkVector::Make(rx, ry))
    , fEdgeType(edgeType) {
    this->setWillReadFragmentPosition();
}

bool EllipseEffect::onIsEqual(const GrEffect& other) const {
    const EllipseEffect& ee = CastEffect<EllipseEffect>(other);
    return fEdgeType == ee.fEdgeType && fCenter == ee.fCenter && fRadii == ee.fRadii;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(EllipseEffect);

GrEffect* EllipseEffect::TestCreate(SkRandom* random,
                                    GrContext*,
                                    const GrDrawTargetCaps& caps,
                                    GrTexture*[]) {
    SkPoint center;
    center.fX = random->nextRangeScalar(0.f, 1000.f);
    center.fY = random->nextRangeScalar(0.f, 1000.f);
    SkScalar rx = random->nextRangeF(0.f, 1000.f);
    SkScalar ry = random->nextRangeF(0.f, 1000.f);
    GrEffectEdgeType et;
    do {
        et = (GrEffectEdgeType)random->nextULessThan(kGrEffectEdgeTypeCnt);
    } while (kHairlineAA_GrEffectEdgeType == et);
    return EllipseEffect::Create(et, center, rx, ry);
}

//////////////////////////////////////////////////////////////////////////////

class GLEllipseEffect : public GrGLEffect {
public:
    GLEllipseEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    GrGLUniformManager::UniformHandle   fEllipseUniform;
    SkPoint                             fPrevCenter;
    SkVector                            fPrevRadii;

    typedef GrGLEffect INHERITED;
};

GLEllipseEffect::GLEllipseEffect(const GrBackendEffectFactory& factory,
                                 const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
    fPrevRadii.fX = -1.f;
}

void GLEllipseEffect::emitCode(GrGLShaderBuilder* builder,
                               const GrDrawEffect& drawEffect,
                               const GrEffectKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const EllipseEffect& ee = drawEffect.castEffect<EllipseEffect>();
    const char *ellipseName;
    // The ellipse uniform is (center.x, center.y, 1 / rx^2, 1 / ry^2)
    fEllipseUniform = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                         kVec4f_GrSLType,
                                         "ellipse",
                                         &ellipseName);
    const char* fragmentPos = builder->fragmentPosition();

    // d is the offset to the ellipse center
    builder->fsCodeAppendf("\t\tvec2 d = %s.xy - %s.xy;\n", fragmentPos, ellipseName);
    builder->fsCodeAppendf("\t\tvec2 Z = d * %s.zw;\n", ellipseName);
    // implicit is the evaluation of (x/rx)^2 + (y/ry)^2 - 1.
    builder->fsCodeAppend("\t\tfloat implicit = dot(Z, d) - 1.0;\n");
    // grad_dot is the squared length of the gradient of the implicit.
    builder->fsCodeAppendf("\t\tfloat grad_dot = 4.0 * dot(Z, Z);\n");
    // avoid calling inversesqrt on zero.
    builder->fsCodeAppend("\t\tgrad_dot = max(grad_dot, 1.0e-4);\n");
    builder->fsCodeAppendf("\t\tfloat approx_dist = implicit * inversesqrt(grad_dot);\n");

    switch (ee.getEdgeType()) {
        case kFillAA_GrEffectEdgeType:
            builder->fsCodeAppend("\t\tfloat alpha = clamp(0.5 - approx_dist, 0.0, 1.0);\n");
            break;
        case kInverseFillAA_GrEffectEdgeType:
            builder->fsCodeAppend("\t\tfloat alpha = clamp(0.5 + approx_dist, 0.0, 1.0);\n");
            break;
        case kFillBW_GrEffectEdgeType:
            builder->fsCodeAppend("\t\tfloat alpha = approx_dist > 0.0 ? 0.0 : 1.0;\n");
            break;
        case kInverseFillBW_GrEffectEdgeType:
            builder->fsCodeAppend("\t\tfloat alpha = approx_dist > 0.0 ? 1.0 : 0.0;\n");
            break;
        case kHairlineAA_GrEffectEdgeType:
            SkFAIL("Hairline not expected here.");
    }

    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLEllipseEffect::GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                             GrEffectKeyBuilder* b) {
    const EllipseEffect& ee = drawEffect.castEffect<EllipseEffect>();
    b->add32(ee.getEdgeType());
}

void GLEllipseEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const EllipseEffect& ee = drawEffect.castEffect<EllipseEffect>();
    if (ee.getRadii() != fPrevRadii || ee.getCenter() != fPrevCenter) {
        SkScalar invRXSqd = 1.f / (ee.getRadii().fX * ee.getRadii().fX);
        SkScalar invRYSqd = 1.f / (ee.getRadii().fY * ee.getRadii().fY);
        uman.set4f(fEllipseUniform, ee.getCenter().fX, ee.getCenter().fY, invRXSqd, invRYSqd);
        fPrevCenter = ee.getCenter();
        fPrevRadii = ee.getRadii();
    }
}

//////////////////////////////////////////////////////////////////////////////

GrEffect* GrOvalEffect::Create(GrEffectEdgeType edgeType, const SkRect& oval) {
    if (kHairlineAA_GrEffectEdgeType == edgeType) {
        return NULL;
    }
    SkScalar w = oval.width();
    SkScalar h = oval.height();
    if (SkScalarNearlyEqual(w, h)) {
        w /= 2;
        return CircleEffect::Create(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + w), w);
    } else {
        w /= 2;
        h /= 2;
        return EllipseEffect::Create(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + h), w, h);
    }

    return NULL;
}
