/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrGaussianConvolutionFragmentProcessor.h"

#include "src/core/SkGpuBlurUtils.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/dsl/priv/DSLFPs.h"

// For brevity
using UniformHandle = GrGLSLProgramDataManager::UniformHandle;
using Direction = GrGaussianConvolutionFragmentProcessor::Direction;

class GrGaussianConvolutionFragmentProcessor::Impl : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

private:
    UniformHandle fKernelUni;
    UniformHandle fIncrementUni;

    using INHERITED = GrGLSLFragmentProcessor;
};

void GrGaussianConvolutionFragmentProcessor::Impl::emitCode(EmitArgs& args) {
    const GrGaussianConvolutionFragmentProcessor& ce =
            args.fFp.cast<GrGaussianConvolutionFragmentProcessor>();

    using namespace SkSL::dsl;
    StartFragmentProcessor(this, &args);
    Var increment(kUniform_Modifier, kHalf2, "Increment");
    fIncrementUni = VarUniformHandle(increment);

    int width = SkGpuBlurUtils::KernelWidth(ce.fRadius);

    int arrayCount = (width + 3) / 4;
    SkASSERT(4 * arrayCount >= width);

    Var kernel(kUniform_Modifier, Array(kHalf4, arrayCount), "Kernel");
    fKernelUni = VarUniformHandle(kernel);

    Var color(kHalf4, "color", Half4(0));
    Declare(color);

    Var coord(kFloat2, "coord", sk_SampleCoord() - ce.fRadius * increment);
    Declare(coord);

    // Manually unroll loop because some drivers don't; yields 20-30% speedup.
    for (int i = 0; i < width; i++) {
        if (i != 0) {
            coord += increment;
        }
        color += SampleChild(/*index=*/0, coord) * kernel[i / 4][i & 0x3];
    }
    Return(color);
    EndFragmentProcessor();
}

void GrGaussianConvolutionFragmentProcessor::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                                             const GrFragmentProcessor& processor) {
    const auto& conv = processor.cast<GrGaussianConvolutionFragmentProcessor>();

    float increment[2] = {};
    increment[static_cast<int>(conv.fDirection)] = 1;
    pdman.set2fv(fIncrementUni, 1, increment);

    int width = SkGpuBlurUtils::KernelWidth(conv.fRadius);
    int arrayCount = (width + 3)/4;
    SkDEBUGCODE(size_t arraySize = 4*arrayCount;)
    SkASSERT(arraySize >= static_cast<size_t>(width));
    SkASSERT(arraySize <= SK_ARRAY_COUNT(GrGaussianConvolutionFragmentProcessor::fKernel));
    pdman.set4fv(fKernelUni, arrayCount, conv.fKernel);
}

void GrGaussianConvolutionFragmentProcessor::Impl::GenKey(const GrProcessor& processor,
                                                          const GrShaderCaps&,
                                                          GrProcessorKeyBuilder* b) {
    const auto& conv = processor.cast<GrGaussianConvolutionFragmentProcessor>();
    b->add32(conv.fRadius);
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrGaussianConvolutionFragmentProcessor::Make(
        GrSurfaceProxyView view,
        SkAlphaType alphaType,
        Direction dir,
        int halfWidth,
        float gaussianSigma,
        GrSamplerState::WrapMode wm,
        const SkIRect& subset,
        const SkIRect* pixelDomain,
        const GrCaps& caps) {
    std::unique_ptr<GrFragmentProcessor> child;
    GrSamplerState sampler(wm, GrSamplerState::Filter::kNearest);
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(gaussianSigma)) {
        halfWidth = 0;
    }
    if (pixelDomain) {
        // Inset because we expect to be invoked at pixel centers.
        SkRect domain = SkRect::Make(*pixelDomain).makeInset(0.5, 0.5f);
        switch (dir) {
            case Direction::kX: domain.outset(halfWidth, 0); break;
            case Direction::kY: domain.outset(0, halfWidth); break;
        }
        child = GrTextureEffect::MakeSubset(std::move(view), alphaType, SkMatrix::I(), sampler,
                                            SkRect::Make(subset), domain, caps);
    } else {
        child = GrTextureEffect::MakeSubset(std::move(view), alphaType, SkMatrix::I(), sampler,
                                            SkRect::Make(subset), caps);
    }

    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(gaussianSigma)) {
        return child;
    }
    return std::unique_ptr<GrFragmentProcessor>(new GrGaussianConvolutionFragmentProcessor(
            std::move(child), dir, halfWidth, gaussianSigma));
}

