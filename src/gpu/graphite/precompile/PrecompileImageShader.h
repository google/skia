/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileImageShader_DEFINED
#define skgpu_graphite_precompile_PrecompileImageShader_DEFINED

#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

class PrecompileImageShader final : public PrecompileShader {
public:
    PrecompileImageShader(SkEnumBitMask<PrecompileShaders::ImageShaderFlags> flags,
                          SkSpan<const SkColorInfo> colorInfos,
                          SkSpan<const SkTileMode> tileModes,
                          bool raw);

    void setImmutableSamplerInfo(const ImmutableSamplerInfo& samplerInfo);

private:
    friend class PrecompileYUVImageShader; // for NonAlphaOnlyDefaultColorInfos

    // In addition to the tile mode options provided by the client, we can precompile two additional
    // sampling/tiling variants: hardware-tiled and cubic sampling (which always uses the most
    // generic tiling shader).
    inline static constexpr int kExtraNumSamplingTilingCombos = 2;
    inline static constexpr int kCubicSampled = 1;
    inline static constexpr int kHWTiled      = 0;

    // These color info objects are defined assuming an sRGB destination.
    // Most specialized color space transform shader, no actual color space handling.
    static SkColorInfo DefaultColorInfoPremul() {
        return { kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB() };
    }
    // sRGB-to-sRGB specialized color space transform shader.
    static SkColorInfo DefaultColorInfoSRGB() {
        return { kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                 sk_srgb_singleton()->makeColorSpin() };
    }
    // Most general color space transform shader.
    static SkColorInfo DefaultColorInfoGeneral() {
        return { kRGBA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear() };
    }
    // Alpha-only, most general color space transform shader.
    static SkColorInfo DefaultColorInfoAlphaOnly() {
        return { kAlpha_8_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear() };
    }

    // A fixed list of SkColorInfos that will trigger each possible combination of alpha-only
    // handling and color space transform variants, when drawn to an sRGB destination.
    static std::vector<SkColorInfo> DefaultColorInfos() {
        return { DefaultColorInfoPremul(), DefaultColorInfoSRGB(), DefaultColorInfoGeneral(),
                 DefaultColorInfoAlphaOnly() };
    }
    // A fixed list of SkColorInfos that will trigger each color space transform shader variant when
    // drawn to an sRGB destination.
    static std::vector<SkColorInfo> NonAlphaOnlyDefaultColorInfos() {
        return { DefaultColorInfoPremul(), DefaultColorInfoSRGB(), DefaultColorInfoGeneral() };
    }
    // A fixed list of SkColorInfos that will trigger each color space transform shader variant
    // possible from a raw image draw. The general shader is still required if the image is
    // alpha-only, because the read swizzle is implemented as a gamut transformation.
    static std::vector<SkColorInfo> RawImageDefaultColorInfos() {
        return { DefaultColorInfoPremul(), DefaultColorInfoAlphaOnly() };
    }

    const int fNumExtraSamplingTilingCombos;

    const std::vector<SkColorInfo> fColorInfos;
    const std::vector<SkTileMode> fTileModes;

    // If true, use the destination color space from the KeyContext provided to addToKey.
    // This is true if and only if the client has provided a list of color infos. Otherwise, we
    // always use an sRGB destination per the default SkColorInfo lists defined above.
    const bool fUseDstColorSpace;

    // Whether this precompiles raw image shaders.
    const bool fRaw;
    ImmutableSamplerInfo fImmutableSamplerInfo;

    int numIntrinsicCombinations() const override;

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileImageShader_DEFINED
