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
#include <new>

class GrMatrixConvolutionEffect : public GrFragmentProcessor {
public:
    // A little bit less than the minimum # uniforms required by DX9SM2 (32).
    // Allows for a 5x5 kernel (or 28x1, for that matter).
    // Must be a multiple of 4, since we upload these in vec4s.
    inline static constexpr int kMaxUniformSize = 28;

    static std::unique_ptr<GrFragmentProcessor> Make(GrRecordingContext*,
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

    const char* name() const override { return "MatrixConvolution"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    class Impl;

    /**
     * Small kernels are represented as float-arrays and uploaded as uniforms.
     * Large kernels go over the uniform limit and are uploaded as textures and sampled.
     * If Float16 textures are supported, we use those. Otherwise we use A8.
     */
    class KernelWrapper {
    public:
        struct BiasAndGain {
            // Only used in A8 mode. Applied before any other math.
            float fBias;
            // Only used in A8 mode. Premultiplied in with user gain to save time.
            float fGain;
            bool operator==(const BiasAndGain&) const;
        };
        using MakeResult = std::tuple<KernelWrapper, std::unique_ptr<GrFragmentProcessor>>;
        static MakeResult Make(GrRecordingContext*, SkISize, const GrCaps&, const float* values);

        KernelWrapper() = default;
        KernelWrapper(const KernelWrapper& that) : fSize(that.fSize) {
            if (that.isSampled()) {
                fBiasAndGain = that.fBiasAndGain;
            } else {
                new (&fArray) std::array<float, kMaxUniformSize>(that.fArray);
            }
        }

        bool isValid() const { return !fSize.isEmpty(); }
        SkISize size() const { return fSize; }
        bool isSampled() const { return fSize.area() > kMaxUniformSize; }
        const std::array<float, kMaxUniformSize>& array() const {
            SkASSERT(!this->isSampled());
            return fArray;
        }
        const BiasAndGain& biasAndGain() const {
            SkASSERT(this->isSampled());
            return fBiasAndGain;
        }
        bool operator==(const KernelWrapper&) const;

    private:
        KernelWrapper(SkISize size) : fSize(size) {
            if (this->isSampled()) {
                fBiasAndGain = {0.f , 1.f};
            }
        }

        SkISize fSize = {};
        union {
            std::array<float, kMaxUniformSize> fArray;
            BiasAndGain fBiasAndGain;
        };
    };

    GrMatrixConvolutionEffect(std::unique_ptr<GrFragmentProcessor> child,
                              const KernelWrapper& kernel,
                              std::unique_ptr<GrFragmentProcessor> kernelFP,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& kernelOffset,
                              bool convolveAlpha);

    explicit GrMatrixConvolutionEffect(const GrMatrixConvolutionEffect&);

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    KernelWrapper    fKernel;
    float            fGain;
    float            fBias;
    SkVector         fKernelOffset;
    bool             fConvolveAlpha;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};

#endif
