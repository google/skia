/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCustomCoordsTextureEffect.h"
#include "gl/builders/GrGLFullProgramBuilder.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLGeometryProcessor.h"
#include "GrTBackendProcessorFactory.h"
#include "GrTexture.h"

class GrGLCustomCoordsTextureEffect : public GrGLGeometryProcessor {
public:
    GrGLCustomCoordsTextureEffect(const GrBackendProcessorFactory& factory, const GrProcessor&)
        : INHERITED (factory) {}

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const GrCustomCoordsTextureEffect& customCoordsTextureEffect =
                geometryProcessor.cast<GrCustomCoordsTextureEffect>();
        SkASSERT(1 == customCoordsTextureEffect.getVertexAttribs().count());

        SkString fsCoordName;
        const char* vsVaryingName;
        const char* fsVaryingNamePtr;
        builder->addVarying(kVec2f_GrSLType, "textureCoords", &vsVaryingName, &fsVaryingNamePtr);
        fsCoordName = fsVaryingNamePtr;

        GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
        const GrShaderVar& inTextureCoords = customCoordsTextureEffect.inTextureCoords();
        vsBuilder->codeAppendf("\t%s = %s;\n", vsVaryingName, inTextureCoords.c_str());

        GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("\t%s = ", outputColor);
        fsBuilder->appendTextureLookupAndModulate(inputColor,
                                                  samplers[0],
                                                  fsCoordName.c_str(),
                                                  kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
    }

    virtual void setData(const GrGLProgramDataManager&,
                         const GrProcessor&) SK_OVERRIDE {}

private:
    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrCustomCoordsTextureEffect::GrCustomCoordsTextureEffect(GrTexture* texture,
                                                         const GrTextureParams& params)
    : fTextureAccess(texture, params)
    , fInTextureCoords(this->addVertexAttrib(GrShaderVar("inTextureCoords",
                                                         kVec2f_GrSLType,
                                                         GrShaderVar::kAttribute_TypeModifier))) {
    this->addTextureAccess(&fTextureAccess);
}

bool GrCustomCoordsTextureEffect::onIsEqual(const GrProcessor& other) const {
    const GrCustomCoordsTextureEffect& cte = other.cast<GrCustomCoordsTextureEffect>();
    return fTextureAccess == cte.fTextureAccess;
}

void GrCustomCoordsTextureEffect::getConstantColorComponents(GrColor* color,
                                                             uint32_t* validFlags) const {
    if ((*validFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(*color) &&
        GrPixelConfigIsOpaque(this->texture(0)->config())) {
        *validFlags = kA_GrColorComponentFlag;
    } else {
        *validFlags = 0;
    }
}

const GrBackendGeometryProcessorFactory& GrCustomCoordsTextureEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<GrCustomCoordsTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrCustomCoordsTextureEffect);

GrGeometryProcessor* GrCustomCoordsTextureEffect::TestCreate(SkRandom* random,
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

    return GrCustomCoordsTextureEffect::Create(textures[texIdx], params);
}
