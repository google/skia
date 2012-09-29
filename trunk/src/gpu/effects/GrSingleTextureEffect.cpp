/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"
#include "gl/GrGLProgramStage.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrProgramStageFactory.h"
#include "GrTexture.h"

class GrGLSingleTextureEffect : public GrGLProgramStage {
public:
    GrGLSingleTextureEffect(const GrProgramStageFactory& factory,
                            const GrCustomStage& stage) : INHERITED (factory) { }

    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray& samplers) SK_OVERRIDE {
        builder->fFSCode.appendf("\t%s = ", outputColor);
        builder->appendTextureLookupAndModulate(&builder->fFSCode, inputColor, samplers[0]);
        builder->fFSCode.append(";\n");
    }

    static inline StageKey GenKey(const GrCustomStage&, const GrGLCaps&) { return 0; }

private:

    typedef GrGLProgramStage INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture)
    : fTextureAccess(texture) {
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}

int GrSingleTextureEffect::numTextures() const {
    return 1;
}

const GrTextureAccess& GrSingleTextureEffect::textureAccess(int index) const {
    GrAssert(0 == index);
    return fTextureAccess;
}

const GrProgramStageFactory& GrSingleTextureEffect::getFactory() const {
    return GrTProgramStageFactory<GrSingleTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_CUSTOM_STAGE_TEST(GrSingleTextureEffect);

GrCustomStage* GrSingleTextureEffect::TestCreate(SkRandom* random,
                                                 GrContext* context,
                                                 GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrCustomStageUnitTest::kSkiaPMTextureIdx :
                                      GrCustomStageUnitTest::kAlphaTextureIdx;
    return SkNEW_ARGS(GrSingleTextureEffect, (textures[texIdx]));
}
