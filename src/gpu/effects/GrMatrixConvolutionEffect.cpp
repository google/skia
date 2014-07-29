/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrMatrixConvolutionEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"

class GrGLMatrixConvolutionEffect : public GrGLEffect {
public:
    GrGLMatrixConvolutionEffect(const GrBackendEffectFactory& factory,
                                const GrDrawEffect& effect);
    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          const GrEffectKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

private:
    typedef GrGLUniformManager::UniformHandle        UniformHandle;
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

void GrGLMatrixConvolutionEffect::emitCode(GrGLShaderBuilder* builder,
                                           const GrDrawEffect& drawEffect,
                                           const GrEffectKey& key,
                                           const char* outputColor,
                                           const char* inputColor,
                                           const TransformedCoordsArray& coords,
                                           const TextureSamplerArray& samplers) {
    sk_ignore_unused_variable(inputColor);
    const GrTextureDomain& domain = drawEffect.castEffect<GrMatrixConvolutionEffect>().domain();
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    fBoundsUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                     kVec4f_GrSLType, "Bounds");
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                             kVec2f_GrSLType, "ImageIncrement");
    fKernelUni = builder->addUniformArray(GrGLShaderBuilder::kFragment_Visibility,
                                          kFloat_GrSLType,
                                          "Kernel",
                                          fKernelSize.width() * fKernelSize.height());
    fKernelOffsetUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                           kVec2f_GrSLType, "KernelOffset");
    fGainUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Gain");
    fBiasUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                   kFloat_GrSLType, "Bias");

    const char* kernelOffset = builder->getUniformCStr(fKernelOffsetUni);
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);
    const char* kernel = builder->getUniformCStr(fKernelUni);
    const char* gain = builder->getUniformCStr(fGainUni);
    const char* bias = builder->getUniformCStr(fBiasUni);
    int kWidth = fKernelSize.width();
    int kHeight = fKernelSize.height();

    builder->fsCodeAppend("\t\tvec4 sum = vec4(0, 0, 0, 0);\n");
    builder->fsCodeAppendf("\t\tvec2 coord = %s - %s * %s;\n", coords2D.c_str(), kernelOffset,
                           imgInc);
    builder->fsCodeAppend("\t\tvec4 c;\n");

    for (int y = 0; y < kHeight; y++) {
        for (int x = 0; x < kWidth; x++) {
            GrGLShaderBuilder::FSBlock block(builder);
            builder->fsCodeAppendf("\t\tfloat k = %s[%d * %d + %d];\n", kernel, y, kWidth, x);
            SkString coord;
            coord.printf("coord + vec2(%d, %d) * %s", x, y, imgInc);
            fDomain.sampleTexture(builder, domain, "c", coord, samplers[0]);
            if (!fConvolveAlpha) {
                builder->fsCodeAppend("\t\tc.rgb /= c.a;\n");
            }
            builder->fsCodeAppend("\t\tsum += c * k;\n");
        }
    }
    if (fConvolveAlpha) {
        builder->fsCodeAppendf("\t\t%s = sum * %s + %s;\n", outputColor, gain, bias);
        builder->fsCodeAppendf("\t\t%s.rgb = clamp(%s.rgb, 0.0, %s.a);\n",
                               outputColor, outputColor, outputColor);
    } else {
        fDomain.sampleTexture(builder, domain, "c", coords2D, samplers[0]);
        builder->fsCodeAppendf("\t\t%s.a = c.a;\n", outputColor);
        builder->fsCodeAppendf("\t\t%s.rgb = sum.rgb * %s + %s;\n", outputColor, gain, bias);
        builder->fsCodeAppendf("\t\t%s.rgb *= %s.a;\n", outputColor, outputColor);
    }
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

void GrGLMatrixConvolutionEffect::setData(const GrGLUniformManager& uman,
                                          const GrDrawEffect& drawEffect) {
    const GrMatrixConvolutionEffect& conv = drawEffect.castEffect<GrMatrixConvolutionEffect>();
    GrTexture& texture = *conv.texture(0);
    // the code we generated was for a specific kernel size
    SkASSERT(conv.kernelSize() == fKernelSize);
    float imageIncrement[2];
    float ySign = texture.origin() == kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    imageIncrement[0] = 1.0f / texture.width();
    imageIncrement[1] = ySign / texture.height();
    uman.set2fv(fImageIncrementUni, 1, imageIncrement);
    uman.set2fv(fKernelOffsetUni, 1, conv.kernelOffset());
    uman.set1fv(fKernelUni, fKernelSize.width() * fKernelSize.height(), conv.kernel());
    uman.set1f(fGainUni, conv.gain());
    uman.set1f(fBiasUni, conv.bias());
    fDomain.setData(uman, conv.domain(), texture.origin());
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
    fKernel = new float[kernelSize.width() * kernelSize.height()];
    for (int i = 0; i < kernelSize.width() * kernelSize.height(); i++) {
        fKernel[i] = SkScalarToFloat(kernel[i]);
    }
    fKernelOffset[0] = static_cast<float>(kernelOffset.x());
    fKernelOffset[1] = static_cast<float>(kernelOffset.y());
    this->setWillNotUseInputColor();
}

GrMatrixConvolutionEffect::~GrMatrixConvolutionEffect() {
    delete[] fKernel;
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
