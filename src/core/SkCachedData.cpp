/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMalloc.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkDiscardableMemory.h"

SkCachedData::SkCachedData(void* data, size_t size)
    : fData(data)
    , fSize(size)
    , fRefCnt(1)
    , fStorageType(kMalloc_StorageType)
    , fInCache(false)
    , fIsLocked(true)
{
    fStorage.fMalloc = data;
}

SkCachedData::SkCachedData(size_t size, SkDiscardableMemory* dm)
    : fData(dm->data())
    , fSize(size)
    , fRefCnt(1)
    , fStorageType(kDiscardableMemory_StorageType)
    , fInCache(false)
    , fIsLocked(true)
{
    fStorage.fDM = dm;
}

SkCachedData::~SkCachedData() {
    switch (fStorageType) {
        case kMalloc_StorageType:
            sk_free(fStorage.fMalloc);
            break;
        case kDiscardableMemory_StorageType:
            delete fStorage.fDM;
            break;
    }
}

class SkCachedData::AutoMutexWritable {
public:
    AutoMutexWritable(const SkCachedData* cd) : fCD(const_cast<SkCachedData*>(cd)) {
        fCD->fMutex.acquire();
        fCD->validate();
    }
    ~AutoMutexWritable() {
        fCD->validate();
        fCD->fMutex.release();
    }

    SkCachedData* get() { return fCD; }
    SkCachedData* operator->() { return fCD; }

private:
    SkCachedData* fCD;
};

void SkCachedData::internalRef(bool fromCache) const {
    AutoMutexWritable(this)->inMutexRef(fromCache);
}

void SkCachedData::internalUnref(bool fromCache) const {
    if (AutoMutexWritable(this)->inMutexUnref(fromCache)) {
        // can't delete inside doInternalUnref, since it is locking a mutex (which we own)
        delete this;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkCachedData::inMutexRef(bool fromCache) {
    if ((1 == fRefCnt) && fInCache) {
        this->inMutexLock();
    }

    fRefCnt += 1;
    if (fromCache) {
        SkASSERT(!fInCache);
        fInCache = true;
    }
}

bool SkCachedData::inMutexUnref(bool fromCache) {
    switch (--fRefCnt) {
        case 0:
            // we're going to be deleted, so we need to be unlocked (for DiscardableMemory)
            if (fIsLocked) {
                this->inMutexUnlock();
            }
            break;
        case 1:
            if (fInCache && !fromCache) {
                // If we're down to 1 owner, and that owner is the cache, this it is safe
                // to unlock (and mutate fData) even if the cache is in a different thread,
                // as the cache is NOT allowed to inspect or use fData.
                this->inMutexUnlock();
            }
            break;
        default:
            break;
    }

    if (fromCache) {
        SkASSERT(fInCache);
        fInCache = false;
    }

    // return true when we need to be deleted
    return 0 == fRefCnt;
}

void SkCachedData::inMutexLock() {
    fMutex.assertHeld();

    SkASSERT(!fIsLocked);
    fIsLocked = true;

    switch (fStorageType) {
        case kMalloc_StorageType:
            this->setData(fStorage.fMalloc);
            break;
        case kDiscardableMemory_StorageType:
            if (fStorage.fDM->lock()) {
                void* ptr = fStorage.fDM->data();
                SkASSERT(ptr);
                this->setData(ptr);
            } else {
                this->setData(nullptr);   // signal failure to lock, contents are gone
            }
            break;
    }
}

void SkCachedData::inMutexUnlock() {
    fMutex.assertHeld();

    SkASSERT(fIsLocked);
    fIsLocked = false;

    switch (fStorageType) {
        case kMalloc_StorageType:
            // nothing to do/check
            break;
        case kDiscardableMemory_StorageType:
            if (fData) {    // did the previous lock succeed?
                fStorage.fDM->unlock();
            }
            break;
    }
    this->setData(nullptr);   // signal that we're in an unlocked state
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkCachedData::validate() const {
    if (fIsLocked) {
        SkASSERT((fInCache && fRefCnt > 1) || !fInCache);
        switch (fStorageType) {
            case kMalloc_StorageType:
                SkASSERT(fData == fStorage.fMalloc);
                break;
            case kDiscardableMemory_StorageType:
                // fData can be null or the actual value, depending if DM's lock succeeded
                break;
        }
    } else {
        SkASSERT((fInCache && 1 == fRefCnt) || (0 == fRefCnt));
        SkASSERT(nullptr == fData);
    }
}
#endif
