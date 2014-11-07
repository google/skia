/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCustomCoordsTextureEffect.h"
#include "gl/builders/GrGLProgramBuilder.h"
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

    virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
        const GrCustomCoordsTextureEffect& customCoordsTextureEffect =
                args.fGP.cast<GrCustomCoordsTextureEffect>();
        SkASSERT(1 == customCoordsTextureEffect.getVertexAttribs().count());

        GrGLVertToFrag v(kVec2f_GrSLType);
        args.fPB->addVarying("TextureCoords", &v);

        GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
        const GrShaderVar& inTextureCoords = customCoordsTextureEffect.inTextureCoords();
        vsBuilder->codeAppendf("%s = %s;", v.vsOut(), inTextureCoords.c_str());

        GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("%s = ", args.fOutput);
        fsBuilder->appendTextureLookupAndModulate(args.fInput,
                                                  args.fSamplers[0],
                                                  v.fsIn(),
                                                  kVec2f_GrSLType);
        fsBuilder->codeAppend(";");
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

bool GrCustomCoordsTextureEffect::onIsEqual(const GrGeometryProcessor& other) const {
    return true;
}

void GrCustomCoordsTextureEffect::onComputeInvariantOutput(InvariantOutput* inout) const {
    if (GrPixelConfigIsOpaque(this->texture(0)->config())) {
        inout->mulByUnknownOpaqueColor();
    } else {
        inout->mulByUnknownColor();
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
