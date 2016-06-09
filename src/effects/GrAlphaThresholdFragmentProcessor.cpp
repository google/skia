/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAlphaThresholdFragmentProcessor.h"

#if SK_SUPPORT_GPU

#include "GrInvariantOutput.h"
#include "GrTextureAccess.h"
#include "SkRefCnt.h"

#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"

sk_sp<GrFragmentProcessor> GrAlphaThresholdFragmentProcessor::Make(GrTexture* texture,
                                                                   GrTexture* maskTexture,
                                                                   float innerThreshold,
                                                                   float outerThreshold,
                                                                   const SkIRect& bounds) {
    return sk_sp<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(
                                                    texture, maskTexture,
                                                    innerThreshold, outerThreshold,
                                                    bounds));
}

static SkMatrix make_div_and_translate_matrix(GrTexture* texture, int x, int y) {
    SkMatrix matrix = GrCoordTransform::MakeDivByTextureWHMatrix(texture);
    matrix.preTranslate(SkIntToScalar(x), SkIntToScalar(y));
    return matrix;
}

GrAlphaThresholdFragmentProcessor::GrAlphaThresholdFragmentProcessor(GrTexture* texture,
                                                                     GrTexture* maskTexture,
                                                                     float innerThreshold,
                                                                     float outerThreshold,
                                                                     const SkIRect& bounds)
    : fInnerThreshold(innerThreshold)
    , fOuterThreshold(outerThreshold)
    , fImageCoordTransform(kLocal_GrCoordSet,
                           GrCoordTransform::MakeDivByTextureWHMatrix(texture), texture,
                           GrTextureParams::kNone_FilterMode)
    , fImageTextureAccess(texture)
    , fMaskCoordTransform(kLocal_GrCoordSet,
                          make_div_and_translate_matrix(maskTexture, -bounds.x(), -bounds.y()),
                          maskTexture,
                          GrTextureParams::kNone_FilterMode)
    , fMaskTextureAccess(maskTexture) {
    this->initClassID<GrAlphaThresholdFragmentProcessor>();
    this->addCoordTransform(&fImageCoordTransform);
    this->addTextureAccess(&fImageTextureAccess);
    this->addCoordTransform(&fMaskCoordTransform);
    this->addTextureAccess(&fMaskTextureAccess);
}

bool GrAlphaThresholdFragmentProcessor::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrAlphaThresholdFragmentProcessor& s = sBase.cast<GrAlphaThresholdFragmentProcessor>();
    return (this->fInnerThreshold == s.fInnerThreshold &&
            this->fOuterThreshold == s.fOuterThreshold);
}

void GrAlphaThresholdFragmentProcessor::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (GrPixelConfigIsAlphaOnly(this->texture(0)->config())) {
        inout->mulByUnknownSingleComponent();
    } else if (GrPixelConfigIsOpaque(this->texture(0)->config()) && fOuterThreshold >= 1.f) {
        inout->mulByUnknownOpaqueFourComponents();
    } else {
        inout->mulByUnknownFourComponents();
    }
}

///////////////////////////////////////////////////////////////////////////////

class GrGLAlphaThresholdFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    GrGLSLProgramDataManager::UniformHandle fInnerThresholdVar;
    GrGLSLProgramDataManager::UniformHandle fOuterThresholdVar;

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

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);
    SkString maskCoords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 1);

    fragBuilder->codeAppendf("vec2 coord = %s;", coords2D.c_str());
    fragBuilder->codeAppendf("vec2 mask_coord = %s;", maskCoords2D.c_str());
    fragBuilder->codeAppend("vec4 input_color = ");
    fragBuilder->appendTextureLookup(args.fTexSamplers[0], "coord");
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
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrAlphaThresholdFragmentProcessor);

sk_sp<GrFragmentProcessor> GrAlphaThresholdFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    GrTexture* bmpTex = d->fTextures[GrProcessorUnitTest::kSkiaPMTextureIdx];
    GrTexture* maskTex = d->fTextures[GrProcessorUnitTest::kAlphaTextureIdx];
    float innerThresh = d->fRandom->nextUScalar1();
    float outerThresh = d->fRandom->nextUScalar1();
    const int kMaxWidth = 1000;
    const int kMaxHeight = 1000;
    uint32_t width = d->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = d->fRandom->nextULessThan(kMaxHeight);
    uint32_t x = d->fRandom->nextULessThan(kMaxWidth - width);
    uint32_t y = d->fRandom->nextULessThan(kMaxHeight - height);
    SkIRect bounds = SkIRect::MakeXYWH(x, y, width, height);
    return GrAlphaThresholdFragmentProcessor::Make(bmpTex, maskTex,
                                                   innerThresh, outerThresh,
                                                   bounds);
}

///////////////////////////////////////////////////////////////////////////////

void GrAlphaThresholdFragmentProcessor::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                              GrProcessorKeyBuilder* b) const {
    GrGLAlphaThresholdFragmentProcessor::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrAlphaThresholdFragmentProcessor::onCreateGLSLInstance() const {
    return new GrGLAlphaThresholdFragmentProcessor;
}

#endif
