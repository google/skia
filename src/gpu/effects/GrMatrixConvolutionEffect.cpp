/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/effects/GrMatrixConvolutionEffect.h"

#include "include/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

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
    GrTextureDomain::GLDomain   fSourceDomain;

    typedef GrGLSLFragmentProcessor INHERITED;
};

static std::vector<float> make_kernel_vector(const SkScalar *kernel, int size) {
    std::vector<float> result;
    result.reserve(size);
    for (int i = 0; i < size; i++) {
        result.push_back(SkScalarToFloat(kernel[i]));
    }
    return result;
}

static GrFragmentProcessor::TextureSampler make_kernel_texture(GrProxyProvider *proxyProvider, const std::vector<float> &kernel) {
    if (kernel.size() < MAX_KERNEL_SIZE) return GrFragmentProcessor::TextureSampler();
    // Need to ensure that the texture we create is power of 2 so a copy isn't needed.
    // We also know that we will not be using mipmaps. If things things weren't true we should
    // go through GrBitmapTextureMaker to handle needed copy.
    const int size = GrNextPow2((kernel.size() + 3) / 4);
    SkImageInfo kernelInfo = SkImageInfo::MakeN32Premul(size, 1);
    SkPixmap kernelPixmap(kernelInfo, kernel.data(), kernelInfo.minRowBytes());
    auto kernelImage = SkImage::MakeFromRaster(kernelPixmap, nullptr, nullptr);
    SkASSERT(SkIsPow2(kernelImage->width()) && SkIsPow2(kernelImage->height()));
    sk_sp<GrTextureProxy> kernelProxy = GrMakeCachedImageProxy(proxyProvider,
                                                                     std::move(kernelImage));
    return GrFragmentProcessor::TextureSampler(std::move(kernelProxy));
}

void GrGLMatrixConvolutionEffect::emitCode(EmitArgs& args) {
    const GrMatrixConvolutionEffect& mce = args.fFp.cast<GrMatrixConvolutionEffect>();
    const GrTextureDomain& sourceDomain = mce.sourceDomain();

    int kWidth = mce.kernelSize().width();
    int kHeight = mce.kernelSize().height();

    int arrayCount = (kWidth * kHeight + 3) / 4;
    SkASSERT(4 * arrayCount >= kWidth * kHeight);

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fImageIncrementUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                    "ImageIncrement");

    const bool kernelIsSampler = mce.kernelSize().area() > MAX_KERNEL_SIZE;
    if (!kernelIsSampler) {
      fKernelUni = uniformHandler->addUniformArray(kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                   "Kernel",
                                                   arrayCount);
    }
    fKernelOffsetUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                  "KernelOffset");
    fGainUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType, "Gain");
    fBiasUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType, "Bias");

    const char* kernelOffset = uniformHandler->getUniformCStr(fKernelOffsetUni);
    const char* imgInc = uniformHandler->getUniformCStr(fImageIncrementUni);
    const char* gain = uniformHandler->getUniformCStr(fGainUni);
    const char* bias = uniformHandler->getUniformCStr(fBiasUni);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint);
    fragBuilder->codeAppend("half4 sum = half4(0, 0, 0, 0);");
    fragBuilder->codeAppendf("float2 coord = %s - %s * %s;", coords2D.c_str(), kernelOffset, imgInc);
    fragBuilder->codeAppend("half4 c;");

    const char* kVecSuffix[4] = { ".x", ".y", ".z", ".w" };
    for (int y = 0; y < kHeight; y++) {
        for (int x = 0; x < kWidth; x++) {
            GrGLSLShaderBuilder::ShaderBlock block(fragBuilder);
            int offset = y*kWidth + x;

            if (kernelIsSampler) {
                SkString kernelCoord;
                // TODO(adlai) Would love to use texelFetch directly here. For now, grab the middle
                // of the texel in [0,1] space. Note the texture width is NextPow2 from the actual
                // kernel width, so account for that.
                float texCoord = ((offset / 4) * 4) / (float)(kWidth * kHeight);
                texCoord /= GrNextPow2(arrayCount) / (float)arrayCount;
                kernelCoord.printf("half2(%f, 0.5)", texCoord);

                fragBuilder->codeAppend("half k =");
                fragBuilder->appendTextureLookup(args.fTexSamplers[1], kernelCoord.c_str());
                fragBuilder->codeAppendf("%s;", kVecSuffix[offset & 0x3]);
            } else {
                const char* kernel = uniformHandler->getUniformCStr(fKernelUni);
                fragBuilder->codeAppendf("half k = %s[%d]%s;", kernel, offset / 4,
                                         kVecSuffix[offset & 0x3]);
            }

            SkString coord;
            coord.printf("coord + half2(%d, %d) * %s", x, y, imgInc);
            fSourceDomain.sampleTexture(fragBuilder,
                                  uniformHandler,
                                  args.fShaderCaps,
                                  sourceDomain,
                                  "c",
                                  coord,
                                  args.fTexSamplers[0]);
            if (!mce.convolveAlpha()) {
                fragBuilder->codeAppend("c.rgb /= c.a;");
                fragBuilder->codeAppend("c.rgb = saturate(c.rgb);");
            }
            fragBuilder->codeAppend("sum += c * k;");
        }
    }
    if (mce.convolveAlpha()) {
        fragBuilder->codeAppendf("%s = sum * %s + %s;", args.fOutputColor, gain, bias);
        fragBuilder->codeAppendf("%s.a = saturate(%s.a);", args.fOutputColor, args.fOutputColor);
        fragBuilder->codeAppendf("%s.rgb = clamp(%s.rgb, 0.0, %s.a);",
                                 args.fOutputColor, args.fOutputColor, args.fOutputColor);
    } else {
        fSourceDomain.sampleTexture(fragBuilder,
                              uniformHandler,
                              args.fShaderCaps,
                              sourceDomain,
                              "c",
                              coords2D,
                              args.fTexSamplers[0]);
        fragBuilder->codeAppendf("%s.a = c.a;", args.fOutputColor);
        fragBuilder->codeAppendf("%s.rgb = saturate(sum.rgb * %s + %s);", args.fOutputColor, gain, bias);
        fragBuilder->codeAppendf("%s.rgb *= %s.a;", args.fOutputColor, args.fOutputColor);
    }
    fragBuilder->codeAppendf("%s *= %s;\n", args.fOutputColor, args.fInputColor);
}

