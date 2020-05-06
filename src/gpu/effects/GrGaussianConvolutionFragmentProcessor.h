/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGaussianConvolutionFragmentProcessor_DEFINED
#define GrGaussianConvolutionFragmentProcessor_DEFINED

#include "src/gpu/GrCoordTransform.h"
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
     * axis indicated by Direction. The WrapMode is applied to the bounds interval. If bounds is
     * nullptr then the full proxy width/height is used.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView view,
                                                     SkAlphaType alphaType,
                                                     Direction dir,
                                                     int halfWidth,
                                                     float gaussianSigma,
                                                     GrSamplerState::WrapMode,
                                                     const int bounds[2],
                                                     const GrCaps& caps);

    const float* kernel() const { return fKernel; }

    int radius() const { return fRadius; }
    int width() const { return 2 * fRadius + 1; }
    Direction direction() const { return fDirection; }

    const char* name() const override { return "GaussianConvolution"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("dir: %s radius: %d", Direction::kX == fDirection ? "X" : "Y", fRadius);
        return str;
    }
#endif

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrGaussianConvolutionFragmentProcessor(*this));
    }

    // This was decided based on the min allowed value for the max texture
    // samples per fragment program run in DX9SM2 (32). A sigma param of 4.0
    // on a blur filter gives a kernel width of 25 while a sigma of 5.0
    // would exceed a 32 wide kernel.
    static const int kMaxKernelRadius = 12;
    // With a C++11 we could have a constexpr version of WidthFromRadius()
    // and not have to duplicate this calculation.
    static const int kMaxKernelWidth = 2 * kMaxKernelRadius + 1;

private:
    GrGaussianConvolutionFragmentProcessor(std::unique_ptr<GrFragmentProcessor>,
                                           Direction,
                                           int halfWidth,
                                           float gaussianSigma);

    explicit GrGaussianConvolutionFragmentProcessor(const GrGaussianConvolutionFragmentProcessor&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    // We really just want the unaltered local coords, but the only way to get that right now is
    // an identity coord transform.
    GrCoordTransform      fCoordTransform = {};
    // TODO: Inline the kernel constants into the generated shader code. This may involve pulling
    // some of the logic from SkGpuBlurUtils into this class related to radius/sigma calculations.
    float                 fKernel[kMaxKernelWidth];
    int                   fRadius;
    Direction             fDirection;

    typedef GrFragmentProcessor INHERITED;
};

#endif
