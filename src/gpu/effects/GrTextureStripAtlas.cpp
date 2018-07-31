/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureStripAtlas.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrSurfaceContext.h"
#include "GrTexture.h"
#include "SkGr.h"
#include "SkPixelRef.h"
#include "SkTSearch.h"

#ifdef SK_DEBUG
    #define VALIDATE this->validate()
#else
    #define VALIDATE
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
GrTextureStripAtlasManager::~GrTextureStripAtlasManager() {
    this->deleteAllAtlases();
}

void GrTextureStripAtlasManager::deleteAllAtlases() {
    AtlasHash::Iter iter(&fAtlasCache);
    while (!iter.done()) {
        AtlasEntry* tmp = &(*iter);
        ++iter;
        delete tmp;
    }
    fAtlasCache.reset();
}

void GrTextureStripAtlasManager::abandon() {
    this->deleteAllAtlases();
}

sk_sp<GrTextureStripAtlasBase> GrTextureStripAtlasManager::addStrip(
                                                          GrContext* context,
                                                          const GrTextureStripAtlasBase::Desc1& desc,
                                                          const SkBitmap& bitmap,
                                                          int* row) {
    SkASSERT(kPremul_SkAlphaType == bitmap.alphaType());

    AtlasEntry* entry = fAtlasCache.find(desc);
    if (!entry) {
        sk_sp<GrTextureStripAtlasBase> atlas;

        if (!context->contextPriv().resourceProvider()) {
            atlas.reset(new GrDDLTextureStripAtlas(context->contextPriv().caps(), desc));
        } else {

        }

        entry = new AtlasEntry(sk_sp<GrTextureStripAtlasBase>(std::move(atlas)));

        fAtlasCache.add(entry);
    }

    *row = entry->fAtlas->addStrip(context, bitmap);
    if (row < 0) {
        return nullptr;
    }

    return entry->fAtlas;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
GrDDLTextureStripAtlas::GrDDLTextureStripAtlas(const GrCaps* caps, const Desc1& desc)
        : INHERITED(desc)
        , fMaxNumRows(desc.fHeight / desc.fRowHeight)
        , fCurRow(0)
        , fRows(new AtlasRow[fMaxNumRows]) {
    SkASSERT(fMaxNumRows * this->desc().fRowHeight == this->desc().fHeight);

    GrPixelConfig pixelConfig = SkColorType2GrPixelConfig(desc.fColorType7);

    fCurProxy = GrProxyProvider::MakeFullyLazyProxy(
            [this, pixelConfig](GrResourceProvider* resourceProvider) -> sk_sp<GrSurface> {
                    if (!resourceProvider) {
                        return sk_sp<GrSurface>();
                    }
                    GrSurfaceDesc desc;
                    desc.fFlags = kNone_GrSurfaceFlags;
                    desc.fWidth = this->desc().fWidth;
                    desc.fHeight = fCurRow;
                    desc.fConfig = pixelConfig;

                    SkPixmap pixMap;
  //                  SkAssertResult(srcImage->peekPixels(&pixMap));
                    GrMipLevel mipLevel = { pixMap.addr(), pixMap.rowBytes() };

                    return resourceProvider->createTexture(desc, SkBudgeted::kYes,
                                                                                  SkBackingFit::kExact, mipLevel);
//                    return sk_ref_sp<GrSurface>(tex.release());
            },
            GrProxyProvider::Renderable::kNo, kTopLeft_GrSurfaceOrigin, pixelConfig, *caps);

    VALIDATE;
}

GrDDLTextureStripAtlas::~GrDDLTextureStripAtlas() { delete[] fRows; }

// Flush the current state of the atlas.
void GrDDLTextureStripAtlas::finish(GrContext* context) {

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    GrUniqueKey key;
    {
        static const GrUniqueKey::Domain kTextureStripAtlasDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&key, kTextureStripAtlasDomain, fCurRow, "Texture Strip Atlas");
        for (int i = 0; i < fCurRow; ++i) {
            builder[i] = fRows[i].fBitmap1.getGenerationID();
        }
        builder.finish();
    }

    if (!proxyProvider->assignUniqueKeyToProxy(key, fCurProxy.get())) {
        sk_sp<GrTextureProxy> newproxy = proxyProvider->findProxyByUniqueKey(key, kTopLeft_GrSurfaceOrigin);
        // miraculously, there is already a proxy with the exact same set of strips
        return;
    }

    for (int i = 0; i < fCurRow; ++i) {
        fRows[i].fBitmap1.reset();
    }
}

