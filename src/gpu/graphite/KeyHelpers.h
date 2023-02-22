/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_KeyHelpers_DEFINED
#define skgpu_graphite_KeyHelpers_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkM44.h"
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

class SkData;
class SkRuntimeEffect;

namespace skgpu::graphite {

class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
class UniquePaintParamsID;
enum class ReadSwizzle;

/**
 * The KeyHelpers can be used to manually construct an SkPaintParamsKey.
 *
 * TODO: If we restructure how the keys are made, we can utilize a single block type for the
 * different blend blocks outlined below. The different Src/Dst pairings could instead be encoded
 * as parent-child relationships.
 */

struct PassthroughShaderBlock {

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);

};

struct PassthroughBlenderBlock {

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
        // TODO: For the sprint we only support 8 stops in the gradients
        static constexpr int kMaxStops = 8;

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
                     float* offsets,
                     const SkGradientShader::Interpolation&);

        bool operator==(const GradientData& rhs) const {
            return fType == rhs.fType &&
                   fPoints[0] == rhs.fPoints[0] &&
                   fPoints[1] == rhs.fPoints[1] &&
                   fRadii[0] == rhs.fRadii[0] &&
                   fRadii[1] == rhs.fRadii[1] &&
                   fBias == rhs.fBias &&
                   fScale == rhs.fScale &&
                   fTM == rhs.fTM &&
                   fNumStops == rhs.fNumStops &&
                   !memcmp(fColors, rhs.fColors, sizeof(fColors)) &&
                   !memcmp(fOffsets, rhs.fOffsets, sizeof(fOffsets));
        }
        bool operator!=(const GradientData& rhs) const { return !(*this == rhs); }

        // Layout options.
        SkShaderBase::GradientType fType;
        SkPoint                    fPoints[2];
        float                      fRadii[2];

        // Layout options for sweep gradient.
        float                  fBias;
        float                  fScale;

        SkTileMode             fTM;
        int                    fNumStops;
        SkPMColor4f            fColors[kMaxStops];
        float                  fOffsets[kMaxStops];

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
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const ImageData*);

};

struct PorterDuffBlendShaderBlock {
    struct PorterDuffBlendShaderData {
        SkSpan<const float> fPorterDuffConstants;
    };

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const PorterDuffBlendShaderData&);
};

struct BlendShaderBlock {
    /**
     * Blend shader blocks are used to blend the output of two shaders.
     */
    struct BlendShaderData {
        SkBlendMode fBM;
    };

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const BlendShaderData&);
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

struct BlendColorFilterBlock {
    /**
     * Blend color filter blocks are used to blend the output of a shader with a color uniform.
     */
    struct BlendColorFilterData {
        BlendColorFilterData(SkBlendMode blendMode, const SkPMColor4f& srcColor)
                : fBlendMode(blendMode)
                , fSrcColor(srcColor) {
        }

        SkBlendMode fBlendMode;
        SkPMColor4f fSrcColor;
    };

    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           const BlendColorFilterData*);
};

struct ComposeColorFilterBlock {
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*);
};

struct TableColorFilterBlock {
    struct TableColorFilterData {
        TableColorFilterData();

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

struct BlendModeBlock {
    /**
     * Blend mode blocks are used to blend a color attachment with the output of a shader.
     */
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           SkBlendMode);
};

struct PrimitiveBlendModeBlock {
    /**
     * Primitive blend mode blocks are used to blend a primitive color emitted by certain draw
     * geometry calls (drawVertices, drawAtlas, etc.) with either the paint color or the output of
     * another shader. Dst: primitiveColor Src: Paint color/shader output
     */
    static void BeginBlock(const KeyContext&,
                           PaintParamsKeyBuilder*,
                           PipelineDataGatherer*,
                           SkBlendMode);
};

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

} // namespace skgpu::graphite

#endif // skgpu_graphite_KeyHelpers_DEFINED
