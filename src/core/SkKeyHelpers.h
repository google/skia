/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyHelpers_DEFINED
#define SkKeyHelpers_DEFINED

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/include/Context.h"
#endif

#include "include/core/SkBlendMode.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkColorData.h"

enum class SkBackend : uint8_t;
class SkPaintParamsKeyBuilder;
class SkPipelineData;
class SkUniquePaintParamsID;
class SkKeyContext;

namespace skgpu { class TextureProxy; }

// The KeyHelpers can be used to manually construct an SkPaintParamsKey

namespace DepthStencilOnlyBlock {

    void AddToKey(const SkKeyContext&,
                  SkPaintParamsKeyBuilder*,
                  SkPipelineData*);

} // namespace DepthStencilOnlyBlock

namespace SolidColorShaderBlock {

    void AddToKey(const SkKeyContext&,
                  SkPaintParamsKeyBuilder*,
                  SkPipelineData*,
                  const SkPMColor4f&);

} // namespace SolidColorShaderBlock

// TODO: move this functionality to the SkLinearGradient, SkRadialGradient, etc classes
namespace GradientShaderBlocks {

    struct GradientData {
        // TODO: For the sprint we only support 4 stops in the gradients
        static constexpr int kMaxStops = 4;

        // This ctor is used during pre-compilation when we don't have enough information to
        // extract uniform data. However, we must be able to provide enough data to make all the
        // relevant decisions about which code snippets to use.
        GradientData(SkShader::GradientType,
                     SkTileMode,
                     int numStops);

        // This ctor is used when extracting information from PaintParams. It must provide
        // enough data to generate the uniform data the selected code snippet will require.
        GradientData(SkShader::GradientType,
                     SkPoint point0, SkPoint point1,
                     float radius0, float radius1,
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
                   fTM == rhs.fTM &&
                   fNumStops == rhs.fNumStops &&
                   !memcmp(fColor4fs, rhs.fColor4fs, sizeof(fColor4fs)) &&
                   !memcmp(fOffsets, rhs.fOffsets, sizeof(fOffsets));
        }
        bool operator!=(const GradientData& rhs) const { return !(*this == rhs); }

        SkShader::GradientType fType;
        SkPoint                fPoints[2];
        float                  fRadii[2];
        SkTileMode             fTM;
        int                    fNumStops;
        SkColor4f              fColor4fs[kMaxStops];
        float                  fOffsets[kMaxStops];
    };

    void AddToKey(const SkKeyContext&,
                  SkPaintParamsKeyBuilder*,
                  SkPipelineData*,
                  const GradientData&);

} // namespace GradientShaderBlocks

namespace ImageShaderBlock {

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
        sk_sp<skgpu::TextureProxy> fTextureProxy;
#endif
    };

    void AddToKey(const SkKeyContext&,
                  SkPaintParamsKeyBuilder*,
                  SkPipelineData*,
                  const ImageData&);

} // namespace ImageShaderBlock

namespace BlendShaderBlock {

    struct BlendData {
        SkShader*   fDst;
        SkShader*   fSrc;
        // TODO: add support for blenders
        SkBlendMode fBM;
    };

    void AddToKey(const SkKeyContext&,
                  SkPaintParamsKeyBuilder*,
                  SkPipelineData*,
                  const BlendData&);

} // namespace BlendShaderBlock

namespace BlendModeBlock {

    void AddToKey(const SkKeyContext&,
                  SkPaintParamsKeyBuilder*,
                  SkPipelineData*,
                  SkBlendMode);

} // namespace BlendModeBlock

#ifdef SK_GRAPHITE_ENABLED
// Bridge between the combinations system and the SkPaintParamsKey
SkUniquePaintParamsID CreateKey(const SkKeyContext&,
                                SkPaintParamsKeyBuilder*,
                                skgpu::ShaderCombo::ShaderType,
                                SkTileMode,
                                SkBlendMode);
#endif

#endif // SkKeyHelpers_DEFINED
