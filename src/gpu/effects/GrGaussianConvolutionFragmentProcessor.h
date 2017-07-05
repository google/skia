/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGaussianConvolutionFragmentProcessor_DEFINED
#define GrGaussianConvolutionFragmentProcessor_DEFINED

#include "Gr1DKernelEffect.h"
#include "GrTextureDomain.h"

/**
 * A 1D Gaussian convolution effect. The kernel is computed as an array of 2 * half-width weights.
 * Each texel is multiplied by it's weight and summed to determine the filtered color. The output
 * color is set to a modulation of the filtered and input colors.
 */
class GrGaussianConvolutionFragmentProcessor : public Gr1DKernelEffect {
public:
    /// Convolve with a Gaussian kernel
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                           Direction dir,
                                           int halfWidth,
                                           float gaussianSigma,
                                           GrTextureDomain::Mode mode,
                                           int* bounds) {
        return sk_sp<GrFragmentProcessor>(new GrGaussianConvolutionFragmentProcessor(
            std::move(proxy), dir, halfWidth, gaussianSigma, mode, bounds));
    }

    ~GrGaussianConvolutionFragmentProcessor() override;

    const float* kernel() const { return fKernel; }

    const int* bounds() const { return fBounds; }
    bool useBounds() const { return fMode != GrTextureDomain::kIgnore_Mode; }

    GrTextureDomain::Mode mode() const { return fMode; }

    const char* name() const override { return "GaussianConvolution"; }

    // This was decided based on the min allowed value for the max texture
    // samples per fragment program run in DX9SM2 (32). A sigma param of 4.0
    // on a blur filter gives a kernel width of 25 while a sigma of 5.0
    // would exceed a 32 wide kernel.
    static const int kMaxKernelRadius = 12;
    // With a C++11 we could have a constexpr version of WidthFromRadius()
    // and not have to duplicate this calculation.
    static const int kMaxKernelWidth = 2 * kMaxKernelRadius + 1;

private:
    /// Convolve with a Gaussian kernel
    GrGaussianConvolutionFragmentProcessor(sk_sp<GrTextureProxy>, Direction,
                                           int halfWidth, float gaussianSigma,
                                           GrTextureDomain::Mode mode, int bounds[2]);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    // TODO: Inline the kernel constants into the generated shader code. This may involve pulling
    // some of the logic from SkGpuBlurUtils into this class related to radius/sigma calculations.
    float fKernel[kMaxKernelWidth];
    int   fBounds[2];
    GrTextureDomain::Mode fMode;

    typedef Gr1DKernelEffect INHERITED;
};

#endif
