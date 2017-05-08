/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMatrixConvolutionEffect_DEFINED
#define GrMatrixConvolutionEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrTextureDomain.h"

// A little bit less than the minimum # uniforms required by DX9SM2 (32).
// Allows for a 5x5 kernel (or 25x1, for that matter).
#define MAX_KERNEL_SIZE 25

class GrMatrixConvolutionEffect : public GrSingleTextureEffect {
public:
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           const SkIRect& bounds,
                                           const SkISize& kernelSize,
                                           const SkScalar* kernel,
                                           SkScalar gain,
                                           SkScalar bias,
                                           const SkIPoint& kernelOffset,
                                           GrTextureDomain::Mode tileMode,
                                           bool convolveAlpha) {
        return sk_sp<GrFragmentProcessor>(
            new GrMatrixConvolutionEffect(resourceProvider, std::move(proxy), bounds, kernelSize,
                                          kernel, gain, bias, kernelOffset, tileMode, convolveAlpha));
    }

    static sk_sp<GrFragmentProcessor> MakeGaussian(GrResourceProvider*,
                                                   sk_sp<GrTextureProxy> proxy,
                                                   const SkIRect& bounds,
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

private:
    GrMatrixConvolutionEffect(GrResourceProvider*,
                              sk_sp<GrTextureProxy> proxy,
                              const SkIRect& bounds,
                              const SkISize& kernelSize,
                              const SkScalar* kernel,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& kernelOffset,
                              GrTextureDomain::Mode tileMode,
                              bool convolveAlpha);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    SkIRect         fBounds;
    SkISize         fKernelSize;
    float           fKernel[MAX_KERNEL_SIZE];
    float           fGain;
    float           fBias;
    float           fKernelOffset[2];
    bool            fConvolveAlpha;
    GrTextureDomain fDomain;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
