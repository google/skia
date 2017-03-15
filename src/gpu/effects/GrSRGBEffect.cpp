/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSRGBEffect.h"

#include "GrFragmentProcessor.h"
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
        : INHERITED(kPreservesOpaqueInput_OptimizationFlag |
                    kConstantOutputForConstantInput_OptimizationFlag)
        , fMode(mode) {
    this->initClassID<GrSRGBEffect>();
}

bool GrSRGBEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrSRGBEffect& other = s.cast<GrSRGBEffect>();
    return other.fMode == fMode;
}

static inline float srgb_to_linear(float srgb) {
    return (srgb <= 0.04045f) ? srgb / 12.92f : powf((srgb + 0.055f) / 1.055f, 2.4f);
}
static inline float linear_to_srgb(float linear) {
    return (linear <= 0.0031308) ? linear * 12.92f : 1.055f * powf(linear, 1.f / 2.4f) - 0.055f;
}

GrColor4f GrSRGBEffect::constantOutputForConstantInput(GrColor4f input) const {
    switch (fMode) {
        case Mode::kLinearToSRGB:
            return GrColor4f(linear_to_srgb(input.fRGBA[0]), linear_to_srgb(input.fRGBA[1]),
                             linear_to_srgb(input.fRGBA[2]), input.fRGBA[3]);
        case Mode::kSRGBToLinear:
            return GrColor4f(srgb_to_linear(input.fRGBA[0]), srgb_to_linear(input.fRGBA[1]),
                             srgb_to_linear(input.fRGBA[2]), input.fRGBA[3]);
    }
    SkFAIL("Unexpected mode");
    return GrColor4f::TransparentBlack();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSRGBEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrSRGBEffect::TestCreate(GrProcessorTestData* d) {
    Mode testMode = static_cast<Mode>(d->fRandom->nextRangeU(0, 1));
    return sk_sp<GrFragmentProcessor>(new GrSRGBEffect(testMode));
}
#endif

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
