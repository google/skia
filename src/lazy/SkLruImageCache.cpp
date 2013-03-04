/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLruImageCache.h"

static intptr_t NextGenerationID() {
    static intptr_t gNextID;
    do {
        gNextID++;
    } while (SkImageCache::UNINITIALIZED_ID == gNextID);
    return gNextID;
}

class CachedPixels : public SkNoncopyable {

public:
    CachedPixels(size_t length)
        : fLength(length)
        , fID(NextGenerationID())
        , fLocked(false) {
        fAddr = sk_malloc_throw(length);
    }

    ~CachedPixels() {
        sk_free(fAddr);
    }

    void* getData() { return fAddr; }

    intptr_t getID() const { return fID; }

    size_t getLength() const { return fLength; }

    void lock() { SkASSERT(!fLocked); fLocked = true; }

    void unlock() { SkASSERT(fLocked); fLocked = false; }

    bool isLocked() const { return fLocked; }

private:
    void*          fAddr;
    size_t         fLength;
    const intptr_t fID;
    bool           fLocked;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(CachedPixels);
};

////////////////////////////////////////////////////////////////////////////////////

SkLruImageCache::SkLruImageCache(size_t budget)
    : fRamBudget(budget)
    , fRamUsed(0) {}

SkLruImageCache::~SkLruImageCache() {
    // Don't worry about updating pointers. All will be deleted.
    Iter iter;
    CachedPixels* pixels = iter.init(fLRU, Iter::kTail_IterStart);
    while (pixels != NULL) {
        CachedPixels* prev = iter.prev();
        SkASSERT(!pixels->isLocked());
#ifdef SK_DEBUG
        fRamUsed -= pixels->getLength();
#endif
        SkDELETE(pixels);
        pixels = prev;
    }
#ifdef SK_DEBUG
    SkASSERT(fRamUsed == 0);
#endif
}

#ifdef SK_DEBUG
SkImageCache::CacheStatus SkLruImageCache::getCacheStatus(intptr_t ID) const {
    SkAutoMutexAcquire ac(&fMutex);
    CachedPixels* pixels = this->findByID(ID);
    if (NULL == pixels) {
        return SkImageCache::kThrownAway_CacheStatus;
    }
    if (pixels->isLocked()) {
        return SkImageCache::kPinned_CacheStatus;
    }
    return SkImageCache::kUnpinned_CacheStatus;
}
#endif

size_t SkLruImageCache::setImageCacheLimit(size_t newLimit) {
    size_t oldLimit = fRamBudget;
    SkAutoMutexAcquire ac(&fMutex);
    fRamBudget = newLimit;
    this->purgeIfNeeded();
    return oldLimit;
}

void* SkLruImageCache::allocAndPinCache(size_t bytes, intptr_t* ID) {
    SkAutoMutexAcquire ac(&fMutex);
    CachedPixels* pixels = SkNEW_ARGS(CachedPixels, (bytes));
    if (ID != NULL) {
        *ID = pixels->getID();
    }
    pixels->lock();
    fRamUsed += bytes;
    fLRU.addToHead(pixels);
    this->purgeIfNeeded();
    return pixels->getData();
}

void* SkLruImageCache::pinCache(intptr_t ID) {
    SkASSERT(ID != SkImageCache::UNINITIALIZED_ID);
    SkAutoMutexAcquire ac(&fMutex);
    CachedPixels* pixels = this->findByID(ID);
    if (NULL == pixels) {
        return NULL;
    }
    if (pixels != fLRU.head()) {
        fLRU.remove(pixels);
        fLRU.addToHead(pixels);
    }
    pixels->lock();
    return pixels->getData();
}

void SkLruImageCache::releaseCache(intptr_t ID) {
    SkASSERT(ID != SkImageCache::UNINITIALIZED_ID);
    SkAutoMutexAcquire ac(&fMutex);
    CachedPixels* pixels = this->findByID(ID);
    SkASSERT(pixels != NULL);
    pixels->unlock();
    this->purgeIfNeeded();
}

void SkLruImageCache::throwAwayCache(intptr_t ID) {
    SkAutoMutexAcquire ac(&fMutex);
    CachedPixels* pixels = this->findByID(ID);
    if (pixels != NULL) {
        if (pixels->isLocked()) {
            pixels->unlock();
        }
        this->removePixels(pixels);
    }
}

void SkLruImageCache::removePixels(CachedPixels* pixels) {
    // Mutex is already locked.
    SkASSERT(!pixels->isLocked());
    const size_t size = pixels->getLength();
    SkASSERT(size <= fRamUsed);
    fLRU.remove(pixels);
    SkDELETE(pixels);
    fRamUsed -= size;
}

CachedPixels* SkLruImageCache::findByID(intptr_t ID) const {
    // Mutex is already locked.
    if (SkImageCache::UNINITIALIZED_ID == ID) {
        return NULL;
    }
    Iter iter;
    // Start from the head, most recently used.
    CachedPixels* pixels = iter.init(fLRU, Iter::kHead_IterStart);
    while (pixels != NULL) {
        if (pixels->getID() == ID) {
            return pixels;
        }
        pixels = iter.next();
    }
    return NULL;
}

void SkLruImageCache::purgeIfNeeded() {
    // Mutex is already locked.
    if (fRamBudget > 0) {
        this->purgeTilAtOrBelow(fRamBudget);
    }
}

void SkLruImageCache::purgeTilAtOrBelow(size_t limit) {
    // Mutex is already locked.
    if (fRamUsed > limit) {
        Iter iter;
        // Start from the tail, least recently used.
        CachedPixels* pixels = iter.init(fLRU, Iter::kTail_IterStart);
        while (pixels != NULL && fRamUsed > limit) {
            CachedPixels* prev = iter.prev();
            if (!pixels->isLocked()) {
                this->removePixels(pixels);
            }
            pixels = prev;
        }
    }
}
