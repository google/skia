/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvolutionEffect.h"
#include "gl/GrGLProgramStage.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrProgramStageFactory.h"

/////////////////////////////////////////////////////////////////////

class GrGLConvolutionEffect : public GrGLProgramStage {

public:

    GrGLConvolutionEffect(const GrProgramStageFactory& factory,
                          const GrCustomStage* stage);
    virtual void setupVSUnis(VarArray* vsUnis, int stage) SK_OVERRIDE;
    virtual void setupFSUnis(VarArray* fsUnis, int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;
    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;

    virtual void setData(const GrGLInterface*, 
                         const GrGLTexture&,
                         GrCustomStage*,
                         int stageNum) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage* s);
    
protected:

    unsigned int   fKernelWidth;
    GrGLShaderVar* fKernelVar;
    GrGLShaderVar* fImageIncrementVar;
 
    GrGLint fKernelLocation;
    GrGLint fImageIncrementLocation;

private:

    typedef GrGLProgramStage INHERITED;
};

GrGLConvolutionEffect::GrGLConvolutionEffect(
                                    const GrProgramStageFactory& factory,
                                    const GrCustomStage* data)
    : GrGLProgramStage(factory)
    , fKernelVar(NULL)
    , fImageIncrementVar(NULL)
    , fKernelLocation(0)
    , fImageIncrementLocation(0) {
    fKernelWidth = static_cast<const GrConvolutionEffect*>(data)->width();
}

void GrGLConvolutionEffect::setupVSUnis(VarArray* vsUnis,
                                        int stage) {
    fImageIncrementVar = &vsUnis->push_back();
    fImageIncrementVar->setType(kVec2f_GrSLType);
    fImageIncrementVar->setTypeModifier(
        GrGLShaderVar::kUniform_TypeModifier);
    (*fImageIncrementVar->accessName()) = "uImageIncrement";
    fImageIncrementVar->accessName()->appendS32(stage);
    fImageIncrementVar->setEmitPrecision(true);

    fImageIncrementLocation = kUseUniform;
}

void GrGLConvolutionEffect::setupFSUnis(VarArray* fsUnis,
                                        int stage) {
    fKernelVar = &fsUnis->push_back();
    fKernelVar->setType(kFloat_GrSLType);
    fKernelVar->setTypeModifier(
        GrGLShaderVar::kUniform_TypeModifier);
    fKernelVar->setArrayCount(fKernelWidth);
    (*fKernelVar->accessName()) = "uKernel";
    fKernelVar->accessName()->appendS32(stage);

    fKernelLocation = kUseUniform;

    // Image increment is used in both vertex & fragment shaders.
    fsUnis->push_back(*fImageIncrementVar).setEmitPrecision(false);
}

void GrGLConvolutionEffect::emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) {
    GrStringBuilder* code = &state->fVSCode;
    float scale = (fKernelWidth - 1) * 0.5f;
    code->appendf("\t\t%s -= vec2(%g, %g) * %s;\n",
                  vertexCoords, scale, scale,
                  fImageIncrementVar->getName().c_str());

}

void GrGLConvolutionEffect::emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) {
    GrStringBuilder* code = &state->fFSCode;
    const char* texFunc = "texture2D";
    bool complexCoord = false;

    GrStringBuilder modulate;
    if (NULL != inputColor) {
        modulate.printf(" * %s", inputColor);
    }

    // Creates the string "kernel[i]" with workarounds for
    // possible driver bugs
    GrStringBuilder kernelIndex;
    fKernelVar->appendArrayAccess("i", &kernelIndex);

    code->appendf("\t\tvec4 sum = vec4(0, 0, 0, 0);\n");
    code->appendf("\t\tvec2 coord = %s;\n", state->fSampleCoords.c_str());
    code->appendf("\t\tfor (int i = 0; i < %d; i++) {\n",
                  fKernelWidth);

    code->appendf("\t\t\tsum += ");
    this->emitTextureLookup(code, samplerName, "coord");
    code->appendf(" * %s;\n", kernelIndex.c_str());

    code->appendf("\t\t\tcoord += %s;\n",
                  fImageIncrementVar->getName().c_str());
    code->appendf("\t\t}\n");
    code->appendf("\t\t%s = sum%s;\n", outputColor, modulate.c_str());
}

void GrGLConvolutionEffect::initUniforms(const GrGLInterface* gl,
                                         int programID) {
    GR_GL_CALL_RET(gl, fKernelLocation,
        GetUniformLocation(programID, fKernelVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fImageIncrementLocation,
        GetUniformLocation(programID,
            fImageIncrementVar->getName().c_str()));
}

void GrGLConvolutionEffect::setData(const GrGLInterface* gl,
                                    const GrGLTexture& texture,
                                    GrCustomStage* data,
                                    int stageNum) {
    const GrConvolutionEffect* conv =
        static_cast<const GrConvolutionEffect*>(data);
    // the code we generated was for a specific kernel width
    GrAssert(conv->width() == fKernelWidth);
    GR_GL_CALL(gl, Uniform1fv(fKernelLocation,
                              fKernelWidth,
                              conv->kernel()));
    float imageIncrement[2] = { 0 };
    switch (conv->direction()) {
        case GrSamplerState::kX_FilterDirection:
            imageIncrement[0] = 1.0f / texture.width();
            break;
        case GrSamplerState::kY_FilterDirection:
            imageIncrement[1] = 1.0f / texture.width();
            break;
        default:
            GrCrash("Unknown filter direction.");
    }
    GR_GL_CALL(gl, Uniform2fv(fImageIncrementLocation, 1, imageIncrement));
}

GrGLProgramStage::StageKey GrGLConvolutionEffect::GenKey(
                                                    const GrCustomStage* s) {
    return static_cast<const GrConvolutionEffect*>(s)->width();
}

/////////////////////////////////////////////////////////////////////

GrConvolutionEffect::GrConvolutionEffect(
        GrSamplerState::FilterDirection direction,
        unsigned int kernelWidth,
        const float* kernel)
    : fDirection (direction)
    , fKernelWidth (kernelWidth) {
    GrAssert(kernelWidth <= MAX_KERNEL_WIDTH);
    for (unsigned int i = 0; i < kernelWidth; i++) {
        fKernel[i] = kernel[i];
    }
}

GrConvolutionEffect::~GrConvolutionEffect() {

}

const GrProgramStageFactory& GrConvolutionEffect::getFactory() const {
    return GrTProgramStageFactory<GrConvolutionEffect>::getInstance();
}

bool GrConvolutionEffect::isEqual(const GrCustomStage * sBase) const {
    const GrConvolutionEffect* s =
        static_cast<const GrConvolutionEffect*>(sBase);

    return (fKernelWidth == s->fKernelWidth &&
            fDirection == s->fDirection &&
            0 == memcmp(fKernel, s->fKernel, fKernelWidth * sizeof(float)));
}



