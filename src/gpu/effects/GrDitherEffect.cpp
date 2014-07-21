/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDitherEffect.h"

#include "gl/GrGLEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

#include "SkRect.h"

//////////////////////////////////////////////////////////////////////////////

class GLDitherEffect;

class DitherEffect : public GrEffect {
public:
    static GrEffect* Create() {
        GR_CREATE_STATIC_EFFECT(gDitherEffect, DitherEffect, ())
        return SkRef(gDitherEffect);
    }

    virtual ~DitherEffect() {};
    static const char* Name() { return "Dither"; }

    typedef GLDitherEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<DitherEffect>::getInstance();
    }

private:
    DitherEffect() {
        this->setWillReadFragmentPosition();
    }

    // All dither effects are equal
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE { return true; }

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

void DitherEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(DitherEffect);

GrEffect* DitherEffect::TestCreate(SkRandom*,
                                   GrContext*,
                                   const GrDrawTargetCaps&,
                                   GrTexture*[]) {
    return DitherEffect::Create();
}

//////////////////////////////////////////////////////////////////////////////

class GLDitherEffect : public GrGLEffect {
public:
    GLDitherEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

private:
    typedef GrGLEffect INHERITED;
};

GLDitherEffect::GLDitherEffect(const GrBackendEffectFactory& factory,
                               const GrDrawEffect& drawEffect)
    : INHERITED (factory) {
}

void GLDitherEffect::emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              const GrEffectKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    // Generate a random number based on the fragment position. For this
    // random number generator, we use the "GLSL rand" function
    // that seems to be floating around on the internet. It works under
    // the assumption that sin(<big number>) oscillates with high frequency
    // and sampling it will generate "randomness". Since we're using this
    // for rendering and not cryptography it should be OK.

    // For each channel c, add the random offset to the pixel to either bump
    // it up or let it remain constant during quantization.
    builder->fsCodeAppendf("\t\tfloat r = "
                           "fract(sin(dot(%s.xy ,vec2(12.9898,78.233))) * 43758.5453);\n",
                           builder->fragmentPosition());
    builder->fsCodeAppendf("\t\t%s = (1.0/255.0) * vec4(r, r, r, r) + %s;\n",
                           outputColor, GrGLSLExpr4(inputColor).c_str());
}

//////////////////////////////////////////////////////////////////////////////

GrEffect* GrDitherEffect::Create() { return DitherEffect::Create(); }
