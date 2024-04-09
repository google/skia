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
#include "include/private/SkColorData.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/shaders/SkShaderBase.h"

class SkColorFilter;
class SkData;
class SkRuntimeEffect;

namespace skgpu::graphite {

class DrawContext;
class KeyContext;
class PaintParamsKeyBuilder;
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

struct DstReadSampleBlock {
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         sk_sp<TextureProxy> dst,
                         SkIPoint dstOffset);
};

struct SolidColorShaderBlock {
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const SkPMColor4f&);
};

struct RGBPaintColorBlock {
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*);
};

struct AlphaOnlyPaintColorBlock {
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*);
};

struct GradientShaderBlocks {
    struct GradientData {
        // The number of stops stored internal to this data structure before falling back to
        // bitmap storage.
        static constexpr int kNumInternalStorageStops = 8;

        // This ctor is used during pre-compilation when we don't have enough information to
        // extract uniform data. However, we must be able to provide enough data to make all the
        // relevant decisions about which code snippets to use.
        GradientData(SkShaderBase::GradientType, int numStops);

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
                     sk_sp<TextureProxy> colorsAndOffsetsProxy,
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

        // For gradients w/ <= kNumInternalStorageStops stops we use fColors and fOffsets.
        // The offsets are packed into a single float4 to save space when the layout is std140.
        // Otherwise we use fColorsAndOffsetsProxy.
        SkPMColor4f            fColors[kNumInternalStorageStops];
        SkV4                   fOffsets[kNumInternalStorageStops / 4];
        sk_sp<TextureProxy>    fColorsAndOffsetsProxy;

        SkGradientShader::Interpolation fInterpolation;
    };

    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const GradientData&);
};

struct LocalMatrixShaderBlock {
    struct LMShaderData {
        LMShaderData(const SkMatrix& localMatrix)
                : fLocalMatrix(localMatrix) {
        }

        const SkM44 fLocalMatrix;
    };

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const LMShaderData&);
};

struct ImageShaderBlock {
    struct ImageData {
        ImageData(const SkSamplingOptions& sampling,
                  SkTileMode tileModeX,
                  SkTileMode tileModeY,
                  SkISize imgSize,
                  SkRect subset,
                  ReadSwizzle readSwizzle);

        SkSamplingOptions fSampling;
        SkTileMode fTileModes[2];
        SkISize fImgSize;
        SkRect fSubset;
        ReadSwizzle fReadSwizzle;

        SkColorSpaceXformSteps fSteps;

        // TODO: Currently this is only filled in when we're generating the key from an actual
        // SkImageShader. In the pre-compile case we will need to create a Graphite promise
        // image which holds the appropriate data.
        sk_sp<TextureProxy> fTextureProxy;
    };

    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const ImageData&);
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
        SkTileMode fTileModes[2];
        SkISize fImgSize;
        SkISize fImgSizeUV;  // Size of UV planes relative to Y's texel space
        SkRect fSubset;
        SkPoint fLinearFilterUVInset = { 0.50001f, 0.50001f };
        SkV4 fChannelSelect[4];
        SkMatrix fYUVtoRGBMatrix;
        SkPoint3 fYUVtoRGBTranslate;

        // TODO: Currently these are only filled in when we're generating the key from an actual
        // SkImageShader. In the pre-compile case we will need to create Graphite promise
        // images which hold the appropriate data.
        sk_sp<TextureProxy> fTextureProxies[4];
    };

    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const ImageData&);
};

struct CoordClampShaderBlock {
    struct CoordClampData {
        CoordClampData(SkRect subset) : fSubset(subset) {}

        SkRect fSubset;
    };

    // The gatherer and data should be null or non-null together
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const CoordClampData&);
};

struct DitherShaderBlock {
    struct DitherData {
        DitherData(float range, sk_sp<TextureProxy> proxy)
            : fRange(range)
            , fLUTProxy(std::move(proxy)) {}

