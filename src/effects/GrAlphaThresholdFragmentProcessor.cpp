/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAlphaThresholdFragmentProcessor.h"

#if SK_SUPPORT_GPU

#include "SkRefCnt.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "../private/GrGLSL.h"

sk_sp<GrFragmentProcessor> GrAlphaThresholdFragmentProcessor::Make(
                                                           GrTexture* texture,
                                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                                           GrTexture* maskTexture,
                                                           float innerThreshold,
                                                           float outerThreshold,
                                                           const SkIRect& bounds) {
    return sk_sp<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(
                                                                texture, std::move(colorSpaceXform),
                                                                maskTexture,
                                                                innerThreshold, outerThreshold,
                                                                bounds));
}

inline GrFragmentProcessor::OptimizationFlags GrAlphaThresholdFragmentProcessor::OptFlags(float outerThreshold) {
    if (outerThreshold >= 1.f) {
        return kPreservesOpaqueInput_OptimizationFlag | kModulatesInput_OptimizationFlag;
    } else {
        return kModulatesInput_OptimizationFlag;
    }
}

GrAlphaThresholdFragmentProcessor::GrAlphaThresholdFragmentProcessor(
                                                           GrTexture* texture,
                                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                                           GrTexture* maskTexture,
                                                           float innerThreshold,
                                                           float outerThreshold,
                                                           const SkIRect& bounds)
        : INHERITED(OptFlags(outerThreshold))
        , fInnerThreshold(innerThreshold)
        , fOuterThreshold(outerThreshold)
        , fImageCoordTransform(SkMatrix::I(), texture, GrSamplerParams::kNone_FilterMode)
        , fImageTextureSampler(texture)
        , fColorSpaceXform(std::move(colorSpaceXform))
        , fMaskCoordTransform(
                  SkMatrix::MakeTrans(SkIntToScalar(-bounds.x()), SkIntToScalar(-bounds.y())),
                  maskTexture,
                  GrSamplerParams::kNone_FilterMode)
        , fMaskTextureSampler(maskTexture) {
    this->initClassID<GrAlphaThresholdFragmentProcessor>();
    this->addCoordTransform(&fImageCoordTransform);
    this->addTextureSampler(&fImageTextureSampler);
    this->addCoordTransform(&fMaskCoordTransform);
    this->addTextureSampler(&fMaskTextureSampler);
}

bool GrAlphaThresholdFragmentProcessor::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrAlphaThresholdFragmentProcessor& s = sBase.cast<GrAlphaThresholdFragmentProcessor>();
    return (this->fInnerThreshold == s.fInnerThreshold &&
            this->fOuterThreshold == s.fOuterThreshold);
}

///////////////////////////////////////////////////////////////////////////////

class GrGLAlphaThresholdFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor& effect, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrAlphaThresholdFragmentProcessor& atfp =
            effect.cast<GrAlphaThresholdFragmentProcessor>();
        b->add32(GrColorSpaceXform::XformKey(atfp.colorSpaceXform()));
    }

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fInnerThresholdVar;
    GrGLSLProgramDataManager::UniformHandle fOuterThresholdVar;
    GrGLSLProgramDataManager::UniformHandle fColorSpaceXformVar;
    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLAlphaThresholdFragmentProcessor::emitCode(EmitArgs& args) {
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fInnerThresholdVar = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    "inner_threshold");
    fOuterThresholdVar = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    "outer_threshold");

    const GrAlphaThresholdFragmentProcessor& atfp =
        args.fFp.cast<GrAlphaThresholdFragmentProcessor>();
    GrGLSLColorSpaceXformHelper colorSpaceHelper(uniformHandler, atfp.colorSpaceXform(),
                                                 &fColorSpaceXformVar);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
    SkString maskCoords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[1]);

    fragBuilder->codeAppendf("vec2 coord = %s;", coords2D.c_str());
    fragBuilder->codeAppendf("vec2 mask_coord = %s;", maskCoords2D.c_str());
    fragBuilder->codeAppend("vec4 input_color = ");
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], "coord", kVec2f_GrSLType,
                                     &colorSpaceHelper);
    fragBuilder->codeAppend(";");
    fragBuilder->codeAppend("vec4 mask_color = ");
    fragBuilder->appendTextureLookup(args.fTexSamplers[1], "mask_coord");
    fragBuilder->codeAppend(";");

    fragBuilder->codeAppendf("float inner_thresh = %s;",
                             uniformHandler->getUniformCStr(fInnerThresholdVar));
    fragBuilder->codeAppendf("float outer_thresh = %s;",
                             uniformHandler->getUniformCStr(fOuterThresholdVar));
    fragBuilder->codeAppend("float mask = mask_color.a;");

    fragBuilder->codeAppend("vec4 color = input_color;");
    fragBuilder->codeAppend("if (mask < 0.5) {"
                            "if (color.a > outer_thresh) {"
                            "float scale = outer_thresh / color.a;"
                            "color.rgb *= scale;"
                            "color.a = outer_thresh;"
                            "}"
                            "} else if (color.a < inner_thresh) {"
                            "float scale = inner_thresh / max(0.001, color.a);"
                            "color.rgb *= scale;"
                            "color.a = inner_thresh;"
                            "}");

    fragBuilder->codeAppendf("%s = %s;", args.fOutputColor,
                             (GrGLSLExpr4(args.fInputColor) * GrGLSLExpr4("color")).c_str());
}

void GrGLAlphaThresholdFragmentProcessor::onSetData(const GrGLSLProgramDataManager& pdman,
                                                    const GrProcessor& proc) {
    const GrAlphaThresholdFragmentProcessor& atfp = proc.cast<GrAlphaThresholdFragmentProcessor>();
    pdman.set1f(fInnerThresholdVar, atfp.innerThreshold());
    pdman.set1f(fOuterThresholdVar, atfp.outerThreshold());
    if (SkToBool(atfp.colorSpaceXform())) {
        pdman.setSkMatrix44(fColorSpaceXformVar, atfp.colorSpaceXform()->srcToDst());
    }
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrAlphaThresholdFragmentProcessor);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrAlphaThresholdFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    GrTexture* bmpTex = d->fTextures[GrProcessorUnitTest::kSkiaPMTextureIdx];
    GrTexture* maskTex = d->fTextures[GrProcessorUnitTest::kAlphaTextureIdx];
    // Make the inner and outer thresholds be in (0, 1) exclusive and be sorted correctly.
    float innerThresh = d->fRandom->nextUScalar1() * .99f + 0.005f;
    float outerThresh = d->fRandom->nextUScalar1() * .99f + 0.005f;
    const int kMaxWidth = 1000;
    const int kMaxHeight = 1000;
    uint32_t width = d->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = d->fRandom->nextULessThan(kMaxHeight);
    uint32_t x = d->fRandom->nextULessThan(kMaxWidth - width);
    uint32_t y = d->fRandom->nextULessThan(kMaxHeight - height);
    SkIRect bounds = SkIRect::MakeXYWH(x, y, width, height);
    auto colorSpaceXform = GrTest::TestColorXform(d->fRandom);
    return GrAlphaThresholdFragmentProcessor::Make(bmpTex, colorSpaceXform, maskTex,
                                                   innerThresh, outerThresh,
                                                   bounds);
}
#endif

///////////////////////////////////////////////////////////////////////////////

void GrAlphaThresholdFragmentProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                              GrProcessorKeyBuilder* b) const {
    GrGLAlphaThresholdFragmentProcessor::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrAlphaThresholdFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLAlphaThresholdFragmentProcessor;
}

#endif
