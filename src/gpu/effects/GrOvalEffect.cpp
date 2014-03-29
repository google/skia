/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

#include "SkRect.h"

//////////////////////////////////////////////////////////////////////////////

class GLCircleEffect;

class CircleEffect : public GrEffect {
public:
    static GrEffectRef* Create(GrEffectEdgeType, const SkPoint& center, SkScalar radius);

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

GrEffectRef* CircleEffect::Create(GrEffectEdgeType edgeType,
                                  const SkPoint& center,
                                  SkScalar radius) {
    SkASSERT(radius >= 0);
    return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(CircleEffect,
                                                      (edgeType, center, radius))));
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
    const CircleEffect& crre = CastEffect<CircleEffect>(other);
    return fEdgeType == crre.fEdgeType && fCenter == crre.fCenter && fRadius == crre.fRadius;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(CircleEffect);

GrEffectRef* CircleEffect::TestCreate(SkRandom* random,
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
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

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
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const CircleEffect& ce = drawEffect.castEffect<CircleEffect>();
    const char *circleName;
    // The circle uniform is (center.x, center.y, radius + 0.5)
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

GrGLEffect::EffectKey GLCircleEffect::GenKey(const GrDrawEffect& drawEffect,
                                             const GrGLCaps&) {
    const CircleEffect& ce = drawEffect.castEffect<CircleEffect>();
    return ce.getEdgeType();
}

void GLCircleEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const CircleEffect& ce = drawEffect.castEffect<CircleEffect>();
    if (ce.getRadius() != fPrevRadius || ce.getCenter() != fPrevCenter) {
        uman.set3f(fCircleUniform, ce.getCenter().fX, ce.getCenter().fY, ce.getRadius() + 0.5f);
        fPrevCenter = ce.getCenter();
        fPrevRadius = ce.getRadius();
    }
}
//////////////////////////////////////////////////////////////////////////////

GrEffectRef* GrOvalEffect::Create(GrEffectEdgeType edgeType, const SkRect& oval) {
    if (kHairlineAA_GrEffectEdgeType == edgeType) {
        return NULL;
    }
    SkScalar w = oval.width();
    SkScalar h = oval.height();
    if (SkScalarNearlyEqual(w, h)) {
        w /= 2;
        return CircleEffect::Create(edgeType, SkPoint::Make(oval.fLeft + w, oval.fTop + w), w);
    }

    return NULL;
}
