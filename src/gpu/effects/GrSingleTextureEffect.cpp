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

// For brevity, and these definitions are likely to move to a different class soon.
typedef GrGLShaderBuilder::UniformHandle UniformHandle;
static const UniformHandle kInvalidUniformHandle = GrGLShaderBuilder::kInvalidUniformHandle;

class GrGLSingleTextureEffect : public GrGLProgramStage {
public:
    GrGLSingleTextureEffect(const GrProgramStageFactory& factory,
                            const GrCustomStage& stage) : INHERITED (factory) { }

    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE {
        builder->emitDefaultFetch(outputColor, samplerName);
    }

    static inline StageKey GenKey(const GrCustomStage&) { return 0; }

private:

    typedef GrGLProgramStage INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture)
    : fTexture (texture) {
    SkSafeRef(fTexture);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
    SkSafeUnref(fTexture);
}

unsigned int GrSingleTextureEffect::numTextures() const {
    return 1;
}

GrTexture* GrSingleTextureEffect::texture(unsigned int index) const {
    GrAssert(0 == index);
    return fTexture;
}

const GrProgramStageFactory& GrSingleTextureEffect::getFactory() const {
    return GrTProgramStageFactory<GrSingleTextureEffect>::getInstance();
}


