/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/precompile/PaintOption.h"

#include "include/core/SkBlender.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"

namespace skgpu::graphite {

PaintOption::PaintOption(bool opaquePaintColor,
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
                         bool analyticClip)
        : fOpaquePaintColor(opaquePaintColor)
        , fFinalBlender(finalBlender)
        , fShader(shader)
        , fColorFilter(colorFilter)
        , fPrimitiveBlendMode(primitiveBlendMode)
        , fHasPrimitiveBlender(hasPrimitiveBlender)
        , fSkipColorXform(skipColorXform)
        , fClipShader(clipShader)
        , fRendererCoverage(coverage)
        , fTargetFormat(targetFormat)
        , fDither(dither)
        , fAnalyticClip(analyticClip) {
    if (!fHasPrimitiveBlender) {
        if (fShader.first && fShader.first->priv().isConstant(fShader.second)) {
            fShader = { nullptr, 0 };
        }
        if (!fShader.first && fColorFilter.first) {
            fColorFilter = { nullptr, 0 };
        }
    }
}

void PaintOption::toKey(const KeyContext& keyContext) const {
    // Root Node 0 is the source color, which is the output of all effects post dithering
    this->handleDithering(keyContext);

    // Root Node 1 is the final blender
    std::optional<SkBlendMode> finalBlendMode =
            this->finalBlender() ? this->finalBlender()->priv().asBlendMode()
                                 : SkBlendMode::kSrcOver;

    Coverage finalCoverage = fRendererCoverage;
    if ((fClipShader.first || fAnalyticClip) && fRendererCoverage == Coverage::kNone) {
        finalCoverage = Coverage::kSingleChannel;
    }
    bool dstReadReq = !finalBlendMode.has_value() ||
                      !CanUseHardwareBlending(keyContext.caps(),
                                              fTargetFormat,
                                              *finalBlendMode,
                                              finalCoverage);

    if (finalBlendMode) {
        if (!dstReadReq) {
            AddFixedBlendMode(keyContext, *finalBlendMode);
        } else {
            AddBlendMode(keyContext, *finalBlendMode);
        }
    } else {
        SkASSERT(this->finalBlender());
        fFinalBlender.first->priv().addToKey(keyContext, fFinalBlender.second);
    }

    // Optional Root Node 2 is the clip
    this->handleClipping(keyContext);
}

void PaintOption::addPaintColorToKey(const KeyContext& keyContext) const {
    if (fShader.first) {
        fShader.first->priv().addToKey(keyContext, fShader.second);
    } else {
        RGBPaintColorBlock::AddBlock(keyContext);
    }
}

void PaintOption::handlePrimitiveColor(const KeyContext& keyContext) const {
    if (!fHasPrimitiveBlender) {
        this->addPaintColorToKey(keyContext);
        return;
    }

    if (fSkipColorXform && fPrimitiveBlendMode == SkBlendMode::kDst) {
        AddPrimitiveColor(keyContext, fSkipColorXform);
        return;
    }

    Blend(keyContext,
            /* addBlendToKey= */ [&] () -> void {
                /**
                 * TODO: Allow clients to provide precompile SkBlender options for primitive
                 * blending. For now we have a back door to internally specify an SkBlendMode.
                 */
                AddToKey(keyContext, GetBlendModeSingleton(fPrimitiveBlendMode));
            },
            /* addSrcToKey= */ [&]() -> void {
                this->addPaintColorToKey(keyContext);
            },
            /* addDstToKey= */ [&]() -> void {
                AddPrimitiveColor(keyContext, fSkipColorXform);
            });
}

void PaintOption::handlePaintAlpha(const KeyContext& keyContext) const {

    if (!fShader.first && !fHasPrimitiveBlender) {
        // If there is no shader and no primitive blending the input to the colorFilter stage
        // is just the premultiplied paint color.
        SolidColorShaderBlock::AddBlock(keyContext, SK_PMColor4fWHITE);
        return;
    }

    if (!fOpaquePaintColor) {
        Blend(keyContext,
              /* addBlendToKey= */ [&] () -> void {
                  AddFixedBlendMode(keyContext, SkBlendMode::kSrcIn);
              },
              /* addSrcToKey= */ [&]() -> void {
                  this->handlePrimitiveColor(keyContext);
              },
              /* addDstToKey= */ [&]() -> void {
                  AlphaOnlyPaintColorBlock::AddBlock(keyContext);
              });
    } else {
        this->handlePrimitiveColor(keyContext);
    }
}

void PaintOption::handleColorFilter(const KeyContext& keyContext) const {
    if (fColorFilter.first) {
        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    this->handlePaintAlpha(keyContext);
                },
                /* addOuterToKey= */ [&]() -> void {
                    fColorFilter.first->priv().addToKey(keyContext, fColorFilter.second);
                });
    } else {
        this->handlePaintAlpha(keyContext);
    }
}

// This should be kept in sync w/ SkPaintPriv::ShouldDither and PaintParams::should_dither
bool PaintOption::shouldDither(SkColorType dstCT) const {
    // The paint dither flag can veto.
    if (!fDither) {
        return false;
    }

    if (dstCT == kUnknown_SkColorType) {
        return false;
    }

    // We always dither 565 or 4444 when requested.
    if (dstCT == kRGB_565_SkColorType || dstCT == kARGB_4444_SkColorType) {
        return true;
    }

    // Otherwise, dither is only needed for non-const paints.
    return fShader.first && !fShader.first->priv().isConstant(fShader.second);
}

void PaintOption::handleDithering(const KeyContext& keyContext) const {

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = keyContext.dstColorInfo().colorType();
    if (this->shouldDither(ct)) {
        Compose(keyContext,
                /* addInnerToKey= */ [&]() -> void {
                    this->handleColorFilter(keyContext);
                },
                /* addOuterToKey= */ [&]() -> void {
                    AddDitherBlock(keyContext, ct);
                });
    } else
#endif
    {
        this->handleColorFilter(keyContext);
    }
}

void PaintOption::handleClipping(const KeyContext& keyContext) const {
    if (fAnalyticClip) {
        NonMSAAClipBlock::NonMSAAClipData data(
                /* rect= */ {},
                /* radiusPlusHalf= */ {},
                /* edgeSelect= */ {},
                /* texCoordOffset= */ {},
                /* maskBounds= */ {},
                // TODO: the kAnalyticAndAtlasClip vs. kAnalyticClip decision is based on this
                // being a valid TextureProxy.
                /* atlasTexture= */ nullptr);
        if (fClipShader.first) {
            // For both an analytic clip and clip shader, we need to compose them together into
            // a single clipping root node.
            Blend(keyContext,
                    /* addBlendToKey= */ [&]() -> void {
                        AddFixedBlendMode(keyContext, SkBlendMode::kModulate);
                    },
                    /* addSrcToKey= */ [&]() -> void {
                        NonMSAAClipBlock::AddBlock(keyContext, data);
                    },
                    /* addDstToKey= */ [&]() -> void {
                        fClipShader.first->priv().addToKey(keyContext, fClipShader.second);
                    });
        } else {
            // Without a clip shader, the analytic clip can be the clipping root node.
            NonMSAAClipBlock::AddBlock(keyContext, data);
        }
    } else if (fClipShader.first) {
        // Since there's no analytic clip, the clipping root node can be fClipShader directly.
        fClipShader.first->priv().addToKey(keyContext, fClipShader.second);
    }
}

} // namespace skgpu::graphite
