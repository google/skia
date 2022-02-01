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
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

enum class SkBackend : uint8_t;
class SkPaintParamsKey;
class SkUniformBlock;

// The KeyHelpers can be used to manually construct an SkPaintParamsKey

namespace DepthStencilOnlyBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*, SkUniformBlock*);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace DepthStencilOnlyBlock

namespace SolidColorShaderBlock {

    void AddToKey(SkBackend,
                  SkPaintParamsKey*,
                  SkUniformBlock*,
                  const SkColor4f&);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

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
                     SkPoint points[2],
                     float radii[2],
                     SkTileMode,
                     int numStops,
                     SkColor colors[kMaxStops],
                     float offsets[kMaxStops]);

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

    private:
        void toColor4fs(int numColors, SkColor colors[kMaxStops]);
        void toOffsets(int numStops, float inputOffsets[kMaxStops]);
    };

    void AddToKey(SkBackend,
                  SkPaintParamsKey*,
                  SkUniformBlock*,
                  const GradientData&);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace GradientShaderBlocks

namespace ImageShaderBlock {

    struct ImageData {
        bool operator==(const ImageData& rhs) const {
            return fTileModes[0] == rhs.fTileModes[0] &&
                   fTileModes[1] == rhs.fTileModes[1];
        }
        bool operator!=(const ImageData& rhs) const { return !(*this == rhs); }

        // TODO: add the other image shader parameters that could impact code snippet selection
        // (e.g., sampling options, subsetting, etc.)
        SkTileMode fTileModes[2];
    };

    void AddToKey(SkBackend,
                  SkPaintParamsKey*,
                  SkUniformBlock*,
                  const ImageData&);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace ImageShaderBlock

namespace BlendShaderBlock {

    struct BlendData {
        SkShader*   fDst;
        SkShader*   fSrc;
        // TODO: add support for blenders
        SkBlendMode fBM;
    };

    void AddToKey(SkBackend,
                  SkPaintParamsKey*,
                  SkUniformBlock*,
                  const BlendData&);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace BlendShaderBlock

namespace BlendModeBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*, SkUniformBlock*, SkBlendMode);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace BlendModeBlock

#ifdef SK_GRAPHITE_ENABLED
// Bridge between the combinations system and the SkPaintParamsKey
SkPaintParamsKey CreateKey(SkBackend, skgpu::ShaderCombo::ShaderType, SkTileMode, SkBlendMode);
#endif

#endif // SkKeyHelpers_DEFINED
