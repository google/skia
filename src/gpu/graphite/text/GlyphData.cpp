/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/text/GlyphData.h"

#include "include/private/SkAssert.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"
#include "src/core/SkZip.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/gpu/graphite/text/TextStrike.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/VertexFiller.h"

using MaskFormat = skgpu::MaskFormat;

namespace skgpu::graphite {

GlyphData::GlyphData(sk_sp<TextStrike> strike,
                     Recorder* recorder,
                     sktext::gpu::RendererData renderData)
        : fTextStrike{std::move(strike)}
        , fRenderData(recorder->priv().atlasProvider()
                                     ->textAtlasManager()
                                     ->resolveRendererData(renderData)) {}

GlyphData::~GlyphData() = default;

Glyph GlyphData::makeGlyphFromID(SkPackedGlyphID id) {
    sktext::gpu::PackedGPUGlyphID gpuID{id,
                                        fRenderData.maskFormat,
                                        fRenderData.srcPadding,
                                        fRenderData.isSDF};
    return Glyph{fTextStrike->getGlyph(gpuID)};
}

std::tuple<bool, int> GlyphData::regenerateAtlas(int begin,
                                                 int end,
                                                 sktext::gpu::GlyphVector& glyphVector,
                                                 Recorder* recorder) {
    SkASSERT(glyphVector.hasBackendData());

    auto atlasManager = recorder->priv().atlasProvider()->textAtlasManager();
    auto tokenTracker = recorder->priv().tokenTracker();

    // TODO: this is not a great place for this -- need a better way to init atlases when needed
    unsigned int numActiveProxies;
    const sk_sp<TextureProxy>* proxies = atlasManager->getProxies(fRenderData.maskFormat,
                                                                  &numActiveProxies);
    if (!proxies) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return {false, 0};
    }

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(fRenderData.maskFormat);

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls.
        fBulkUseUpdater.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fTextStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        SkSpan<const Glyph> glyphs = glyphVector.accessBackendGlyphs<Glyph>();
        for (int i = begin; i < end; i++) {
            const Glyph& glyph = glyphs[i];
            SkASSERT(glyph.entry().fKey.maskFormat() == fRenderData.maskFormat);
            SkASSERT(glyph.entry().fKey.padding() == fRenderData.srcPadding);
            SkASSERT(glyph.entry().fKey.isSDF() == fRenderData.isSDF);
            if (!atlasManager->hasGlyph(glyph.entry())) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(glyph.packedID());
                auto code = atlasManager->addGlyphToAtlas(skGlyph, &glyph.entry());
                if (code != DrawAtlas::ErrorCode::kSucceeded) {
                    success = code != DrawAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(&fBulkUseUpdater,
                                                       glyph.entry(),
                                                       tokenTracker->nextFlushToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == glyphVector.glyphCount()) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(fRenderData.maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == glyphVector.glyphCount()) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(
                    fBulkUseUpdater, tokenTracker->nextFlushToken(), fRenderData.maskFormat);
        }
        return {true, end - begin};
    }
}

namespace {
struct AtlasPt {
    uint16_t u;
    uint16_t v;
};
}

void GlyphData::fillInstanceData(const sktext::gpu::VertexFiller& vf,
                                 SkSpan<const Glyph> glyphs,
                                 DrawWriter* dw,
                                 int offset,
                                 int count,
                                 unsigned short flags,
                                 uint32_t ssboIndex,
                                 SkScalar depth) const {
    auto quadData = [&]() {
        return SkMakeZip(glyphs.subspan(offset, count), vf.topLefts().subspan(offset, count));
    };

    DrawWriter::Instances instances{*dw, {}, {}, 4};
    instances.reserve(count);
    // Need to send width, height, uvPos, xyPos, and strikeToSourceScale
    // pre-transform coords = (s*w*b_x + t_x, s*h*b_y + t_y)
    // where (b_x, b_y) are the vertexID coords
    for (auto [glyph, leftTop] : quadData()) {
        auto [al, at, ar, ab] = glyph.entry().fAtlasLocator.getUVs();
        instances.append(1) << AtlasPt{uint16_t(ar-al), uint16_t(ab-at)}
                            << AtlasPt{uint16_t(al & 0x1fff), at}
                            << leftTop << /*index=*/uint16_t(al >> 13) << flags
                            << 1.0f
                            << depth << ssboIndex;
    }
}

}  // namespace skgpu::graphite
