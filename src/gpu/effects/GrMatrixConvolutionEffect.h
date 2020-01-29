/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMatrixConvolutionEffect_DEFINED
#define GrMatrixConvolutionEffect_DEFINED

#include "src/gpu/effects/GrTextureDomain.h"

// A little bit less than the minimum # uniforms required by DX9SM2 (32).
// Allows for a 5x5 kernel (or 25x1, for that matter).
#define MAX_KERNEL_SIZE 25

class GrMatrixConvolutionEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView srcView,
                                                     const SkIRect& srcBounds,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     GrTextureDomain::Mode tileMode,
                                                     bool convolveAlpha) {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrMatrixConvolutionEffect(std::move(srcView), srcBounds, kernelSize, kernel,
                                              gain, bias, kernelOffset, tileMode, convolveAlpha));
    }

    static std::unique_ptr<GrFragmentProcessor> MakeGaussian(GrSurfaceProxyView srcView,
                                                             const SkIRect& srcBounds,
                                                             const SkISize& kernelSize,
                                                             SkScalar gain,
                                                             SkScalar bias,
                                                             const SkIPoint& kernelOffset,
                                                             GrTextureDomain::Mode tileMode,
                                                             bool convolveAlpha,
                                                             SkScalar sigmaX,
                                                             SkScalar sigmaY);

    const SkIRect& bounds() const { return fBounds; }
    const SkISize& kernelSize() const { return fKernelSize; }
    const float* kernelOffset() const { return fKernelOffset; }
    const float* kernel() const { return fKernel; }
    float gain() const { return fGain; }
    float bias() const { return fBias; }
    bool convolveAlpha() const { return fConvolveAlpha; }
    const GrTextureDomain& domain() const { return fDomain; }

    const char* name() const override { return "MatrixConvolution"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    // srcProxy is the texture that is going to be convolved
    // srcBounds is the subset of 'srcProxy' that will be used (e.g., for clamp mode)
    GrMatrixConvolutionEffect(GrSurfaceProxyView srcView,
                              const SkIRect& srcBounds,
                              const SkISize& kernelSize,
                              const SkScalar* kernel,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& kernelOffset,
                              GrTextureDomain::Mode tileMode,
                              bool convolveAlpha);

    GrMatrixConvolutionEffect(const GrMatrixConvolutionEffect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSampler; }

    GrCoordTransform fCoordTransform;
    GrTextureDomain  fDomain;
    TextureSampler   fTextureSampler;
    SkIRect          fBounds;
    SkISize          fKernelSize;
    float            fKernel[MAX_KERNEL_SIZE];
    float            fGain;
    float            fBias;
    float            fKernelOffset[2];
    bool             fConvolveAlpha;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

#endif
