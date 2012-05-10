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

    GrGLConvolutionEffect(GrConvolutionEffect* data);
    virtual const char* name() const SK_OVERRIDE;
    virtual void setupVSUnis(VarArray* vsUnis, int stage) SK_OVERRIDE;
    virtual void setupFSUnis(VarArray* fsUnis, int stage) SK_OVERRIDE;
    virtual void emitVS(GrStringBuilder* code,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrStringBuilder* code,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName,
                        const char* sampleCoords) SK_OVERRIDE;
    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;

    virtual void setData(const GrGLInterface*, GrCustomStage*,
                         const GrGLTexture*) SK_OVERRIDE;

protected:

    GrConvolutionEffect* fData;
 
    GrGLShaderVar* fKernelVar;
    GrGLShaderVar* fImageIncrementVar;
 
    GrGLint fKernelLocation;
    GrGLint fImageIncrementLocation;

private:

    typedef GrGLProgramStage INHERITED;
};

GrGLConvolutionEffect::GrGLConvolutionEffect(GrConvolutionEffect* data)
    : fData(data)
    , fKernelVar(NULL)
    , fImageIncrementVar(NULL)
    , fKernelLocation(0)
    , fImageIncrementLocation(0)
{

}

const char* GrGLConvolutionEffect::name() const SK_OVERRIDE {
    return fData->name();
}

void GrGLConvolutionEffect::setupVSUnis(VarArray* vsUnis,
                                        int stage) SK_OVERRIDE {
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
                                        int stage) SK_OVERRIDE {
    fKernelVar = &fsUnis->push_back();
    fKernelVar->setType(kFloat_GrSLType);
    fKernelVar->setTypeModifier(
        GrGLShaderVar::kUniform_TypeModifier);
    fKernelVar->setArrayCount(fData->fKernelWidth);
    (*fKernelVar->accessName()) = "uKernel";
    fKernelVar->accessName()->appendS32(stage);

    fKernelLocation = kUseUniform;

    // Image increment is used in both vertex & fragment shaders.
    fsUnis->push_back(*fImageIncrementVar).setEmitPrecision(false);
}

void GrGLConvolutionEffect::emitVS(GrStringBuilder* code,
                        const char* vertexCoords) SK_OVERRIDE {
    float scale = (fData->fKernelWidth - 1) * 0.5f;
    code->appendf("\t\t%s -= vec2(%g, %g) * %s;\n",
                  vertexCoords, scale, scale,
                  fImageIncrementVar->getName().c_str());

}

void GrGLConvolutionEffect::emitFS(GrStringBuilder* code,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName,
                        const char* sampleCoords) SK_OVERRIDE {
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
    code->appendf("\t\tvec2 coord = %s;\n", sampleCoords);
    code->appendf("\t\tfor (int i = 0; i < %d; i++) {\n",
                  fData->fKernelWidth);

    code->appendf("\t\t\tsum += ");
    this->emitTextureLookup(code, samplerName, "coord");
    code->appendf(" * %s;\n", kernelIndex.c_str());

    code->appendf("\t\t\tcoord += %s;\n",
                  fImageIncrementVar->getName().c_str());
    code->appendf("\t\t}\n");
    code->appendf("\t\t%s = sum%s;\n", outputColor, modulate.c_str());
}

void GrGLConvolutionEffect::initUniforms(const GrGLInterface* gl,
                                         int programID) SK_OVERRIDE {
    GR_GL_CALL_RET(gl, fKernelLocation,
        GetUniformLocation(programID, fKernelVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fImageIncrementLocation,
        GetUniformLocation(programID,
            fImageIncrementVar->getName().c_str()));
}

void GrGLConvolutionEffect::setData(const GrGLInterface* gl,
                                    GrCustomStage* data,
                                    const GrGLTexture* texture) SK_OVERRIDE {
    fData = static_cast<GrConvolutionEffect*>(data);
    GR_GL_CALL(gl, Uniform1fv(fKernelLocation,
                              fData->fKernelWidth,
                              fData->fKernel));
    float imageIncrement[2] = { 0 };
    switch (fData->fDirection) {
        case GrSamplerState::kX_FilterDirection:
            imageIncrement[0] = 1.0f / texture->width();
            break;
        case GrSamplerState::kY_FilterDirection:
            imageIncrement[1] = 1.0f / texture->width();
            break;
        default:
            GrCrash("Unknown filter direction.");
    }
    GR_GL_CALL(gl, Uniform2fv(fImageIncrementLocation, 1, imageIncrement));
}

/////////////////////////////////////////////////////////////////////
// TODO: stageKey() and sEffectId are the only non-boilerplate in
// this class; we ought to be able to templatize?

class GrConvolutionEffectFactory : public GrProgramStageFactory {

public:

    virtual ~GrConvolutionEffectFactory();

    virtual uint16_t stageKey(const GrCustomStage* s) SK_OVERRIDE;
    virtual GrGLProgramStage* createGLInstance(GrCustomStage* s) SK_OVERRIDE;

    static GrConvolutionEffectFactory* getInstance();

protected:

    GrConvolutionEffectFactory();

    // TODO: find a more reliable installation than hand-coding
    // id values like '1'. 
    static const int sEffectId = 1;

private:

    typedef GrProgramStageFactory INHERITED;
};

GrConvolutionEffectFactory::~GrConvolutionEffectFactory() {

}

uint16_t GrConvolutionEffectFactory::stageKey(const GrCustomStage* s)
    SK_OVERRIDE {
    const GrConvolutionEffect* c =
        static_cast<const GrConvolutionEffect*>(s);
    GrAssert(c->width() < 256);
    return (sEffectId << 8) | (c->width() & 0xff);
}

GrGLProgramStage* GrConvolutionEffectFactory::createGLInstance(
    GrCustomStage* s) SK_OVERRIDE {
    return new GrGLConvolutionEffect(static_cast<GrConvolutionEffect*>(s));
}

GrConvolutionEffectFactory* GrConvolutionEffectFactory::getInstance() {
    static GrConvolutionEffectFactory* instance =
        new GrConvolutionEffectFactory;
    return instance;
}

GrConvolutionEffectFactory::GrConvolutionEffectFactory() {

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

const char* GrConvolutionEffect::name() const {
    return "Convolution";
}

GrProgramStageFactory* GrConvolutionEffect::getFactory() const {
    return GrConvolutionEffectFactory::getInstance();
}

bool GrConvolutionEffect::isEqual(const GrCustomStage * sBase) const {
    const GrConvolutionEffect* s =
        static_cast<const GrConvolutionEffect*>(sBase);

    return (fKernelWidth == s->fKernelWidth &&
            fDirection == s->fDirection &&
            0 == memcmp(fKernel, s->fKernel, fKernelWidth * sizeof(float)));
}



