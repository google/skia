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

struct PriorOutputBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);
};

struct DstReadSampleBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           sk_sp<TextureProxy> dst,
                           SkIPoint dstOffset);
};

struct DstReadFetchBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);
};

struct SolidColorShaderBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const SkPMColor4f&);
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
        // Otherwise we use fColorsAndOffsetsProxy.
        SkPMColor4f            fColors[kNumInternalStorageStops];
        float                  fOffsets[kNumInternalStorageStops];
        sk_sp<TextureProxy>    fColorsAndOffsetsProxy;

        SkGradientShader::Interpolation fInterpolation;
    };

    static void BeginBlock(const KeyContext&,
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
                           const LMShaderData*);
};

struct ImageShaderBlock {
    struct ImageData {
        ImageData(const SkSamplingOptions& sampling,
                  SkTileMode tileModeX,
                  SkTileMode tileModeY,
                  SkRect subset,
                  ReadSwizzle readSwizzle);

        SkSamplingOptions fSampling;
        SkTileMode fTileModes[2];
        SkRect fSubset;
        ReadSwizzle fReadSwizzle;

        SkColorSpaceXformSteps fSteps;

        // TODO: Currently this is only filled in when we're generating the key from an actual
        // SkImageShader. In the pre-compile case we will need to create a Graphite promise
        // image which holds the appropriate data.
        sk_sp<TextureProxy> fTextureProxy;
    };

    // The gatherer and imageData should be null or non-null together
    // If imageData is not null, it's sampling options must have useCubic == false
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const ImageData*);

    // If imageData is not null, it's sampling options must have useCubic == true
    static void BeginCubicBlock(const KeyContext&,
                                PaintParamsKeyBuilder*,
                                PipelineDataGatherer*,
                                const ImageData*);
};

struct YUVImageShaderBlock {
    struct ImageData {
        ImageData(const SkSamplingOptions& sampling,
                  SkTileMode tileModeX,
                  SkTileMode tileModeY,
                  SkRect subset);

        SkPoint fImgSize;
        SkSamplingOptions fSampling;
        SkTileMode fTileModes[2];
        SkRect fSubset;
        SkColor4f fChannelSelect[4];
        SkMatrix fYUVtoRGBMatrix;
        SkPoint3 fYUVtoRGBTranslate;

        SkColorSpaceXformSteps fSteps;

        // TODO: Currently these are only filled in when we're generating the key from an actual
        // SkImageShader. In the pre-compile case we will need to create Graphite promise
        // images which hold the appropriate data.
        sk_sp<TextureProxy> fTextureProxies[4];
    };

    // The gatherer and imageData should be null or non-null together
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const ImageData*);
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
                           const CoordClampData*);
};

struct DitherShaderBlock {
    struct DitherData {
        DitherData(float range) : fRange(range) {}

        float fRange;
    };

    // The gatherer and data should be null or non-null together
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const DitherData*);
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
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const PerlinNoiseData*);
};

struct BlendShaderBlock {
    static void BeginBlock(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*);
};

struct BlendModeBlenderBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           SkBlendMode blendMode);
};

struct CoeffBlenderBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           SkSpan<const float> coeffs);
};

struct DstColorBlock {
    static void BeginBlock(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*);
};

struct PrimitiveColorBlock {
    static void BeginBlock(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*);
};

struct ColorFilterShaderBlock {
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
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const MatrixColorFilterData*);
};

struct ComposeColorFilterBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);
};

struct TableColorFilterBlock {
    struct TableColorFilterData {
        TableColorFilterData(sk_sp<TextureProxy> proxy) : fTextureProxy(std::move(proxy)) {}

        sk_sp<TextureProxy> fTextureProxy;
    };

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const TableColorFilterData&);
};

struct GaussianColorFilterBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);
};

struct ColorSpaceTransformBlock {
    struct ColorSpaceTransformData {
        ColorSpaceTransformData(const SkColorSpace* src,
                                SkAlphaType srcAT,
                                const SkColorSpace* dst,
                                SkAlphaType dstAT);
        SkColorSpaceXformSteps fSteps;
    };

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const ColorSpaceTransformData*);
};

/**
 * Dst blend blocks are used to blend the output of a shader with a color attachment.
 */
void AddDstBlendBlock(const KeyContext&,
                      PaintParamsKeyBuilder*,
                      PipelineDataGatherer*,
                      const SkBlender*);

/**
 * Primitive blend blocks are used to blend either the paint color or the output of another shader
 * with a primitive color emitted by certain draw geometry calls (drawVertices, drawAtlas, etc.).
 * Dst: primitiveColor Src: Paint color/shader output
 */
void AddPrimitiveBlendBlock(const KeyContext&,
                            PaintParamsKeyBuilder*,
                            PipelineDataGatherer*,
                            const SkBlender*);

/**
 * Color filter blend blocks are used to blend a color uniform with the output of a shader.
 */
void AddColorBlendBlock(const KeyContext&,
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

} // namespace skgpu::graphite

#endif // skgpu_graphite_KeyHelpers_DEFINED
