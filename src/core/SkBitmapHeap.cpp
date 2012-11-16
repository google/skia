
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapHeap.h"

#include "SkBitmap.h"
#include "SkFlattenableBuffers.h"
#include "SkTSearch.h"

SK_DEFINE_INST_COUNT(SkBitmapHeapReader)
SK_DEFINE_INST_COUNT(SkBitmapHeap::ExternalStorage)

SkBitmapHeapEntry::SkBitmapHeapEntry()
    : fSlot(-1)
    , fRefCount(0)
    , fBytesAllocated(0) {
}

SkBitmapHeapEntry::~SkBitmapHeapEntry() {
    SkASSERT(0 == fRefCount);
}

void SkBitmapHeapEntry::addReferences(int count) {
    if (0 == fRefCount) {
        // If there are no current owners then the heap manager
        // will be the only one able to modify it, so it does not
        // need to be an atomic operation.
        fRefCount = count;
    } else {
        sk_atomic_add(&fRefCount, count);
    }
}

///////////////////////////////////////////////////////////////////////////////

int SkBitmapHeap::LookupEntry::Compare(const SkBitmapHeap::LookupEntry *a,
                                       const SkBitmapHeap::LookupEntry *b) {
    if (a->fGenerationId < b->fGenerationId) {
        return -1;
    } else if (a->fGenerationId > b->fGenerationId) {
        return 1;
    } else if (a->fPixelOffset < b->fPixelOffset) {
        return -1;
    } else if (a->fPixelOffset > b->fPixelOffset) {
        return 1;
    } else if (a->fWidth < b->fWidth) {
        return -1;
    } else if (a->fWidth > b->fWidth) {
        return 1;
    } else if (a->fHeight < b->fHeight) {
        return -1;
    } else if (a->fHeight > b->fHeight) {
        return 1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

SkBitmapHeap::SkBitmapHeap(int32_t preferredSize, int32_t ownerCount)
    : INHERITED()
    , fExternalStorage(NULL)
    , fMostRecentlyUsed(NULL)
    , fLeastRecentlyUsed(NULL)
    , fPreferredCount(preferredSize)
    , fOwnerCount(ownerCount)
    , fBytesAllocated(0)
    , fDeferAddingOwners(false) {
}

SkBitmapHeap::SkBitmapHeap(ExternalStorage* storage, int32_t preferredSize)
    : INHERITED()
    , fExternalStorage(storage)
    , fMostRecentlyUsed(NULL)
    , fLeastRecentlyUsed(NULL)
    , fPreferredCount(preferredSize)
    , fOwnerCount(IGNORE_OWNERS)
    , fBytesAllocated(0)
    , fDeferAddingOwners(false) {
    SkSafeRef(storage);
}

SkBitmapHeap::~SkBitmapHeap() {
    SkDEBUGCODE(
    for (int i = 0; i < fStorage.count(); i++) {
        bool unused = false;
        for (int j = 0; j < fUnusedSlots.count(); j++) {
            if (fUnusedSlots[j] == fStorage[i]->fSlot) {
                unused = true;
                break;
            }
        }
        if (!unused) {
            fBytesAllocated -= fStorage[i]->fBytesAllocated;
        }
    }
    fBytesAllocated -= (fStorage.count() * sizeof(SkBitmapHeapEntry));
    )
    SkASSERT(0 == fBytesAllocated);
    fStorage.deleteAll();
    SkSafeUnref(fExternalStorage);
    fLookupTable.deleteAll();
}

SkTRefArray<SkBitmap>* SkBitmapHeap::extractBitmaps() const {
    const int size = fStorage.count();
    SkTRefArray<SkBitmap>* array = NULL;
    if (size > 0) {
        array = SkTRefArray<SkBitmap>::Create(size);
        for (int i = 0; i < size; i++) {
            // make a shallow copy of the bitmap
            array->writableAt(i) = fStorage[i]->fBitmap;
        }
    }
    return array;
}

void SkBitmapHeap::removeFromLRU(SkBitmapHeap::LookupEntry* entry) {
    if (fMostRecentlyUsed == entry) {
        fMostRecentlyUsed = entry->fLessRecentlyUsed;
        if (NULL == fMostRecentlyUsed) {
            SkASSERT(fLeastRecentlyUsed == entry);
            fLeastRecentlyUsed = NULL;
        } else {
            fMostRecentlyUsed->fMoreRecentlyUsed = NULL;
        }
    } else {
        // Remove entry from its prior place, and make sure to cover the hole.
        if (fLeastRecentlyUsed == entry) {
            SkASSERT(entry->fMoreRecentlyUsed != NULL);
            fLeastRecentlyUsed = entry->fMoreRecentlyUsed;
        }
        // Since we have already considered the case where entry is the most recently used, it must
        // have a more recently used at this point.
        SkASSERT(entry->fMoreRecentlyUsed != NULL);
        entry->fMoreRecentlyUsed->fLessRecentlyUsed = entry->fLessRecentlyUsed;

        if (entry->fLessRecentlyUsed != NULL) {
            SkASSERT(fLeastRecentlyUsed != entry);
            entry->fLessRecentlyUsed->fMoreRecentlyUsed = entry->fMoreRecentlyUsed;
        }
    }
    entry->fMoreRecentlyUsed = NULL;
}

void SkBitmapHeap::appendToLRU(SkBitmapHeap::LookupEntry* entry) {
    if (fMostRecentlyUsed != NULL) {
        SkASSERT(NULL == fMostRecentlyUsed->fMoreRecentlyUsed);
        fMostRecentlyUsed->fMoreRecentlyUsed = entry;
        entry->fLessRecentlyUsed = fMostRecentlyUsed;
    }
    fMostRecentlyUsed = entry;
    if (NULL == fLeastRecentlyUsed) {
        fLeastRecentlyUsed = entry;
    }
}

// iterate through our LRU cache and try to find an entry to evict
SkBitmapHeap::LookupEntry* SkBitmapHeap::findEntryToReplace(const SkBitmap& replacement) {
    SkASSERT(fPreferredCount != UNLIMITED_SIZE);
    SkASSERT(fStorage.count() >= fPreferredCount);

    SkBitmapHeap::LookupEntry* iter = fLeastRecentlyUsed;
    while (iter != NULL) {
        SkBitmapHeapEntry* heapEntry = fStorage[iter->fStorageSlot];
        if (heapEntry->fRefCount > 0) {
            // If the least recently used bitmap has not been unreferenced
            // by its owner, then according to our LRU specifications a more
            // recently used one can not have used all its references yet either.
            return NULL;
        }
        if (replacement.getGenerationID() == iter->fGenerationId) {
            // Do not replace a bitmap with a new one using the same
            // pixel ref. Instead look for a different one that will
            // potentially free up more space.
            iter = iter->fMoreRecentlyUsed;
        } else {
            return iter;
        }
    }
    return NULL;
}

size_t SkBitmapHeap::freeMemoryIfPossible(size_t bytesToFree) {
    if (UNLIMITED_SIZE == fPreferredCount) {
        return 0;
    }
    LookupEntry* iter = fLeastRecentlyUsed;
    size_t origBytesAllocated = fBytesAllocated;
    // Purge starting from LRU until a non-evictable bitmap is found or until
    // everything is evicted.
    while (iter != NULL) {
        SkBitmapHeapEntry* heapEntry = fStorage[iter->fStorageSlot];
        if (heapEntry->fRefCount > 0) {
            break;
        }
        LookupEntry* next = iter->fMoreRecentlyUsed;
        this->removeEntryFromLookupTable(iter);
        // Free the pixel memory. removeEntryFromLookupTable already reduced
        // fBytesAllocated properly.
        heapEntry->fBitmap.reset();
        // Add to list of unused slots which can be reused in the future.
        fUnusedSlots.push(heapEntry->fSlot);
        iter = next;
        if (origBytesAllocated - fBytesAllocated >= bytesToFree) {
            break;
        }
    }

    if (fLeastRecentlyUsed != iter) {
        // There was at least one eviction.
        fLeastRecentlyUsed = iter;
        if (NULL == fLeastRecentlyUsed) {
            // Everything was evicted
            fMostRecentlyUsed = NULL;
            fBytesAllocated -= (fStorage.count() * sizeof(SkBitmapHeapEntry));
            fStorage.deleteAll();
            fUnusedSlots.reset();
            SkASSERT(0 == fBytesAllocated);
        } else {
            fLeastRecentlyUsed->fLessRecentlyUsed = NULL;
        }
    }

    return origBytesAllocated - fBytesAllocated;
}

int SkBitmapHeap::findInLookupTable(const LookupEntry& indexEntry, SkBitmapHeapEntry** entry) {
    int index = SkTSearch<const LookupEntry>((const LookupEntry**)fLookupTable.begin(),
                                             fLookupTable.count(),
                                             &indexEntry, sizeof(void*), LookupEntry::Compare);

    if (index < 0) {
        // insert ourselves into the bitmapIndex
        index = ~index;
        *fLookupTable.insert(index) = SkNEW_ARGS(LookupEntry, (indexEntry));
    } else if (entry != NULL) {
        // populate the entry if needed
        *entry = fStorage[fLookupTable[index]->fStorageSlot];
    }

    return index;
}

bool SkBitmapHeap::copyBitmap(const SkBitmap& originalBitmap, SkBitmap& copiedBitmap) {
    SkASSERT(!fExternalStorage);

    // If the bitmap is mutable, we need to do a deep copy, since the
    // caller may modify it afterwards.
    if (originalBitmap.isImmutable()) {
        copiedBitmap = originalBitmap;
// TODO if we have the pixel ref in the heap we could pass it here to avoid a potential deep copy
//    else if (sharedPixelRef != NULL) {
//        copiedBitmap = orig;
//        copiedBitmap.setPixelRef(sharedPixelRef, originalBitmap.pixelRefOffset());
    } else if (originalBitmap.empty()) {
        copiedBitmap.reset();
    } else if (!originalBitmap.deepCopyTo(&copiedBitmap, originalBitmap.getConfig())) {
        return false;
    }
    copiedBitmap.setImmutable();
    return true;
}

int SkBitmapHeap::removeEntryFromLookupTable(LookupEntry* entry) {
    // remove the bitmap index for the deleted entry
    SkDEBUGCODE(int count = fLookupTable.count();)
    int index = this->findInLookupTable(*entry, NULL);
    // Verify that findInLookupTable found an existing entry rather than adding
    // a new entry to the lookup table.
    SkASSERT(count == fLookupTable.count());
    fBytesAllocated -= fStorage[entry->fStorageSlot]->fBytesAllocated;
    SkDELETE(fLookupTable[index]);
    fLookupTable.remove(index);
    return index;
}

int32_t SkBitmapHeap::insert(const SkBitmap& originalBitmap) {
    SkBitmapHeapEntry* entry = NULL;
    int searchIndex = this->findInLookupTable(LookupEntry(originalBitmap), &entry);

    if (entry) {
        // Already had a copy of the bitmap in the heap.
        if (fOwnerCount != IGNORE_OWNERS) {
            if (fDeferAddingOwners) {
                *fDeferredEntries.append() = entry->fSlot;
            } else {
                entry->addReferences(fOwnerCount);
            }
        }
        if (fPreferredCount != UNLIMITED_SIZE) {
            LookupEntry* lookupEntry = fLookupTable[searchIndex];
            if (lookupEntry != fMostRecentlyUsed) {
                this->removeFromLRU(lookupEntry);
                this->appendToLRU(lookupEntry);
            }
        }
        return entry->fSlot;
    }

    // decide if we need to evict an existing heap entry or create a new one
    if (fPreferredCount != UNLIMITED_SIZE && fStorage.count() >= fPreferredCount) {
        // iterate through our LRU cache and try to find an entry to evict
        LookupEntry* lookupEntry = this->findEntryToReplace(originalBitmap);
        if (lookupEntry != NULL) {
            // we found an entry to evict
            entry = fStorage[lookupEntry->fStorageSlot];
            // Remove it from the LRU. The new entry will be added to the LRU later.
            this->removeFromLRU(lookupEntry);
            int index = this->removeEntryFromLookupTable(lookupEntry);

            // update the current search index now that we have removed one
            if (index < searchIndex) {
                searchIndex--;
            }
        }
    }

    // if we didn't have an entry yet we need to create one
    if (!entry) {
        if (fPreferredCount != UNLIMITED_SIZE && fUnusedSlots.count() > 0) {
            int slot;
            fUnusedSlots.pop(&slot);
            entry = fStorage[slot];
        } else {
            entry = SkNEW(SkBitmapHeapEntry);
            fStorage.append(1, &entry);
            entry->fSlot = fStorage.count() - 1;
            fBytesAllocated += sizeof(SkBitmapHeapEntry);
        }
    }

    // create a copy of the bitmap
    bool copySucceeded;
    if (fExternalStorage) {
        copySucceeded = fExternalStorage->insert(originalBitmap, entry->fSlot);
    } else {
        copySucceeded = copyBitmap(originalBitmap, entry->fBitmap);
    }

    // if the copy failed then we must abort
    if (!copySucceeded) {
        // delete the index
        SkDELETE(fLookupTable[searchIndex]);
        fLookupTable.remove(searchIndex);
        // If entry is the last slot in storage, it is safe to delete it.
        if (fStorage.count() - 1 == entry->fSlot) {
            // free the slot
            fStorage.remove(entry->fSlot);
            fBytesAllocated -= sizeof(SkBitmapHeapEntry);
            SkDELETE(entry);
        } else {
            fUnusedSlots.push(entry->fSlot);
        }
        return INVALID_SLOT;
    }

    // update the index with the appropriate slot in the heap
    fLookupTable[searchIndex]->fStorageSlot = entry->fSlot;

    // compute the space taken by this entry
    // TODO if there is a shared pixel ref don't count it
    // If the SkBitmap does not share an SkPixelRef with an SkBitmap already
    // in the SharedHeap, also include the size of its pixels.
    entry->fBytesAllocated = originalBitmap.getSize();

    // add the bytes from this entry to the total count
    fBytesAllocated += entry->fBytesAllocated;

    if (fOwnerCount != IGNORE_OWNERS) {
        if (fDeferAddingOwners) {
            *fDeferredEntries.append() = entry->fSlot;
        } else {
            entry->addReferences(fOwnerCount);
        }
    }
    if (fPreferredCount != UNLIMITED_SIZE) {
        this->appendToLRU(fLookupTable[searchIndex]);
    }
    return entry->fSlot;
}

void SkBitmapHeap::deferAddingOwners() {
    fDeferAddingOwners = true;
}

void SkBitmapHeap::endAddingOwnersDeferral(bool add) {
    if (add) {
        for (int i = 0; i < fDeferredEntries.count(); i++) {
            SkASSERT(fOwnerCount != IGNORE_OWNERS);
            SkBitmapHeapEntry* heapEntry = this->getEntry(fDeferredEntries[i]);
            SkASSERT(heapEntry != NULL);
            heapEntry->addReferences(fOwnerCount);
        }
    }
    fDeferAddingOwners = false;
    fDeferredEntries.reset();
}
