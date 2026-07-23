/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PaintOption_DEFINED
#define skgpu_graphite_precompile_PaintOption_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColorType.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileShader;

class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
class StorageBufferManager;

class PaintOption {
public:
    PaintOption(bool opaquePaintColor,
                const std::pair<sk_sp<PrecompileBlender>, int>& finalBlender,
                const std::pair<sk_sp<PrecompileShader>, int>& shader,
                const std::pair<sk_sp<PrecompileColorFilter>, int>& colorFilter,
                bool hasPrimitiveBlender,
                SkBlendMode primitiveBlendMode,
                bool skipColorXform,
                const std::pair<sk_sp<PrecompileShader>, int>& clipShader,
                Coverage coverage,
                TextureFormat targetFormat,
                bool dither,
                bool analyticClip);

    const PrecompileBlender* finalBlender() const { return fFinalBlender.first.get(); }

    void toKey(const KeyContext&) const;

private:
    // These all return true if the resulting PaintOption is known to be opaque; false otherwise.
    bool addPaintColorToKey(const KeyContext&) const;
    bool handlePrimitiveColor(const KeyContext&) const;
    bool handlePaintAlpha(const KeyContext&) const;
    bool handleColorFilter(const KeyContext&) const;
    bool handleDithering(const KeyContext&) const;

    void handleClipping(const KeyContext&) const;

    bool shouldDither(SkColorType dstCT) const;

    bool fOpaquePaintColor;
    std::pair<sk_sp<PrecompileBlender>, int> fFinalBlender;
    std::pair<sk_sp<PrecompileShader>, int> fShader;
    std::pair<sk_sp<PrecompileColorFilter>, int> fColorFilter;
    SkBlendMode fPrimitiveBlendMode;
    bool fHasPrimitiveBlender;
    bool fSkipColorXform;
    std::pair<sk_sp<PrecompileShader>, int> fClipShader;
    Coverage fRendererCoverage;
    TextureFormat fTargetFormat;
    bool fDither;
    bool fAnalyticClip;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PaintOption_DEFINED
