
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

SkBitmapHeapEntry::SkBitmapHeapEntry()
    : fSlot(-1)
    , fRefCount(0)
    , fBytesAllocated(0)
    , fMoreRecentlyUsed(NULL)
    , fLessRecentlyUsed(NULL) {
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

SkBitmapHeap::SkBitmapHeap(int32_t preferredSize, int32_t ownerCount)
    : INHERITED()
    , fExternalStorage(NULL)
    , fMostRecentlyUsed(NULL)
    , fLeastRecentlyUsed(NULL)
    , fPreferredCount(preferredSize)
    , fOwnerCount(ownerCount)
    , fBytesAllocated(0) {
}

SkBitmapHeap::SkBitmapHeap(ExternalStorage* storage, int32_t preferredSize)
    : INHERITED()
    , fExternalStorage(storage)
    , fMostRecentlyUsed(NULL)
    , fLeastRecentlyUsed(NULL)
    , fPreferredCount(preferredSize)
    , fOwnerCount(IGNORE_OWNERS)
    , fBytesAllocated(0) {
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

// We just "used" the entry. Update our LRU accordingly
void SkBitmapHeap::setMostRecentlyUsed(SkBitmapHeapEntry* entry) {
    SkASSERT(entry != NULL);
    if (entry == fMostRecentlyUsed) {
        return;
    }
    // Remove info from its prior place, and make sure to cover the hole.
    if (fLeastRecentlyUsed == entry) {
        SkASSERT(entry->fMoreRecentlyUsed != NULL);
        fLeastRecentlyUsed = entry->fMoreRecentlyUsed;
    }
    if (entry->fMoreRecentlyUsed != NULL) {
        SkASSERT(fMostRecentlyUsed != entry);
        entry->fMoreRecentlyUsed->fLessRecentlyUsed = entry->fLessRecentlyUsed;
    }
    if (entry->fLessRecentlyUsed != NULL) {
        SkASSERT(fLeastRecentlyUsed != entry);
        entry->fLessRecentlyUsed->fMoreRecentlyUsed = entry->fMoreRecentlyUsed;
    }
    entry->fMoreRecentlyUsed = NULL;
    // Set up the head and tail pointers properly.
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
SkBitmapHeapEntry* SkBitmapHeap::findEntryToReplace(const SkBitmap& replacement) {
    SkASSERT(fPreferredCount != UNLIMITED_SIZE);
    SkASSERT(fStorage.count() >= fPreferredCount);

    SkBitmapHeapEntry* iter = fLeastRecentlyUsed;
    while (iter != NULL) {
        if (iter->fRefCount > 0) {
            // If the least recently used bitmap has not been unreferenced
            // by its owner, then according to our LRU specifications a more
            // recently used one can not have used all it's references yet either.
            return NULL;
        }
        if (replacement.pixelRef() && replacement.pixelRef() == iter->fBitmap.pixelRef()) {
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
    SkBitmapHeapEntry* iter = fLeastRecentlyUsed;
    size_t origBytesAllocated = fBytesAllocated;
    // Purge starting from LRU until a non-evictable bitmap is found or until
    // everything is evicted.
    while (iter && 0 == iter->fRefCount) {
        SkBitmapHeapEntry* next = iter->fMoreRecentlyUsed;
        this->removeEntryFromLookupTable(*iter);
        // Free the pixel memory. removeEntryFromLookupTable already reduced
        // fBytesAllocated properly.
        iter->fBitmap.reset();
        // Add to list of unused slots which can be reused in the future.
        fUnusedSlots.push(iter->fSlot);
        // Remove its LRU pointers, so that it does not pretend it is already in
        // the list the next time it is used.
        iter->fMoreRecentlyUsed = iter->fLessRecentlyUsed = NULL;
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
    int index = SkTSearch<const LookupEntry>(fLookupTable.begin(),
                                             fLookupTable.count(),
                                             indexEntry, sizeof(indexEntry));

    if (index < 0) {
        // insert ourselves into the bitmapIndex
        index = ~index;
        fLookupTable.insert(index, 1, &indexEntry);
    } else if (entry != NULL) {
        // populate the entry if needed
        *entry = fStorage[fLookupTable[index].fStorageSlot];
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

int SkBitmapHeap::removeEntryFromLookupTable(const SkBitmapHeapEntry& entry) {
    // remove the bitmap index for the deleted entry
    SkDEBUGCODE(int count = fLookupTable.count();)
    // FIXME: If copying bitmaps retained the generation ID, we could
    // just grab the generation ID from entry.fBitmap
    LookupEntry key(entry.fBitmap, entry.fGenerationID);
    int index = this->findInLookupTable(key, NULL);
    // Verify that findInLookupTable found an existing entry rather than adding
    // a new entry to the lookup table.
    SkASSERT(count == fLookupTable.count());
    
    fLookupTable.remove(index);
    fBytesAllocated -= entry.fBytesAllocated;
    return index;
}

int32_t SkBitmapHeap::insert(const SkBitmap& originalBitmap) {
    SkBitmapHeapEntry* entry = NULL;
    int searchIndex = this->findInLookupTable(LookupEntry(originalBitmap), &entry);

    // check to see if we already had a copy of the bitmap in the heap
    if (entry) {
        if (fOwnerCount != IGNORE_OWNERS) {
            entry->addReferences(fOwnerCount);
        }
        if (fPreferredCount != UNLIMITED_SIZE) {
            this->setMostRecentlyUsed(entry);
        }
        return entry->fSlot;
    }

    // decide if we need to evict an existing heap entry or create a new one
    if (fPreferredCount != UNLIMITED_SIZE && fStorage.count() >= fPreferredCount) {
        // iterate through our LRU cache and try to find an entry to evict
        entry = this->findEntryToReplace(originalBitmap);
        // we found an entry to evict
        if (entry) {
            int index = this->removeEntryFromLookupTable(*entry);

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
        fLookupTable.remove(searchIndex);
        // If entry is the last slot in storage, it is safe to delete it.
        if (fStorage.count() - 1 == entry->fSlot) {
            // free the slot
            fStorage.remove(entry->fSlot);
            fBytesAllocated -= sizeof(SkBitmapHeapEntry);
            SkDELETE(entry);
        }
        return INVALID_SLOT;
    }

    // update the index with the appropriate slot in the heap
    fLookupTable[searchIndex].fStorageSlot = entry->fSlot;

    // compute the space taken by the this entry
    // TODO if there is a shared pixel ref don't count it
    // If the SkBitmap does not share an SkPixelRef with an SkBitmap already
    // in the SharedHeap, also include the size of its pixels.
    entry->fBytesAllocated += originalBitmap.getSize();

    // add the bytes from this entry to the total count
    fBytesAllocated += entry->fBytesAllocated;

    entry->fGenerationID = originalBitmap.getGenerationID();

    if (fOwnerCount != IGNORE_OWNERS) {
        entry->addReferences(fOwnerCount);
    }
    if (fPreferredCount != UNLIMITED_SIZE) {
        this->setMostRecentlyUsed(entry);
    }
    return entry->fSlot;
}
