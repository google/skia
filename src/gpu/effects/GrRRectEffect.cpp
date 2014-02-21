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

class GLRRectEffect;

class RRectEffect : public GrEffect {
public:
    // This effect only supports circular corner rrects where all corners have the same radius
    // which must be <= kRadiusMin.
    static const SkScalar kRadiusMin;

    static GrEffectRef* Create(const SkRRect&);

    virtual ~RRectEffect() {};
    static const char* Name() { return "RRect"; }

    const SkRRect& getRRect() const { return fRRect; }

    typedef GLRRectEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    RRectEffect(const SkRRect&);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    SkRRect fRRect;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

const SkScalar RRectEffect::kRadiusMin = 0.5f;

GrEffectRef* RRectEffect::Create(const SkRRect& rrect) {
    return CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(RRectEffect, (rrect))));
}

void RRectEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

const GrBackendEffectFactory& RRectEffect::getFactory() const {
    return GrTBackendEffectFactory<RRectEffect>::getInstance();
}

RRectEffect::RRectEffect(const SkRRect& rrect)
    : fRRect(rrect) {
    this->setWillReadFragmentPosition();
}

bool RRectEffect::onIsEqual(const GrEffect& other) const {
    const RRectEffect& rre = CastEffect<RRectEffect>(other);
    return fRRect == rre.fRRect;
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
    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeWH(w, h), r, r);

    return GrRRectEffect::Create(rrect);
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

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&) { return 0; }

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
    const char *rectName;
    const char *radiusPlusHalfName;
    // The inner rect is the rrect bounds inset by the radius. Its top, left, right, and bottom
    // edges correspond to components x, y, z, and w, respectively.
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
    builder->fsCodeAppendf("\t\tvec2 dxy0 = %s.xy - %s.xy;\n", rectName, fragmentPos);
    builder->fsCodeAppendf("\t\tvec2 dxy1 = %s.xy - %s.zw;\n", fragmentPos, rectName);
    builder->fsCodeAppend("\t\tvec2 dxy = max(max(dxy0, dxy1), 0.0);\n");
    builder->fsCodeAppendf("\t\tfloat alpha = clamp(%s - length(dxy), 0.0, 1.0);\n",
                           radiusPlusHalfName);

    builder->fsCodeAppendf("\t\t%s = %s;\n", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("alpha")).c_str());
}

void GLRRectEffect::setData(const GrGLUniformManager& uman, const GrDrawEffect& drawEffect) {
    const RRectEffect& rre = drawEffect.castEffect<RRectEffect>();
    const SkRRect& rrect = rre.getRRect();
    if (rrect != fPrevRRect) {
        SkASSERT(rrect.isSimpleCircular());
        SkRect rect = rrect.getBounds();
        SkScalar radius = rrect.getSimpleRadii().fX;
        SkASSERT(radius >= RRectEffect::kRadiusMin);
        rect.inset(radius, radius);
        uman.set4f(fInnerRectUniform, rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
        uman.set1f(fRadiusPlusHalfUniform, radius + 0.5f);
        fPrevRRect = rrect;
    }
}

//////////////////////////////////////////////////////////////////////////////

GrEffectRef* GrRRectEffect::Create(const SkRRect& rrect) {
    if (!rrect.isSimpleCircular()) {
        return NULL;
    }

    if (rrect.getSimpleRadii().fX < RRectEffect::kRadiusMin) {
        return NULL;
    }

    return RRectEffect::Create(rrect);
}
