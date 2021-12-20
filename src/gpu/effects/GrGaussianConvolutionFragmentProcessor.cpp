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
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/dsl/priv/DSLFPs.h"

// For brevity
using UniformHandle = GrGLSLProgramDataManager::UniformHandle;
using Direction = GrGaussianConvolutionFragmentProcessor::Direction;

class GrGaussianConvolutionFragmentProcessor::Impl : public ProgramImpl {
public:
    void emitCode(EmitArgs&) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    UniformHandle fKernelUni;
    UniformHandle fOffsetsUni;
    UniformHandle fKernelWidthUni;
    UniformHandle fIncrementUni;
};

enum class LoopType {
    kUnrolled,
    kFixedLength,
    kVariableLength,
};

static LoopType loop_type(const GrShaderCaps& caps) {
    // This checks that bitwise integer operations and array indexing by non-consts are allowed.
    if (caps.generation() < SkSL::GLSLGeneration::k130) {
        return LoopType::kUnrolled;
    }
    // If we're in reduced shader mode and we can have a loop then use a uniform to limit the
    // number of iterations so we don't need a code variation for each width.
    return caps.reducedShaderMode() ? LoopType::kVariableLength : LoopType::kFixedLength;
}

void GrGaussianConvolutionFragmentProcessor::Impl::emitCode(EmitArgs& args) {
    const GrGaussianConvolutionFragmentProcessor& ce =
            args.fFp.cast<GrGaussianConvolutionFragmentProcessor>();

    using namespace SkSL::dsl;
    StartFragmentProcessor(this, &args);
    GlobalVar increment(kUniform_Modifier, kHalf2_Type, "Increment");
    Declare(increment);
    fIncrementUni = VarUniformHandle(increment);

    int width = SkGpuBlurUtils::LinearKernelWidth(ce.fRadius);

    LoopType loopType = loop_type(*args.fShaderCaps);

    int arrayCount;
    if (loopType == LoopType::kVariableLength) {
        // Size the kernel uniform for the maximum width.
        arrayCount = (SkGpuBlurUtils::LinearKernelWidth(kMaxKernelRadius) + 3) / 4;
    } else {
        arrayCount = (width + 3) / 4;
        SkASSERT(4 * arrayCount >= width);
    }

    GlobalVar kernel(kUniform_Modifier, Array(kHalf4_Type, arrayCount), "Kernel");
    Declare(kernel);
    fKernelUni = VarUniformHandle(kernel);


    GlobalVar offsets(kUniform_Modifier, Array(kHalf4_Type, arrayCount), "Offsets");
    Declare(offsets);
    fOffsetsUni = VarUniformHandle(offsets);

    Var color(kHalf4_Type, "color", Half4(0));
    Declare(color);

    Var coord(kFloat2_Type, "coord", sk_SampleCoord());
    Declare(coord);

    switch (loopType) {
        case LoopType::kUnrolled:
            for (int i = 0; i < width; i++) {
                color += SampleChild(/*index=*/0, coord + offsets[i / 4][i & 3] * increment) *
                         kernel[i / 4][i & 0x3];
            }
            break;
        case LoopType::kFixedLength: {
            Var i(kInt_Type, "i", 0);
            For(Declare(i), i < width, i++,
                color += SampleChild(/*index=*/0, coord + offsets[i / 4][i & 3] * increment) *
                         kernel[i / 4][i & 0x3]);
            break;
        }
        case LoopType::kVariableLength: {
            GlobalVar kernelWidth(kUniform_Modifier, kInt_Type, "kernelWidth");
            Declare(kernelWidth);
            fKernelWidthUni = VarUniformHandle(kernelWidth);
            Var i(kInt_Type, "i", 0);
            For(Declare(i), i < kernelWidth, i++,
                color += SampleChild(/*index=*/0, coord + offsets[i / 4][i & 3] * increment) *
                         kernel[i / 4][i & 0x3]);
            break;
        }
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

    int width = SkGpuBlurUtils::LinearKernelWidth(conv.fRadius);
    int arrayCount = (width + 3)/4;
    SkDEBUGCODE(size_t arraySize = 4*arrayCount;)
    SkASSERT(arraySize >= static_cast<size_t>(width));
    SkASSERT(arraySize <= SK_ARRAY_COUNT(GrGaussianConvolutionFragmentProcessor::fKernel));
    pdman.set4fv(fKernelUni, arrayCount, conv.fKernel);
    pdman.set4fv(fOffsetsUni, arrayCount, conv.fOffsets);
    if (fKernelWidthUni.isValid()) {
        pdman.set1i(fKernelWidthUni, width);
    }
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
    bool is_zero_sigma = SkGpuBlurUtils::IsEffectivelyZeroSigma(gaussianSigma);
    // We should sample as nearest if there will be no shader to preserve existing behaviour, but
    // the linear blur requires a linear sample.
    GrSamplerState::Filter filter = is_zero_sigma ?
        GrSamplerState::Filter::kNearest : GrSamplerState::Filter::kLinear;
    GrSamplerState sampler(wm, filter);
    if (is_zero_sigma) {
        halfWidth = 0;
    }
    // It's pretty common to blur a subset of an input texture. In reduced shader mode we always
    // apply the wrap mode in the shader.
    bool alwaysUseShaderTileMode = caps.reducedShaderMode();
    if (pixelDomain && !alwaysUseShaderTileMode) {
        // Inset because we expect to be invoked at pixel centers.
        SkRect domain = SkRect::Make(*pixelDomain).makeInset(0.5, 0.5f);
        switch (dir) {
            case Direction::kX: domain.outset(halfWidth, 0); break;
            case Direction::kY: domain.outset(0, halfWidth); break;
        }
        child = GrTextureEffect::MakeSubset(std::move(view),
                                            alphaType,
                                            SkMatrix::I(),
                                            sampler,
                                            SkRect::Make(subset),
                                            domain,
                                            caps,
                                            GrTextureEffect::kDefaultBorder);
    } else {
        child = GrTextureEffect::MakeSubset(std::move(view),
                                            alphaType,
                                            SkMatrix::I(),
                                            sampler,
                                            SkRect::Make(subset),
                                            caps,
                                            GrTextureEffect::kDefaultBorder,
                                            alwaysUseShaderTileMode);
    }

    if (is_zero_sigma) {
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
    SkGpuBlurUtils::Compute1DLinearGaussianKernel(fKernel, fOffsets, gaussianSigma, fRadius);
    this->setUsesSampleCoordsDirectly();
}

GrGaussianConvolutionFragmentProcessor::GrGaussianConvolutionFragmentProcessor(
        const GrGaussianConvolutionFragmentProcessor& that)
        : INHERITED(that)
        , fRadius(that.fRadius)
        , fDirection(that.fDirection) {
    memcpy(fKernel, that.fKernel, SkGpuBlurUtils::LinearKernelWidth(fRadius) * sizeof(float));
    memcpy(fOffsets, that.fOffsets, SkGpuBlurUtils::LinearKernelWidth(fRadius) * sizeof(float));
}

void GrGaussianConvolutionFragmentProcessor::onAddToKey(const GrShaderCaps& shaderCaps,
                                                        skgpu::KeyBuilder* b) const {
    if (loop_type(shaderCaps) != LoopType::kVariableLength) {
        b->add32(fRadius);
    }
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl>
GrGaussianConvolutionFragmentProcessor::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

bool GrGaussianConvolutionFragmentProcessor::onIsEqual(const GrFragmentProcessor& sBase) const {
    const auto& that = sBase.cast<GrGaussianConvolutionFragmentProcessor>();
    return fRadius == that.fRadius && fDirection == that.fDirection &&
           std::equal(fKernel, fKernel + SkGpuBlurUtils::LinearKernelWidth(fRadius), that.fKernel) &&
           std::equal(fOffsets, fOffsets + SkGpuBlurUtils::LinearKernelWidth(fRadius), that.fOffsets);
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
