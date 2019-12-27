/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSimpleTextureEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"

std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::Make(sk_sp<GrSurfaceProxy> proxy,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState::Filter filter) {
    return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), alphaType, matrix,
                                      GrSamplerState(GrSamplerState::WrapMode::kClamp, filter)));
}

std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::Make(sk_sp<GrSurfaceProxy> proxy,
                                                                 SkAlphaType alphaType,
                                                                 const SkMatrix& matrix,
                                                                 GrSamplerState sampler) {
    return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), alphaType, matrix, sampler));
}

GrGLSLFragmentProcessor* GrSimpleTextureEffect::onCreateGLSLInstance() const {
    class Impl : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            const char* coords;
            GrSLType coordsType;
            if (args.fFp.coordTransformsApplyToLocalCoords()) {
                coords = args.fTransformedCoords[0].fVaryingPoint.c_str();
                coordsType = args.fTransformedCoords[0].fVaryingPoint.getType();
            } else {
                coords = "_coords";
                coordsType = kHalf2_GrSLType;
            }
            auto* fb = args.fFragBuilder;
            fb->codeAppendf("%s = ", args.fOutputColor);
            fb->appendTextureLookupAndBlend(args.fInputColor, SkBlendMode::kModulate,
                                            args.fTexSamplers[0], coords, coordsType);
            fb->codeAppendf(";");
        }
    };
    return new Impl;
}

void GrSimpleTextureEffect::onGetGLSLProcessorKey(const GrShaderCaps&,
                                                  GrProcessorKeyBuilder*) const {}

bool GrSimpleTextureEffect::onIsEqual(const GrFragmentProcessor&) const { return true; }

static inline bool uses_border(const GrSamplerState s) {
    return s.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder ||
           s.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder;
}

GrSimpleTextureEffect::GrSimpleTextureEffect(sk_sp<GrSurfaceProxy> texture, SkAlphaType alphaType,
                                             const SkMatrix& matrix, GrSamplerState sampler)
        : GrFragmentProcessor(kGrSimpleTextureEffect_ClassID,
                              ModulateForSamplerOptFlags(alphaType, uses_border(sampler)))
        , fCoordTransform(matrix, texture.get())
        , fSampler(std::move(texture), sampler) {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&fCoordTransform);
}

GrSimpleTextureEffect::GrSimpleTextureEffect(const GrSimpleTextureEffect& src)
        : INHERITED(kGrSimpleTextureEffect_ClassID, src.optimizationFlags())
        , fCoordTransform(src.fCoordTransform)
        , fSampler(src.fSampler) {
    this->setTextureSamplerCnt(1);
    this->addCoordTransform(&fCoordTransform);
}

std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSimpleTextureEffect(*this));
}

const GrFragmentProcessor::TextureSampler& GrSimpleTextureEffect::onTextureSampler(int) const {
    return fSampler;
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSimpleTextureEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrSimpleTextureEffect::TestCreate(
        GrProcessorTestData* testData) {
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
    return GrSimpleTextureEffect::Make(std::move(proxy), at, matrix, params);
}
#endif
