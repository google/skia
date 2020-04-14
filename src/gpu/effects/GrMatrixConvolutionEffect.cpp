/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/effects/GrMatrixConvolutionEffect.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/GrTextureEffect.h"
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
    UniformHandle               fKernelOffsetUni;
    UniformHandle               fGainUni;
    UniformHandle               fBiasUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

void GrGLMatrixConvolutionEffect::emitCode(EmitArgs& args) {
    const GrMatrixConvolutionEffect& mce = args.fFp.cast<GrMatrixConvolutionEffect>();

    int kWidth = mce.kernelSize().width();
    int kHeight = mce.kernelSize().height();

    int arrayCount = (kWidth * kHeight + 3) / 4;
    SkASSERT(4 * arrayCount >= kWidth * kHeight);

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fKernelUni = uniformHandler->addUniformArray(&mce, kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                 "Kernel",
                                                 arrayCount);
    fKernelOffsetUni = uniformHandler->addUniform(&mce, kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                  "KernelOffset");
    fGainUni = uniformHandler->addUniform(&mce, kFragment_GrShaderFlag, kHalf_GrSLType, "Gain");
    fBiasUni = uniformHandler->addUniform(&mce, kFragment_GrShaderFlag, kHalf_GrSLType, "Bias");

    const char* kernelOffset = uniformHandler->getUniformCStr(fKernelOffsetUni);
    const char* kernel = uniformHandler->getUniformCStr(fKernelUni);
    const char* gain = uniformHandler->getUniformCStr(fGainUni);
    const char* bias = uniformHandler->getUniformCStr(fBiasUni);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString coords2D = fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint,
                                                    mce.sampleMatrix());
    fragBuilder->codeAppend("half4 sum = half4(0, 0, 0, 0);");
    fragBuilder->codeAppendf("float2 coord = %s - %s;", coords2D.c_str(), kernelOffset);
    fragBuilder->codeAppend("half4 c;");

    const char* kVecSuffix[4] = { ".x", ".y", ".z", ".w" };
    for (int y = 0; y < kHeight; y++) {
        for (int x = 0; x < kWidth; x++) {
            GrGLSLShaderBuilder::ShaderBlock block(fragBuilder);
            int offset = y*kWidth + x;

            fragBuilder->codeAppendf("half k = %s[%d]%s;", kernel, offset / 4,
                                     kVecSuffix[offset & 0x3]);
            SkSL::String coord;
            coord.appendf("coord + half2(%d, %d)", x, y);
            auto sample = this->invokeChild(0, args, coord);
            fragBuilder->codeAppendf("half4 c = %s;", sample.c_str());
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
        auto sample = this->invokeChild(0, args, coords2D.c_str());
        fragBuilder->codeAppendf("c = %s;", sample.c_str());
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
}

void GrGLMatrixConvolutionEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                            const GrFragmentProcessor& processor) {
    const GrMatrixConvolutionEffect& conv = processor.cast<GrMatrixConvolutionEffect>();
    pdman.set2fv(fKernelOffsetUni, 1, conv.kernelOffset().ptr());
    int kernelCount = conv.kernelSize().width() * conv.kernelSize().height();
    int arrayCount = (kernelCount + 3) / 4;
    SkASSERT(4 * arrayCount >= kernelCount);
    pdman.set4fv(fKernelUni, arrayCount, conv.kernel());
    pdman.set1f(fGainUni, conv.gain());
    pdman.set1f(fBiasUni, conv.bias());
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(std::unique_ptr<GrFragmentProcessor> child,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     bool convolveAlpha)
        // To advertise either the modulation or opaqueness optimizations we'd have to examine the
        // parameters.
        : INHERITED(kGrMatrixConvolutionEffect_ClassID, kNone_OptimizationFlags)
        , fKernelSize(kernelSize)
        , fGain(SkScalarToFloat(gain))
        , fBias(SkScalarToFloat(bias) / 255.0f)
        , fConvolveAlpha(convolveAlpha) {
    child->setSampledWithExplicitCoords();
    this->registerChildProcessor(std::move(child));
    for (int i = 0; i < kernelSize.width() * kernelSize.height(); i++) {
        fKernel[i] = SkScalarToFloat(kernel[i]);
    }
    fKernelOffset = {static_cast<float>(kernelOffset.x()),
                     static_cast<float>(kernelOffset.y())};
    this->addCoordTransform(&fCoordTransform);
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(const GrMatrixConvolutionEffect& that)
        : INHERITED(kGrMatrixConvolutionEffect_ClassID, kNone_OptimizationFlags)
        , fKernelSize(that.fKernelSize)
        , fGain(that.fGain)
        , fBias(that.fBias)
        , fKernelOffset(that.fKernelOffset)
        , fConvolveAlpha(that.fConvolveAlpha) {
    auto child = that.childProcessor(0).clone();
    child->setSampledWithExplicitCoords();
    this->registerChildProcessor(std::move(child));
    std::copy_n(that.fKernel, fKernelSize.width() * fKernelSize.height(), fKernel);
    this->addCoordTransform(&fCoordTransform);
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
           std::equal(fKernel, fKernel + fKernelSize.area(), s.fKernel) &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           fKernelOffset == s.kernelOffset() &&
           fConvolveAlpha == s.convolveAlpha();
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
    SkASSERT(width * height <= MAX_KERNEL_SIZE);
    const float twoSigmaSqrdX = 2.0f * SkScalarToFloat(SkScalarSquare(sigmaX));
    const float twoSigmaSqrdY = 2.0f * SkScalarToFloat(SkScalarSquare(sigmaY));

    // TODO: in all of these degenerate cases we're uploading (and using) a whole lot of zeros.
    if (SkScalarNearlyZero(twoSigmaSqrdX, SK_ScalarNearlyZero) ||
        SkScalarNearlyZero(twoSigmaSqrdY, SK_ScalarNearlyZero)) {
        // In this case the 2D Gaussian degenerates to a 1D Gaussian (in X or Y) or a point
        SkASSERT(3 == width || 3 == height);
        std::fill_n(kernel, width*height, 0);

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

std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::Make(GrSurfaceProxyView srcView,
                                                                     const SkIRect& srcBounds,
                                                                     const SkISize& kernelSize,
                                                                     const SkScalar* kernel,
                                                                     SkScalar gain,
                                                                     SkScalar bias,
                                                                     const SkIPoint& kernelOffset,
                                                                     GrSamplerState::WrapMode wm,
                                                                     bool convolveAlpha,
                                                                     const GrCaps& caps) {
    GrSamplerState sampler(wm, GrSamplerState::Filter::kNearest);
    auto child = GrTextureEffect::MakeSubset(std::move(srcView), kPremul_SkAlphaType, SkMatrix::I(),
                                             sampler, SkRect::Make(srcBounds), caps);
    return std::unique_ptr<GrFragmentProcessor>(new GrMatrixConvolutionEffect(
            std::move(child), kernelSize, kernel, gain, bias, kernelOffset, convolveAlpha));
}

std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::MakeGaussian(
        GrSurfaceProxyView srcView,
        const SkIRect& srcBounds,
        const SkISize& kernelSize,
        SkScalar gain,
        SkScalar bias,
        const SkIPoint& kernelOffset,
        GrSamplerState::WrapMode wm,
        bool convolveAlpha,
        SkScalar sigmaX,
        SkScalar sigmaY,
        const GrCaps& caps) {
    float kernel[MAX_KERNEL_SIZE];

    fill_in_2D_gaussian_kernel(kernel, kernelSize.width(), kernelSize.height(), sigmaX, sigmaY);
    return Make(std::move(srcView), srcBounds, kernelSize, kernel, gain, bias, kernelOffset, wm,
                convolveAlpha, caps);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMatrixConvolutionEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();

    int width = d->fRandom->nextRangeU(1, MAX_KERNEL_SIZE);
    int height = d->fRandom->nextRangeU(1, MAX_KERNEL_SIZE / width);
    SkISize kernelSize = SkISize::Make(width, height);
    std::unique_ptr<SkScalar[]> kernel(new SkScalar[width * height]);
    for (int i = 0; i < width * height; i++) {
        kernel.get()[i] = d->fRandom->nextSScalar1();
    }
    SkScalar gain = d->fRandom->nextSScalar1();
    SkScalar bias = d->fRandom->nextSScalar1();

    uint32_t kernalOffsetX = d->fRandom->nextRangeU(0, kernelSize.width());
    uint32_t kernalOffsetY = d->fRandom->nextRangeU(0, kernelSize.height());
    SkIPoint kernelOffset = SkIPoint::Make(kernalOffsetX, kernalOffsetY);

    uint32_t boundsX = d->fRandom->nextRangeU(0, view.width());
    uint32_t boundsY = d->fRandom->nextRangeU(0, view.height());
    uint32_t boundsW = d->fRandom->nextRangeU(0, view.width());
    uint32_t boundsH = d->fRandom->nextRangeU(0, view.height());
    SkIRect bounds = SkIRect::MakeXYWH(boundsX, boundsY, boundsW, boundsH);

    auto wm = static_cast<GrSamplerState::WrapMode>(
            d->fRandom->nextULessThan(GrSamplerState::kWrapModeCount));
    bool convolveAlpha = d->fRandom->nextBool();

    return GrMatrixConvolutionEffect::Make(std::move(view),
                                           bounds,
                                           kernelSize,
                                           kernel.get(),
                                           gain,
                                           bias,
                                           kernelOffset,
                                           wm,
                                           convolveAlpha,
                                           *d->caps());
}
#endif
