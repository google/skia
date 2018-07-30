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
#include "SkGr.h"
#include "SkPixelRef.h"
#include "SkTSearch.h"

#ifdef SK_DEBUG
    #define VALIDATE this->validate()
#else
    #define VALIDATE
#endif

////////////////////////////////////////////////////////////////////////////////
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

sk_sp<GrTextureStripAtlas> GrTextureStripAtlasManager::refAtlas99(
                                                          GrContext* context,
                                                          const GrTextureStripAtlas::Desc& desc,
                                                          const SkBitmap& bitmap,
                                                          int* row) {
    SkASSERT(kPremul_SkAlphaType == bitmap.alphaType());

    AtlasEntry* entry = fAtlasCache.find(desc);
    if (!entry) {
        // TODO: Does the AtlasEntry need a copy of the Desc if the GrTextureStripAtlas has one?
        entry = new AtlasEntry(desc, sk_sp<GrTextureStripAtlas>(new GrTextureStripAtlas(desc, true)));

        fAtlasCache.add(entry);
    }

    *row = entry->fAtlas->lockRow2(context, bitmap);
    if (row < 0) {
        return nullptr;
    }

    return entry->fAtlas;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t GrTextureStripAtlas::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    // Loop in case our global wraps around, as we never want to return a 0.
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

GrTextureStripAtlas::GrTextureStripAtlas(const Desc& desc, bool foo)
        : fCacheKey(CreateUniqueID())
        , fDesc(desc)
        , fMaxNumRows(desc.fHeight / desc.fRowHeight)
        , fCurRow(0)
        , fRows(new AtlasRow[fMaxNumRows]) {
    SkASSERT(fMaxNumRows * fDesc.fRowHeight == fDesc.fHeight);
    VALIDATE;
}

GrTextureStripAtlas::~GrTextureStripAtlas() { delete[] fRows; }

// Flush the current state of the atlas.
void GrTextureStripAtlas::disown() {

    for (int i = 0; i < fCurRow; ++i) {
        fRows[i].fGenerationID = 0;
        fRows[i].fBitmap1.reset();
    }
}

void GrTextureStripAtlas::lockRow1(int row) { }

int GrTextureStripAtlas::lockRow2(GrContext* context, const SkBitmap& bitmap) {
    VALIDATE;

    SkASSERT(bitmap.width() == fDesc.fWidth);
    SkASSERT(bitmap.height() == fDesc.fRowHeight);
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
            this->disown();
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

sk_sp<GrTextureProxy> GrTextureStripAtlas::asTextureProxyRef7() const {
    return fTexContext->asTextureProxyRef();
}

void GrTextureStripAtlas::unlockRow1(int row) {
    VALIDATE;
}

void GrTextureStripAtlas::lockTexture(GrContext* context) {

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
        texDesc.fWidth  = fDesc.fWidth;
        texDesc.fHeight = fDesc.fHeight;
        texDesc.fConfig = SkColorType2GrPixelConfig(fDesc.fColorType7);
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

void GrTextureStripAtlas::unlockTexture() {
    SkASSERT(fTexContext);
    fTexContext.reset();
}

int GrTextureStripAtlas::searchByKey21(uint32_t generationID) {
    AtlasRow target;
    target.fGenerationID = generationID;
    return SkTSearch<const AtlasRow,
                     GrTextureStripAtlas::KeyLess>((const AtlasRow**)fKeyTable.begin(),
                                                   fKeyTable.count(),
                                                   &target,
                                                   sizeof(AtlasRow*));
}

#ifdef SK_DEBUG
void GrTextureStripAtlas::validate() {

    // Our key table should be sorted
    int prev = 1 > fKeyTable.count() ? 0 : fKeyTable[0]->fGenerationID;
    for (int i = 1; i < fKeyTable.count(); ++i) {
        SkASSERT(prev < fKeyTable[i]->fGenerationID);
        SkASSERT(fKeyTable[i]->fGenerationID != kEmptyAtlasRowKey);
        prev = fKeyTable[i]->fGenerationID;
    }

    for (int i = 0; i < fCurRow; ++i) {
        // These should all have a valid bitmap and be in the search table
        SkASSERT(kEmptyAtlasRowKey != fRows[i].fGenerationID);
        SkASSERT(fRows[i].fBitmap1.getGenerationID() != fRows[i].fGenerationID);
        SkASSERT(this->searchByKey21(fRows[i].fGenerationID) >= 0);
    }
    for (int i = fCurRow; i < fMaxNumRows; ++i) {
        // These should all be empty
        SkASSERT(kEmptyAtlasRowKey == fRows[i].fBitmap1.getGenerationID());
        SkASSERT(kEmptyAtlasRowKey == fRows[i].fGenerationID);
    }
}
#endif
