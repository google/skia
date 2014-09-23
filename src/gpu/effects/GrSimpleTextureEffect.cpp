/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrSimpleTextureEffect.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendProcessorFactory.h"
#include "GrTexture.h"

class GrGLSimpleTextureEffect : public GrGLFragmentProcessor {
public:
    GrGLSimpleTextureEffect(const GrBackendProcessorFactory& factory, const GrProcessor&)
        : INHERITED (factory) {
    }

    virtual void emitCode(GrGLProgramBuilder* builder,
                          const GrFragmentProcessor& fp,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("\t%s = ", outputColor);
        fsBuilder->appendTextureLookupAndModulate(inputColor,
                                                  samplers[0],
                                                  coords[0].c_str(),
                                                  coords[0].getType());
        fsBuilder->codeAppend(";\n");
    }

private:
    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void GrSimpleTextureEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    this->updateConstantColorComponentsForModulation(color, validFlags);
}

const GrBackendFragmentProcessorFactory& GrSimpleTextureEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrSimpleTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSimpleTextureEffect);

GrFragmentProcessor* GrSimpleTextureEffect::TestCreate(SkRandom* random,
                                                       GrContext*,
                                                       const GrDrawTargetCaps&,
                                                       GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrTextureParams params(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                                                           GrTextureParams::kNone_FilterMode);

    static const GrCoordSet kCoordSets[] = {
        kLocal_GrCoordSet,
        kPosition_GrCoordSet
    };
    GrCoordSet coordSet = kCoordSets[random->nextULessThan(SK_ARRAY_COUNT(kCoordSets))];

    const SkMatrix& matrix = GrProcessorUnitTest::TestMatrix(random);
    return GrSimpleTextureEffect::Create(textures[texIdx], matrix, coordSet);
}
