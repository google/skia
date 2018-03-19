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

sk_sp<GrTextureStripAtlas> GrTextureStripAtlasManager::refAtlas(
                                                          const GrTextureStripAtlas::Desc& desc) {
    AtlasEntry* entry = fAtlasCache.find(desc);
    if (!entry) {
        // TODO: Does the AtlasEntry need a copy of the Desc if the GrTextureStripAtlas has one?
        entry = new AtlasEntry(desc, sk_sp<GrTextureStripAtlas>(new GrTextureStripAtlas(desc)));

        fAtlasCache.add(entry);
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

GrTextureStripAtlas::GrTextureStripAtlas(const Desc& desc)
    : fCacheKey(CreateUniqueID())
    , fLockedRows(0)
    , fDesc(desc)
    , fNumRows(desc.fHeight / desc.fRowHeight)
    , fRows(new AtlasRow[fNumRows])
    , fLRUFront(nullptr)
    , fLRUBack(nullptr) {
    SkASSERT(fNumRows * fDesc.fRowHeight == fDesc.fHeight);
    this->initLRU();
    fNormalizedYHeight = SK_Scalar1 / fDesc.fHeight;
    VALIDATE;
}

GrTextureStripAtlas::~GrTextureStripAtlas() { delete[] fRows; }

void GrTextureStripAtlas::lockRow(int row) {
    // This should only be called on a row that is already locked.
    SkASSERT(fRows[row].fLocks);
    fRows[row].fLocks++;
    ++fLockedRows;
}

int GrTextureStripAtlas::lockRow(GrContext* context, const SkBitmap& bitmap) {
    VALIDATE;

    if (!context->contextPriv().resourceProvider()) {
        // DDL TODO: For DDL we need to schedule inline & ASAP uploads. However these systems
        // currently use the flushState which we can't use for the opList-based DDL phase.
        // For the opList-based solution every texture strip will get its own texture proxy.
        // We will revisit this for the flushState-based solution.
        return -1;
    }

    if (0 == fLockedRows) {
        this->lockTexture(context);
        if (!fTexContext) {
            return -1;
        }
    }

    int key = bitmap.getGenerationID();
    int rowNumber = -1;
    int index = this->searchByKey(key);

    if (index >= 0) {
        // We already have the data in a row, so we can just return that row
        AtlasRow* row = fKeyTable[index];
        if (0 == row->fLocks) {
            this->removeFromLRU(row);
        }
        ++row->fLocks;
        ++fLockedRows;

        // Since all the rows are always stored in a contiguous array, we can save the memory
        // required for storing row numbers and just compute it with some pointer arithmetic
        rowNumber = static_cast<int>(row - fRows);
    } else {
        // ~index is the index where we will insert the new key to keep things sorted
        index = ~index;

        // We don't have this data cached, so pick the least recently used row to copy into
        AtlasRow* row = this->getLRU();

        ++fLockedRows;

        if (nullptr == row) {
            // force a flush, which should unlock all the rows; then try again
            context->contextPriv().flush(nullptr); // tighten this up?
            row = this->getLRU();
            if (nullptr == row) {
                --fLockedRows;
                return -1;
            }
        }

        this->removeFromLRU(row);

        uint32_t oldKey = row->fKey;

        // If we are writing into a row that already held bitmap data, we need to remove the
        // reference to that genID which is stored in our sorted table of key values.
        if (oldKey != kEmptyAtlasRowKey) {

            // Find the entry in the list; if it's before the index where we plan on adding the new
            // entry, we decrement since it will shift elements ahead of it back by one.
            int oldIndex = this->searchByKey(oldKey);
            if (oldIndex < index) {
                --index;
            }

            fKeyTable.remove(oldIndex);
        }

        row->fKey = key;
        row->fLocks = 1;
        fKeyTable.insert(index, 1, &row);
        rowNumber = static_cast<int>(row - fRows);

        SkASSERT(bitmap.width() == fDesc.fWidth);
        SkASSERT(bitmap.height() == fDesc.fRowHeight);

        // Pass in the kDontFlush flag, since we know we're writing to a part of this texture
        // that is not currently in use
        fTexContext->writePixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                 0, rowNumber * fDesc.fRowHeight,
                                 GrContextPriv::kDontFlush_PixelOpsFlag);
    }

    SkASSERT(rowNumber >= 0);
    VALIDATE;
    return rowNumber;
}

sk_sp<GrTextureProxy> GrTextureStripAtlas::asTextureProxyRef() const {
    return fTexContext->asTextureProxyRef();
}

void GrTextureStripAtlas::unlockRow(int row) {
    VALIDATE;
    --fRows[row].fLocks;
    --fLockedRows;
    SkASSERT(fRows[row].fLocks >= 0 && fLockedRows >= 0);
    if (0 == fRows[row].fLocks) {
        this->appendLRU(fRows + row);
    }
    if (0 == fLockedRows) {
        this->unlockTexture();
    }
    VALIDATE;
}

GrTextureStripAtlas::AtlasRow* GrTextureStripAtlas::getLRU() {
    // Front is least-recently-used
    AtlasRow* row = fLRUFront;
    return row;
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
        texDesc.fConfig = fDesc.fConfig;

        proxy = proxyProvider->createProxy(texDesc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact,
                                           SkBudgeted::kYes, GrInternalSurfaceFlags::kNoPendingIO);
        if (!proxy) {
            return;
        }

        SkASSERT(proxy->origin() == kTopLeft_GrSurfaceOrigin);
        proxyProvider->assignUniqueKeyToProxy(key, proxy.get());
        // This is a new texture, so all of our cache info is now invalid
        this->initLRU();
        fKeyTable.rewind();
    }
    SkASSERT(proxy);
    fTexContext = context->contextPriv().makeWrappedSurfaceContext(std::move(proxy));
}

