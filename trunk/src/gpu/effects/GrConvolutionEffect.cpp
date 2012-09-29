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

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;
static const UniformHandle kInvalidUniformHandle = GrGLUniformManager::kInvalidUniformHandle;

class GrGLConvolutionEffect : public GrGLProgramStage {
public:
    GrGLConvolutionEffect(const GrProgramStageFactory& factory,
                          const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* builder) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE {};
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager& uman,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage&, const GrGLCaps&);

private:
    int width() const { return Gr1DKernelEffect::WidthFromRadius(fRadius); }

    int             fRadius;
    UniformHandle   fKernelUni;
    UniformHandle   fImageIncrementUni;

    typedef GrGLProgramStage INHERITED;
};

GrGLConvolutionEffect::GrGLConvolutionEffect(const GrProgramStageFactory& factory,
                                             const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fKernelUni(kInvalidUniformHandle)
    , fImageIncrementUni(kInvalidUniformHandle) {
    const GrConvolutionEffect& c =
        static_cast<const GrConvolutionEffect&>(stage);
    fRadius = c.radius();
}

void GrGLConvolutionEffect::setupVariables(GrGLShaderBuilder* builder) {
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec2f_GrSLType, "ImageIncrement");
    fKernelUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_ShaderType,
                                          kFloat_GrSLType, "Kernel", this->width());
}

void GrGLConvolutionEffect::emitFS(GrGLShaderBuilder* builder,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const TextureSamplerArray& samplers) {
    SkString* code = &builder->fFSCode;

    code->appendf("\t\t%s = vec4(0, 0, 0, 0);\n", outputColor);

    int width = this ->width();
    const GrGLShaderVar& kernel = builder->getUniformVariable(fKernelUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    code->appendf("\t\tvec2 coord = %s - %d.0 * %s;\n",
                  builder->defaultTexCoordsName(), fRadius, imgInc);

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    for (int i = 0; i < width; i++) {
        SkString index;
        SkString kernelIndex;
        index.appendS32(i);
        kernel.appendArrayAccess(index.c_str(), &kernelIndex);
        code->appendf("\t\t%s += ", outputColor);
        builder->appendTextureLookup(&builder->fFSCode, samplers[0], "coord");
        code->appendf(" * %s;\n", kernelIndex.c_str());
        code->appendf("\t\tcoord += %s;\n", imgInc);
    }
    GrGLSLMulVarBy4f(&builder->fFSCode, 2, outputColor, inputColor);
}

void GrGLConvolutionEffect::setData(const GrGLUniformManager& uman,
                                    const GrCustomStage& data,
                                    const GrRenderTarget*,
                                    int stageNum) {
    const GrConvolutionEffect& conv =
        static_cast<const GrConvolutionEffect&>(data);
    GrTexture& texture = *data.texture(0);
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
    uman.set2fv(fImageIncrementUni, 0, 1, imageIncrement);
    uman.set1fv(fKernelUni, 0, this->width(), conv.kernel());
}

GrGLProgramStage::StageKey GrGLConvolutionEffect::GenKey(const GrCustomStage& s,
                                                         const GrGLCaps& caps) {
    return static_cast<const GrConvolutionEffect&>(s).radius();
}

///////////////////////////////////////////////////////////////////////////////

GrConvolutionEffect::GrConvolutionEffect(GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         const float* kernel)
    : Gr1DKernelEffect(texture, direction, radius) {
    GrAssert(radius <= kMaxKernelRadius);
    int width = this->width();
    if (NULL != kernel) {
        for (int i = 0; i < width; i++) {
            fKernel[i] = kernel[i];
        }
    }
}

GrConvolutionEffect::GrConvolutionEffect(GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         float gaussianSigma)
    : Gr1DKernelEffect(texture, direction, radius) {
    GrAssert(radius <= kMaxKernelRadius);
    int width = this->width();

    float sum = 0.0f;
    float denom = 1.0f / (2.0f * gaussianSigma * gaussianSigma);
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

GrConvolutionEffect::~GrConvolutionEffect() {
}

const GrProgramStageFactory& GrConvolutionEffect::getFactory() const {
    return GrTProgramStageFactory<GrConvolutionEffect>::getInstance();
}

bool GrConvolutionEffect::isEqual(const GrCustomStage& sBase) const {
     const GrConvolutionEffect& s =
        static_cast<const GrConvolutionEffect&>(sBase);
    return (INHERITED::isEqual(sBase) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            0 == memcmp(fKernel, s.fKernel, this->width() * sizeof(float)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_CUSTOM_STAGE_TEST(GrConvolutionEffect);

GrCustomStage* GrConvolutionEffect::TestCreate(SkRandom* random,
                                              GrContext* context,
                                              GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrCustomStageUnitTest::kSkiaPMTextureIdx :
                                      GrCustomStageUnitTest::kAlphaTextureIdx;
    Direction dir = random->nextBool() ? kX_Direction : kY_Direction;
    int radius = random->nextRangeU(1, kMaxKernelRadius);
    float kernel[kMaxKernelRadius];
    for (int i = 0; i < kMaxKernelRadius; ++i) {
        kernel[i] = random->nextSScalar1();
    }

    return SkNEW_ARGS(GrConvolutionEffect, (textures[texIdx], dir, radius, kernel));
}

