/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrMatrixConvolutionEffect.h"

#include "GrTextureProxy.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "../private/GrGLSL.h"

class GrGLMatrixConvolutionEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    UniformHandle               fKernelUni;
    UniformHandle               fImageIncrementUni;
    UniformHandle               fKernelOffsetUni;
    UniformHandle               fGainUni;
    UniformHandle               fBiasUni;
    GrTextureDomain::GLDomain   fDomain;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLMatrixConvolutionEffect::emitCode(EmitArgs& args) {
    const GrMatrixConvolutionEffect& mce = args.fFp.cast<GrMatrixConvolutionEffect>();
    const GrTextureDomain& domain = mce.domain();

    int kWidth = mce.kernelSize().width();
    int kHeight = mce.kernelSize().height();

    int arrayCount = (kWidth * kHeight + 3) / 4;
    SkASSERT(4 * arrayCount >= kWidth * kHeight);

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fImageIncrementUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                    "ImageIncrement");
    fKernelUni = uniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                 kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                 "Kernel",
                                                 arrayCount);
    fKernelOffsetUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                  kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                  "KernelOffset");
    fGainUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                          kFloat_GrSLType, kDefault_GrSLPrecision, "Gain");
    fBiasUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                          kFloat_GrSLType, kDefault_GrSLPrecision, "Bias");

    const char* kernelOffset = uniformHandler->getUniformCStr(fKernelOffsetUni);
    const char* imgInc = uniformHandler->getUniformCStr(fImageIncrementUni);
    const char* kernel = uniformHandler->getUniformCStr(fKernelUni);
    const char* gain = uniformHandler->getUniformCStr(fGainUni);
    const char* bias = uniformHandler->getUniformCStr(fBiasUni);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
    fragBuilder->codeAppend("vec4 sum = vec4(0, 0, 0, 0);");
    fragBuilder->codeAppendf("vec2 coord = %s - %s * %s;", coords2D.c_str(), kernelOffset, imgInc);
    fragBuilder->codeAppend("vec4 c;");

    const char* kVecSuffix[4] = { ".x", ".y", ".z", ".w" };
    for (int y = 0; y < kHeight; y++) {
        for (int x = 0; x < kWidth; x++) {
            GrGLSLShaderBuilder::ShaderBlock block(fragBuilder);
            int offset = y*kWidth + x;

            fragBuilder->codeAppendf("float k = %s[%d]%s;", kernel, offset / 4,
                                     kVecSuffix[offset & 0x3]);
            SkString coord;
            coord.printf("coord + vec2(%d, %d) * %s", x, y, imgInc);
            fDomain.sampleTexture(fragBuilder,
                                  uniformHandler,
                                  args.fShaderCaps,
                                  domain,
                                  "c",
                                  coord,
                                  args.fTexSamplers[0]);
            if (!mce.convolveAlpha()) {
                fragBuilder->codeAppend("c.rgb /= c.a;");
                fragBuilder->codeAppend("c.rgb = clamp(c.rgb, 0.0, 1.0);");
            }
            fragBuilder->codeAppend("sum += c * k;");
        }
    }
    if (mce.convolveAlpha()) {
        fragBuilder->codeAppendf("%s = sum * %s + %s;", args.fOutputColor, gain, bias);
        fragBuilder->codeAppendf("%s.a = clamp(%s.a, 0, 1);", args.fOutputColor, args.fOutputColor);
        fragBuilder->codeAppendf("%s.rgb = clamp(%s.rgb, 0.0, %s.a);",
                                 args.fOutputColor, args.fOutputColor, args.fOutputColor);
    } else {
        fDomain.sampleTexture(fragBuilder,
                              uniformHandler,
                              args.fShaderCaps,
                              domain,
                              "c",
                              coords2D,
                              args.fTexSamplers[0]);
        fragBuilder->codeAppendf("%s.a = c.a;", args.fOutputColor);
        fragBuilder->codeAppendf("%s.rgb = clamp(sum.rgb * %s + %s, 0, 1);", args.fOutputColor, gain, bias);
        fragBuilder->codeAppendf("%s.rgb *= %s.a;", args.fOutputColor, args.fOutputColor);
    }

    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    fragBuilder->codeAppend(modulate.c_str());
}

void GrGLMatrixConvolutionEffect::GenKey(const GrProcessor& processor,
                                         const GrShaderCaps&, GrProcessorKeyBuilder* b) {
    const GrMatrixConvolutionEffect& m = processor.cast<GrMatrixConvolutionEffect>();
    SkASSERT(m.kernelSize().width() <= 0x7FFF && m.kernelSize().height() <= 0xFFFF);
    uint32_t key = m.kernelSize().width() << 16 | m.kernelSize().height();
    key |= m.convolveAlpha() ? 1U << 31 : 0;
    b->add32(key);
    b->add32(GrTextureDomain::GLDomain::DomainKey(m.domain()));
}

void GrGLMatrixConvolutionEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                            const GrFragmentProcessor& processor) {
    const GrMatrixConvolutionEffect& conv = processor.cast<GrMatrixConvolutionEffect>();
    GrTexture* texture = conv.textureSampler(0).texture();

    float imageIncrement[2];
    float ySign = texture->origin() == kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    imageIncrement[0] = 1.0f / texture->width();
    imageIncrement[1] = ySign / texture->height();
    pdman.set2fv(fImageIncrementUni, 1, imageIncrement);
    pdman.set2fv(fKernelOffsetUni, 1, conv.kernelOffset());
    int kernelCount = conv.kernelSize().width() * conv.kernelSize().height();
    int arrayCount = (kernelCount + 3) / 4;
    SkASSERT(4 * arrayCount >= kernelCount);
    pdman.set4fv(fKernelUni, arrayCount, conv.kernel());
    pdman.set1f(fGainUni, conv.gain());
    pdman.set1f(fBiasUni, conv.bias());
    fDomain.setData(pdman, conv.domain(), texture);
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(GrResourceProvider* resourceProvider,
                                                     sk_sp<GrTextureProxy> proxy,
                                                     const SkIRect& bounds,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     GrTextureDomain::Mode tileMode,
                                                     bool convolveAlpha)
    // To advertise either the modulation or opaqueness optimizations we'd have to examine the
    // parameters.
    : INHERITED(resourceProvider, kNone_OptimizationFlags, proxy, nullptr, SkMatrix::I())
    , fKernelSize(kernelSize)
    , fGain(SkScalarToFloat(gain))
    , fBias(SkScalarToFloat(bias) / 255.0f)
    , fConvolveAlpha(convolveAlpha)
    , fDomain(proxy.get(), GrTextureDomain::MakeTexelDomainForMode(bounds, tileMode), tileMode) {
    this->initClassID<GrMatrixConvolutionEffect>();
    for (int i = 0; i < kernelSize.width() * kernelSize.height(); i++) {
        fKernel[i] = SkScalarToFloat(kernel[i]);
    }
    fKernelOffset[0] = static_cast<float>(kernelOffset.x());
    fKernelOffset[1] = static_cast<float>(kernelOffset.y());
}

void GrMatrixConvolutionEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                      GrProcessorKeyBuilder* b) const {
    GrGLMatrixConvolutionEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrMatrixConvolutionEffect::onCreateGLSLInstance() const  {
    return new GrGLMatrixConvolutionEffect;
}

bool GrMatrixConvolutionEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrMatrixConvolutionEffect& s = sBase.cast<GrMatrixConvolutionEffect>();
    return fKernelSize == s.kernelSize() &&
           !memcmp(fKernel, s.kernel(),
                   fKernelSize.width() * fKernelSize.height() * sizeof(float)) &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           fKernelOffset == s.kernelOffset() &&
           fConvolveAlpha == s.convolveAlpha() &&
           fDomain == s.domain();
}

static void fill_in_2D_gaussian_kernel(float* kernel, int width, int height,
                                       SkScalar sigmaX, SkScalar sigmaY) {
    SkASSERT(width * height <= MAX_KERNEL_SIZE);
    const float sigmaXDenom = 1.0f / (2.0f * SkScalarToFloat(SkScalarSquare(sigmaX)));
    const float sigmaYDenom = 1.0f / (2.0f * SkScalarToFloat(SkScalarSquare(sigmaY)));
    const int xRadius = width / 2;
    const int yRadius = height / 2;

    float sum = 0.0f;
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
}


// Static function to create a 2D convolution
sk_sp<GrFragmentProcessor> GrMatrixConvolutionEffect::MakeGaussian(
                                                            GrResourceProvider* resourceProvider,
                                                            sk_sp<GrTextureProxy> proxy,
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

    fill_in_2D_gaussian_kernel(kernel, kernelSize.width(), kernelSize.height(), sigmaX, sigmaY);

    return sk_sp<GrFragmentProcessor>(
        new GrMatrixConvolutionEffect(resourceProvider, std::move(proxy), bounds, kernelSize,
                                      kernel, gain, bias, kernelOffset, tileMode, convolveAlpha));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMatrixConvolutionEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrMatrixConvolutionEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    sk_sp<GrTextureProxy> proxy = d->textureProxy(texIdx);

    int width = d->fRandom->nextRangeU(1, MAX_KERNEL_SIZE);
    int height = d->fRandom->nextRangeU(1, MAX_KERNEL_SIZE / width);
    SkISize kernelSize = SkISize::Make(width, height);
    std::unique_ptr<SkScalar[]> kernel(new SkScalar[width * height]);
    for (int i = 0; i < width * height; i++) {
        kernel.get()[i] = d->fRandom->nextSScalar1();
    }
    SkScalar gain = d->fRandom->nextSScalar1();
    SkScalar bias = d->fRandom->nextSScalar1();
    SkIPoint kernelOffset = SkIPoint::Make(d->fRandom->nextRangeU(0, kernelSize.width()),
                                           d->fRandom->nextRangeU(0, kernelSize.height()));
    SkIRect bounds = SkIRect::MakeXYWH(d->fRandom->nextRangeU(0, proxy->width()),
                                       d->fRandom->nextRangeU(0, proxy->height()),
                                       d->fRandom->nextRangeU(0, proxy->width()),
                                       d->fRandom->nextRangeU(0, proxy->height()));
    GrTextureDomain::Mode tileMode =
            static_cast<GrTextureDomain::Mode>(d->fRandom->nextRangeU(0, 2));
    bool convolveAlpha = d->fRandom->nextBool();
    return GrMatrixConvolutionEffect::Make(d->resourceProvider(),
                                           std::move(proxy),
                                           bounds,
                                           kernelSize,
                                           kernel.get(),
                                           gain,
                                           bias,
                                           kernelOffset,
                                           tileMode,
                                           convolveAlpha);
}
#endif
