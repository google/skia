/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/builders/GrGLProgramBuilder.h"
#include "GrMatrixConvolutionEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"

class GrGLMatrixConvolutionEffect : public GrGLEffect {
public:
    GrGLMatrixConvolutionEffect(const GrBackendEffectFactory& factory,
                                const GrDrawEffect& effect);
    virtual void emitCode(GrGLProgramBuilder*,
                          const GrDrawEffect&,
                          const GrEffectKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    SkISize                     fKernelSize;
    bool                        fConvolveAlpha;

    UniformHandle               fBoundsUni;
    UniformHandle               fKernelUni;
    UniformHandle               fImageIncrementUni;
    UniformHandle               fKernelOffsetUni;
    UniformHandle               fGainUni;
    UniformHandle               fBiasUni;
    GrTextureDomain::GLDomain   fDomain;

    typedef GrGLEffect INHERITED;
};

GrGLMatrixConvolutionEffect::GrGLMatrixConvolutionEffect(const GrBackendEffectFactory& factory,
                                                         const GrDrawEffect& drawEffect)
    : INHERITED(factory) {
    const GrMatrixConvolutionEffect& m = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    fKernelSize = m.kernelSize();
    fConvolveAlpha = m.convolveAlpha();
}

void GrGLMatrixConvolutionEffect::emitCode(GrGLProgramBuilder* builder,
                                           const GrDrawEffect& drawEffect,
                                           const GrEffectKey& key,
                                           const char* outputColor,
                                           const char* inputColor,
                                           const TransformedCoordsArray& coords,
                                           const TextureSamplerArray& samplers) {
    sk_ignore_unused_variable(inputColor);
    const GrTextureDomain& domain = drawEffect.castEffect<GrMatrixConvolutionEffect>().domain();

    fBoundsUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                     kVec4f_GrSLType, "Bounds");
    fImageIncrementUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "ImageIncrement");
    fKernelUni = builder->addUniformArray(GrGLProgramBuilder::kFragment_Visibility,
                                          kFloat_GrSLType,
                                          "Kernel",
                                          fKernelSize.width() * fKernelSize.height());
    fKernelOffsetUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                           kVec2f_GrSLType, "KernelOffset");
    fGainUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Gain");
    fBiasUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Bias");

    const char* kernelOffset = builder->getUniformCStr(fKernelOffsetUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    const char* kernel = builder->getUniformCStr(fKernelUni);
    const char* gain = builder->getUniformCStr(fGainUni);
    const char* bias = builder->getUniformCStr(fBiasUni);
    int kWidth = fKernelSize.width();
    int kHeight = fKernelSize.height();

    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    SkString coords2D = fsBuilder->ensureFSCoords2D(coords, 0);
    fsBuilder->codeAppend("vec4 sum = vec4(0, 0, 0, 0);");
    fsBuilder->codeAppendf("vec2 coord = %s - %s * %s;", coords2D.c_str(), kernelOffset,
                           imgInc);
    fsBuilder->codeAppend("vec4 c;");

    for (int y = 0; y < kHeight; y++) {
        for (int x = 0; x < kWidth; x++) {
            GrGLShaderBuilder::ShaderBlock block(fsBuilder);
            fsBuilder->codeAppendf("float k = %s[%d * %d + %d];", kernel, y, kWidth, x);
            SkString coord;
            coord.printf("coord + vec2(%d, %d) * %s", x, y, imgInc);
            fDomain.sampleTexture(fsBuilder, domain, "c", coord, samplers[0]);
            if (!fConvolveAlpha) {
                fsBuilder->codeAppend("c.rgb /= c.a;");
            }
            fsBuilder->codeAppend("sum += c * k;");
        }
    }
    if (fConvolveAlpha) {
        fsBuilder->codeAppendf("%s = sum * %s + %s;", outputColor, gain, bias);
        fsBuilder->codeAppendf("%s.rgb = clamp(%s.rgb, 0.0, %s.a);",
                               outputColor, outputColor, outputColor);
    } else {
        fDomain.sampleTexture(fsBuilder, domain, "c", coords2D, samplers[0]);
        fsBuilder->codeAppendf("%s.a = c.a;", outputColor);
        fsBuilder->codeAppendf("%s.rgb = sum.rgb * %s + %s;", outputColor, gain, bias);
        fsBuilder->codeAppendf("%s.rgb *= %s.a;", outputColor, outputColor);
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
    fsBuilder->codeAppend(modulate.c_str());
}

void GrGLMatrixConvolutionEffect::GenKey(const GrDrawEffect& drawEffect,
                                         const GrGLCaps&, GrEffectKeyBuilder* b) {
    const GrMatrixConvolutionEffect& m = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    SkASSERT(m.kernelSize().width() <= 0x7FFF && m.kernelSize().height() <= 0xFFFF);
    uint32_t key = m.kernelSize().width() << 16 | m.kernelSize().height();
    key |= m.convolveAlpha() ? 1 << 31 : 0;
    b->add32(key);
    b->add32(GrTextureDomain::GLDomain::DomainKey(m.domain()));
}

void GrGLMatrixConvolutionEffect::setData(const GrGLProgramDataManager& pdman,
                                          const GrDrawEffect& drawEffect) {
    const GrMatrixConvolutionEffect& conv = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    GrTexture& texture = *conv.texture(0);
    // the code we generated was for a specific kernel size
    SkASSERT(conv.kernelSize() == fKernelSize);
    float imageIncrement[2];
    float ySign = texture.origin() == kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    imageIncrement[0] = 1.0f / texture.width();
    imageIncrement[1] = ySign / texture.height();
    pdman.set2fv(fImageIncrementUni, 1, imageIncrement);
    pdman.set2fv(fKernelOffsetUni, 1, conv.kernelOffset());
    pdman.set1fv(fKernelUni, fKernelSize.width() * fKernelSize.height(), conv.kernel());
    pdman.set1f(fGainUni, conv.gain());
    pdman.set1f(fBiasUni, conv.bias());
    fDomain.setData(pdman, conv.domain(), texture.origin());
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(GrTexture* texture,
                                                     const SkIRect& bounds,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     GrTextureDomain::Mode tileMode,
                                                     bool convolveAlpha)
  : INHERITED(texture, MakeDivByTextureWHMatrix(texture)),
    fKernelSize(kernelSize),
    fGain(SkScalarToFloat(gain)),
    fBias(SkScalarToFloat(bias) / 255.0f),
    fConvolveAlpha(convolveAlpha),
    fDomain(GrTextureDomain::MakeTexelDomain(texture, bounds), tileMode) {
    for (int i = 0; i < kernelSize.width() * kernelSize.height(); i++) {
        fKernel[i] = SkScalarToFloat(kernel[i]);
    }
    fKernelOffset[0] = static_cast<float>(kernelOffset.x());
    fKernelOffset[1] = static_cast<float>(kernelOffset.y());
}

GrMatrixConvolutionEffect::~GrMatrixConvolutionEffect() {
}

const GrBackendEffectFactory& GrMatrixConvolutionEffect::getFactory() const {
    return GrTBackendEffectFactory<GrMatrixConvolutionEffect>::getInstance();
}

bool GrMatrixConvolutionEffect::onIsEqual(const GrEffect& sBase) const {
    const GrMatrixConvolutionEffect& s = CastEffect<GrMatrixConvolutionEffect>(sBase);
    return this->texture(0) == s.texture(0) &&
           fKernelSize == s.kernelSize() &&
           !memcmp(fKernel, s.kernel(),
                   fKernelSize.width() * fKernelSize.height() * sizeof(float)) &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           fKernelOffset == s.kernelOffset() &&
           fConvolveAlpha == s.convolveAlpha() &&
           fDomain == s.domain();
}

// Static function to create a 2D convolution
GrEffect* GrMatrixConvolutionEffect::CreateGaussian(GrTexture* texture,
                                                    const SkIRect& bounds,
                                                    const SkISize& kernelSize,
                                                    SkScalar gain,
                                                    SkScalar bias,
                                                    const SkIPoint& kernelOffset,
                                                    GrTextureDomain::Mode tileMode,
                                                    bool convolveAlpha,
                                                    SkScalar sigmaX,
                                                    SkScalar sigmaY) {
    float kernel[MAX_KERNEL_SIZE];
    int width = kernelSize.width();
    int height = kernelSize.height();
    SkASSERT(width * height <= MAX_KERNEL_SIZE);
    float sum = 0.0f;
    float sigmaXDenom = 1.0f / (2.0f * SkScalarToFloat(SkScalarSquare(sigmaX)));
    float sigmaYDenom = 1.0f / (2.0f * SkScalarToFloat(SkScalarSquare(sigmaY)));
    int xRadius = width / 2;
    int yRadius = height / 2;
    for (int x = 0; x < width; x++) {
      float xTerm = static_cast<float>(x - xRadius);
      xTerm = xTerm * xTerm * sigmaXDenom;
      for (int y = 0; y < height; y++) {
        float yTerm = static_cast<float>(y - yRadius);
        float xyTerm = sk_float_exp(-(xTerm + yTerm * yTerm * sigmaYDenom));
        // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
       // is dropped here, since we renormalize the kernel below.
        kernel[y * width + x] = xyTerm;
        sum += xyTerm;
      }
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < width * height; ++i) {
        kernel[i] *= scale;
    }
    return SkNEW_ARGS(GrMatrixConvolutionEffect, (texture,
                                                  bounds,
                                                  kernelSize,
                                                  kernel,
                                                  gain,
                                                  bias,
                                                  kernelOffset,
                                                  tileMode,
                                                  convolveAlpha));
}

GR_DEFINE_EFFECT_TEST(GrMatrixConvolutionEffect);

GrEffect* GrMatrixConvolutionEffect::TestCreate(SkRandom* random,
                                                GrContext* context,
                                                const GrDrawTargetCaps&,
                                                GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    int width = random->nextRangeU(1, MAX_KERNEL_SIZE);
    int height = random->nextRangeU(1, MAX_KERNEL_SIZE / width);
    SkISize kernelSize = SkISize::Make(width, height);
    SkAutoTDeleteArray<SkScalar> kernel(new SkScalar[width * height]);
    for (int i = 0; i < width * height; i++) {
        kernel.get()[i] = random->nextSScalar1();
    }
    SkScalar gain = random->nextSScalar1();
    SkScalar bias = random->nextSScalar1();
    SkIPoint kernelOffset = SkIPoint::Make(random->nextRangeU(0, kernelSize.width()),
                                           random->nextRangeU(0, kernelSize.height()));
    SkIRect bounds = SkIRect::MakeXYWH(random->nextRangeU(0, textures[texIdx]->width()),
                                       random->nextRangeU(0, textures[texIdx]->height()),
                                       random->nextRangeU(0, textures[texIdx]->width()),
                                       random->nextRangeU(0, textures[texIdx]->height()));
    GrTextureDomain::Mode tileMode = static_cast<GrTextureDomain::Mode>(random->nextRangeU(0, 2));
    bool convolveAlpha = random->nextBool();
    return GrMatrixConvolutionEffect::Create(textures[texIdx],
                                             bounds,
                                             kernelSize,
                                             kernel.get(),
                                             gain,
                                             bias,
                                             kernelOffset,
                                             tileMode,
                                             convolveAlpha);
}
