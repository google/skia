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
    static std::unique_ptr<GrFragmentProcessor> Make(GrRecordingContext *context,
                                                     sk_sp<GrSurfaceProxy> srcProxy,
                                                     const SkIRect& srcBounds,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     GrTextureDomain::Mode tileMode,
                                                     bool convolveAlpha) {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrMatrixConvolutionEffect(context, std::move(srcProxy), srcBounds, kernelSize, kernel,
                                              gain, bias, kernelOffset, tileMode, convolveAlpha));
    }

    static std::unique_ptr<GrFragmentProcessor> MakeGaussian(GrRecordingContext *context,
                                                             sk_sp<GrTextureProxy> srcProxy,
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
    // For large kernels that we texture-sample, kernelBias gets us from [0,1] to the
    // kernel's range, before applying the gain (all gains are stacked and applied at once.)
    float kernelBias() const { return fKernelBias; }
    // `bias` is applied to final color, passed in from outside.
    float bias() const { return fBias; }
    bool convolveAlpha() const { return fConvolveAlpha; }
    const GrTextureDomain& sourceDomain() const { return fSourceDomain; }

    const char* name() const override { return "MatrixConvolution"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    // srcProxy is the texture that is going to be convolved
    // srcBounds is the subset of 'srcProxy' that will be used (e.g., for clamp mode)
    // bias is normalized float
    GrMatrixConvolutionEffect(GrRecordingContext *context,
                              sk_sp<GrSurfaceProxy> srcProxy,
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

    const TextureSampler& onTextureSampler(int i) const override { return IthTextureSampler(i, fSourceSampler, fKernelSampler); }

    GrCoordTransform     fCoordTransform;
    GrTextureDomain      fSourceDomain;
    TextureSampler       fSourceSampler;
    std::vector<uint8_t> fKernelPixels;            // only used for large kernels
    TextureSampler       fKernelSampler;           // only used for large kernels
    SkIRect              fBounds;
    SkISize              fKernelSize;
    float                fKernel[MAX_KERNEL_SIZE]; // only used for small kernels
    float                fGain;
    float                fBias;
    float                fKernelBias;              // only used for large kernels
    float                fKernelOffset[2];
    bool                 fConvolveAlpha;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

#endif
