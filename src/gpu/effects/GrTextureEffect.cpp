/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrTextureEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::Make(sk_sp<GrSurfaceProxy> proxy,
                                                           SkAlphaType alphaType,
                                                           const SkMatrix& matrix,
                                                           GrSamplerState sampler) {
    return std::unique_ptr<GrFragmentProcessor>(
            new GrTextureEffect(std::move(proxy), alphaType, matrix, sampler));
}

GrGLSLFragmentProcessor* GrTextureEffect::onCreateGLSLInstance() const {
    class Impl : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            const char* coords;
            if (args.fFp.coordTransformsApplyToLocalCoords()) {
                coords = args.fTransformedCoords[0].fVaryingPoint.c_str();
            } else {
                coords = "_coords";
            }
            auto* fb = args.fFragBuilder;
            fb->codeAppendf("%s = ", args.fOutputColor);
            fb->appendTextureLookupAndBlend(args.fInputColor, SkBlendMode::kModulate,
                                            args.fTexSamplers[0], coords);
            fb->codeAppendf(";");
        }
    };
    return new Impl;
}

void GrTextureEffect::onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const {}

bool GrTextureEffect::onIsEqual(const GrFragmentProcessor&) const { return true; }

static inline bool uses_border(const GrSamplerState s) {
    return s.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder ||
           s.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder;
}

GrTextureEffect::GrTextureEffect(sk_sp<GrSurfaceProxy> texture, SkAlphaType alphaType,
                                 const SkMatrix& matrix, GrSamplerState sampler)
        : GrFragmentProcessor(kGrTextureEffect_ClassID,
                              ModulateForSamplerOptFlags(alphaType, uses_border(sampler)))
        , fCoordTransform(matrix, texture.get())
        , fSampler(std::move(texture), sampler) {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&fCoordTransform);
}

GrTextureEffect::GrTextureEffect(const GrTextureEffect& src)
        : INHERITED(kGrTextureEffect_ClassID, src.optimizationFlags())
        , fCoordTransform(src.fCoordTransform)
        , fSampler(src.fSampler) {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&fCoordTransform);
}

std::unique_ptr<GrFragmentProcessor> GrTextureEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrTextureEffect(*this));
}

const GrFragmentProcessor::TextureSampler& GrTextureEffect::onTextureSampler(int) const {
    return fSampler;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrTextureEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrTextureEffect::TestCreate(GrProcessorTestData* testData) {
    auto [proxy, ct, at] = testData->randomProxy();
    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(testData->fRandom, wrapModes);
    if (!testData->caps()->npotTextureTileSupport()) {
        // Performing repeat sampling on npot textures will cause asserts on HW
        // that lacks support.
        wrapModes[0] = GrSamplerState::WrapMode::kClamp;
        wrapModes[1] = GrSamplerState::WrapMode::kClamp;
    }

    GrSamplerState params(wrapModes, testData->fRandom->nextBool()
                                             ? GrSamplerState::Filter::kBilerp
                                             : GrSamplerState::Filter::kNearest);

    const SkMatrix& matrix = GrTest::TestMatrix(testData->fRandom);
    return GrTextureEffect::Make(std::move(proxy), at, matrix, params);
}
#endif
