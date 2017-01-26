/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSRGBEffect.h"

#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrGLSRGBEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrSRGBEffect& srgbe = args.fFp.cast<GrSRGBEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        SkString srgbFuncName;
        static const GrShaderVar gSrgbArgs[] = {
            GrShaderVar("x", kFloat_GrSLType),
        };
        switch (srgbe.mode()) {
            case GrSRGBEffect::Mode::kLinearToSRGB:
                fragBuilder->emitFunction(kFloat_GrSLType,
                                          "linear_to_srgb",
                                          SK_ARRAY_COUNT(gSrgbArgs),
                                          gSrgbArgs,
                                          "return (x <= 0.0031308) ? (x * 12.92) "
                                          ": (1.055 * pow(x, 0.416666667) - 0.055);",
                                          &srgbFuncName);
                break;
            case GrSRGBEffect::Mode::kSRGBToLinear:
                fragBuilder->emitFunction(kFloat_GrSLType,
                                          "srgb_to_linear",
                                          SK_ARRAY_COUNT(gSrgbArgs),
                                          gSrgbArgs,
                                          "return (x <= 0.04045) ? (x / 12.92) "
                                          ": pow((x + 0.055) / 1.055, 2.4);",
                                          &srgbFuncName);
                break;
        }

        if (nullptr == args.fInputColor) {
            args.fInputColor = "vec4(1)";
        }

        fragBuilder->codeAppendf("%s = vec4(%s(%s.r), %s(%s.g), %s(%s.b), %s.a);",
                                    args.fOutputColor,
                                    srgbFuncName.c_str(), args.fInputColor,
                                    srgbFuncName.c_str(), args.fInputColor,
                                    srgbFuncName.c_str(), args.fInputColor,
                                    args.fInputColor);
    }

    static inline void GenKey(const GrProcessor& processor, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrSRGBEffect& srgbe = processor.cast<GrSRGBEffect>();
        uint32_t key = static_cast<uint32_t>(srgbe.mode());
        b->add32(key);
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrSRGBEffect::GrSRGBEffect(Mode mode)
    : fMode(mode) {
    this->initClassID<GrSRGBEffect>();
}

bool GrSRGBEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrSRGBEffect& other = s.cast<GrSRGBEffect>();
    return other.fMode == fMode;
}

void GrSRGBEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSRGBEffect);

sk_sp<GrFragmentProcessor> GrSRGBEffect::TestCreate(GrProcessorTestData* d) {
    Mode testMode = static_cast<Mode>(d->fRandom->nextRangeU(0, 1));
    return sk_sp<GrFragmentProcessor>(new GrSRGBEffect(testMode));
}

///////////////////////////////////////////////////////////////////////////////

void GrSRGBEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                          GrProcessorKeyBuilder* b) const {
    GrGLSRGBEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrSRGBEffect::onCreateGLSLInstance() const {
    return new GrGLSRGBEffect();
}

sk_sp<GrFragmentProcessor> GrSRGBEffect::Make(Mode mode) {
    return sk_sp<GrFragmentProcessor>(new GrSRGBEffect(mode));
}
