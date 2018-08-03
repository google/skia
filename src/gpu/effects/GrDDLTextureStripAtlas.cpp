/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDDLTextureStripAtlas.h"

#include "GrContextPriv.h"
#include "GrTexture.h"
#include "SkGr.h"
#include "SkTSearch.h"

GrDDLTextureStripAtlas::GrDDLTextureStripAtlas(const Desc& desc)
        : INHERITED(desc)
        , fAtlasBitmap(nullptr)
        , fMaxNumRows(desc.fHeight / desc.fRowHeight)
        , fCurRow(0)
        , fRows(new AtlasRow[fMaxNumRows]) {
    SkASSERT(fMaxNumRows * fDesc.fRowHeight == fDesc.fHeight);
    SkDEBUGCODE(this->validate();)
}

GrDDLTextureStripAtlas::~GrDDLTextureStripAtlas() { delete[] fRows; }

// Flush the current state of the atlas.
void GrDDLTextureStripAtlas::finish(GrProxyProvider* proxyProvider) {
    SkDEBUGCODE(this->validate();)

    if (!fCurRow) {
        SkASSERT(!fCurProxy && !fAtlasBitmap);
        return;
    }

    int height = fCurRow * fDesc.fRowHeight;
    SkASSERT(height <= fDesc.fHeight);

    SkImageInfo ii = SkImageInfo::Make(fDesc.fWidth, height,
                                       fDesc.fColorType, kPremul_SkAlphaType);
    fAtlasBitmap->allocPixels(ii);

    for (int i = 0; i < fCurRow; ++i) {
        SkASSERT(fRows[i].fBitmap.height() == fDesc.fRowHeight);

        int yPos = i * fDesc.fRowHeight;
        fAtlasBitmap->writePixels(fRows[i].fBitmap.pixmap(), 0, yPos);
    }

    GrUniqueKey key;
    {
        static const GrUniqueKey::Domain kTextureStripAtlasDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&key, kTextureStripAtlasDomain, fCurRow,
                                     "DDL Texture Strip Atlas");
        for (int i = 0; i < fCurRow; ++i) {
            builder[i] = fRows[i].fBitmap.getGenerationID();
        }
        builder.finish();
    }

    sk_sp<GrTextureProxy> interloper = proxyProvider->findProxyByUniqueKey(key,
                                                                           fCurProxy->origin());
    if (!interloper) {
        // In the unlikely event that there is already a proxy with this key (i.e., it has exactly
        // the same strips in exactly the same order) we'll just let it keep the key.
        proxyProvider->assignUniqueKeyToProxy(key, fCurProxy.get());
    }

    // reset the state for the next aggregate texture
    for (int i = 0; i < fCurRow; ++i) {
        fRows[i].fBitmap.reset();
    }
    fCurRow = 0;
    fCurProxy = nullptr;
    fAtlasBitmap = nullptr;
    fKeyTable.rewind();
    SkDEBUGCODE(this->validate();)
}

int GrDDLTextureStripAtlas::addStrip(GrContext* context, const SkBitmap& bitmap) {
    SkDEBUGCODE(this->validate();)

    const int key = bitmap.getGenerationID();
    int index = this->searchByKey(key);

    if (fCurRow >= fMaxNumRows && index < 0) {
        // The current atlas is full and adding another strip would make it overflow. Calve it off
        // and allow the next block to start a new one.
        this->finish(context->contextPriv().proxyProvider());
        index = this->searchByKey(key); // 'finish' cleared the table
    }

    if (!fCurProxy) {
        SkASSERT(!fAtlasBitmap);

        const GrCaps* caps = context->contextPriv().caps();
        GrPixelConfig pixelConfig = SkColorType2GrPixelConfig(fDesc.fColorType);
        SkASSERT(kUnknown_GrPixelConfig != pixelConfig);

        SkBitmap* atlasBitmap = new SkBitmap();

        fCurProxy = GrProxyProvider::MakeFullyLazyProxy(
                [atlasBitmap, pixelConfig](GrResourceProvider* provider) -> sk_sp<GrSurface> {
                        if (!provider) {
                            delete atlasBitmap;
                            return sk_sp<GrSurface>();
                        }
                        // When this is called 'atlasBitmap' should've been filled in and be
                        // non-empty
                        SkASSERT(atlasBitmap->width() && atlasBitmap->height());
                        GrSurfaceDesc desc;
                        desc.fFlags = kNone_GrSurfaceFlags;
                        desc.fWidth = atlasBitmap->width();
                        desc.fHeight = atlasBitmap->height();
                        desc.fConfig = pixelConfig;

                        GrMipLevel mipLevel = { atlasBitmap->getPixels(), atlasBitmap->rowBytes() };

                        return provider->createTexture(desc, SkBudgeted::kYes,
                                                       SkBackingFit::kExact, mipLevel);
                },
                GrProxyProvider::Renderable::kNo, kTopLeft_GrSurfaceOrigin, pixelConfig, *caps);

        fAtlasBitmap = atlasBitmap;
    }

    SkASSERT(bitmap.width() == fDesc.fWidth);
    SkASSERT(bitmap.height() == fDesc.fRowHeight);
    SkASSERT(!context->contextPriv().resourceProvider());  // This atlas class is DDL specific
    SkASSERT(fCurRow < fMaxNumRows);

    int rowNumber = -1;

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
        fRows[fCurRow].fBitmap = bitmap;

        AtlasRow* row = &fRows[rowNumber];
        fKeyTable.insert(index, 1, &row);

        ++fCurRow;
        SkASSERT(fCurRow <= fMaxNumRows);
    }

    SkASSERT(rowNumber >= 0);
    SkDEBUGCODE(this->validate();)
    return rowNumber;
}

int GrDDLTextureStripAtlas::searchByKey(uint32_t generationID) {
    static struct AtlasRowLessFunctor {
        bool operator()(const AtlasRow* row, const uint32_t& id) const {
            return row->fBitmap.getGenerationID() < id;
        }
        bool operator()(const uint32_t& id, const AtlasRow* row) const {
            return id < row->fBitmap.getGenerationID();
        }
    } functor;

    return SkTSearch(fKeyTable.begin(), fKeyTable.count(), generationID, sizeof(AtlasRow*),
                     functor);
}

#ifdef SK_DEBUG
void GrDDLTextureStripAtlas::validate() {
    static const int kBitmapInvalidGenID = 0;

    // Our key table should be sorted
    uint32_t prev = fKeyTable.count() >= 1 ? fKeyTable[0]->fBitmap.getGenerationID() : 0;
    for (int i = 1; i < fKeyTable.count(); ++i) {
        AtlasRow* row = fKeyTable[i];
        SkASSERT(prev < row->fBitmap.getGenerationID());
        SkASSERT(row->fBitmap.getGenerationID() != kBitmapInvalidGenID);
        prev = row->fBitmap.getGenerationID();
    }

    for (int i = 0; i < fCurRow; ++i) {
        // These should all have a valid bitmap and be in the search table
        SkASSERT(fRows[i].fBitmap.getGenerationID() != kBitmapInvalidGenID);
        SkASSERT(this->searchByKey(fRows[i].fBitmap.getGenerationID()) >= 0);
    }
    for (int i = fCurRow; i < fMaxNumRows; ++i) {
        // These should all be empty
        SkASSERT(fRows[i].fBitmap.getGenerationID() == kBitmapInvalidGenID);
    }
}
#endif
