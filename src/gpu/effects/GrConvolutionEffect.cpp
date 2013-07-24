/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConvolutionEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;
static const UniformHandle kInvalidUniformHandle = GrGLUniformManager::kInvalidUniformHandle;

class GrGLConvolutionEffect : public GrGLEffect {
public:
    GrGLConvolutionEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager& uman, const GrDrawEffect&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

private:
    int width() const { return Gr1DKernelEffect::WidthFromRadius(fRadius); }
    bool useCropRect() const { return fUseCropRect; }

    int                 fRadius;
    bool                fUseCropRect;
    UniformHandle       fKernelUni;
    UniformHandle       fImageIncrementUni;
    UniformHandle       fCropRectUni;
    GrGLEffectMatrix    fEffectMatrix;

    typedef GrGLEffect INHERITED;
};

GrGLConvolutionEffect::GrGLConvolutionEffect(const GrBackendEffectFactory& factory,
                                             const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fKernelUni(kInvalidUniformHandle)
    , fImageIncrementUni(kInvalidUniformHandle)
    , fCropRectUni(kInvalidUniformHandle)
    , fEffectMatrix(drawEffect.castEffect<GrConvolutionEffect>().coordsType()) {
    const GrConvolutionEffect& c = drawEffect.castEffect<GrConvolutionEffect>();
    fRadius = c.radius();
    fUseCropRect = c.useCropRect();
}

void GrGLConvolutionEffect::emitCode(GrGLShaderBuilder* builder,
                                     const GrDrawEffect&,
                                     EffectKey key,
                                     const char* outputColor,
                                     const char* inputColor,
                                     const TextureSamplerArray& samplers) {
    const char* coords;
    fEffectMatrix.emitCodeMakeFSCoords2D(builder, key, &coords);
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec2f_GrSLType, "ImageIncrement");
    if (this->useCropRect()) {
        fCropRectUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                           kVec4f_GrSLType, "CropRect");
    }
    fKernelUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_ShaderType,
                                          kFloat_GrSLType, "Kernel", this->width());

    builder->fsCodeAppendf("\t\t%s = vec4(0, 0, 0, 0);\n", outputColor);

    int width = this->width();
    const GrGLShaderVar& kernel = builder->getUniformVariable(fKernelUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    builder->fsCodeAppendf("\t\tvec2 coord = %s - %d.0 * %s;\n", coords, fRadius, imgInc);

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    for (int i = 0; i < width; i++) {
        SkString index;
        SkString kernelIndex;
        index.appendS32(i);
        kernel.appendArrayAccess(index.c_str(), &kernelIndex);
        builder->fsCodeAppendf("\t\t%s += ", outputColor);
        builder->appendTextureLookup(GrGLShaderBuilder::kFragment_ShaderType, samplers[0], "coord");
        if (this->useCropRect()) {
            const char* cropRect = builder->getUniformCStr(fCropRectUni);
            builder->fsCodeAppendf(" * float(coord.x >= %s.x && coord.x <= %s.y && coord.y >= %s.z && coord.y <= %s.w)",
            cropRect, cropRect, cropRect, cropRect);
        }
        builder->fsCodeAppendf(" * %s;\n", kernelIndex.c_str());
        builder->fsCodeAppendf("\t\tcoord += %s;\n", imgInc);
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    builder->fsCodeAppend(modulate.c_str());
}

void GrGLConvolutionEffect::setData(const GrGLUniformManager& uman,
                                    const GrDrawEffect& drawEffect) {
    const GrConvolutionEffect& conv = drawEffect.castEffect<GrConvolutionEffect>();
    GrTexture& texture = *conv.texture(0);
    // the code we generated was for a specific kernel radius
    GrAssert(conv.radius() == fRadius);
    float imageIncrement[2] = { 0 };
    float ySign = texture.origin() != kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    switch (conv.direction()) {
        case Gr1DKernelEffect::kX_Direction:
            imageIncrement[0] = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            imageIncrement[1] = ySign / texture.height();
            break;
        default:
            GrCrash("Unknown filter direction.");
    }
    uman.set2fv(fImageIncrementUni, 0, 1, imageIncrement);
    if (conv.useCropRect()) {
        float c[4];
        memcpy(c, conv.cropRect(), sizeof(c));
        if (texture.origin() != kTopLeft_GrSurfaceOrigin) {
            float tmp = 1.0f - c[2];
            c[2] = 1.0f - c[3];
            c[3] = tmp;
        }
        uman.set4fv(fCropRectUni, 0, 1, c);
    }
    uman.set1fv(fKernelUni, 0, this->width(), conv.kernel());
    fEffectMatrix.setData(uman, conv.getMatrix(), drawEffect, conv.texture(0));
}

GrGLEffect::EffectKey GrGLConvolutionEffect::GenKey(const GrDrawEffect& drawEffect,
                                                    const GrGLCaps&) {
    const GrConvolutionEffect& conv = drawEffect.castEffect<GrConvolutionEffect>();
    EffectKey key = conv.radius() << 1;
    key |= conv.useCropRect() ? 0x1 : 0x0;
    key <<= GrGLEffectMatrix::kKeyBits;
    EffectKey matrixKey = GrGLEffectMatrix::GenKey(conv.getMatrix(),
                                                   drawEffect,
                                                   conv.coordsType(),
                                                   conv.texture(0));
    return key | matrixKey;
}

///////////////////////////////////////////////////////////////////////////////

GrConvolutionEffect::GrConvolutionEffect(GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         const float* kernel,
                                         bool useCropRect,
                                         float cropRect[4])
    : Gr1DKernelEffect(texture, direction, radius), fUseCropRect(useCropRect) {
    GrAssert(radius <= kMaxKernelRadius);
    GrAssert(NULL != kernel);
    int width = this->width();
    for (int i = 0; i < width; i++) {
        fKernel[i] = kernel[i];
    }
    memcpy(fCropRect, cropRect, sizeof(fCropRect));
}

GrConvolutionEffect::GrConvolutionEffect(GrTexture* texture,
                                         Direction direction,
                                         int radius,
                                         float gaussianSigma,
                                         bool useCropRect,
                                         float cropRect[4])
    : Gr1DKernelEffect(texture, direction, radius), fUseCropRect(useCropRect) {
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
    memcpy(fCropRect, cropRect, sizeof(fCropRect));
}

GrConvolutionEffect::~GrConvolutionEffect() {
}

const GrBackendEffectFactory& GrConvolutionEffect::getFactory() const {
    return GrTBackendEffectFactory<GrConvolutionEffect>::getInstance();
}

bool GrConvolutionEffect::onIsEqual(const GrEffect& sBase) const {
    const GrConvolutionEffect& s = CastEffect<GrConvolutionEffect>(sBase);
    return (this->texture(0) == s.texture(0) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            0 == memcmp(fCropRect, s.fCropRect, sizeof(fCropRect)) &&
            0 == memcmp(fKernel, s.fKernel, this->width() * sizeof(float)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrConvolutionEffect);

GrEffectRef* GrConvolutionEffect::TestCreate(SkMWCRandom* random,
                                             GrContext*,
                                             const GrDrawTargetCaps&,
                                             GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    Direction dir = random->nextBool() ? kX_Direction : kY_Direction;
    int radius = random->nextRangeU(1, kMaxKernelRadius);
    float kernel[kMaxKernelRadius];
    float cropRect[4];
    for (int i = 0; i < kMaxKernelRadius; ++i) {
        kernel[i] = random->nextSScalar1();
    }
    for (int i = 0; i < 4; ++i) {
        cropRect[i] = random->nextF();
    }

    bool useCropRect = random->nextBool();
    return GrConvolutionEffect::Create(textures[texIdx],
                                       dir,
                                       radius,
                                       kernel,
                                       useCropRect,
                                       cropRect);
}
