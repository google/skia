/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomainEffect.h"
#include "gl/GrGLProgramStage.h"
#include "GrProgramStageFactory.h"

// For brevity, and these definitions are likely to move to a different class soon.
typedef GrGLShaderBuilder::UniformHandle UniformHandle;
static const UniformHandle kInvalidUniformHandle = GrGLShaderBuilder::kInvalidUniformHandle;

class GrGLTextureDomainEffect : public GrGLProgramStage {
public:
    GrGLTextureDomainEffect(const GrProgramStageFactory& factory,
                            const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* builder,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    virtual void initUniforms(const GrGLShaderBuilder* builder,
                              const GrGLInterface*,
                              int programID) SK_OVERRIDE;

    virtual void setData(const GrGLInterface*,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage&) { return 0; }

private:
    UniformHandle fNameUni;
    int           fNameLocation;

    typedef GrGLProgramStage INHERITED;
};

GrGLTextureDomainEffect::GrGLTextureDomainEffect(const GrProgramStageFactory& factory,
                                                 const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fNameUni(kInvalidUniformHandle)
    , fNameLocation(0) {
}

void GrGLTextureDomainEffect::setupVariables(GrGLShaderBuilder* builder,
                                             int stage) {
    fNameUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                   kVec4f_GrSLType, "uTexDom", stage);
    fNameLocation = kUseUniform;
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

void GrGLTextureDomainEffect::initUniforms(const GrGLShaderBuilder* builder,
                                           const GrGLInterface* gl, int programID) {
    GR_GL_CALL_RET(gl, fNameLocation,
                   GetUniformLocation(programID, builder->getUniformCStr(fNameUni)));
    GrAssert(kUnusedUniform != fNameLocation);
}

void GrGLTextureDomainEffect::setData(const GrGLInterface* gl,
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

    GR_GL_CALL(gl, Uniform4fv(fNameLocation, 1, values));
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


