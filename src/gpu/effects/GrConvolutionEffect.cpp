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

class GrGLConvolutionEffect : public GrGLProgramStage {
public:
    GrGLConvolutionEffect(const GrProgramStageFactory& factory,
                          const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;

    virtual void setData(const GrGLInterface*,
                         const GrGLTexture&,
                         const GrCustomStage&,
                         int stageNum) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage&);

private:
    int width() const { return Gr1DKernelEffect::WidthFromRadius(fRadius); }

    int                   fRadius;
    const GrGLShaderVar*  fKernelVar;
    GrGLint               fKernelLocation;
    const GrGLShaderVar*  fImageIncrementVar;
    GrGLint               fImageIncrementLocation;

    typedef GrGLProgramStage INHERITED;
};

GrGLConvolutionEffect::GrGLConvolutionEffect(const GrProgramStageFactory& factory,
                                             const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fKernelVar(NULL)
    , fKernelLocation(0)
    , fImageIncrementVar(NULL)
    , fImageIncrementLocation(0) {
    const GrConvolutionEffect& c =
        static_cast<const GrConvolutionEffect&>(stage);
    fRadius = c.radius();
}

void GrGLConvolutionEffect::setupVariables(GrGLShaderBuilder* state,
                                           int stage) {
    fImageIncrementVar = &state->addUniform(
        GrGLShaderBuilder::kBoth_VariableLifetime,
        kVec2f_GrSLType, "uImageIncrement", stage);
    fKernelVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_VariableLifetime,
        kFloat_GrSLType, "uKernel", stage, this->width());

    fImageIncrementLocation = kUseUniform;
    fKernelLocation = kUseUniform;
}

void GrGLConvolutionEffect::emitVS(GrGLShaderBuilder* state,
                                   const char* vertexCoords) {
    SkString* code = &state->fVSCode;
    code->appendf("\t\t%s -= vec2(%d, %d) * %s;\n",
                  vertexCoords, fRadius, fRadius,
                  fImageIncrementVar->getName().c_str());
}

void GrGLConvolutionEffect::emitFS(GrGLShaderBuilder* state,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const char* samplerName) {
    SkString* code = &state->fFSCode;

    code->appendf("\t\t%s = vec4(0, 0, 0, 0);\n", outputColor);

    code->appendf("\t\tvec2 coord = %s;\n", state->fSampleCoords.c_str());
    
    int width = this ->width();
    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    for (int i = 0; i < width; i++) {
        SkString index;
        SkString kernelIndex;
        index.appendS32(i);
        fKernelVar->appendArrayAccess(index.c_str(), &kernelIndex);
        code->appendf("\t\t%s += ", outputColor);
        state->emitTextureLookup(samplerName, "coord");
        code->appendf(" * %s;\n", kernelIndex.c_str());
        code->appendf("\t\tcoord += %s;\n",
                      fImageIncrementVar->getName().c_str());
    }

    if (state->fModulate.size()) {
        code->appendf("\t\t%s = %s%s;\n", outputColor, outputColor,
                      state->fModulate.c_str());
    }
}

void GrGLConvolutionEffect::initUniforms(const GrGLInterface* gl,
                                         int programID) {
    GR_GL_CALL_RET(gl, fImageIncrementLocation,
        GetUniformLocation(programID,
            fImageIncrementVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fKernelLocation,
        GetUniformLocation(programID, fKernelVar->getName().c_str()));
}

void GrGLConvolutionEffect::setData(const GrGLInterface* gl,
                                    const GrGLTexture& texture,
                                    const GrCustomStage& data,
                                    int stageNum) {
    const GrConvolutionEffect& conv =
        static_cast<const GrConvolutionEffect&>(data);
    // the code we generated was for a specific kernel radius
    GrAssert(conv.radius() == fRadius);
    float imageIncrement[2] = { 0 };
    switch (conv.direction()) {
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

    GR_GL_CALL(gl, Uniform1fv(fKernelLocation, this->width(), conv.kernel()));
}

GrGLProgramStage::StageKey GrGLConvolutionEffect::GenKey(
                                                    const GrCustomStage& s) {
    return static_cast<const GrConvolutionEffect&>(s).radius();
}

///////////////////////////////////////////////////////////////////////////////

GrConvolutionEffect::GrConvolutionEffect(Direction direction,
                                         int radius,
                                         const float* kernel)
    : Gr1DKernelEffect(direction, radius) {
    GrAssert(radius <= kMaxKernelRadius);
    int width = this->width();
    if (NULL != kernel) {
        for (int i = 0; i < width; i++) {
            fKernel[i] = kernel[i];
        }
    }
}

GrConvolutionEffect::~GrConvolutionEffect() {
}

const GrProgramStageFactory& GrConvolutionEffect::getFactory() const {
    return GrTProgramStageFactory<GrConvolutionEffect>::getInstance();
}

bool GrConvolutionEffect::isEqual(const GrCustomStage& sBase) const {
     const GrConvolutionEffect& s =
        static_cast<const GrConvolutionEffect&>(sBase);
    return (this->radius() == s.radius() &&
             this->direction() == s.direction() &&
             0 == memcmp(fKernel, s.fKernel, this->width() * sizeof(float)));
}

void GrConvolutionEffect::setGaussianKernel(float sigma) {
    int width = this->width();
    float sum = 0.0f;
    float denom = 1.0f / (2.0f * sigma * sigma);
    for (int i = 0; i < width; ++i) {
        float x = static_cast<float>(i - this->radius());
        // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
        // is dropped here, since we renormalize the kernel below.
        fKernel[i] = sk_float_exp(- x * x * denom);
        sum += fKernel[i];
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < width; ++i) {
        fKernel[i] *= scale;
    }
}
