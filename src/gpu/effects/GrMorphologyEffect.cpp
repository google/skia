/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMorphologyEffect.h"
#include "gl/GrGLProgramStage.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrProgramStageFactory.h"

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect  : public GrGLProgramStage {
public:
    GrGLMorphologyEffect (const GrProgramStageFactory& factory,
                          const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* builder,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage& s);

    virtual void initUniforms(const GrGLShaderBuilder*,
                              const GrGLInterface*,
                              int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface*,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

private:
    int width() const { return GrMorphologyEffect::WidthFromRadius(fRadius); }

    int                                fRadius;
    GrMorphologyEffect::MorphologyType fType;
    GrGLShaderBuilder::UniformHandle   fImageIncrementUni;
    GrGLint                            fImageIncrementLocation;

    typedef GrGLProgramStage INHERITED;
};

GrGLMorphologyEffect ::GrGLMorphologyEffect(const GrProgramStageFactory& factory,
                                            const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fImageIncrementUni(GrGLShaderBuilder::kInvalidUniformHandle)
    , fImageIncrementLocation(0) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(stage);
    fRadius = m.radius();
    fType = m.type();
}

void GrGLMorphologyEffect::setupVariables(GrGLShaderBuilder* builder, int stage) {
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType |
                                             GrGLShaderBuilder::kVertex_ShaderType,
                                             kVec2f_GrSLType, "uImageIncrement", stage);
}

void GrGLMorphologyEffect::emitVS(GrGLShaderBuilder* builder,
                                  const char* vertexCoords) {
    SkString* code = &builder->fVSCode;
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    code->appendf("\t\t%s -= vec2(%d, %d) * %s;\n", vertexCoords, fRadius, fRadius, imgInc);
}

void GrGLMorphologyEffect::initUniforms(const GrGLShaderBuilder* builder,
                                        const GrGLInterface* gl,
                                        int programID) {
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    GR_GL_CALL_RET(gl, fImageIncrementLocation, GetUniformLocation(programID, imgInc));
}

void GrGLMorphologyEffect ::emitFS(GrGLShaderBuilder* builder,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const char* samplerName) {
    SkString* code = &builder->fFSCode;

    const char* func;
    switch (fType) {
        case GrContext::kErode_MorphologyType:
            code->appendf("\t\tvec4 value = vec4(1, 1, 1, 1);\n");
            func = "min";
            break;
        case GrContext::kDilate_MorphologyType:
            code->appendf("\t\tvec4 value = vec4(0, 0, 0, 0);\n");
            func = "max";
            break;
        default:
            GrCrash("Unexpected type");
            func = ""; // suppress warning
            break;
    }
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    code->appendf("\t\tvec2 coord = %s;\n", builder->fSampleCoords.c_str());
    code->appendf("\t\tfor (int i = 0; i < %d; i++) {\n", this->width());
    code->appendf("\t\t\tvalue = %s(value, ", func);
    builder->emitTextureLookup(samplerName, "coord");
    code->appendf(");\n");
    code->appendf("\t\t\tcoord += %s;\n", imgInc);
    code->appendf("\t\t}\n");
    code->appendf("\t\t%s = value%s;\n", outputColor, builder->fModulate.c_str());
}

GrGLProgramStage::StageKey GrGLMorphologyEffect::GenKey(
                                                    const GrCustomStage& s) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(s);
    StageKey key = static_cast<StageKey>(m.radius());
    key |= (m.type() << 8);
    return key;
}

void GrGLMorphologyEffect ::setData(const GrGLInterface* gl,
                                    const GrCustomStage& data,
                                    const GrRenderTarget*,
                                    int stageNum) {
    const Gr1DKernelEffect& kern =
        static_cast<const Gr1DKernelEffect&>(data);
    GrGLTexture& texture =
        *static_cast<GrGLTexture*>(data.texture(0));
    // the code we generated was for a specific kernel radius
    GrAssert(kern.radius() == fRadius);
    float imageIncrement[2] = { 0 };
    switch (kern.direction()) {
        case Gr1DKernelEffect::kX_Direction:
            imageIncrement[0] = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            imageIncrement[1] = 1.0f / texture.height();
            break;
        default:
            GrCrash("Unknown filter direction.");
    }
    GR_GL_CALL(gl, Uniform2fv(fImageIncrementLocation, 1, imageIncrement));
}

///////////////////////////////////////////////////////////////////////////////

GrMorphologyEffect::GrMorphologyEffect(GrTexture* texture,
                                       Direction direction,
                                       int radius,
                                       MorphologyType type)
    : Gr1DKernelEffect(texture, direction, radius)
    , fType(type) {
}

GrMorphologyEffect::~GrMorphologyEffect() {
}

const GrProgramStageFactory& GrMorphologyEffect::getFactory() const {
    return GrTProgramStageFactory<GrMorphologyEffect>::getInstance();
}

bool GrMorphologyEffect::isEqual(const GrCustomStage& sBase) const {
    const GrMorphologyEffect& s =
        static_cast<const GrMorphologyEffect&>(sBase);
    return (INHERITED::isEqual(sBase) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->type() == s.type());
}
