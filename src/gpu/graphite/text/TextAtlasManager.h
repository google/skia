/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextAtlasManager_DEFINED
#define skgpu_graphite_TextAtlasManager_DEFINED

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawAtlas.h"

namespace sktext::gpu {
class Glyph;
}
class SkGlyph;

namespace skgpu::graphite {

class Recorder;
class UploadList;

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The TextAtlasManager manages the lifetime of and access to DrawAtlases used in glyph rendering.
 */
class TextAtlasManager : public AtlasGenerationCounter {
public:
    TextAtlasManager(Recorder*);
    ~TextAtlasManager();

    // If getProxies returns nullptr, the client must not try to use other functions on the
    // StrikeCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas.
    const sk_sp<TextureProxy>* getProxies(MaskFormat format,
                                          unsigned int* numActiveProxies) {
        format = this->resolveMaskFormat(format);
        if (this->initAtlas(format)) {
            *numActiveProxies = this->getAtlas(format)->numActivePages();
            return this->getAtlas(format)->getProxies();
        }
        *numActiveProxies = 0;
        return nullptr;
    }

    void freeAll();

    bool hasGlyph(MaskFormat, sktext::gpu::Glyph*);

    DrawAtlas::ErrorCode addGlyphToAtlas(const SkGlyph&,
                                         sktext::gpu::Glyph*,
                                         int srcPadding);

    // To ensure the DrawAtlas does not evict the Glyph Mask from its texture backing store,
    // the client must pass in the current draw token along with the sktext::gpu::Glyph.
    // A BulkUsePlotUpdater is used to manage bulk last use token updating in the Atlas.
    // For convenience, this function will also set the use token for the current glyph if required
    // NOTE: the bulk uploader is only valid if the subrun has a valid atlasGeneration
    void addGlyphToBulkAndSetUseToken(BulkUsePlotUpdater*, MaskFormat,
                                      sktext::gpu::Glyph*, AtlasToken);

    void setUseTokenBulk(const BulkUsePlotUpdater& updater,
                         AtlasToken token,
                         MaskFormat format) {
        this->getAtlas(format)->setLastUseTokenBulk(updater, token);
    }

    bool recordUploads(DrawContext* dc);

    void evictAtlases() {
        for (int i = 0; i < kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->evictAllPlots();
            }
        }
    }

    void postFlush();

    // Some clients may wish to verify the integrity of the texture backing store of the
    // GrDrawOpAtlas. The atlasGeneration returned below is a monotonically increasing number which
    // changes every time something is removed from the texture backing store.
    uint64_t atlasGeneration(skgpu::MaskFormat format) const {
        return this->getAtlas(format)->atlasGeneration();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended debug only

    void setAtlasDimensionsToMinimum_ForTesting();
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    bool initAtlas(MaskFormat);
    // Change an expected 565 mask format to 8888 if 565 is not supported (will happen when using
    // Metal on Intel MacOS). The actual conversion of the data is handled in
    // get_packed_glyph_image() in StrikeCache.cpp
    MaskFormat resolveMaskFormat(MaskFormat format) const;

    // There is a 1:1 mapping between skgpu::MaskFormats and atlas indices
    static int MaskFormatToAtlasIndex(skgpu::MaskFormat format) {
        return static_cast<int>(format);
    }
    static skgpu::MaskFormat AtlasIndexToMaskFormat(int idx) {
        return static_cast<skgpu::MaskFormat>(idx);
    }

    DrawAtlas* getAtlas(skgpu::MaskFormat format) const {
        format = this->resolveMaskFormat(format);
        int atlasIndex = MaskFormatToAtlasIndex(format);
        SkASSERT(fAtlases[atlasIndex]);
        return fAtlases[atlasIndex].get();
    }

    Recorder* fRecorder;
    DrawAtlas::AllowMultitexturing fAllowMultitexturing;
    std::unique_ptr<DrawAtlas> fAtlases[kMaskFormatCount];
    static_assert(kMaskFormatCount == 3);
    bool fSupportBilerpAtlas;
    DrawAtlasConfig fAtlasConfig;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_TextAtlasManager_DEFINED
