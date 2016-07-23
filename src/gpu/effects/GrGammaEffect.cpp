/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGammaEffect.h"

#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrGLGammaEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrGammaEffect& ge = args.fFp.cast<GrGammaEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        const char* gammaUniName = nullptr;
        if (GrGammaEffect::Mode::kExponential == ge.mode()) {
            fGammaUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType,
                                                   kDefault_GrSLPrecision, "Gamma", &gammaUniName);
        }

        SkString srgbFuncName;
        static const GrGLSLShaderVar gSrgbArgs[] = {
            GrGLSLShaderVar("x", kFloat_GrSLType),
        };
        switch (ge.mode()) {
            case GrGammaEffect::Mode::kLinearToSRGB:
                fragBuilder->emitFunction(kFloat_GrSLType,
                                          "linear_to_srgb",
                                          SK_ARRAY_COUNT(gSrgbArgs),
                                          gSrgbArgs,
                                          "return (x <= 0.0031308) ? (x * 12.92) "
                                          ": (1.055 * pow(x, 0.416666667) - 0.055);",
                                          &srgbFuncName);
                break;
            case GrGammaEffect::Mode::kSRGBToLinear:
                fragBuilder->emitFunction(kFloat_GrSLType,
                                          "srgb_to_linear",
                                          SK_ARRAY_COUNT(gSrgbArgs),
                                          gSrgbArgs,
                                          "return (x <= 0.04045) ? (x / 12.92) "
                                          ": pow((x + 0.055) / 1.055, 2.4);",
                                          &srgbFuncName);
            default:
                // No helper function needed
                break;
        }

        if (nullptr == args.fInputColor) {
            args.fInputColor = "vec4(1)";
        }

        if (GrGammaEffect::Mode::kExponential == ge.mode()) {
            fragBuilder->codeAppendf("%s = vec4(pow(%s.rgb, vec3(%s)), %s.a);",
                                     args.fOutputColor, args.fInputColor, gammaUniName,
                                     args.fInputColor);
        } else {
            fragBuilder->codeAppendf("%s = vec4(%s(%s.r), %s(%s.g), %s(%s.b), %s.a);",
                                     args.fOutputColor,
                                     srgbFuncName.c_str(), args.fInputColor,
                                     srgbFuncName.c_str(), args.fInputColor,
                                     srgbFuncName.c_str(), args.fInputColor,
                                     args.fInputColor);
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
        const GrGammaEffect& ge = proc.cast<GrGammaEffect>();
        if (GrGammaEffect::Mode::kExponential == ge.mode()) {
            pdman.set1f(fGammaUni, ge.gamma());
        }
    }

    static inline void GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrGammaEffect& ge = processor.cast<GrGammaEffect>();
        uint32_t key = static_cast<uint32_t>(ge.mode());
        b->add32(key);
    }

private:
    GrGLSLProgramDataManager::UniformHandle fGammaUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrGammaEffect::GrGammaEffect(Mode mode, SkScalar gamma)
    : fMode(mode)
    , fGamma(gamma) {
    this->initClassID<GrGammaEffect>();
}

bool GrGammaEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrGammaEffect& other = s.cast<GrGammaEffect>();
    return
        other.fMode == fMode &&
        (fMode != Mode::kExponential || other.fGamma == fGamma);
}

void GrGammaEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrGammaEffect);

sk_sp<GrFragmentProcessor> GrGammaEffect::TestCreate(GrProcessorTestData* d) {
    // We want to be sure and test sRGB sometimes
    Mode testMode = static_cast<Mode>(d->fRandom->nextRangeU(0, 2));
    SkScalar gamma = d->fRandom->nextRangeScalar(0.5f, 2.0f);
    return sk_sp<GrFragmentProcessor>(new GrGammaEffect(testMode, gamma));
}

///////////////////////////////////////////////////////////////////////////////

void GrGammaEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLGammaEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrGammaEffect::onCreateGLSLInstance() const {
    return new GrGLGammaEffect();
}

sk_sp<GrFragmentProcessor> GrGammaEffect::Make(SkScalar gamma) {
    // TODO: Once our public-facing API for specifying gamma curves settles down, expose this,
    // and allow clients to explicitly request sRGB, rather than inferring from the exponent.
    // Note that AdobeRGB (for example) is speficied as x^2.2, not the Rec.709 curves.
    if (SkScalarNearlyEqual(gamma, 2.2f)) {
        return sk_sp<GrFragmentProcessor>(new GrGammaEffect(Mode::kSRGBToLinear, 2.2f));
    } else if (SkScalarNearlyEqual(gamma, 1.0f / 2.2f)) {
        return sk_sp<GrFragmentProcessor>(new GrGammaEffect(Mode::kLinearToSRGB, 1.0f / 2.2f));
    } else {
        return sk_sp<GrFragmentProcessor>(new GrGammaEffect(Mode::kExponential, gamma));
    }
}
