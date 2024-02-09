/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkColorSpace.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <functional>

enum class Mode {
    kMultipleImage,  // Transforms multiple images via one pipeline. (skcms doesn't support this.)
    kSingleImage,    // Transforms a single image at a time.
    kSingleScanline, // Transforms an image scanline-by-scanline.
};

static const char* mode_name(Mode m) {
    switch (m) {
        case Mode::kMultipleImage:  return "MultipleImage";
        case Mode::kSingleImage:    return "SingleImage";
        case Mode::kSingleScanline: return "SingleScanline";
        default:                    SkUNREACHABLE;
    }
}

class ColorSpaceTransformBench : public Benchmark {
public:
    ColorSpaceTransformBench(Mode mode) : fMode(mode) {
        fName = std::string("ColorSpace") + mode_name(fMode) + "Transform";
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void onDelayedSetup() override {
        // Set the image buffer to solid white.
        std::fill(std::begin(fSrcPixels), std::end(fSrcPixels), 0xFF);

        // Create a conversion from sRGB to Adobe.
        fSRGBCS = SkColorSpace::MakeSRGB();
        fAdobeCS = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);

        fXformSteps = SkColorSpaceXformSteps(fSRGBCS.get(),  kPremul_SkAlphaType,
                                             fAdobeCS.get(), kPremul_SkAlphaType);

        // Build up a pipeline.
        fSrcCtx = SkRasterPipeline_MemoryCtx{fSrcPixels, kWidth};
        fDstCtx = SkRasterPipeline_MemoryCtx{fDstPixels, kWidth};

        fPipeline.append(SkRasterPipelineOp::load_8888, &fSrcCtx);
        fXformSteps.apply(&fPipeline);
        fPipeline.append(SkRasterPipelineOp::store_8888, &fDstCtx);

        fConversionPipeline = fPipeline.compile();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        switch (fMode) {
            case Mode::kMultipleImage:
                for (int i = 0; i < loops; i++) {
                    fConversionPipeline(0, 0, kWidth, kHeight);
                }
                break;

            case Mode::kSingleImage:
                for (int i = 0; i < loops; i++) {
                    fPipeline.run(0, 0, kWidth, kHeight);
                }
                break;

            case Mode::kSingleScanline:
                for (int i = 0; i < loops; i++) {
                    for (int y = 0; y < kHeight; ++y) {
                        fPipeline.run(0, y, kWidth, 1);
                    }
                }
                break;
        }
    }

private:
    using INHERITED = Benchmark;

    Mode fMode;
    std::string fName;

    sk_sp<SkColorSpace> fAdobeCS;
    sk_sp<SkColorSpace> fSRGBCS;

    SkRasterPipeline_<256> fPipeline;
    SkRasterPipeline_MemoryCtx fSrcCtx;
    SkRasterPipeline_MemoryCtx fDstCtx;
    SkColorSpaceXformSteps fXformSteps;

    static constexpr int kWidth = 512;
    static constexpr int kHeight = 512;
    static constexpr int kBytesPerPixel = 4;
    uint8_t fSrcPixels[kWidth * kHeight * kBytesPerPixel];
    uint8_t fDstPixels[kWidth * kHeight * kBytesPerPixel];

    std::function<void(size_t, size_t, size_t, size_t)> fConversionPipeline;
};

class SkcmsTransformBench : public Benchmark {
public:
    SkcmsTransformBench(Mode mode) : fMode(mode) {
        fName = std::string("Skcms") + mode_name(fMode) + "Transform";
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    // Call before draw, allows the benchmark to do setup work outside of the
    // timer. When a benchmark is repeatedly drawn, this should be called once
    // before the initial draw.
    void onDelayedSetup() override {
        // Set the image buffer to solid white. NANs/denorms might affect timing, but otherwise it
        // doesn't really matter.
        std::fill(std::begin(fSrcPixels), std::end(fSrcPixels), 0xFF);

        // Create profiles for sRGB to Adobe.
        SkColorSpace::MakeSRGB()->toProfile(&fSRGBProfile);
        SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2,
                              SkNamedGamut::kAdobeRGB)->toProfile(&fAdobeProfile);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        switch (fMode) {
            case Mode::kMultipleImage:
                SkUNREACHABLE;

            case Mode::kSingleImage:
                for (int i = 0; i < loops; i++) {
                    skcms_Transform(fSrcPixels,
                                    skcms_PixelFormat_RGBA_8888,
                                    skcms_AlphaFormat_PremulAsEncoded,
                                    &fSRGBProfile,
                                    fDstPixels,
                                    skcms_PixelFormat_RGBA_8888,
                                    skcms_AlphaFormat_PremulAsEncoded,
                                    &fAdobeProfile,
                                    kWidth * kHeight);
                }
                break;

            case Mode::kSingleScanline:
                for (int i = 0; i < loops; i++) {
                    uint8_t* src = fSrcPixels;
                    uint8_t* dst = fDstPixels;
                    for (int y = 0; y < kHeight; ++y) {
                        skcms_Transform(src,
                                        skcms_PixelFormat_RGBA_8888,
                                        skcms_AlphaFormat_PremulAsEncoded,
                                        &fSRGBProfile,
                                        dst,
                                        skcms_PixelFormat_RGBA_8888,
                                        skcms_AlphaFormat_PremulAsEncoded,
                                        &fAdobeProfile,
                                        kWidth);
                        src += kWidth * kBytesPerPixel;
                        dst += kWidth * kBytesPerPixel;
                    }
                }
                break;
        }
    }

private:
    using INHERITED = Benchmark;

    Mode fMode;
    std::string fName;

    skcms_ICCProfile fAdobeProfile;
    skcms_ICCProfile fSRGBProfile;

    static constexpr int kWidth = 512;
    static constexpr int kHeight = 512;
    static constexpr int kBytesPerPixel = 4;
    uint8_t fSrcPixels[kWidth * kHeight * kBytesPerPixel];
    uint8_t fDstPixels[kWidth * kHeight * kBytesPerPixel];
};

DEF_BENCH(return new ColorSpaceTransformBench(Mode::kMultipleImage);)
DEF_BENCH(return new ColorSpaceTransformBench(Mode::kSingleImage);)
DEF_BENCH(return new ColorSpaceTransformBench(Mode::kSingleScanline);)
DEF_BENCH(return new SkcmsTransformBench(Mode::kSingleImage);)
DEF_BENCH(return new SkcmsTransformBench(Mode::kSingleScanline);)