void GrGLMatrixConvolutionEffect::GenKey(const GrProcessor& processor,
                                         const GrShaderCaps&, GrProcessorKeyBuilder* b) {
    const GrMatrixConvolutionEffect& m = processor.cast<GrMatrixConvolutionEffect>();
    SkASSERT(m.kernelSize().width() <= 0x7FFF && m.kernelSize().height() <= 0xFFFF);
    uint32_t key = m.kernelSize().width() << 16 | m.kernelSize().height();
    key |= m.convolveAlpha() ? 1U << 31 : 0;
    b->add32(key);
    b->add32(GrTextureDomain::GLDomain::DomainKey(m.sourceDomain()));
}

void GrGLMatrixConvolutionEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                            const GrFragmentProcessor& processor) {
    const GrMatrixConvolutionEffect& conv = processor.cast<GrMatrixConvolutionEffect>();
    const auto& view = conv.textureSampler(0).view();
    SkISize textureDims = view.proxy()->backingStoreDimensions();

    float imageIncrement[2];
    float ySign = view.origin() == kTopLeft_GrSurfaceOrigin ? 1.0f : -1.0f;
    imageIncrement[0] = 1.0f / textureDims.width();
    imageIncrement[1] = ySign / textureDims.height();
    pdman.set2fv(fImageIncrementUni, 1, imageIncrement);
    pdman.set2fv(fKernelOffsetUni, 1, conv.kernelOffset());
    int kernelCount = conv.kernelSize().width() * conv.kernelSize().height();
    int arrayCount = (kernelCount + 3) / 4;
    SkASSERT(4 * arrayCount >= kernelCount);
    if (kernelCount <= MAX_KERNEL_SIZE) {
        pdman.set4fv(fKernelUni, arrayCount, conv.kernel().data());
    }
    pdman.set1f(fGainUni, conv.gain());
    pdman.set1f(fBiasUni, conv.bias());
    fSourceDomain.setData(pdman, conv.sourceDomain(), view, conv.textureSampler(0).samplerState());
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(GrProxyProvider *proxyProvider,
                                                     sk_sp<GrSurfaceProxy> srcProxy,
                                                     const SkIRect& srcBounds,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     GrTextureDomain::Mode tileMode,
                                                     bool convolveAlpha)
        // To advertise either the modulation or opaqueness optimizations we'd have to examine the
        // parameters.
        : INHERITED(kGrMatrixConvolutionEffect_ClassID, kNone_OptimizationFlags)
        , fCoordTransform(srcProxy.get())
        , fSourceDomain(srcProxy.get(), GrTextureDomain::MakeTexelDomain(srcBounds, tileMode),
                  tileMode, tileMode, 0)
        , fSourceSampler(std::move(srcProxy))
        , fKernel(make_kernel_vector(kernel, kernelSize.area()))
        , fKernelSampler(make_kernel_texture(proxyProvider, fKernel))
        , fKernelSize(kernelSize)
        , fGain(SkScalarToFloat(gain))
        , fBias(SkScalarToFloat(bias) / 255.0f)
        , fConvolveAlpha(convolveAlpha) {
    this->addCoordTransform(&fCoordTransform);
    if (kernelSize.area() > MAX_KERNEL_SIZE) {
        this->setTextureSamplerCnt(2);
    } else {
        this->setTextureSamplerCnt(1);
    }
    fKernelOffset[0] = static_cast<float>(kernelOffset.x());
    fKernelOffset[1] = static_cast<float>(kernelOffset.y());
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(const GrMatrixConvolutionEffect& that)
        : INHERITED(kGrMatrixConvolutionEffect_ClassID, kNone_OptimizationFlags)
        , fCoordTransform(that.fCoordTransform)
        , fSourceDomain(that.fSourceDomain)
        , fSourceSampler(that.fSourceSampler)
        , fKernelSampler(that.fKernelSampler)
        , fKernelSize(that.fKernelSize)
        , fKernel(that.fKernel)
        , fGain(that.fGain)
        , fBias(that.fBias)
        , fConvolveAlpha(that.fConvolveAlpha) {
    this->addCoordTransform(&fCoordTransform);
    this->setTextureSamplerCnt(that.numTextureSamplers());
    memcpy(fKernelOffset, that.fKernelOffset, sizeof(fKernelOffset));
}

std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrMatrixConvolutionEffect(*this));
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
           fKernel == s.kernel() &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           !memcmp(fKernelOffset, s.kernelOffset(), sizeof(fKernelOffset)) &&
           fConvolveAlpha == s.convolveAlpha() &&
           fSourceDomain == s.sourceDomain();
}

