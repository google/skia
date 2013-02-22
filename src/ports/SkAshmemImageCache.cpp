/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAshmemImageCache.h"
#include "SkThread.h"

#ifdef SK_DEBUG
    #include "SkTSearch.h"
#endif

#include "android/ashmem.h"
#include <sys/mman.h>
#include <unistd.h>


SkAshmemImageCache::SkAshmemImageCache() {}

SK_DECLARE_STATIC_MUTEX(gAshmemMutex);

SkAshmemImageCache* SkAshmemImageCache::GetAshmemImageCache() {
    SkAutoMutexAcquire ac(&gAshmemMutex);
    static SkAshmemImageCache gCache;
    return &gCache;
}

#ifdef SK_DEBUG
SkAshmemImageCache::~SkAshmemImageCache() {
    SkASSERT(fRecs.count() == 0);
}
#endif

// ashmem likes lengths on page boundaries.
static size_t roundToPageSize(size_t size) {
    const size_t mask = getpagesize() - 1;
    size_t newSize = (size + mask) & ~mask;
    return newSize;
}

void* SkAshmemImageCache::allocAndPinCache(size_t bytes, intptr_t* ID) {
    AshmemRec rec;
    rec.fSize = roundToPageSize(bytes);

    SkAutoMutexAcquire ac(&gAshmemMutex);

    rec.fFD = ashmem_create_region(NULL, rec.fSize);
    if (-1 == rec.fFD) {
        SkDebugf("ashmem_create_region failed\n");
        return NULL;
    }
    int err = ashmem_set_prot_region(rec.fFD, PROT_READ | PROT_WRITE);
    if (err != 0) {
        SkDebugf("ashmem_set_prot_region failed\n");
        close(rec.fFD);
        return NULL;
    }
    rec.fAddr = mmap(NULL, rec.fSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, rec.fFD, 0);
    if (-1 == (long) rec.fAddr) {
        SkDebugf("mmap failed\n");
        close(rec.fFD);
        return NULL;
    }
    (void) ashmem_pin_region(rec.fFD, 0, 0);
#ifdef SK_DEBUG
    rec.fPinned = true;
#endif
    // In release mode, we do not keep a pointer to this object. It will be destroyed
    // either when pinCache returns NULL or when throwAwayCache is called.
    AshmemRec* pRec = SkNEW_ARGS(AshmemRec, (rec));
    SkASSERT(ID != NULL);
    *ID = reinterpret_cast<intptr_t>(pRec);
#ifdef SK_DEBUG
    this->appendRec(pRec);
#endif
    return rec.fAddr;
}

void* SkAshmemImageCache::pinCache(intptr_t ID) {
    SkAutoMutexAcquire ac(&gAshmemMutex);
    AshmemRec* rec = reinterpret_cast<AshmemRec*>(ID);
    const int fd = rec->fFD;
    int pin = ashmem_pin_region(fd, 0, 0);
    if (ASHMEM_NOT_PURGED == pin) {
#ifdef SK_DEBUG
        rec->fPinned = true;
#endif
        return rec->fAddr;
    }
    // Purged. Remove the associated AshmemRec:
    this->removeRec(rec);
    ashmem_unpin_region(fd, 0, 0);
    return NULL;
}

void SkAshmemImageCache::releaseCache(intptr_t ID) {
    SkAutoMutexAcquire ac(&gAshmemMutex);
    AshmemRec* rec = reinterpret_cast<AshmemRec*>(ID);
    ashmem_unpin_region(rec->fFD, 0, 0);
#ifdef SK_DEBUG
    rec->fPinned = false;
#endif
}

void SkAshmemImageCache::throwAwayCache(intptr_t ID) {
    SkAutoMutexAcquire ac(&gAshmemMutex);
    AshmemRec* rec = reinterpret_cast<AshmemRec*>(ID);
#ifdef SK_DEBUG
    SkASSERT(!rec->fPinned);
#endif
    this->removeRec(rec);
}

void SkAshmemImageCache::removeRec(SkAshmemImageCache::AshmemRec* rec) {
    munmap(rec->fAddr, rec->fSize);
    close(rec->fFD);
#ifdef SK_DEBUG
    int index = this->findRec(rec);
    SkASSERT(index >= 0);
    fRecs.remove(index);
#endif
    SkDELETE(rec);
}

#ifdef SK_DEBUG
void SkAshmemImageCache::appendRec(SkAshmemImageCache::AshmemRec* rec) {
    int index = this->findRec(rec);
    // Should not already exist.
    SkASSERT(index < 0);
    fRecs.insert(~index, 1, &rec);
}

int SkAshmemImageCache::AshmemRec::Compare(const SkAshmemImageCache::AshmemRec* a,
                                           const SkAshmemImageCache::AshmemRec* b) {
    return reinterpret_cast<intptr_t>(a) - reinterpret_cast<intptr_t>(b);
}

int SkAshmemImageCache::findRec(const SkAshmemImageCache::AshmemRec* rec) const {
    return SkTSearch<AshmemRec>((const AshmemRec**)fRecs.begin(), fRecs.count(), rec,
                                sizeof(intptr_t), AshmemRec::Compare);
}

SkImageCache::CacheStatus SkAshmemImageCache::getCacheStatus(intptr_t ID) const {
    SkAutoMutexAcquire ac(&gAshmemMutex);
    AshmemRec* rec = reinterpret_cast<AshmemRec*>(ID);
    int index = this->findRec(rec);
    if (index < 0) {
        return SkImageCache::kThrownAway_CacheStatus;
    }
    return rec->fPinned ? SkImageCache::kPinned_CacheStatus
                        : SkImageCache::kUnpinned_CacheStatus;
}
#endif
