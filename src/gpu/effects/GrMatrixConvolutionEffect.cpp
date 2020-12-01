/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/effects/GrMatrixConvolutionEffect.h"

#include "include/private/SkHalf.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/dsl/DSL.h"

class GrGLMatrixConvolutionEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    void emitKernelBlock(EmitArgs&, SkIPoint, SkSL::dsl::Var& kernelVar, SkSL::dsl::Var& coord,
                         SkSL::dsl::Var& sum);

    UniformHandle               fKernelUni;
    UniformHandle               fKernelOffsetUni;
    UniformHandle               fGainUni;
    UniformHandle               fBiasUni;
    UniformHandle               fKernelBiasUni;

    using INHERITED = GrGLSLFragmentProcessor;
};

GrMatrixConvolutionEffect::KernelWrapper::MakeResult
GrMatrixConvolutionEffect::KernelWrapper::Make(GrRecordingContext* rContext,
                                               SkISize size,
                                               const GrCaps& caps,
                                               const SkScalar* values) {
    if (!rContext || !values || size.isEmpty()) {
        return {};
    }

    const int length = size.area();
    // Small kernel -> just fill the array.
    KernelWrapper result(size);
    if (length <= kMaxUniformSize) {
        for (int i = 0; i < length; i++) {
            result.fArray[i] = SkScalarToFloat(values[i]);
        }
        return {result, nullptr};
    }

    BiasAndGain& scalableSampler = result.fBiasAndGain;
    bool useA16 =
        rContext->defaultBackendFormat(kA16_float_SkColorType, GrRenderable::kNo).isValid();
    SkScalar min = values[0];
    if (!useA16) {
        // Determine min and max values to figure out inner gain & bias.
        SkScalar max = values[0];
        for (int i = 1; i < length; i++) {
            if (values[i] < min) {
                min = values[i];
            }
            if (values[i] > max) {
                max = values[i];
            }
        }
        // Treat near-0 gain (i.e. box blur) as 1, and let the kernelBias
        // move everything up to the final value.
        const SkScalar computedGain = max - min;
        scalableSampler.fGain =
            SkScalarNearlyZero(computedGain) ? 1.0f : SkScalarToFloat(computedGain);
        // Inner bias is pre-inner-gain so we divide that out.
        scalableSampler.fBias = SkScalarToFloat(min) / scalableSampler.fGain;
    }

    // TODO: Pick cache or dont-cache based on observed perf.
    static constexpr bool kCacheKernelTexture = true;

    GrUniqueKey key;
    if (kCacheKernelTexture) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&key, kDomain, length, "Matrix Convolution Kernel");
        // Texture cache key is the exact content of the kernel.
        static_assert(sizeof(float) == 4);
        for (int i = 0; i < length; i++) {
            builder[i] = *(const uint32_t*)&values[i];
        }
        builder.finish();
    }

    // Find or create a texture.
    auto threadSafeCache = rContext->priv().threadSafeCache();

    SkColorType colorType = useA16 ? kA16_float_SkColorType : kAlpha_8_SkColorType;

    GrSurfaceProxyView view;
    if (kCacheKernelTexture && (view = threadSafeCache->find(key))) {
        SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
        auto kernelFP = GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType);
        return {result, std::move(kernelFP)};
    }

    SkBitmap bm;
    auto info = SkImageInfo::Make({length, 1}, colorType, kPremul_SkAlphaType, nullptr);
    if (!bm.tryAllocPixels(info)) {
        return {};
    }
    for (int i = 0; i < length; i++) {
        if (useA16) {
            *bm.getAddr16(i, 0) = SkFloatToHalf(values[i]);
        } else {
            *bm.getAddr8(i, 0) =
                SkScalarRoundToInt((values[i] - min) / scalableSampler.fGain * 255);
        }
    }
    bm.setImmutable();

    GrBitmapTextureMaker maker(rContext, bm, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
    view = maker.view(GrMipmapped::kNo);
    if (!view) {
        return {};
    }

    if (kCacheKernelTexture) {
        view = threadSafeCache->add(key, view);
    }

    SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
    auto kernelFP = GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType);
    return {result, std::move(kernelFP)};
}

bool GrMatrixConvolutionEffect::KernelWrapper::operator==(const KernelWrapper& k) const {
    if (fSize != k.fSize) {
        return false;
    } else if (this->isSampled()) {
        return fBiasAndGain == k.fBiasAndGain;
    } else {
        return std::equal(fArray.begin(), fArray.begin() + fSize.area(), k.fArray.begin());
    }
}

bool GrMatrixConvolutionEffect::KernelWrapper::BiasAndGain::operator==(
                                                                const BiasAndGain& k) const {
    return fGain == k.fGain && fBias == k.fBias;
}

