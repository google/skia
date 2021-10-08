/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGaussianConvolutionFragmentProcessor_DEFINED
#define GrGaussianConvolutionFragmentProcessor_DEFINED

#include "src/gpu/GrFragmentProcessor.h"

/**
 * A 1D Gaussian convolution effect. The kernel is computed as an array of 2 * half-width weights.
 * Each texel is multiplied by it's weight and summed to determine the filtered color. The output
 * color is set to a modulation of the filtered and input colors.
 */
class GrGaussianConvolutionFragmentProcessor : public GrFragmentProcessor {
public:
    enum class Direction { kX, kY };

    /**
     * Convolve with a Gaussian kernel. Bounds limits the coords sampled by the effect along the
     * axis indicated by Direction. The WrapMode is applied to the subset. If present, the
     * pixelDomain indicates the domain of pixels that this effect will be called with. It should
     * not account for outsetting due to the filter radius, this effect will handle that. It is
     * assumed that the effect is only invoked at pixel centers within the pixelDomain, the
     * effect will optimize for that, and may produce incorrect results if it is not the case. If
     * pixelDomain is null then the effect will work correctly with any sample coordinates.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView,
                                                     SkAlphaType,
                                                     Direction,
                                                     int halfWidth,
                                                     float gaussianSigma,
                                                     GrSamplerState::WrapMode,
                                                     const SkIRect& subset,
                                                     const SkIRect* pixelDomain,
                                                     const GrCaps&);

    const char* name() const override { return "GaussianConvolution"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrGaussianConvolutionFragmentProcessor(*this));
    }

    // This was decided based on the min allowed value for the max texture
    // samples per fragment program run in DX9SM2 (32). A sigma param of 4.0
    // on a blur filter gives a kernel width of 25 while a sigma of 5.0
    // would exceed a 32 wide kernel.
    inline static constexpr int kMaxKernelRadius = 12;

private:
    class Impl;

    GrGaussianConvolutionFragmentProcessor(std::unique_ptr<GrFragmentProcessor>,
                                           Direction,
                                           int halfWidth,
                                           float gaussianSigma);

    explicit GrGaussianConvolutionFragmentProcessor(const GrGaussianConvolutionFragmentProcessor&);

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        return SkStringPrintf("(dir=%s, radius=%d)",
                              Direction::kX == fDirection ? "X" : "Y", fRadius);
    }
#endif

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    inline static constexpr int kMaxKernelWidth = kMaxKernelRadius + 1;

    // The array size must be a multiple of 4 because we pass it as an array of float4 uniform
    // values.
    float                 fKernel[SkAlign4(kMaxKernelWidth)];
    float                 fOffsets[SkAlign4(kMaxKernelWidth)];
    int                   fRadius;
    Direction             fDirection;

    using INHERITED = GrFragmentProcessor;
};

#endif
