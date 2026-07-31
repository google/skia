/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextAtlasManager_DEFINED
#define skgpu_graphite_TextAtlasManager_DEFINED

#include "include/core/SkRefCnt.h"  // IWYU pragma: keep
#include "include/private/SkAssert.h"
#include "src/gpu/MaskFormat.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/text/gpu/GlyphVector.h"

#include <cstdint>
#include <memory>

class SkGlyph;

namespace skgpu::graphite {

class DrawContext;
struct GlyphEntry;
class Recorder;
class TextureProxy;

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The TextAtlasManager manages the lifetime of and access to DrawAtlases used in glyph rendering.
 */
class TextAtlasManager : public DrawAtlas::GenerationCounter {
public:
    // For text there are three atlases (A8, 565, ARGB) that are kept in relation with one another.
    // In general, because A8 is the most frequently used mask format its dimensions are 2x the 565
    // and ARGB dimensions, with the constraint that an atlas size will always contain at least one
    // plot. Since the ARGB atlas takes the most space, its dimensions are used to size the other
    // two atlases.
    class AtlasConfig {
    public:
        // The capabilities of the GPU define maxTextureSize. The client provides maxBytes, and this
        // represents the largest they want a single atlas texture to be. Due to multitexturing, we
        // may expand temporarily to use more space as needed.
        AtlasConfig(int maxTextureSize, size_t maxBytes);

        SkISize atlasDimensions(MaskFormat type) const;
        SkISize plotDimensions(MaskFormat type) const;

    private:
        // On some systems texture coordinates are represented using half-precision floating point
        // with 11 significant bits, which limits the largest atlas dimensions to 2048x2048.
        // For simplicity we'll use this constraint for all of our atlas textures.
        // This can be revisited later if we need larger atlases.
        inline static constexpr int kMaxAtlasDim = 2048;

        SkISize fARGBDimensions;
        int fMaxTextureSize;
    };

    TextAtlasManager(Recorder*);
    ~TextAtlasManager();

    // If getProxies returns nullptr, the client must not try to use other functions on the
    // StrikeCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas.
    const sk_sp<TextureProxy>* getProxies(MaskFormat resolvedMaskformat,
                                          unsigned int* numActiveProxies) {
        if (this->initAtlas(resolvedMaskformat)) {
            *numActiveProxies = this->getAtlas(resolvedMaskformat)->numActivePages();
            return this->getAtlas(resolvedMaskformat)->getProxies();
        }
        *numActiveProxies = 0;
        return nullptr;
    }

    sktext::gpu::RendererData resolveRendererData(sktext::gpu::RendererData data) {
        // Bump direct mask glyphs (zero padding) up to 1px when the atlas can be bilerp'ed
        data.srcPadding = data.srcPadding == 0 && fSupportBilerpAtlas ? 1 : data.srcPadding;
        data.maskFormat = this->resolveMaskFormat(data.maskFormat);
        // Leave SDF and LCD properties alone
        return data;
    }

    void freeGpuResources();

    bool hasGlyph(const GlyphEntry&);

    DrawAtlas::ErrorCode addGlyphToAtlas(const SkGlyph&, GlyphEntry*);

    // To ensure the DrawAtlas does not evict the Glyph Mask from its texture backing store,
    // the client must pass in the current draw token along with the Glyph.
    // A BulkUsePlotUpdater is used to manage bulk last use token updating in the Atlas.
    // For convenience, this function will also set the use token for the current glyph if required
    // NOTE: the bulk uploader is only valid if the subrun has a valid atlasGeneration
    void addGlyphToBulkAndSetUseToken(DrawAtlas::BulkUsePlotUpdater*, const GlyphEntry&, Token);

    void setUseTokenBulk(const DrawAtlas::BulkUsePlotUpdater& updater,
                         Token token,
                         MaskFormat resolvedMaskFormat) {
        this->getAtlas(resolvedMaskFormat)->setLastUseTokenBulk(updater, token);
    }

    bool recordUploads(DrawContext* dc);

    void evictAtlases() {
        for (int i = 0; i < kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->evictAllPlots();
            }
        }
    }

    void compact();

    // Some clients may wish to verify the integrity of the texture backing store of the
    // DrawAtlas. The atlasGeneration returned below is a monotonically increasing number which
    // changes every time something is removed from the texture backing store.
    uint64_t atlasGeneration(skgpu::MaskFormat resolvedMaskFormat) const {
        return this->getAtlas(resolvedMaskFormat)->atlasGeneration();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended debug only

    void setAtlasDimensionsToMinimum_ForTesting();
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    bool initAtlas(MaskFormat resolvedMaskFormat);
    // Change an expected 565 mask format to 8888 if 565 is not supported (will happen when using
    // Metal on Intel MacOS). The actual conversion of the data is handled in
    // get_packed_glyph_image() in StrikeCache.cpp
    MaskFormat resolveMaskFormat(MaskFormat format) const;

    // There is a 1:1 mapping between skgpu::MaskFormats and atlas indices
    static int MaskFormatToAtlasIndex(MaskFormat format) {
        return static_cast<int>(format);
    }
    static MaskFormat AtlasIndexToMaskFormat(int idx) {
        return static_cast<MaskFormat>(idx);
    }

    DrawAtlas* getAtlas(MaskFormat resolvedMaskFormat) const {
        SkASSERT(resolvedMaskFormat == this->resolveMaskFormat(resolvedMaskFormat));
        int atlasIndex = MaskFormatToAtlasIndex(resolvedMaskFormat);
        SkASSERT(fAtlases[atlasIndex]);
        return fAtlases[atlasIndex].get();
    }

    Recorder* fRecorder;
    DrawAtlas::AllowMultitexturing fAllowMultitexturing;
    std::unique_ptr<DrawAtlas> fAtlases[kMaskFormatCount];
    static_assert(kMaskFormatCount == 3);
    bool fSupportBilerpAtlas;
    AtlasConfig fAtlasConfig;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_TextAtlasManager_DEFINED