// For sampled kernels, emit a for loop that does all the kernel accumulation.
// For uniform kernels, emit a single iteration. Function is called repeatedly in a for loop.
// loc is ignored for sampled kernels.
void GrGLMatrixConvolutionEffect::emitKernelBlock(EmitArgs& args, SkIPoint loc,
                                                  SkSL::dsl::Var& kernelVar, SkSL::dsl::Var& coord,
                                                  SkSL::dsl::Var& sum) {
    const GrMatrixConvolutionEffect& mce = args.fFp.cast<GrMatrixConvolutionEffect>();
    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    int kernelWidth = mce.kernelSize().width();
    int kernelHeight = mce.kernelSize().height();
    int kernelArea = kernelWidth * kernelHeight;

    using namespace SkSL::dsl;
    SkSL::StatementArray body;

    Block block;
    Var i(kInt, "i");
    Var k(kHalf, "k");
    Var sourceOffset(kHalf2, "sourceOffset");
    block.append(Declare(k));
    block.append(Declare(sourceOffset));
    if (mce.kernelIsSampled()) {
        block.append(k = sampleChild(1, Float2(Float(i) + 0.5, 0.5)).w() + kernelVar);
        block.append(sourceOffset.y() = floor(i / kernelWidth));
        block.append(sourceOffset.x() = i - sourceOffset.y() * kernelWidth);
    } else {
        block.append(sourceOffset = Half2(loc.x(), loc.y()));
        int offset = loc.y() * kernelWidth + loc.x();
        static constexpr SwizzleComponent kComponent[4] = { X, Y, Z, W };
        block.append(k = Swizzle(kernelVar[offset / 4], kComponent[offset & 0x3]));
    }

    Var c(kHalf4, "c");
    block.append(Declare(c, sampleChild(0, coord + sourceOffset)));
    if (!mce.convolveAlpha()) {
        block.append(c = unpremul(c));
        block.append(c.rgb() = saturate(c.rgb()));
    }
    block.append(sum += c * k);

    if (mce.kernelIsSampled()) {
        fragBuilder->codeAppend(For(Declare(i, 0), i < kernelArea, ++i,
                                    Statement(std::move(block))));
    } else {
        fragBuilder->codeAppend(std::move(block));
    }
}

