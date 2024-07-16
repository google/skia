/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"

#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
class PrecompileYUVImageShader : public PrecompileShader {
public:
    PrecompileYUVImageShader() {}

private:
    // There are 8 intrinsic YUV shaders:
    //  4 tiling modes
    //    HW tiling w/o swizzle
    //    HW tiling w/ swizzle
    //    cubic shader tiling
    //    non-cubic shader tiling
    //  crossed with two postambles:
    //    just premul
    //    full-blown colorSpace transform
    inline static constexpr int kNumTilingModes     = 4;
    inline static constexpr int kHWTiledNoSwizzle   = 3;
    inline static constexpr int kHWTiledWithSwizzle = 2;
    inline static constexpr int kCubicShaderTiled   = 1;
    inline static constexpr int kShaderTiled        = 0;

    inline static constexpr int kNumPostambles       = 2;
    inline static constexpr int kWithColorSpaceXform = 1;
    inline static constexpr int kJustPremul          = 0;

    inline static constexpr int kNumIntrinsicCombinations = kNumTilingModes * kNumPostambles;

    int numIntrinsicCombinations() const override { return kNumIntrinsicCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < kNumIntrinsicCombinations);

        int desiredPostamble = desiredCombination % kNumPostambles;
        int desiredTiling = desiredCombination / kNumPostambles;
        SkASSERT(desiredTiling < kNumTilingModes);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;

        YUVImageShaderBlock::ImageData imgData(desiredTiling == kCubicShaderTiled
                                                                       ? kDefaultCubicSampling
                                                                       : kDefaultSampling,
                                               SkTileMode::kClamp, SkTileMode::kClamp,
                                               /* imgSize= */ { 1, 1 },
                                               /* subset= */ desiredTiling == kShaderTiled
                                                                     ? SkRect::MakeEmpty()
                                                                     : SkRect::MakeWH(1, 1));

        static constexpr SkV4 kRedChannel{ 1.f, 0.f, 0.f, 0.f };
        imgData.fChannelSelect[0] = kRedChannel;
        imgData.fChannelSelect[1] = kRedChannel;
        if (desiredTiling == kHWTiledNoSwizzle) {
            imgData.fChannelSelect[2] = kRedChannel;
        } else {
            // Having a non-red channel selector forces a swizzle
            imgData.fChannelSelect[2] = { 0.f, 1.f, 0.f, 0.f};
        }
        imgData.fChannelSelect[3] = kRedChannel;

        imgData.fYUVtoRGBMatrix.setAll(1, 0, 0, 0, 1, 0, 0, 0, 0);
        imgData.fYUVtoRGBTranslate = { 0, 0, 0 };

        ColorSpaceTransformBlock::ColorSpaceTransformData colorXformData(
                                      skgpu::graphite::ReadSwizzle::kRGBA);

        if (desiredPostamble == kJustPremul) {
            Compose(keyContext, builder, gatherer,
                    /* addInnerToKey= */ [&]() -> void {
                        YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                    },
                    /* addOuterToKey= */ [&]() -> void {
                        builder->addBlock(BuiltInCodeSnippetID::kPremulAlphaColorFilter);
                    });
        } else {
            Compose(keyContext, builder, gatherer,
                    /* addInnerToKey= */ [&]() -> void {
                        YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                    },
                    /* addOuterToKey= */ [&]() -> void {
                        ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                           colorXformData);
                    });
        }

    }
};

sk_sp<PrecompileShader> PrecompileShaders::YUVImage() {
    return PrecompileShaders::LocalMatrix({ sk_make_sp<PrecompileYUVImageShader>() });
}

} // namespace skgpu::graphite
