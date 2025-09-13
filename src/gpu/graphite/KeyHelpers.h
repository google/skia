/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_KeyHelpers_DEFINED
#define skgpu_graphite_KeyHelpers_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/graphite/Context.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

class SkColorFilter;
class SkData;
class SkRuntimeEffect;

namespace skgpu::graphite {

class DrawContext;
class FloatStorageManager;
class PipelineDataGatherer;
class UniquePaintParamsID;
enum class ReadSwizzle;

// Types of logical "destinations" that a blender might blend with.
enum class DstColorType {
    // A color read from the framebuffer.
    kSurface,
    // A color provided by geometry.
    kPrimitive,
    // A color evaluated by a child shader.
    kChildOutput,
};

/**
 * The KeyHelpers can be used to manually construct an SkPaintParamsKey.
 *
 * TODO: If we restructure how the keys are made, we can utilize a single block type for the
 * different blend blocks outlined below. The different Src/Dst pairings could instead be encoded
 * as parent-child relationships.
 */

struct SolidColorShaderBlock {
    static void AddBlock(const KeyContext&, const SkPMColor4f&);
};

struct RGBPaintColorBlock {
    static void AddBlock(const KeyContext&);
};

struct AlphaOnlyPaintColorBlock {
    static void AddBlock(const KeyContext&);
};

struct GradientShaderBlocks {
    struct GradientData {
        // The number of stops stored internal to this data structure before falling back to
        // bitmap storage.
        static constexpr int kNumInternalStorageStops = 8;

        // This ctor is used during pre-compilation when we don't have enough information to
        // extract uniform data. However, we must be able to provide enough data to make all the
        // relevant decisions about which code snippets to use.
        GradientData(SkShaderBase::GradientType, int numStops, bool useStorageBuffer);

        // This ctor is used when extracting information from PaintParams. It must provide
        // enough data to generate the uniform data the selected code snippet will require.
        GradientData(SkShaderBase::GradientType,
                     SkPoint point0, SkPoint point1,
                     float radius0, float radius1,
                     float bias, float scale,
                     SkTileMode,
                     int numStops,
                     const SkPMColor4f* colors,
                     const float* offsets,
                     const SkGradientBaseShader* shader,
                     sk_sp<TextureProxy> colorsAndOffsetsProxy,
                     bool useStorageBuffer,
                     const SkGradientShader::Interpolation&);

        bool operator==(const GradientData& rhs) const = delete;
        bool operator!=(const GradientData& rhs) const = delete;

        // Layout options.
        SkShaderBase::GradientType fType;
        SkPoint                    fPoints[2];
        float                      fRadii[2];

        // Layout options for sweep gradient.
        float                  fBias;
        float                  fScale;

        SkTileMode             fTM;
        int                    fNumStops;
        bool                   fUseStorageBuffer;

        // For gradients w/ <= kNumInternalStorageStops stops we use fColors and fOffsets.
        // The offsets are packed into a single float4 to save space when the layout is std140.
        //
        // Otherwise when storage buffers are preferred, we save the colors and offsets pointers
        // to fSrcColors and fSrcOffsets so we can directly copy to the gatherer gradient buffer,
        // else we pack the data into the fColorsAndOffsetsProxy texture.
        SkPMColor4f                   fColors[kNumInternalStorageStops];
        SkV4                          fOffsets[kNumInternalStorageStops / 4];
        sk_sp<TextureProxy>           fColorsAndOffsetsProxy;
        const SkPMColor4f*            fSrcColors;
        const float*                  fSrcOffsets;
        const SkGradientBaseShader*   fSrcShader;

        SkGradientShader::Interpolation fInterpolation;
    };

    static void AddBlock(const KeyContext&, const GradientData&);
};

struct LocalMatrixShaderBlock {
    struct LMShaderData {
        LMShaderData(const SkMatrix& localMatrix)
                : fLocalMatrix(localMatrix) {}

