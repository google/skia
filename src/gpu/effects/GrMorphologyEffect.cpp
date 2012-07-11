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

    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage& s);

    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface*, 
                         const GrGLTexture&,
                         const GrCustomStage&,
                         int stageNum) SK_OVERRIDE;

private:
    int width() const { return GrMorphologyEffect::WidthFromRadius(fRadius); }

    int                                fRadius;
    GrMorphologyEffect::MorphologyType fType;
    const GrGLShaderVar*               fImageIncrementVar;
    GrGLint                            fImageIncrementLocation;

    typedef GrGLProgramStage INHERITED;
};

GrGLMorphologyEffect ::GrGLMorphologyEffect(const GrProgramStageFactory& factory,
                                            const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fImageIncrementVar(NULL)
    , fImageIncrementLocation(0) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(stage);
    fRadius = m.radius();
    fType = m.type();
}

void GrGLMorphologyEffect::setupVariables(GrGLShaderBuilder* state, int stage) {
    fImageIncrementVar = &state->addUniform(GrGLShaderBuilder::kFragment_ShaderType |
                                            GrGLShaderBuilder::kVertex_ShaderType,
                                            kVec2f_GrSLType, "uImageIncrement", stage);
}

void GrGLMorphologyEffect::emitVS(GrGLShaderBuilder* state,
                                  const char* vertexCoords) {
    SkString* code = &state->fVSCode;
    code->appendf("\t\t%s -= vec2(%d, %d) * %s;\n",
                  vertexCoords, fRadius, fRadius,
                  fImageIncrementVar->getName().c_str());
}

void GrGLMorphologyEffect::initUniforms(const GrGLInterface* gl,
                                        int programID) {
    GR_GL_CALL_RET(gl, fImageIncrementLocation,
        GetUniformLocation(programID,
            fImageIncrementVar->getName().c_str()));
}

void GrGLMorphologyEffect ::emitFS(GrGLShaderBuilder* state,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const char* samplerName) {
    SkString* code = &state->fFSCode;
    // const char* texFunc = "texture2D";
    // bool complexCoord = false;

    const char* func;
    switch (fType) {
        case GrContext::kErode_MorphologyType:
            state->fFSCode.appendf("\t\tvec4 value = vec4(1, 1, 1, 1);\n");
            func = "min";
            break;
        case GrContext::kDilate_MorphologyType:
            state->fFSCode.appendf("\t\tvec4 value = vec4(0, 0, 0, 0);\n");
            func = "max";
            break;
        default:
            GrCrash("Unexpected type");
            func = ""; // suppress warning
            break;
    }

    code->appendf("\t\tvec2 coord = %s;\n", state->fSampleCoords.c_str());
    code->appendf("\t\tfor (int i = 0; i < %d; i++) {\n", this->width());
    state->fFSCode.appendf("\t\t\tvalue = %s(value, ", func);
    state->emitTextureLookup(samplerName, "coord");
    state->fFSCode.appendf(");\n");
    code->appendf("\t\t\tcoord += %s;\n",
                  fImageIncrementVar->getName().c_str());
    code->appendf("\t\t}\n");

    state->fFSCode.appendf("\t\t%s = value%s;\n",
                           outputColor,
                           state->fModulate.c_str());
}

GrGLProgramStage::StageKey GrGLMorphologyEffect::GenKey(
                                                    const GrCustomStage& s) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(s);
    StageKey key = static_cast<StageKey>(m.radius());
    key |= (m.type() << 8);
    return key;
}

void GrGLMorphologyEffect ::setData(const GrGLInterface* gl,
                                    const GrGLTexture& texture,
                                    const GrCustomStage& data,
                                    int stageNum) {
    const Gr1DKernelEffect& kern =
        static_cast<const Gr1DKernelEffect&>(data);
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

GrMorphologyEffect::GrMorphologyEffect(Direction direction,
                                       int radius,
                                       MorphologyType type)
    : Gr1DKernelEffect(direction, radius)
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
    return (this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->type() == s.type());
}
