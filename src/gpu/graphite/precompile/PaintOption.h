/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PaintOption_DEFINED
#define skgpu_graphite_precompile_PaintOption_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/Caps.h"

namespace skgpu::graphite {

class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileShader;

class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;

class PaintOption {
public:
    PaintOption(bool opaquePaintColor,
                const std::pair<sk_sp<PrecompileBlender>, int>& finalBlender,
                const std::pair<sk_sp<PrecompileShader>, int>& shader,
                const std::pair<sk_sp<PrecompileColorFilter>, int>& colorFilter,
                bool hasPrimitiveBlender,
                const std::pair<sk_sp<PrecompileShader>, int>& clipShader,
                bool dstReadRequired,
                bool dither)
        : fOpaquePaintColor(opaquePaintColor)
        , fFinalBlender(finalBlender)
        , fShader(shader)
        , fColorFilter(colorFilter)
        , fHasPrimitiveBlender(hasPrimitiveBlender)
        , fClipShader(clipShader)
        , fDstReadRequired(dstReadRequired)
        , fDither(dither) {
    }

    const PrecompileBlender* finalBlender() const { return fFinalBlender.first.get(); }

    void toKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

private:
    void addPaintColorToKey(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handlePrimitiveColor(const KeyContext&,
                              PaintParamsKeyBuilder*,
                              PipelineDataGatherer*) const;
    void handlePaintAlpha(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    void handleColorFilter(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;
    bool shouldDither(SkColorType dstCT) const;
    void handleDithering(const KeyContext&, PaintParamsKeyBuilder*, PipelineDataGatherer*) const;

    bool fOpaquePaintColor;
    std::pair<sk_sp<PrecompileBlender>, int> fFinalBlender;
    std::pair<sk_sp<PrecompileShader>, int> fShader;
    std::pair<sk_sp<PrecompileColorFilter>, int> fColorFilter;
    bool fHasPrimitiveBlender;
    std::pair<sk_sp<PrecompileShader>, int> fClipShader;
    bool fDstReadRequired;
    bool fDither;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PaintOption_DEFINED
