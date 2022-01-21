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

// The KeyHelpers can be used to manually construct an SkPaintParamsKey

namespace DepthStencilOnlyBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace DepthStencilOnlyBlock

namespace SolidColorShaderBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace SolidColorShaderBlock

// TODO: move this functionality to the SkLinearGradient, SkRadialGradient, etc classes
namespace GradientShaderBlocks {

    struct GradientData {
        bool operator==(const GradientData& rhs) const {
            return fType == rhs.fType &&
                   fTM == rhs.fTM &&
                   fNumStops == rhs.fNumStops;
        }
        bool operator!=(const GradientData& rhs) const { return !(*this == rhs); }

        SkShader::GradientType fType;
        SkTileMode             fTM;
        int                    fNumStops;
    };

    void AddToKey(SkBackend, SkPaintParamsKey*, const GradientData&);
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

    void AddToKey(SkBackend, SkPaintParamsKey*, const ImageData&);
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

    void AddToKey(SkBackend, SkPaintParamsKey*, const BlendData&);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace BlendShaderBlock

namespace BlendModeBlock {

    void AddToKey(SkBackend, SkPaintParamsKey*, SkBlendMode);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace BlendModeBlock

#ifdef SK_GRAPHITE_ENABLED
// Bridge between the combinations system and the SkPaintParamsKey
SkPaintParamsKey CreateKey(SkBackend, skgpu::ShaderCombo::ShaderType, SkTileMode, SkBlendMode);
#endif

#endif // SkKeyHelpers_DEFINED
