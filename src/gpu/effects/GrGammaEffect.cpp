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
        if (!ge.gammaIsSRGB()) {
            fGammaUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType,
                                                   kDefault_GrSLPrecision, "Gamma", &gammaUniName);
        }

        GrGLSLShaderVar tmpVar("tmpColor", kVec4f_GrSLType, 0, kHigh_GrSLPrecision);
        SkString tmpDecl;
        tmpVar.appendDecl(args.fGLSLCaps, &tmpDecl);

        SkString srgbFuncName;
        if (ge.gammaIsSRGB()) {
            static const GrGLSLShaderVar gSrgbArgs[] = {
                GrGLSLShaderVar("x", kFloat_GrSLType),
            };

            fragBuilder->emitFunction(kFloat_GrSLType,
                                        "linear_to_srgb",
                                        SK_ARRAY_COUNT(gSrgbArgs),
                                        gSrgbArgs,
                                        "return (x <= 0.0031308) ? (x * 12.92) "
                                        ": (1.055 * pow(x, 0.416666667) - 0.055);",
                                        &srgbFuncName);
        }

        fragBuilder->codeAppendf("%s;", tmpDecl.c_str());

        fragBuilder->codeAppendf("%s = ", tmpVar.c_str());
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], args.fCoords[0].c_str(),
                                            args.fCoords[0].getType());
        fragBuilder->codeAppend(";");

        if (ge.gammaIsSRGB()) {
            fragBuilder->codeAppendf("%s = vec4(%s(%s.r), %s(%s.g), %s(%s.b), %s.a);",
                                        args.fOutputColor,
                                        srgbFuncName.c_str(), tmpVar.c_str(),
                                        srgbFuncName.c_str(), tmpVar.c_str(),
                                        srgbFuncName.c_str(), tmpVar.c_str(),
                                        tmpVar.c_str());
        } else {
            fragBuilder->codeAppendf("%s = vec4(pow(%s.rgb, vec3(%s)), %s.a);",
                                        args.fOutputColor, tmpVar.c_str(), gammaUniName,
                                        tmpVar.c_str());
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
        const GrGammaEffect& ge = proc.cast<GrGammaEffect>();
        if (!ge.gammaIsSRGB()) {
            pdman.set1f(fGammaUni, ge.gamma());
        }
    }

    static inline void GenKey(const GrProcessor& processor, const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrGammaEffect& ge = processor.cast<GrGammaEffect>();
        uint32_t key = ge.gammaIsSRGB() ? 0x1 : 0x0;
        b->add32(key);
    }

private:
    GrGLSLProgramDataManager::UniformHandle fGammaUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrGammaEffect::GrGammaEffect(GrTexture* texture, SkScalar gamma)
    : INHERITED(texture, GrCoordTransform::MakeDivByTextureWHMatrix(texture)) {
    this->initClassID<GrGammaEffect>();

    fGamma = gamma;
    fGammaIsSRGB = SkScalarNearlyEqual(gamma, 1.0f / 2.2f);
}

bool GrGammaEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrGammaEffect& other = s.cast<GrGammaEffect>();
    return
        other.fGammaIsSRGB == fGammaIsSRGB &&
        other.fGamma == fGamma;
}

void GrGammaEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrGammaEffect);

const GrFragmentProcessor* GrGammaEffect::TestCreate(GrProcessorTestData* d) {
    // We want to be sure and test sRGB sometimes
    bool srgb = d->fRandom->nextBool();
    return new GrGammaEffect(d->fTextures[GrProcessorUnitTest::kSkiaPMTextureIdx],
                             srgb ? 1.0f / 2.2f : d->fRandom->nextRangeScalar(0.5f, 2.0f));
}

///////////////////////////////////////////////////////////////////////////////

void GrGammaEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLGammaEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrGammaEffect::onCreateGLSLInstance() const {
    return new GrGLGammaEffect();
}

const GrFragmentProcessor* GrGammaEffect::Create(GrTexture* texture, SkScalar gamma) {
    return new GrGammaEffect(texture, gamma);
}