        // Local matrices are applied to coords.xy01, so a 4x4 matrix can be flattened to a 3x3
        // for less data upload to the GPU at this point (as there will be no more coordinate
        // space manipulation that might require the full 4x4).
        const SkMatrix fLocalMatrix;
    };

    static void BeginBlock(const KeyContext&, const LMShaderData&);
};

struct ImageShaderBlock {
    struct ImageData {
        ImageData(const SkSamplingOptions& sampling,
                  SkTileMode tileModeX,
                  SkTileMode tileModeY,
                  SkISize imgSize,
                  SkRect subset,
                  ImmutableSamplerInfo immutableSamplerInfo = {});
        SkSamplingOptions fSampling;
        std::pair<SkTileMode, SkTileMode> fTileModes;
        SkISize fImgSize;
        SkRect fSubset;

        // When we're generating the key from an actual SkImageShader fTextureProxy will be
        // non-null. Otherwise, fImmutableSamplerInfo will be filled in.
        sk_sp<TextureProxy> fTextureProxy;
        ImmutableSamplerInfo fImmutableSamplerInfo;
    };

    static void AddBlock(const KeyContext&, const ImageData&);
};

struct YUVImageShaderBlock {
    struct ImageData {
        ImageData(const SkSamplingOptions& sampling,
                  SkTileMode tileModeX,
                  SkTileMode tileModeY,
                  SkISize imgSize,
                  SkRect subset);

        SkSamplingOptions fSampling;
        SkSamplingOptions fSamplingUV;
        std::pair<SkTileMode, SkTileMode> fTileModes;
        SkISize fImgSize;
        SkISize fImgSizeUV;  // Size of UV planes relative to Y's texel space
        SkRect fSubset;
        SkPoint fLinearFilterUVInset = { 0.50001f, 0.50001f };
        SkV4 fChannelSelect[4];
        float fAlphaParam = 0;
        SkMatrix fYUVtoRGBMatrix;
        SkPoint3 fYUVtoRGBTranslate;

        // TODO: Currently these are only filled in when we're generating the key from an actual
        // SkImageShader. In the pre-compile case we will need to create Graphite promise
        // images which hold the appropriate data.
        sk_sp<TextureProxy> fTextureProxies[4];
    };

    static void AddBlock(const KeyContext&, const ImageData&);
};

struct CoordNormalizeShaderBlock {
    struct CoordNormalizeData {
        CoordNormalizeData(SkSize dimensions)
                : fInvDimensions(
                          SkSize::Make(1.0f / dimensions.width(), 1.0f / dimensions.height())) {}
        SkSize fInvDimensions;
    };

    static void BeginBlock(const KeyContext&, const CoordNormalizeData&);
};

struct CoordClampShaderBlock {
    struct CoordClampData {
        CoordClampData(SkRect subset) : fSubset(subset) {}

        SkRect fSubset;
    };

    static void BeginBlock(const KeyContext&, const CoordClampData&);
};

struct DitherShaderBlock {
    struct DitherData {
        DitherData(float range, sk_sp<TextureProxy> proxy)
            : fRange(range)
            , fLUTProxy(std::move(proxy)) {}

        float fRange;
        sk_sp<TextureProxy> fLUTProxy;
    };

    static void AddBlock(const KeyContext&, const DitherData&);
};

struct PerlinNoiseShaderBlock {
    enum class Type {
        kFractalNoise,
        kTurbulence,
    };

    struct PerlinNoiseData {
        PerlinNoiseData(Type type,
                        SkVector baseFrequency,
                        int numOctaves,
                        SkISize stitchData)
            : fType(type)
            , fBaseFrequency(baseFrequency)
            , fNumOctaves(numOctaves)
            , fStitchData{ SkIntToFloat(stitchData.fWidth), SkIntToFloat(stitchData.fHeight) } {
        }

        bool stitching() const { return !fStitchData.isZero(); }

