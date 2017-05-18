/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDitherEffect.h"
#include "GrFragmentProcessor.h"
#include "SkRect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "../private/GrGLSL.h"

//////////////////////////////////////////////////////////////////////////////

class DitherEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make() {
        return sk_sp<GrFragmentProcessor>(new DitherEffect);
    }

    ~DitherEffect() override {}

    const char* name() const override { return "Dither"; }

private:
    DitherEffect() : INHERITED(kNone_OptimizationFlags) { this->initClassID<DitherEffect>(); }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    // All dither effects are equal
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(DitherEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> DitherEffect::TestCreate(GrProcessorTestData*) {
    return DitherEffect::Make();
}
#endif

//////////////////////////////////////////////////////////////////////////////

class GLDitherEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override;

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GLDitherEffect::emitCode(EmitArgs& args) {
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Generate a random number based on the fragment position. For this
    // random number generator, we use the "GLSL rand" function
    // that seems to be floating around on the internet. It works under
    // the assumption that sin(<big number>) oscillates with high frequency
    // and sampling it will generate "randomness". Since we're using this
    // for rendering and not cryptography it should be OK.

    // For each channel c, add the random offset to the pixel to either bump
    // it up or let it remain constant during quantization.
    fragBuilder->codeAppendf("\t\tfloat r = "
                             "fract(sin(dot(sk_FragCoord.xy, vec2(12.9898,78.233))) * "
                                                            "43758.5453);\n");
    fragBuilder->codeAppendf("\t\t%s = clamp((1.0/255.0) * vec4(r, r, r, r) + %s, 0, 1);\n",
                             args.fOutputColor, args.fInputColor);
}

//////////////////////////////////////////////////////////////////////////////

void DitherEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                         GrProcessorKeyBuilder* b) const {
    GLDitherEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* DitherEffect::onCreateGLSLInstance() const  {
    return new GLDitherEffect;
}

sk_sp<GrFragmentProcessor> GrDitherEffect::Make() { return DitherEffect::Make(); }