void GrGLMatrixConvolutionEffect::emitCode(EmitArgs& args) {
    const GrMatrixConvolutionEffect& mce = args.fFp.cast<GrMatrixConvolutionEffect>();

    int kernelWidth = mce.kernelSize().width();
    int kernelHeight = mce.kernelSize().height();

    using namespace SkSL::dsl;
    Start(this, &args);
    Var kernelOffset(Modifiers::kUniform_Flag, kHalf2, "kernelOffset");
    fKernelOffsetUni = kernelOffset.uniformHandle();
    Var gain(Modifiers::kUniform_Flag, kHalf, "gain");
    fGainUni = gain.uniformHandle();
    Var bias(Modifiers::kUniform_Flag, kHalf, "bias");
    fBiasUni = bias.uniformHandle();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    Var sum(kHalf4, "sum");
    fragBuilder->codeAppend(Declare(sum, Half4(0, 0, 0, 0)));
    Var coord(kFloat2, "coord");
    fragBuilder->codeAppend(Declare(coord, sk_SampleCoord - kernelOffset));

    if (mce.kernelIsSampled()) {
        Var kernelBias(Modifiers::kUniform_Flag, kHalf, "kernelBias");
        fKernelBiasUni = kernelBias.uniformHandle();
        this->emitKernelBlock(args, {}, kernelBias, coord, sum);
    } else {
        int arrayCount = (kernelWidth * kernelHeight + 3) / 4;
        SkASSERT(4 * arrayCount >= kernelWidth * kernelHeight);
        Var kernel(Modifiers::kUniform_Flag, Array(kHalf4, arrayCount), "kernel");
        fKernelUni = kernel.uniformHandle();
        for (int x = 0; x < kernelWidth; ++x) {
            for (int y = 0; y < kernelHeight; ++y) {
                this->emitKernelBlock(args, SkIPoint::Make(x, y), kernel, coord, sum);
            }
        }
    }

    if (mce.convolveAlpha()) {
        fragBuilder->codeAppend(sk_OutColor = sum * gain + bias);
        fragBuilder->codeAppend(sk_OutColor.a() = saturate(sk_OutColor.a()));
        fragBuilder->codeAppend(sk_OutColor.rgb() = clamp(sk_OutColor.rgb(), 0.0, sk_OutColor.a()));
    } else {
        auto sample = this->invokeChild(0, args);
        Var c(kHalf4, "c");
        fragBuilder->codeAppend(Declare(c,sampleChild(0)));
        fragBuilder->codeAppend(sk_OutColor.a() = c.a());
        fragBuilder->codeAppend(sk_OutColor.rgb() = saturate(sum.rgb() * gain + bias));
        fragBuilder->codeAppend(sk_OutColor.rgb() *= sk_OutColor.a());
    }
    fragBuilder->codeAppend(sk_OutColor *= sk_InColor);
    End();
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
    pdman.set2f(fKernelOffsetUni, conv.kernelOffset().fX, conv.kernelOffset().fY);
    float totalGain = conv.gain();
    if (conv.kernelIsSampled()) {
        totalGain *= conv.kernelSampleGain();
        pdman.set1f(fKernelBiasUni, conv.kernelSampleBias());
    } else {
        int kernelCount = conv.kernelSize().area();
        int arrayCount = (kernelCount + 3) / 4;
        SkASSERT(4 * arrayCount >= kernelCount);
        pdman.set4fv(fKernelUni, arrayCount, conv.kernel());
    }
    pdman.set1f(fBiasUni, conv.bias());
    pdman.set1f(fGainUni, totalGain);
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(std::unique_ptr<GrFragmentProcessor> child,
                                                     const KernelWrapper& kernel,
                                                     std::unique_ptr<GrFragmentProcessor> kernelFP,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     bool convolveAlpha)
        // To advertise either the modulation or opaqueness optimizations we'd have to examine the
        // parameters.
        : INHERITED(kGrMatrixConvolutionEffect_ClassID, kNone_OptimizationFlags)
        , fKernel(kernel)
        , fGain(SkScalarToFloat(gain))
        , fBias(SkScalarToFloat(bias) / 255.0f)
        , fConvolveAlpha(convolveAlpha) {
    this->registerChild(std::move(child), SkSL::SampleUsage::Explicit());
    this->registerChild(std::move(kernelFP), SkSL::SampleUsage::Explicit());
    fKernelOffset = {static_cast<float>(kernelOffset.x()),
                     static_cast<float>(kernelOffset.y())};
    this->setUsesSampleCoordsDirectly();
}

GrMatrixConvolutionEffect::GrMatrixConvolutionEffect(const GrMatrixConvolutionEffect& that)
        : INHERITED(kGrMatrixConvolutionEffect_ClassID, kNone_OptimizationFlags)
        , fKernel(that.fKernel)
        , fGain(that.fGain)
        , fBias(that.fBias)
        , fKernelOffset(that.fKernelOffset)
        , fConvolveAlpha(that.fConvolveAlpha) {
    this->cloneAndRegisterAllChildProcessors(that);
    this->setUsesSampleCoordsDirectly();
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
    return fKernel == s.fKernel &&
           fGain == s.gain() &&
           fBias == s.bias() &&
           fKernelOffset == s.kernelOffset() &&
           fConvolveAlpha == s.convolveAlpha();
}

std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::Make(GrRecordingContext* context,
                                                                     GrSurfaceProxyView srcView,
                                                                     const SkIRect& srcBounds,
                                                                     const SkISize& kernelSize,
                                                                     const SkScalar* kernel,
                                                                     SkScalar gain,
                                                                     SkScalar bias,
                                                                     const SkIPoint& kernelOffset,
                                                                     GrSamplerState::WrapMode wm,
                                                                     bool convolveAlpha,
                                                                     const GrCaps& caps) {
    auto [kernelWrapper, kernelFP] = KernelWrapper::Make(context, kernelSize, caps, kernel);
    if (!kernelWrapper.isValid()) {
        return nullptr;
    }
    GrSamplerState sampler(wm, GrSamplerState::Filter::kNearest);
    auto child = GrTextureEffect::MakeSubset(std::move(srcView), kPremul_SkAlphaType, SkMatrix::I(),
                                             sampler, SkRect::Make(srcBounds), caps);
    return std::unique_ptr<GrFragmentProcessor>(
            new GrMatrixConvolutionEffect(std::move(child), kernelWrapper, std::move(kernelFP),
                                          gain, bias, kernelOffset, convolveAlpha));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrMatrixConvolutionEffect);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrMatrixConvolutionEffect::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();

    static constexpr size_t kMaxTestKernelSize = 2 * kMaxUniformSize;
    int width = d->fRandom->nextRangeU(1, kMaxTestKernelSize);
    int height = d->fRandom->nextRangeU(1, kMaxTestKernelSize / width);
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
    return GrMatrixConvolutionEffect::Make(d->context(),
                                           std::move(view),
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