        Type fType;
        SkVector fBaseFrequency;
        int fNumOctaves;
        SkVector fStitchData;

        sk_sp<TextureProxy> fPermutationsProxy;
        sk_sp<TextureProxy> fNoiseProxy;
    };

    static void AddBlock(const KeyContext&, const PerlinNoiseData&);
};

struct BlendComposeBlock {
    static void BeginBlock(const KeyContext&);
};

struct PorterDuffBlenderBlock {
    static void AddBlock(const KeyContext&, SkSpan<const float> coeffs);
};

struct HSLCBlenderBlock {
    static void AddBlock(const KeyContext&, SkSpan<const float> coeffs);
};

struct ComposeBlock {
    static void BeginBlock(const KeyContext&);
};

struct MatrixColorFilterBlock {
    struct MatrixColorFilterData {
        MatrixColorFilterData(const float matrix[20], bool inHSLA, bool clamp)
                : fMatrix(matrix[ 0], matrix[ 1], matrix[ 2], matrix[ 3],
                          matrix[ 5], matrix[ 6], matrix[ 7], matrix[ 8],
                          matrix[10], matrix[11], matrix[12], matrix[13],
                          matrix[15], matrix[16], matrix[17], matrix[18])
                , fTranslate{matrix[4], matrix[9], matrix[14], matrix[19]}
                , fInHSLA(inHSLA)
                , fClamp(clamp) {
        }

        SkM44 fMatrix;
        SkV4  fTranslate;
        bool  fInHSLA;
        bool  fClamp;
    };

    static void AddBlock(const KeyContext&, const MatrixColorFilterData&);
};

struct TableColorFilterBlock {
    struct TableColorFilterData {
        TableColorFilterData(sk_sp<TextureProxy> proxy) : fTextureProxy(std::move(proxy)) {}

        sk_sp<TextureProxy> fTextureProxy;
    };

    static void AddBlock(const KeyContext&, const TableColorFilterData&);
};

struct ColorSpaceTransformBlock {
    struct ColorSpaceTransformData {
        ColorSpaceTransformData(const SkColorSpace* src,
                                SkAlphaType srcAT,
                                const SkColorSpace* dst,
                                SkAlphaType dstAT);
        ColorSpaceTransformData(const SkColorSpaceXformSteps& steps) { fSteps = steps; }
        ColorSpaceTransformData(ReadSwizzle swizzle) : fReadSwizzle(swizzle) {
            SkASSERT(fSteps.fFlags.mask() == 0);  // By default, the colorspace should have no effect
        }
        SkColorSpaceXformSteps fSteps;
        ReadSwizzle            fReadSwizzle = ReadSwizzle::kRGBA;
    };

    static void AddBlock(const KeyContext&, const ColorSpaceTransformData&);
};

struct NonMSAAClipBlock {
    struct NonMSAAClipData {
        NonMSAAClipData(SkRect rect,
                        SkPoint radiusPlusHalf,
                        SkRect edgeSelect,
                        SkPoint texCoordOffset,
                        SkRect maskBounds,
                        sk_sp<TextureProxy> atlasTexture)
                : fRect(rect)
                , fRadiusPlusHalf(radiusPlusHalf)
                , fEdgeSelect(edgeSelect)
                , fTexCoordOffset(texCoordOffset)
                , fMaskBounds(maskBounds)
                , fAtlasTexture(std::move(atlasTexture)){}
        // analytic clip
        SkRect  fRect;            // bounds, outset by 0.5
        SkPoint fRadiusPlusHalf;  // abs() of .x is radius+0.5, if < 0 indicates inverse fill
                                  // .y is 1/(radius+0.5)
        SkRect  fEdgeSelect;      // 1 indicates a rounded corner on that side (LTRB), 0 otherwise

        // atlas clip
        SkPoint fTexCoordOffset;  // translation from local coords to unnormalized texel coords
        SkRect  fMaskBounds;      // bounds of mask area, in unnormalized texel coords