GrGaussianConvolutionFragmentProcessor::GrGaussianConvolutionFragmentProcessor(
        std::unique_ptr<GrFragmentProcessor> child,
        Direction direction,
        int radius,
        float gaussianSigma)
        : INHERITED(kGrGaussianConvolutionFragmentProcessor_ClassID,
                    ProcessorOptimizationFlags(child.get()))
        , fRadius(radius)
        , fDirection(direction) {
    this->registerChild(std::move(child), SkSL::SampleUsage::Explicit());
    SkASSERT(radius <= kMaxKernelRadius);
    SkGpuBlurUtils::Compute1DGaussianKernel(fKernel, gaussianSigma, fRadius);
    this->setUsesSampleCoordsDirectly();
}

GrGaussianConvolutionFragmentProcessor::GrGaussianConvolutionFragmentProcessor(
        const GrGaussianConvolutionFragmentProcessor& that)
        : INHERITED(kGrGaussianConvolutionFragmentProcessor_ClassID, that.optimizationFlags())
        , fRadius(that.fRadius)
        , fDirection(that.fDirection) {
    this->cloneAndRegisterAllChildProcessors(that);
    memcpy(fKernel, that.fKernel, SkGpuBlurUtils::KernelWidth(fRadius) * sizeof(float));
    this->setUsesSampleCoordsDirectly();
}

void GrGaussianConvolutionFragmentProcessor::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                                   GrProcessorKeyBuilder* b) const {
    Impl::GenKey(*this, caps, b);
}

std::unique_ptr<GrGLSLFragmentProcessor>
GrGaussianConvolutionFragmentProcessor::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

bool GrGaussianConvolutionFragmentProcessor::onIsEqual(const GrFragmentProcessor& sBase) const {
    const auto& that = sBase.cast<GrGaussianConvolutionFragmentProcessor>();
    return fRadius == that.fRadius && fDirection == that.fDirection &&
           std::equal(fKernel, fKernel + SkGpuBlurUtils::KernelWidth(fRadius), that.fKernel);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrGaussianConvolutionFragmentProcessor);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrGaussianConvolutionFragmentProcessor::TestCreate(
        GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomView();

    Direction dir = d->fRandom->nextBool() ? Direction::kY : Direction::kX;
    SkIRect subset{
            static_cast<int>(d->fRandom->nextRangeU(0, view.width()  - 1)),
            static_cast<int>(d->fRandom->nextRangeU(0, view.height() - 1)),
            static_cast<int>(d->fRandom->nextRangeU(0, view.width()  - 1)),
            static_cast<int>(d->fRandom->nextRangeU(0, view.height() - 1)),
    };
    subset.sort();

    auto wm = static_cast<GrSamplerState::WrapMode>(
            d->fRandom->nextULessThan(GrSamplerState::kWrapModeCount));
    int radius = d->fRandom->nextRangeU(1, kMaxKernelRadius);
    float sigma = radius / 3.f;
    SkIRect temp;
    SkIRect* domain = nullptr;
    if (d->fRandom->nextBool()) {
        temp = {
                static_cast<int>(d->fRandom->nextRangeU(0, view.width()  - 1)),
                static_cast<int>(d->fRandom->nextRangeU(0, view.height() - 1)),
                static_cast<int>(d->fRandom->nextRangeU(0, view.width()  - 1)),
                static_cast<int>(d->fRandom->nextRangeU(0, view.height() - 1)),
        };
        temp.sort();
        domain = &temp;
    }

    return GrGaussianConvolutionFragmentProcessor::Make(std::move(view), at, dir, radius, sigma, wm,
                                                        subset, domain, *d->caps());
}
#endif
