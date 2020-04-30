/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMatrixConvolutionEffect_DEFINED
#define GrMatrixConvolutionEffect_DEFINED

#include "src/gpu/GrFragmentProcessor.h"
#include <array>
#include <variant>

class GrMatrixConvolutionEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(GrRecordingContext *context,
                                                     GrSurfaceProxyView srcView,
                                                     const SkIRect& srcBounds,
                                                     const SkISize& kernelSize,
                                                     const SkScalar* kernel,
                                                     SkScalar gain,
                                                     SkScalar bias,
                                                     const SkIPoint& kernelOffset,
                                                     GrSamplerState::WrapMode,
                                                     bool convolveAlpha,
                                                     const GrCaps&);

    static std::unique_ptr<GrFragmentProcessor> MakeGaussian(GrRecordingContext *context,
                                                             GrSurfaceProxyView srcView,
                                                             const SkIRect& srcBounds,
                                                             const SkISize& kernelSize,
                                                             SkScalar gain,
                                                             SkScalar bias,
                                                             const SkIPoint& kernelOffset,
                                                             GrSamplerState::WrapMode,
                                                             bool convolveAlpha,
                                                             SkScalar sigmaX,
                                                             SkScalar sigmaY,
                                                             const GrCaps&);

    const SkIRect& bounds() const { return fBounds; }
    const SkISize& kernelSize() const { return fKernel.fSize; }
    const SkV2 kernelOffset() const { return fKernelOffset; }
    bool kernelIsSampled() const { return fKernel.isSampled(); }
    const float *kernel() const { return fKernel.array().data(); }
    float kernelSampleGain() const { return fKernel.scalableSampler().fGain; }
    float kernelSampleBias() const { return fKernel.scalableSampler().fBias; }
    float gain() const { return fGain; }
    float bias() const { return fBias; }
    bool convolveAlpha() const { return fConvolveAlpha; }

    const char* name() const override { return "MatrixConvolution"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    /**
     * Small kernels are represented as float-arrays and are shaded using
     * uniforms. Large kernels go over the argument limit and are sampled.
     */
    struct KernelWrapper {
        // A little bit less than the minimum # uniforms required by DX9SM2 (32).
        // Allows for a 5x5 kernel (or 25x1, for that matter).
        static constexpr int kMaxUniformSize = 25;
        struct ScalableSampler {
            TextureSampler fSampler;
            // Applied before any other math.
            float fBias = 0.0f;
            // Premultiplied in with user gain to save time.
            float fGain = 1.0f;
            bool operator==(const ScalableSampler &) const;
        };

        SkISize fSize = {};
        std::variant<std::array<float, kMaxUniformSize>, ScalableSampler> fData;

        // Fails only if texture creation fails – returns empty wrapper.
        static KernelWrapper TryMake(GrRecordingContext *context,
                                     const SkISize &size, const float *values);
        bool isSampled() const { return fData.index() == 1; }
        const std::array<float, kMaxUniformSize> &array() const {
            return SkASSERT(!isSampled()), std::get<0>(fData);
        }
        const ScalableSampler &scalableSampler() const {
            return SkASSERT(isSampled()), std::get<1>(fData);
        }
        bool operator==(const KernelWrapper &) const;
    };

    GrMatrixConvolutionEffect(std::unique_ptr<GrFragmentProcessor> child,
                              KernelWrapper kernel,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& kernelOffset,
                              bool convolveAlpha);

    explicit GrMatrixConvolutionEffect(const GrMatrixConvolutionEffect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const GrFragmentProcessor::TextureSampler& onTextureSampler(int index) const override;

    // We really just want the unaltered local coords, but the only way to get that right now is
    // an identity coord transform.
    GrCoordTransform fCoordTransform = {};
    SkIRect          fBounds;
    KernelWrapper    fKernel;
    float            fGain;
    float            fBias;
    SkV2             fKernelOffset;
    bool             fConvolveAlpha;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

#endif