        sk_sp<TextureProxy> fAtlasTexture;
    };

    static void AddBlock(const KeyContext&, const NonMSAAClipData&);
};

/**
 * Adds a block that references the primitive color produced by the RenderStep and accounts for
 * color space transformation.
 */
void AddPrimitiveColor(const KeyContext&, bool skipColorXform);

/**
 * Blend mode color filters blend their input (as the dst color) with some given color (supplied
 * via a uniform) as the src color.
 */
void AddBlendModeColorFilter(const KeyContext&,
                             SkBlendMode,
                             const SkPMColor4f& srcColor);

struct RuntimeEffectBlock {
    struct ShaderData {
        // This ctor is used during pre-compilation when we don't have enough information to
        // extract uniform data.
        ShaderData(sk_sp<const SkRuntimeEffect> effect);

        // This ctor is used when extracting information from PaintParams.
        ShaderData(sk_sp<const SkRuntimeEffect> effect,
                   sk_sp<const SkData> uniforms);

        bool operator==(const ShaderData& rhs) const;
        bool operator!=(const ShaderData& rhs) const { return !(*this == rhs); }

        // Runtime shader data.
        sk_sp<const SkRuntimeEffect> fEffect;
        sk_sp<const SkData>          fUniforms;
    };

    // On a false return, no block has been started
    static bool BeginBlock(const KeyContext&, const ShaderData&);

    // Add a no-op placeholder for an incorrect runtime effect
    static void AddNoOpEffect(const KeyContext&, SkRuntimeEffect*);

    // Add a post-amble for runtime effects that use the toLinearSrgb/fromLinearSrgb intrinsics
    static void HandleIntrinsics(const KeyContext&, const SkRuntimeEffect*);
};

void AddToKey(const KeyContext&, const SkBlender*);

/**
 *  Add implementation details, for the specified backend, of this SkColorFilter to the
 *  provided key.
 *
 *  @param keyContext backend context for key creation
 *  @param filter     This function is a no-op if filter is null.
 */
void AddToKey(const KeyContext& keyContext, const SkColorFilter* filter);

/**
 *  Add implementation details, for the specified backend, of this SkShader to the
 *  provided key.
 *
 *  @param keyContext backend context for key creation
 *  @param shader     This function is a no-op if shader is null.
 */
void AddToKey(const KeyContext& keyContext, const SkShader* shader);

// TODO(b/330864257) These visitation functions are redundant with AddToKey, except that they are
// executed in the Device::drawGeometry() stack frame, whereas the keys are currently deferred until
// DrawPass::Make. Image use needs to be detected in the draw frame to split tasks to match client
// actions. Once paint keys are extracted in the draw frame, this can go away entirely.
void NotifyImagesInUse(Recorder*, DrawContext*, const SkBlender*);
void NotifyImagesInUse(Recorder*, DrawContext*, const SkColorFilter*);
void NotifyImagesInUse(Recorder*, DrawContext*, const SkShader*);

template <typename AddBlendToKeyT, typename AddSrcToKeyT, typename AddDstToKeyT>
void Blend(const KeyContext& keyContext,
           AddBlendToKeyT addBlendToKey,
           AddSrcToKeyT addSrcToKey,
           AddDstToKeyT addDstToKey) {
    BlendComposeBlock::BeginBlock(keyContext);

        addSrcToKey();

        addDstToKey();

        addBlendToKey();

    keyContext.paintParamsKeyBuilder()->endBlock();  // BlendComposeBlock
}

template <typename AddInnerToKeyT, typename AddOuterToKeyT>
void Compose(const KeyContext& keyContext,
             AddInnerToKeyT addInnerToKey,
             AddOuterToKeyT addOuterToKey) {
    ComposeBlock::BeginBlock(keyContext);

        addInnerToKey();

        addOuterToKey();

    keyContext.paintParamsKeyBuilder()->endBlock();  // ComposeBlock
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_KeyHelpers_DEFINED
