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
        const GrShaderVar gSrgbArgs[] = {
            GrShaderVar("x", kHalf_GrSLType),
        };
        switch (srgbe.mode()) {
            case GrSRGBEffect::Mode::kLinearToSRGB:
                fragBuilder->emitFunction(kHalf_GrSLType,
                                          "linear_to_srgb",
                                          SK_ARRAY_COUNT(gSrgbArgs),
                                          gSrgbArgs,
                                          "return (x <= 0.0031308) ? (x * 12.92) "
                                          ": (1.055 * pow(x, 0.416666667) - 0.055);",
                                          &srgbFuncName);
                break;
            case GrSRGBEffect::Mode::kSRGBToLinear:
                fragBuilder->emitFunction(kHalf_GrSLType,
                                          "srgb_to_linear",
                                          SK_ARRAY_COUNT(gSrgbArgs),
                                          gSrgbArgs,
                                          "return (x <= 0.04045) ? (x / 12.92) "
                                          ": pow((x + 0.055) / 1.055, 2.4);",
                                          &srgbFuncName);
                break;
        }

        // Mali Bifrost uses fp16 for mediump. Making the intermediate color variable highp causes
        // calculations to be performed with sufficient precision.
        fragBuilder->codeAppendf("float4 color = %s;", args.fInputColor);
        if (srgbe.alpha() == GrSRGBEffect::Alpha::kPremul) {
            fragBuilder->codeAppendf("float nonZeroAlpha = max(color.a, 0.00001);");
            fragBuilder->codeAppendf("color = float4(color.rgb / nonZeroAlpha, color.a);");
        }
        fragBuilder->codeAppendf("color = float4(%s(half(color.r)), %s(half(color.g)), "
                                 "%s(half(color.b)), color.a);",
                                 srgbFuncName.c_str(),
                                 srgbFuncName.c_str(),
                                 srgbFuncName.c_str());
        if (srgbe.alpha() == GrSRGBEffect::Alpha::kPremul) {
            fragBuilder->codeAppendf("color = float4(color.rgb, 1) * color.a;");
        }
        fragBuilder->codeAppendf("%s = half4(color);", args.fOutputColor);
    }

    static inline void GenKey(const GrProcessor& processor, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrSRGBEffect& srgbe = processor.cast<GrSRGBEffect>();
        uint32_t key = static_cast<uint32_t>(srgbe.mode()) |
                      (static_cast<uint32_t>(srgbe.alpha()) << 1);
        b->add32(key);
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrSRGBEffect::GrSRGBEffect(Mode mode, Alpha alpha)
    : INHERITED(kGrSRGBEffect_ClassID, kPreservesOpaqueInput_OptimizationFlag |
                kConstantOutputForConstantInput_OptimizationFlag)
    , fMode(mode)
    , fAlpha(alpha)
{
}

std::unique_ptr<GrFragmentProcessor> GrSRGBEffect::clone() const { return Make(fMode, fAlpha); }

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

SkPMColor4f GrSRGBEffect::constantOutputForConstantInput(const SkPMColor4f& inColor) const {
    SkColor4f color = inColor.unpremul();
    switch (fMode) {
        case Mode::kLinearToSRGB:
            color = { linear_to_srgb(color.fR), linear_to_srgb(color.fG), linear_to_srgb(color.fB),
                      color.fA };
            break;
        case Mode::kSRGBToLinear:
            color = { srgb_to_linear(color.fR), srgb_to_linear(color.fG), srgb_to_linear(color.fB),
                      color.fA };
            break;
    }
    return color.premul();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSRGBEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrSRGBEffect::TestCreate(GrProcessorTestData* d) {
    Mode testMode = static_cast<Mode>(d->fRandom->nextRangeU(0, 1));
    return GrSRGBEffect::Make(testMode, Alpha::kPremul);
}
#endif

///////////////////////////////////////////////////////////////////////////////

void GrSRGBEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                          GrProcessorKeyBuilder* b) const {
    GrGLSRGBEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrSRGBEffect::onCreateGLSLInstance() const {
    return new GrGLSRGBEffect;
}