void GrTextureStripAtlas::unlockTexture() {
    SkASSERT(fTexContext && 0 == fLockedRows);
    fTexContext.reset();
}

void GrTextureStripAtlas::initLRU() {
    fLRUFront = nullptr;
    fLRUBack = nullptr;
    // Initially all the rows are in the LRU list
    for (int i = 0; i < fNumRows; ++i) {
        fRows[i].fKey = kEmptyAtlasRowKey;
        fRows[i].fNext = nullptr;
        fRows[i].fPrev = nullptr;
        this->appendLRU(fRows + i);
    }
    SkASSERT(nullptr == fLRUFront || nullptr == fLRUFront->fPrev);
    SkASSERT(nullptr == fLRUBack || nullptr == fLRUBack->fNext);
}

void GrTextureStripAtlas::appendLRU(AtlasRow* row) {
    SkASSERT(nullptr == row->fPrev && nullptr == row->fNext);
    if (nullptr == fLRUFront && nullptr == fLRUBack) {
        fLRUFront = row;
        fLRUBack = row;
    } else {
        row->fPrev = fLRUBack;
        fLRUBack->fNext = row;
        fLRUBack = row;
    }
}

void GrTextureStripAtlas::removeFromLRU(AtlasRow* row) {
    SkASSERT(row);
    if (row->fNext && row->fPrev) {
        row->fPrev->fNext = row->fNext;
        row->fNext->fPrev = row->fPrev;
    } else {
        if (nullptr == row->fNext) {
            SkASSERT(row == fLRUBack);
            fLRUBack = row->fPrev;
            if (fLRUBack) {
                fLRUBack->fNext = nullptr;
            }
        }
        if (nullptr == row->fPrev) {
            SkASSERT(row == fLRUFront);
            fLRUFront = row->fNext;
            if (fLRUFront) {
                fLRUFront->fPrev = nullptr;
            }
        }
    }
    row->fNext = nullptr;
    row->fPrev = nullptr;
}

int GrTextureStripAtlas::searchByKey(uint32_t key) {
    AtlasRow target;
    target.fKey = key;
    return SkTSearch<const AtlasRow,
                     GrTextureStripAtlas::KeyLess>((const AtlasRow**)fKeyTable.begin(),
                                                   fKeyTable.count(),
                                                   &target,
                                                   sizeof(AtlasRow*));
}

#ifdef SK_DEBUG
void GrTextureStripAtlas::validate() {

    // Our key table should be sorted
    uint32_t prev = 1 > fKeyTable.count() ? 0 : fKeyTable[0]->fKey;
    for (int i = 1; i < fKeyTable.count(); ++i) {
        SkASSERT(prev < fKeyTable[i]->fKey);
        SkASSERT(fKeyTable[i]->fKey != kEmptyAtlasRowKey);
        prev = fKeyTable[i]->fKey;
    }

    int lruCount = 0;
    // Validate LRU pointers, and count LRU entries
    SkASSERT(nullptr == fLRUFront || nullptr == fLRUFront->fPrev);
    SkASSERT(nullptr == fLRUBack  || nullptr == fLRUBack->fNext);
    for (AtlasRow* r = fLRUFront; r != nullptr; r = r->fNext) {
        if (nullptr == r->fNext) {
            SkASSERT(r == fLRUBack);
        } else {
            SkASSERT(r->fNext->fPrev == r);
        }
        ++lruCount;
    }

    int rowLocks = 0;
    int freeRows = 0;

    for (int i = 0; i < fNumRows; ++i) {
        rowLocks += fRows[i].fLocks;
        if (0 == fRows[i].fLocks) {
            ++freeRows;
            bool inLRU = false;
            // Step through the LRU and make sure it's present
            for (AtlasRow* r = fLRUFront; r != nullptr; r = r->fNext) {
                if (r == &fRows[i]) {
                    inLRU = true;
                    break;
                }
            }
            SkASSERT(inLRU);
        } else {
            // If we are locked, we should have a key
            SkASSERT(kEmptyAtlasRowKey != fRows[i].fKey);
        }

        // If we have a key != kEmptyAtlasRowKey, it should be in the key table
        SkASSERT(fRows[i].fKey == kEmptyAtlasRowKey || this->searchByKey(fRows[i].fKey) >= 0);
    }

    // Our count of locks should equal the sum of row locks, unless we ran out of rows and flushed,
    // in which case we'll have one more lock than recorded in the rows (to represent the pending
    // lock of a row; which ensures we don't unlock the texture prematurely).
    SkASSERT(rowLocks == fLockedRows || rowLocks + 1 == fLockedRows);

    // We should have one lru entry for each free row
    SkASSERT(freeRows == lruCount);

    // If we have locked rows, we should have a locked texture, otherwise
    // it should be unlocked
    if (fLockedRows == 0) {
        SkASSERT(!fTexContext);
    } else {
        SkASSERT(fTexContext);
    }
}
#endif