int GrDDLTextureStripAtlas::addStrip(GrContext* context, const SkBitmap& bitmap) {
    VALIDATE;

    SkASSERT(bitmap.width() == this->desc().fWidth);
    SkASSERT(bitmap.height() == this->desc().fRowHeight);
    SkASSERT(!context->contextPriv().resourceProvider());  // This is DDL specific
    SkASSERT(fCurRow < fMaxNumRows);

#if 0
    if (!fAtlasBitmap) {
        SkImageInfo ii = SkImageInfo::Make(fDesc.fWidth, fDesc.fHeight,
                                           fDesc.fColorType7, kPremul_SkAlphaType);

        fAtlasBitmap = new SkBitmap;

    }


    this->lockTexture(context);
    if (!fTexContext) {
        return -1;
    }
#endif

    int key = bitmap.getGenerationID();
    int rowNumber = -1;
    int index = this->searchByKey21(key);

    if (index >= 0) {
        // We already have the data in a row, so we can just return that row
        AtlasRow* row = fKeyTable[index];

        // Since all the rows are always stored in a contiguous array, we can save the memory
        // required for storing row numbers and just compute it with some pointer arithmetic
        rowNumber = static_cast<int>(row - fRows);
    } else {
        // ~index is the index where we will insert the new key to keep things sorted
        index = ~index;

        rowNumber = fCurRow;
        fRows[fCurRow].fBitmap1 = bitmap;

        AtlasRow* row = &fRows[rowNumber];
        fKeyTable.insert(index, 1, &row);

        ++fCurRow;
        if (fCurRow >= fMaxNumRows) {
            this->finish(context);
        }

#if 0
        // Pass in the kDontFlush flag, since we know we're writing to a part of this texture
        // that is not currently in use
        fTexContext->writePixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                 0, rowNumber * fDesc.fRowHeight,
                                 GrContextPriv::kDontFlush_PixelOpsFlag);
#endif
    }

    SkASSERT(rowNumber >= 0);
    VALIDATE;
    return rowNumber;
}

sk_sp<GrTextureProxy> GrDDLTextureStripAtlas::asTextureProxyRef7() const { return fCurProxy; }

#if 0
void GrDDLTextureStripAtlas::lockTexture(GrContext* context) {

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 1);
    builder[0] = static_cast<uint32_t>(fCacheKey);
    builder.finish();

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->findOrCreateProxyByUniqueKey(
                                                                key, kTopLeft_GrSurfaceOrigin);
    if (!proxy) {
        GrSurfaceDesc texDesc;
        texDesc.fWidth  = this->desc().fWidth;
        texDesc.fHeight = this->desc().fHeight;
        texDesc.fConfig = SkColorType2GrPixelConfig(this->desc().fColorType7);
        SkASSERT(kUnknown_GrPixelConfig != texDesc.fConfig);

        proxy = proxyProvider->createProxy(texDesc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact,
                                           SkBudgeted::kYes, GrInternalSurfaceFlags::kNoPendingIO);
        if (!proxy) {
            return;
        }

        SkASSERT(proxy->origin() == kTopLeft_GrSurfaceOrigin);
        proxyProvider->assignUniqueKeyToProxy(key, proxy.get());
        fKeyTable.rewind();
    }
    SkASSERT(proxy);
    fTexContext = context->contextPriv().makeWrappedSurfaceContext(std::move(proxy));
}

void GrDDLTextureStripAtlas::unlockTexture() {
    SkASSERT(fTexContext);
    fTexContext.reset();
}
#endif

int GrDDLTextureStripAtlas::searchByKey21(uint32_t generationID) {
    static struct foo {
        bool operator()(const AtlasRow* row, const uint32_t& id) const { return row->fBitmap1.getGenerationID() < id; }
        bool operator()(const uint32_t& id, const AtlasRow* row) const { return id < row->fBitmap1.getGenerationID(); }
    } bar;

    return SkTSearch<const AtlasRow*, uint32_t, struct foo>(const_cast<const AtlasRow**>(fKeyTable.begin()),
                                                      fKeyTable.count(),
                                                      generationID,
                                                      sizeof(AtlasRow*),
                                                      bar);
}

#ifdef SK_DEBUG
void GrDDLTextureStripAtlas::validate() {

    // Our key table should be sorted
    uint32_t prev = fKeyTable.count() >= 1 ? fKeyTable[0]->fBitmap1.getGenerationID() : 0;
    for (int i = 1; i < fKeyTable.count(); ++i) {
        SkASSERT(prev < fKeyTable[i]->fBitmap1.getGenerationID());
        SkASSERT(fKeyTable[i]->fBitmap1.getGenerationID() != SkBitmap::kInvalidGenID);
        prev = fKeyTable[i]->fBitmap1.getGenerationID();
    }

    for (int i = 0; i < fCurRow; ++i) {
        // These should all have a valid bitmap and be in the search table
        SkASSERT(fRows[i].fBitmap1.getGenerationID() != SkBitmap::kInvalidGenID);
        SkASSERT(this->searchByKey21(fRows[i].fBitmap1.getGenerationID()) >= 0);
    }
    for (int i = fCurRow; i < fMaxNumRows; ++i) {
        // These should all be empty
        SkASSERT(fRows[i].fBitmap1.getGenerationID() == SkBitmap::kInvalidGenID);
    }
}
#endif
