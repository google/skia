/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasManager_DEFINED
#define GrAtlasManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrOnFlushResourceProvider.h"
#include "src/gpu/ganesh/GrProxyProvider.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class GrDeferredUploadTarget;
class GrResourceProvider;
class GrSurfaceProxyView;
class SkGlyph;

namespace sktext::gpu {
class Glyph;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The GrAtlasManager manages the lifetime of and access to GrDrawOpAtlases.
 *  It is only available at flush and only via the GrOpFlushState.
 *
 *  This implies that all of the advanced atlasManager functionality (i.e.,
 *  adding glyphs to the atlas) are only available at flush time.
 */
class GrAtlasManager : public GrOnFlushCallbackObject, public skgpu::AtlasGenerationCounter {
public:
    GrAtlasManager(GrProxyProvider*,
                   size_t maxTextureBytes,
                   GrDrawOpAtlas::AllowMultitexturing,
                   bool supportBilerpAtlas);
    ~GrAtlasManager() override;

    // if getViews returns nullptr, the client must not try to use other functions on the
    // StrikeCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas. Note that we can have proxies available but none active
    // (i.e., none instantiated).
    const GrSurfaceProxyView* getViews(skgpu::MaskFormat format, unsigned int* numActiveProxies) {
        format = this->resolveMaskFormat(format);
        if (this->initAtlas(format)) {
            *numActiveProxies = this->getAtlas(format)->numActivePages();
            return this->getAtlas(format)->getViews();
        }
        *numActiveProxies = 0;
        return nullptr;
    }

    void freeAll();

    bool hasGlyph(skgpu::MaskFormat, sktext::gpu::Glyph*);

    GrDrawOpAtlas::ErrorCode addGlyphToAtlas(const SkGlyph&,
                                             sktext::gpu::Glyph*,
                                             int srcPadding,
                                             GrResourceProvider*,
                                             GrDeferredUploadTarget*);

    // To ensure the GrDrawOpAtlas does not evict the Glyph Mask from its texture backing store,
    // the client must pass in the current op token along with the sktext::gpu::Glyph.
    // A BulkUsePlotUpdater is used to manage bulk last use token updating in the Atlas.
    // For convenience, this function will also set the use token for the current glyph if required
    // NOTE: the bulk uploader is only valid if the subrun has a valid atlasGeneration
    void addGlyphToBulkAndSetUseToken(skgpu::BulkUsePlotUpdater*, skgpu::MaskFormat,
                                      sktext::gpu::Glyph*, skgpu::AtlasToken);

    void setUseTokenBulk(const skgpu::BulkUsePlotUpdater& updater,
                         skgpu::AtlasToken token,
                         skgpu::MaskFormat format) {
        this->getAtlas(format)->setLastUseTokenBulk(updater, token);
    }

    // add to texture atlas that matches this format
    GrDrawOpAtlas::ErrorCode addToAtlas(GrResourceProvider*, GrDeferredUploadTarget*,
                                        skgpu::MaskFormat, int width, int height, const void* image,
                                        skgpu::AtlasLocator*);

    // Some clients may wish to verify the integrity of the texture backing store of the
    // GrDrawOpAtlas. The atlasGeneration returned below is a monotonically increasing number which
    // changes every time something is removed from the texture backing store.
    uint64_t atlasGeneration(skgpu::MaskFormat format) const {
        return this->getAtlas(format)->atlasGeneration();
    }

    // GrOnFlushCallbackObject overrides

    bool preFlush(GrOnFlushResourceProvider* onFlushRP) override {
#if defined(GPU_TEST_UTILS)
        if (onFlushRP->failFlushTimeCallbacks()) {
            return false;
        }
#endif

        for (int i = 0; i < skgpu::kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->instantiate(onFlushRP);
            }
        }
        return true;
    }

    void postFlush(skgpu::AtlasToken startTokenForNextFlush) override {
        for (int i = 0; i < skgpu::kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->compact(startTokenForNextFlush);
            }
        }
    }

    // The AtlasGlyph cache always survives freeGpuResources so we want it to remain in the active
    // OnFlushCallbackObject list
    bool retainOnFreeGpuResources() override { return true; }

private:
    friend class GrAtlasManagerTools;
    bool initAtlas(skgpu::MaskFormat);
    // Change an expected 565 mask format to 8888 if 565 is not supported (will happen when using
    // Metal on macOS). The actual conversion of the data is handled in get_packed_glyph_image() in
    // StrikeCache.cpp
    skgpu::MaskFormat resolveMaskFormat(skgpu::MaskFormat format) const {
        if (skgpu::MaskFormat::kA565 == format &&
            !fProxyProvider->caps()->getDefaultBackendFormat(GrColorType::kBGR_565,
                                                             GrRenderable::kNo).isValid()) {
            format = skgpu::MaskFormat::kARGB;
        }
        return format;
    }

    // There is a 1:1 mapping between skgpu::MaskFormats and atlas indices
    static int MaskFormatToAtlasIndex(skgpu::MaskFormat format) {
        return static_cast<int>(format);
    }
    static skgpu::MaskFormat AtlasIndexToMaskFormat(int idx) {
        return static_cast<skgpu::MaskFormat>(idx);
    }

    GrDrawOpAtlas* getAtlas(skgpu::MaskFormat format) const {
        format = this->resolveMaskFormat(format);
        int atlasIndex = MaskFormatToAtlasIndex(format);
        SkASSERT(fAtlases[atlasIndex]);
        return fAtlases[atlasIndex].get();
    }

    GrDrawOpAtlas::AllowMultitexturing fAllowMultitexturing;
    std::unique_ptr<GrDrawOpAtlas> fAtlases[skgpu::kMaskFormatCount];
    static_assert(skgpu::kMaskFormatCount == 3);
    bool fSupportBilerpAtlas;
    GrProxyProvider* fProxyProvider;
    sk_sp<const GrCaps> fCaps;
    GrDrawOpAtlasConfig fAtlasConfig;

    using INHERITED = GrOnFlushCallbackObject;
};

#endif // GrAtlasManager_DEFINED
