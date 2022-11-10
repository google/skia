/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyHelpers_DEFINED
#define SkKeyHelpers_DEFINED

#ifdef SK_GRAPHITE_ENABLED
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/TextureProxy.h"
#endif

#include "include/core/SkBlendMode.h"
#include "include/core/SkM44.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkColorData.h"
#include "src/shaders/SkShaderBase.h"

class SkData;
class SkPipelineDataGatherer;
class SkRuntimeEffect;
class SkUniquePaintParamsID;
class SkKeyContext;

#ifdef SK_ENABLE_PRECOMPILE
namespace skgpu::graphite {
class PaintParamsKeyBuilder;
enum class ShaderType : uint32_t;
}
#endif

/**
 * The KeyHelpers can be used to manually construct an SkPaintParamsKey.
 *
 * TODO: If we restructure how the keys are made, we can utilize a single block type for the
 * different blend blocks outlined below. The different Src/Dst pairings could instead be encoded
 * as parent-child relationships.
 */

struct PassthroughShaderBlock {

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*);

};

struct PassthroughBlenderBlock {

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*);

};

struct SolidColorShaderBlock {

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
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
                     SkColor4f* colors,
                     float* offsets);

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
                   !memcmp(fColor4fs, rhs.fColor4fs, sizeof(fColor4fs)) &&
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
        SkColor4f              fColor4fs[kMaxStops];
        float                  fOffsets[kMaxStops];
    };

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const GradientData&);

};

struct LocalMatrixShaderBlock {

    struct LMShaderData {
        LMShaderData(const SkMatrix& localMatrix)
                : fLocalMatrix(localMatrix) {
        }

        const SkM44 fLocalMatrix;
    };

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const LMShaderData&);

};

struct ImageShaderBlock {

    struct ImageData {
        ImageData(const SkSamplingOptions& sampling,
                  SkTileMode tileModeX,
                  SkTileMode tileModeY,
                  SkRect subset);

        SkSamplingOptions fSampling;
        SkTileMode fTileModes[2];
        SkRect fSubset;

#ifdef SK_GRAPHITE_ENABLED
        // TODO: Currently this is only filled in when we're generating the key from an actual
        // SkImageShader. In the pre-compile case we will need to create a Graphite promise
        // image which holds the appropriate data.
        sk_sp<skgpu::graphite::TextureProxy> fTextureProxy;
#endif
    };

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const ImageData&);

};

struct PorterDuffBlendShaderBlock {
    struct PorterDuffBlendShaderData {
        SkSpan<const float> fPorterDuffConstants;
    };

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const PorterDuffBlendShaderData&);
};

struct BlendShaderBlock {
    /**
     * Blend shader blocks are used to blend the output of two shaders.
     */
    struct BlendShaderData {
        SkBlendMode fBM;
    };

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const BlendShaderData&);
};

struct ColorFilterShaderBlock {
    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*);
};

struct MatrixColorFilterBlock {
    struct MatrixColorFilterData {
        MatrixColorFilterData(const float matrix[20],
                              bool inHSLA)
                : fMatrix(matrix[ 0], matrix[ 1], matrix[ 2], matrix[ 3],
                          matrix[ 5], matrix[ 6], matrix[ 7], matrix[ 8],
                          matrix[10], matrix[11], matrix[12], matrix[13],
                          matrix[15], matrix[16], matrix[17], matrix[18])
                , fTranslate(matrix[4], matrix[9], matrix[14], matrix[19])
                , fInHSLA(inHSLA) {
        }

        SkM44        fMatrix;
        skvx::float4 fTranslate;
        bool         fInHSLA;
    };

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const MatrixColorFilterData&);
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

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const BlendColorFilterData&);
};

struct ComposeColorFilterBlock {
    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*);
};

struct TableColorFilterBlock {
    struct TableColorFilterData {
        TableColorFilterData();

#ifdef SK_GRAPHITE_ENABLED
        sk_sp<skgpu::graphite::TextureProxy> fTextureProxy;
#endif
    };

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const TableColorFilterData&);
};

struct GaussianColorFilterBlock {
    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*);
};

struct BlendModeBlock {
    /**
     * Blend mode blocks are used to blend a color attachment with the output of a shader.
     */
    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           SkBlendMode);
};

struct PrimitiveBlendModeBlock {
    /**
     * Primitive blend mode blocks are used to blend a primitive color emitted by certain draw
     * geometry calls (drawVertices, drawAtlas, etc.) with either the paint color or the output of
     * another shader. Dst: primitiveColor Src: Paint color/shader output
     */
    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
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

    static void BeginBlock(const SkKeyContext&,
                           skgpu::graphite::PaintParamsKeyBuilder*,
                           SkPipelineDataGatherer*,
                           const ShaderData&);
};

#endif // SkKeyHelpers_DEFINED
