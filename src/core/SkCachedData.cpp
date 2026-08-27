/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkCachedData.h"

#include "include/private/SkMalloc.h"
#include "include/private/chromium/SkDiscardableMemory.h"

SkCachedData::SkCachedData(void* data, size_t size)
        : fStorage(std::in_place_type<MallocStorage>, data), fData(data), fSize(size) {}

SkCachedData::SkCachedData(size_t size, SkDiscardableMemory* dm)
        : fStorage(std::in_place_type<DiscardableStorage>, dm)
        , fData(dm ? dm->data() : nullptr)
        , fSize(size) {}

SkCachedData::~SkCachedData() = default;

SkDiscardableMemory* SkCachedData::diagnostic_only_getDiscardable() const {
    if (auto* dm = std::get_if<DiscardableStorage>(&fStorage)) {
        return dm->get();
    }
    return nullptr;
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
    bool shouldDelete = false;
    {
        AutoMutexWritable amw(this);
        shouldDelete = amw->inMutexUnref(fromCache);
    }
    if (shouldDelete) {
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

    if (auto* dm = std::get_if<DiscardableStorage>(&fStorage)) {
        if ((*dm)->lock()) {
            void* ptr = (*dm)->data();
            SkASSERT(ptr);
            this->setData(ptr);
        } else {
            this->setData(nullptr);  // signal failure to lock, contents are gone
        }
    } else if (auto* m = std::get_if<MallocStorage>(&fStorage)) {
        this->setData(m->get());
    }
}

void SkCachedData::inMutexUnlock() {
    fMutex.assertHeld();

    SkASSERT(fIsLocked);
    fIsLocked = false;

    if (auto* dm = std::get_if<DiscardableStorage>(&fStorage)) {
        if (fData) {  // did the previous lock succeed?
            (*dm)->unlock();
        }
    }
    // else MallocStorage: nothing to do/check
    this->setData(nullptr);  // signal that we're in an unlocked state
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkCachedData::validate() const {
    if (fIsLocked) {
        SkASSERT((fInCache && fRefCnt > 1) || !fInCache);
        if (auto* m = std::get_if<MallocStorage>(&fStorage)) {
            SkASSERT(fData == m->get());
        }
        // else DiscardableStorage: fData can be null or the actual value, depending if DM's lock
        // succeeded
    } else {
        SkASSERT((fInCache && 1 == fRefCnt) || (0 == fRefCnt));
        SkASSERT(nullptr == fData);
    }
}
#endif
