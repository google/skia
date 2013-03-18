/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThread.h"
#include "SkPurgeableImageCache.h"
#include "SkPurgeableMemoryBlock.h"

#ifdef SK_DEBUG
    #include "SkTSearch.h"
#endif

SK_DECLARE_STATIC_MUTEX(gPurgeableImageMutex);

SkImageCache* SkPurgeableImageCache::Create() {
    if (!SkPurgeableMemoryBlock::IsSupported()) {
        return NULL;
    }
    SkAutoMutexAcquire ac(&gPurgeableImageMutex);
    static SkPurgeableImageCache gCache;
    gCache.ref();
    return &gCache;
}

SkPurgeableImageCache::SkPurgeableImageCache() {}

#ifdef SK_DEBUG
SkPurgeableImageCache::~SkPurgeableImageCache() {
    SkASSERT(fRecs.count() == 0);
}
#endif


void* SkPurgeableImageCache::allocAndPinCache(size_t bytes, intptr_t* ID) {
    SkAutoMutexAcquire ac(&gPurgeableImageMutex);

    SkPurgeableMemoryBlock* block = SkPurgeableMemoryBlock::Create(bytes);
    if (NULL == block) {
        return NULL;
    }

    SkPurgeableMemoryBlock::PinResult pinResult;
    void* data = block->pin(&pinResult);
    if (NULL == data) {
        SkDELETE(block);
        return NULL;
    }

    SkASSERT(ID != NULL);
    *ID = reinterpret_cast<intptr_t>(block);
#ifdef SK_DEBUG
    // Insert into the array of all recs:
    int index = this->findRec(*ID);
    SkASSERT(index < 0);
    fRecs.insert(~index, 1, ID);
#endif
    return data;
}

void* SkPurgeableImageCache::pinCache(intptr_t ID, SkImageCache::DataStatus* status) {
    SkASSERT(ID != SkImageCache::UNINITIALIZED_ID);
    SkAutoMutexAcquire ac(&gPurgeableImageMutex);

    SkASSERT(this->findRec(ID) >= 0);
    SkPurgeableMemoryBlock* block = reinterpret_cast<SkPurgeableMemoryBlock*>(ID);
    SkPurgeableMemoryBlock::PinResult pinResult;
    void* data = block->pin(&pinResult);
    if (NULL == data) {
        this->removeRec(ID);
        return NULL;
    }

    switch (pinResult) {
        case SkPurgeableMemoryBlock::kRetained_PinResult:
            *status = SkImageCache::kRetained_DataStatus;
            break;

        case SkPurgeableMemoryBlock::kUninitialized_PinResult:
            *status = SkImageCache::kUninitialized_DataStatus;
            break;

        default:
            // Invalid value. Treat as a failure to pin.
            SkASSERT(false);
            this->removeRec(ID);
            return NULL;
    }

    return data;
}

void SkPurgeableImageCache::releaseCache(intptr_t ID) {
    SkASSERT(ID != SkImageCache::UNINITIALIZED_ID);
    SkAutoMutexAcquire ac(&gPurgeableImageMutex);

    SkASSERT(this->findRec(ID) >= 0);
    SkPurgeableMemoryBlock* block = reinterpret_cast<SkPurgeableMemoryBlock*>(ID);
    block->unpin();
}

void SkPurgeableImageCache::throwAwayCache(intptr_t ID) {
    SkASSERT(ID != SkImageCache::UNINITIALIZED_ID);
    SkAutoMutexAcquire ac(&gPurgeableImageMutex);

    this->removeRec(ID);
}

#ifdef SK_DEBUG
SkImageCache::MemoryStatus SkPurgeableImageCache::getMemoryStatus(intptr_t ID) const {
    SkAutoMutexAcquire ac(&gPurgeableImageMutex);
    if (SkImageCache::UNINITIALIZED_ID == ID || this->findRec(ID) < 0) {
        return SkImageCache::kFreed_MemoryStatus;
    }

    SkPurgeableMemoryBlock* block = reinterpret_cast<SkPurgeableMemoryBlock*>(ID);
    if (block->isPinned()) {
        return SkImageCache::kPinned_MemoryStatus;
    }
    return SkImageCache::kUnpinned_MemoryStatus;
}

void SkPurgeableImageCache::purgeAllUnpinnedCaches() {
    SkAutoMutexAcquire ac(&gPurgeableImageMutex);
    if (SkPurgeableMemoryBlock::PlatformSupportsPurgingAllUnpinnedBlocks()) {
        SkPurgeableMemoryBlock::PurgeAllUnpinnedBlocks();
    } else {
        // Go through the blocks, and purge them individually.
        // Rather than deleting the blocks, which would interfere with further calls, purge them
        // and keep them around.
        for (int i = 0; i < fRecs.count(); i++) {
            SkPurgeableMemoryBlock* block = reinterpret_cast<SkPurgeableMemoryBlock*>(fRecs[i]);
            if (!block->isPinned()) {
                if (!block->purge()) {
                    // FIXME: This should be more meaningful (which one, etc...)
                    SkDebugf("Failed to purge\n");
                }
            }
        }
    }
}

int SkPurgeableImageCache::findRec(intptr_t rec) const {
    return SkTSearch(fRecs.begin(), fRecs.count(), rec, sizeof(intptr_t));
}
#endif

void SkPurgeableImageCache::removeRec(intptr_t ID) {
#ifdef SK_DEBUG
    int index = this->findRec(ID);
    SkASSERT(index >= 0);
    fRecs.remove(index);
#endif
    SkPurgeableMemoryBlock* block = reinterpret_cast<SkPurgeableMemoryBlock*>(ID);
    SkASSERT(!block->isPinned());
    SkDELETE(block);
}
