/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrDitherEffect.h"

#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "GrTBackendProcessorFactory.h"

#include "SkRect.h"

//////////////////////////////////////////////////////////////////////////////

class GLDitherEffect;

class DitherEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create() {
        GR_CREATE_STATIC_FRAGMENT_PROCESSOR(gDitherEffect, DitherEffect, ())
        return SkRef(gDitherEffect);
    }

    virtual ~DitherEffect() {};
    static const char* Name() { return "Dither"; }

    typedef GLDitherEffect GLProcessor;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendFragmentProcessorFactory<DitherEffect>::getInstance();
    }

private:
    DitherEffect() {
        this->setWillReadFragmentPosition();
    }

    // All dither effects are equal
    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE { return true; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

void DitherEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    *validFlags = 0;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(DitherEffect);

GrFragmentProcessor* DitherEffect::TestCreate(SkRandom*,
                                              GrContext*,
                                              const GrDrawTargetCaps&,
                                              GrTexture*[]) {
    return DitherEffect::Create();
}

//////////////////////////////////////////////////////////////////////////////

class GLDitherEffect : public GrGLFragmentProcessor {
public:
    GLDitherEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLProgramBuilder* builder,
                          const GrFragmentProcessor& fp,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

private:
    typedef GrGLFragmentProcessor INHERITED;
};

GLDitherEffect::GLDitherEffect(const GrBackendProcessorFactory& factory,
                               const GrProcessor&)
    : INHERITED (factory) {
}

void GLDitherEffect::emitCode(GrGLProgramBuilder* builder,
                              const GrFragmentProcessor& fp,
                              const GrProcessorKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    // Generate a random number based on the fragment position. For this
    // random number generator, we use the "GLSL rand" function
    // that seems to be floating around on the internet. It works under
    // the assumption that sin(<big number>) oscillates with high frequency
    // and sampling it will generate "randomness". Since we're using this
    // for rendering and not cryptography it should be OK.

    // For each channel c, add the random offset to the pixel to either bump
    // it up or let it remain constant during quantization.
    fsBuilder->codeAppendf("\t\tfloat r = "
                           "fract(sin(dot(%s.xy ,vec2(12.9898,78.233))) * 43758.5453);\n",
                           fsBuilder->fragmentPosition());
    fsBuilder->codeAppendf("\t\t%s = (1.0/255.0) * vec4(r, r, r, r) + %s;\n",
                           outputColor, GrGLSLExpr4(inputColor).c_str());
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor* GrDitherEffect::Create() { return DitherEffect::Create(); }