        float fRange;
        sk_sp<TextureProxy> fLUTProxy;
    };

    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const DitherData&);
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

    // The gatherer and data should be null or non-null together
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const PerlinNoiseData&);
};

struct BlendShaderBlock {
    static void BeginBlock(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*);
};

struct BlendModeBlenderBlock {
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         SkBlendMode);
};

struct CoeffBlenderBlock {
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         SkSpan<const float> coeffs);
};

struct ClipShaderBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);
};

struct ComposeBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);
};

struct MatrixColorFilterBlock {
    struct MatrixColorFilterData {
        MatrixColorFilterData(const float matrix[20],
                              bool inHSLA)
                : fMatrix(matrix[ 0], matrix[ 1], matrix[ 2], matrix[ 3],
                          matrix[ 5], matrix[ 6], matrix[ 7], matrix[ 8],
                          matrix[10], matrix[11], matrix[12], matrix[13],
                          matrix[15], matrix[16], matrix[17], matrix[18])
                , fTranslate{matrix[4], matrix[9], matrix[14], matrix[19]}
                , fInHSLA(inHSLA) {
        }

        SkM44 fMatrix;
        SkV4  fTranslate;
        bool  fInHSLA;
    };

    // The gatherer and matrixCFData should be null or non-null together
    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const MatrixColorFilterData&);
};

struct TableColorFilterBlock {
    struct TableColorFilterData {
        TableColorFilterData(sk_sp<TextureProxy> proxy) : fTextureProxy(std::move(proxy)) {}

        sk_sp<TextureProxy> fTextureProxy;
    };

    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const TableColorFilterData&);
};

struct ColorSpaceTransformBlock {
    struct ColorSpaceTransformData {
        ColorSpaceTransformData(const SkColorSpace* src,
                                SkAlphaType srcAT,
                                const SkColorSpace* dst,
                                SkAlphaType dstAT);
        ColorSpaceTransformData(const SkColorSpaceXformSteps& steps) { fSteps = steps; }
        SkColorSpaceXformSteps fSteps;
    };

    static void AddBlock(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         const ColorSpaceTransformData&);
};

/**
 * Blend mode color filters blend their input (as the dst color) with some given color (supplied
 * via a uniform) as the src color.
 */
void AddBlendModeColorFilter(const KeyContext&,
                             PaintParamsKeyBuilder*,
                             PipelineDataGatherer*,
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

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const ShaderData&);
};

void AddToKey(const KeyContext&,
              PaintParamsKeyBuilder*,
              PipelineDataGatherer*,
              const SkBlender*);

/**
 *  Add implementation details, for the specified backend, of this SkColorFilter to the
 *  provided key.
 *
 *  @param keyContext backend context for key creation
 *  @param builder    builder for creating the key for this SkShader
 *  @param gatherer   if non-null, storage for this colorFilter's data
 *  @param filter     This function is a no-op if filter is null.
 */
void AddToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              const SkColorFilter* filter);

/**
 *  Add implementation details, for the specified backend, of this SkShader to the
 *  provided key.
 *
 *  @param keyContext backend context for key creation
 *  @param builder    builder for creating the key for this SkShader
 *  @param gatherer   if non-null, storage for this colorFilter's data
 *  @param shader     This function is a no-op if shader is null.
 */
void AddToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              const SkShader* shader);

// TODO(b/330864257) These visitation functions are redundant with AddToKey, except that they are
// executed in the Device::drawGeometry() stack frame, whereas the keys are currently deferred until
// DrawPass::Make. Image use needs to be detected in the draw frame to split tasks to match client
// actions. Once paint keys are extracted in the draw frame, this can go away entirely.
void NotifyImagesInUse(Recorder*, DrawContext*, const SkBlender*);
void NotifyImagesInUse(Recorder*, DrawContext*, const SkColorFilter*);
void NotifyImagesInUse(Recorder*, DrawContext*, const SkShader*);

} // namespace skgpu::graphite

#endif // skgpu_graphite_KeyHelpers_DEFINED