static void fill_in_1D_gaussian_kernel_with_stride(float* kernel, int size, int stride,
                                                   float twoSigmaSqrd) {
    SkASSERT(!SkScalarNearlyZero(twoSigmaSqrd, SK_ScalarNearlyZero));

    const float sigmaDenom = 1.0f / twoSigmaSqrd;
    const int radius = size / 2;

    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        float term = static_cast<float>(i - radius);
        // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
        // is dropped here, since we renormalize the kernel below.
        kernel[i * stride] = sk_float_exp(-term * term * sigmaDenom);
        sum += kernel[i * stride];
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < size; ++i) {
        kernel[i * stride] *= scale;
    }
}

static void fill_in_2D_gaussian_kernel(float* kernel, int width, int height,
                                       SkScalar sigmaX, SkScalar sigmaY) {
    const float twoSigmaSqrdX = 2.0f * SkScalarToFloat(SkScalarSquare(sigmaX));
    const float twoSigmaSqrdY = 2.0f * SkScalarToFloat(SkScalarSquare(sigmaY));

    // TODO: in all of these degenerate cases we're uploading (and using) a whole lot of zeros.
    if (SkScalarNearlyZero(twoSigmaSqrdX, SK_ScalarNearlyZero) ||
        SkScalarNearlyZero(twoSigmaSqrdY, SK_ScalarNearlyZero)) {
        // In this case the 2D Gaussian degenerates to a 1D Gaussian (in X or Y) or a point
        SkASSERT(3 == width || 3 == height);
        memset(kernel, 0, width*height*sizeof(float));

        if (SkScalarNearlyZero(twoSigmaSqrdX, SK_ScalarNearlyZero) &&
            SkScalarNearlyZero(twoSigmaSqrdY, SK_ScalarNearlyZero)) {
            // A point
            SkASSERT(3 == width && 3 == height);
            kernel[4] = 1.0f;
        } else if (SkScalarNearlyZero(twoSigmaSqrdX, SK_ScalarNearlyZero)) {
            // A 1D Gaussian in Y
            SkASSERT(3 == width);
            // Down the middle column of the kernel with a stride of width
            fill_in_1D_gaussian_kernel_with_stride(&kernel[1], height, width, twoSigmaSqrdY);
        } else {
            // A 1D Gaussian in X
            SkASSERT(SkScalarNearlyZero(twoSigmaSqrdY, SK_ScalarNearlyZero));
            SkASSERT(3 == height);
            // Down the middle row of the kernel with a stride of 1
            fill_in_1D_gaussian_kernel_with_stride(&kernel[width], width, 1, twoSigmaSqrdX);
        }
        return;
    }

    const float sigmaXDenom = 1.0f / twoSigmaSqrdX;
    const float sigmaYDenom = 1.0f / twoSigmaSqrdY;
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
std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::MakeGaussian(
        GrProxyProvider *proxyProvider,
        sk_sp<GrTextureProxy> srcProxy,
        const SkIRect& srcBounds,
        const SkISize& kernelSize,
        SkScalar gain,
        SkScalar bias,
        const SkIPoint& kernelOffset,
        GrTextureDomain::Mode tileMode,
        bool convolveAlpha,
        SkScalar sigmaX,
        SkScalar sigmaY) {
    std::vector<float> kernel(kernelSize.area());

    fill_in_2D_gaussian_kernel(kernel.data(), kernelSize.width(), kernelSize.height(), sigmaX, sigmaY);

    return std::unique_ptr<GrFragmentProcessor>(
            new GrMatrixConvolutionEffect(proxyProvider, std::move(srcProxy), srcBounds, kernelSize, kernel.data(),
                                          gain, bias, kernelOffset, tileMode, convolveAlpha));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMatrixConvolutionEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::TestCreate(GrProcessorTestData* d) {
    auto [proxy, ct, at] = d->randomProxy();

    static constexpr size_t kMaxTestKernelSize = 2 * MAX_KERNEL_SIZE;
    int width = d->fRandom->nextRangeU(1, kMaxTestKernelSize);
    int height = d->fRandom->nextRangeU(1, kMaxTestKernelSize / width);
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
    return GrMatrixConvolutionEffect::Make(d->proxyProvider(),
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
