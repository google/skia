/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrGaussianConvolutionFragmentProcessor.h"

#include "src/core/SkGpuBlurUtils.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

// For brevity
using UniformHandle = GrGLSLProgramDataManager::UniformHandle;
using Direction = GrGaussianConvolutionFragmentProcessor::Direction;

class GrGaussianConvolutionFragmentProcessor::Impl : public ProgramImpl {
public:
    void emitCode(EmitArgs&) override;

private:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

    UniformHandle fOffsetsAndKernelUni;
    UniformHandle fKernelWidthUni;
    UniformHandle fIncrementUni;
};

static bool should_use_variable_length_loop(const GrShaderCaps& caps) {
    // If we're in reduced-shader mode, and we can use variable length loops, then use a uniform to
    // limit the number of iterations, so we don't need a code variation for each width.
    return (caps.fGLSLGeneration >= SkSL::GLSLGeneration::k300es && caps.fReducedShaderMode);
}

void GrGaussianConvolutionFragmentProcessor::Impl::emitCode(EmitArgs& args) {
    const GrGaussianConvolutionFragmentProcessor& ce =
            args.fFp.cast<GrGaussianConvolutionFragmentProcessor>();

    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    const char* increment;
    fIncrementUni = uniformHandler->addUniform(&ce, kFragment_GrShaderFlag, SkSLType::kHalf2,
                                               "Increment", &increment);

    // For variable-length loops, size the kernel uniform for the maximum width so we can reuse the
    // same code for any kernel width.
    bool variableLengthLoop = should_use_variable_length_loop(*args.fShaderCaps);
    int width = SkGpuBlurUtils::LinearKernelWidth(ce.fRadius);
    int arrayCount = variableLengthLoop ? SkGpuBlurUtils::LinearKernelWidth(kMaxKernelRadius)
                                        : width;

    const char* offsetsAndKernel;
    fOffsetsAndKernelUni = uniformHandler->addUniformArray(&ce, kFragment_GrShaderFlag,
                                                           SkSLType::kHalf2, "OffsetsAndKernel",
                                                           arrayCount, &offsetsAndKernel);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    // Create a "Smooth" helper function that computes one sample from the child using the kernel.
    SkString smoothFuncName = fragBuilder->getMangledFunctionName("Smooth");
    const GrShaderVar smoothArgs[] = {{args.fInputColor,  SkSLType::kHalf4},
                                      {"coord",           SkSLType::kFloat2},
                                      {"offsetAndKernel", SkSLType::kHalf2}};

    std::string childCoord = SkSL::String::printf("(coord + offsetAndKernel.x * %s)", increment);
    SkString sample = this->invokeChild(/*childIndex=*/0, args, childCoord);

    std::string smoothBody = SkSL::String::printf("return %s * offsetAndKernel.y;", sample.c_str());
    fragBuilder->emitFunction(SkSLType::kHalf4, smoothFuncName.c_str(),
                              {smoothArgs, std::size(smoothArgs)},
                              smoothBody.c_str());

    // Implement the main() function.
    fragBuilder->codeAppendf("half4 color = half4(0);"
                             "float2 coord = %s;", args.fSampleCoord);
    if (variableLengthLoop) {
        const char* kernelWidth;
        fKernelWidthUni = uniformHandler->addUniform(&ce, kFragment_GrShaderFlag, SkSLType::kInt,
                                                     "KernelWidth", &kernelWidth);
        fragBuilder->codeAppendf("for (int i=0; i<%s; ++i) {"
                                 "    color += %s(%s, coord, %s[i]);"
                                 "}",
                                 kernelWidth, smoothFuncName.c_str(),
                                 args.fInputColor, offsetsAndKernel);
    } else {
        fragBuilder->codeAppendf("for (int i=0; i<%d; ++i) {"
                                 "    color += %s(%s, coord, %s[i]);"
                                 "}",
                                 width, smoothFuncName.c_str(),
                                 args.fInputColor, offsetsAndKernel);
    }
    fragBuilder->codeAppendf("return color;\n");
}

void GrGaussianConvolutionFragmentProcessor::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                                             const GrFragmentProcessor& processor) {
    const auto& conv = processor.cast<GrGaussianConvolutionFragmentProcessor>();

    float increment[2] = {};
    increment[static_cast<int>(conv.fDirection)] = 1;
    pdman.set2fv(fIncrementUni, 1, increment);

    int kernelWidth = SkGpuBlurUtils::LinearKernelWidth(conv.fRadius);
    SkASSERT(kernelWidth <= kMaxKernelWidth);
    pdman.set2fv(fOffsetsAndKernelUni, kernelWidth, conv.fOffsetsAndKernel[0].ptr());
    if (fKernelWidthUni.isValid()) {
        pdman.set1i(fKernelWidthUni, kernelWidth);
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
    this->setUsesSampleCoordsDirectly();

    // Assemble a gaussian kernel and offset list.
    float kernel[kMaxKernelWidth] = {};
    float offsets[kMaxKernelWidth] = {};
    SkGpuBlurUtils::Compute1DLinearGaussianKernel(kernel, offsets, gaussianSigma, fRadius);

    // Interleave the kernel and offset values into an array of SkV2s.
    for (int index = 0; index < kMaxKernelWidth; ++index) {
        fOffsetsAndKernel[index] = {offsets[index], kernel[index]};
    }
}

GrGaussianConvolutionFragmentProcessor::GrGaussianConvolutionFragmentProcessor(
        const GrGaussianConvolutionFragmentProcessor& that)
        : INHERITED(that)
        , fRadius(that.fRadius)
        , fDirection(that.fDirection) {
    memcpy(fOffsetsAndKernel, that.fOffsetsAndKernel, sizeof(fOffsetsAndKernel));
}

void GrGaussianConvolutionFragmentProcessor::onAddToKey(const GrShaderCaps& shaderCaps,
                                                        skgpu::KeyBuilder* b) const {
    if (!should_use_variable_length_loop(shaderCaps)) {
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
           std::equal(fOffsetsAndKernel,
                      fOffsetsAndKernel + SkGpuBlurUtils::LinearKernelWidth(fRadius),
                      that.fOffsetsAndKernel);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrGaussianConvolutionFragmentProcessor)

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
