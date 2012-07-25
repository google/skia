/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomainEffect.h"
#include "gl/GrGLProgramStage.h"
#include "GrProgramStageFactory.h"

class GrGLTextureDomainEffect : public GrGLProgramStage {
public:
    GrGLTextureDomainEffect(const GrProgramStageFactory& factory,
                            const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* builder) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage&) { return 0; }

private:
    GrGLUniformManager::UniformHandle fNameUni;

    typedef GrGLProgramStage INHERITED;
};

GrGLTextureDomainEffect::GrGLTextureDomainEffect(const GrProgramStageFactory& factory,
                                                 const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fNameUni(GrGLUniformManager::kInvalidUniformHandle) {
}

void GrGLTextureDomainEffect::setupVariables(GrGLShaderBuilder* builder) {
    fNameUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                   kVec4f_GrSLType, "TexDom");
};

void GrGLTextureDomainEffect::emitFS(GrGLShaderBuilder* builder,
                                     const char* outputColor,
                                     const char* inputColor,
                                     const char* samplerName) {
    SkString coordVar("clampCoord");
    builder->fFSCode.appendf("\t%s %s = clamp(%s, %s.xy, %s.zw);\n",
                           GrGLShaderVar::TypeString(GrSLFloatVectorType(builder->fCoordDims)),
                           coordVar.c_str(),
                           builder->fSampleCoords.c_str(),
                           builder->getUniformCStr(fNameUni),
                           builder->getUniformCStr(fNameUni));
    builder->fSampleCoords = coordVar;

    builder->emitDefaultFetch(outputColor, samplerName);
}

void GrGLTextureDomainEffect::setData(const GrGLUniformManager& uman,
                                      const GrCustomStage& data,
                                      const GrRenderTarget*,
                                      int stageNum) {
    const GrTextureDomainEffect& effect = static_cast<const GrTextureDomainEffect&>(data);
    const GrRect& domain = effect.domain();

    float values[4] = {
        GrScalarToFloat(domain.left()),
        GrScalarToFloat(domain.top()),
        GrScalarToFloat(domain.right()),
        GrScalarToFloat(domain.bottom())
    };
    // vertical flip if necessary
    const GrGLTexture* texture = static_cast<const GrGLTexture*>(effect.texture(0));
    if (GrGLTexture::kBottomUp_Orientation == texture->orientation()) {
        values[1] = 1.0f - values[1];
        values[3] = 1.0f - values[3];
        // The top and bottom were just flipped, so correct the ordering
        // of elements so that values = (l, t, r, b).
        SkTSwap(values[1], values[3]);
    }
    uman.set4fv(fNameUni, 0, 1, values);
}


///////////////////////////////////////////////////////////////////////////////

GrTextureDomainEffect::GrTextureDomainEffect(GrTexture* texture, GrRect domain)
    : GrSingleTextureEffect(texture)
    , fTextureDomain(domain) {
}

GrTextureDomainEffect::~GrTextureDomainEffect() {

}

const GrProgramStageFactory& GrTextureDomainEffect::getFactory() const {
    return GrTProgramStageFactory<GrTextureDomainEffect>::getInstance();
}

bool GrTextureDomainEffect::isEqual(const GrCustomStage& sBase) const {
    const GrTextureDomainEffect& s = static_cast<const GrTextureDomainEffect&>(sBase);
    return (INHERITED::isEqual(sBase) && this->fTextureDomain == s.fTextureDomain);
}


