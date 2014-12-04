/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCustomCoordsTextureEffect.h"
#include "GrInvariantOutput.h"
#include "GrTexture.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

class GrGLCustomCoordsTextureEffect : public GrGLGeometryProcessor {
public:
    GrGLCustomCoordsTextureEffect(const GrGeometryProcessor&,
                                  const GrBatchTracker&) {}

    virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
        const GrCustomCoordsTextureEffect& cte =
                args.fGP.cast<GrCustomCoordsTextureEffect>();

        GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

        GrGLVertToFrag v(kVec2f_GrSLType);
        args.fPB->addVarying("TextureCoords", &v);
        vsBuilder->codeAppendf("%s = %s;", v.vsOut(), cte.inTextureCoords()->fName);

        if (cte.inColor()) {
            args.fPB->addPassThroughAttribute(cte.inColor(), args.fOutputColor);
        }

        // setup output coords
        vsBuilder->codeAppendf("%s = %s;", vsBuilder->positionCoords(), cte.inPosition()->fName);
        vsBuilder->codeAppendf("%s = %s;", vsBuilder->localCoords(), cte.inPosition()->fName);

        // setup position varying
        vsBuilder->codeAppendf("%s = %s * vec3(%s, 1);", vsBuilder->glPosition(),
                               vsBuilder->uViewM(), cte.inPosition()->fName);

        GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("%s = ", args.fOutputCoverage);
        fsBuilder->appendTextureLookup(args.fSamplers[0], v.fsIn(), kVec2f_GrSLType);
        fsBuilder->codeAppend(";");
    }

    virtual void setData(const GrGLProgramDataManager&,
                         const GrGeometryProcessor&,
                         const GrBatchTracker&) SK_OVERRIDE {}

    static inline void GenKey(const GrGeometryProcessor& proc,
                              const GrBatchTracker&,
                              const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrCustomCoordsTextureEffect& gp = proc.cast<GrCustomCoordsTextureEffect>();

        b->add32(SkToBool(gp.inColor()));
    }


private:
    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrCustomCoordsTextureEffect::GrCustomCoordsTextureEffect(GrTexture* texture,
                                                         const GrTextureParams& params,
                                                         bool hasColor)
    : fTextureAccess(texture, params), fInColor(NULL) {
    this->initClassID<GrCustomCoordsTextureEffect>();
    fInPosition = &this->addVertexAttrib(GrAttribute("inPosition", kVec2f_GrVertexAttribType));
    if (hasColor) {
        fInColor = &this->addVertexAttrib(GrAttribute("inColor", kVec4ub_GrVertexAttribType));
        this->setHasVertexColor();
    }
    fInTextureCoords = &this->addVertexAttrib(GrAttribute("inTextureCoords",
                                                          kVec2f_GrVertexAttribType));
    this->addTextureAccess(&fTextureAccess);
}

bool GrCustomCoordsTextureEffect::onIsEqual(const GrGeometryProcessor& other) const {
    const GrCustomCoordsTextureEffect& gp = other.cast<GrCustomCoordsTextureEffect>();
    return SkToBool(this->inColor()) == SkToBool(gp.inColor());
}

void GrCustomCoordsTextureEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (GrPixelConfigIsAlphaOnly(this->texture(0)->config())) {
        inout->mulByUnknownAlpha();
    } else if (GrPixelConfigIsOpaque(this->texture(0)->config())) {
        inout->mulByUnknownOpaqueColor();
    } else {
        inout->mulByUnknownColor();
    }
}

void GrCustomCoordsTextureEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                                    const GrGLCaps& caps,
                                                    GrProcessorKeyBuilder* b) const {
    GrGLCustomCoordsTextureEffect::GenKey(*this, bt, caps, b);
}

GrGLGeometryProcessor*
GrCustomCoordsTextureEffect::createGLInstance(const GrBatchTracker& bt) const {
    return SkNEW_ARGS(GrGLCustomCoordsTextureEffect, (*this, bt));
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

    return GrCustomCoordsTextureEffect::Create(textures[texIdx], params, random->nextBool());
}
