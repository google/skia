/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChecksum.h"
#include "SkResourceCache.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"

// This can be defined by the caller's build system
//#define SK_USE_DISCARDABLE_SCALEDIMAGECACHE

#ifndef SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT
#   define SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT   1024
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
    #define SK_DEFAULT_IMAGE_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

void SkResourceCache::Key::init(size_t length) {
    SkASSERT(SkAlign4(length) == length);
    // 2 is fCount32 and fHash
    fCount32 = SkToS32(2 + (length >> 2));
    // skip both of our fields whe computing the murmur
    fHash = SkChecksum::Murmur3(this->as32() + 2, (fCount32 - 2) << 2);
}

#include "SkTDynamicHash.h"

class SkResourceCache::Hash :
    public SkTDynamicHash<SkResourceCache::Rec, SkResourceCache::Key> {};


///////////////////////////////////////////////////////////////////////////////

// experimental hash to speed things up
#define USE_HASH

#if !defined(USE_HASH)
static inline SkResourceCache::Rec* find_rec_in_list(
        SkResourceCache::Rec* head, const Key & key) {
    SkResourceCache::Rec* rec = head;
    while ((rec != NULL) && (rec->fKey != key)) {
        rec = rec->fNext;
    }
    return rec;
}
#endif

void SkResourceCache::init() {
    fHead = NULL;
    fTail = NULL;
#ifdef USE_HASH
    fHash = new Hash;
#else
    fHash = NULL;
#endif
    fTotalBytesUsed = 0;
    fCount = 0;
    fSingleAllocationByteLimit = 0;
    fAllocator = NULL;

    // One of these should be explicit set by the caller after we return.
    fTotalByteLimit = 0;
    fDiscardableFactory = NULL;
}

#include "SkDiscardableMemory.h"

class SkOneShotDiscardablePixelRef : public SkPixelRef {
public:
    SK_DECLARE_INST_COUNT(SkOneShotDiscardablePixelRef)
    // Ownership of the discardablememory is transfered to the pixelref
    SkOneShotDiscardablePixelRef(const SkImageInfo&, SkDiscardableMemory*, size_t rowBytes);
    ~SkOneShotDiscardablePixelRef();

protected:
    virtual bool onNewLockPixels(LockRec*) SK_OVERRIDE;
    virtual void onUnlockPixels() SK_OVERRIDE;
    virtual size_t getAllocatedSizeInBytes() const SK_OVERRIDE;

private:
    SkDiscardableMemory* fDM;
    size_t               fRB;
    bool                 fFirstTime;

    typedef SkPixelRef INHERITED;
};

SkOneShotDiscardablePixelRef::SkOneShotDiscardablePixelRef(const SkImageInfo& info,
                                             SkDiscardableMemory* dm,
                                             size_t rowBytes)
    : INHERITED(info)
    , fDM(dm)
    , fRB(rowBytes)
{
    SkASSERT(dm->data());
    fFirstTime = true;
}

SkOneShotDiscardablePixelRef::~SkOneShotDiscardablePixelRef() {
    SkDELETE(fDM);
}

bool SkOneShotDiscardablePixelRef::onNewLockPixels(LockRec* rec) {
    if (fFirstTime) {
        // we're already locked
        SkASSERT(fDM->data());
        fFirstTime = false;
        goto SUCCESS;
    }

    // A previous call to onUnlock may have deleted our DM, so check for that
    if (NULL == fDM) {
        return false;
    }

    if (!fDM->lock()) {
        // since it failed, we delete it now, to free-up the resource
        delete fDM;
        fDM = NULL;
        return false;
    }

SUCCESS:
    rec->fPixels = fDM->data();
    rec->fColorTable = NULL;
    rec->fRowBytes = fRB;
    return true;
}

void SkOneShotDiscardablePixelRef::onUnlockPixels() {
    SkASSERT(!fFirstTime);
    fDM->unlock();
}

size_t SkOneShotDiscardablePixelRef::getAllocatedSizeInBytes() const {
    return this->info().getSafeSize(fRB);
}

class SkResourceCacheDiscardableAllocator : public SkBitmap::Allocator {
public:
    SkResourceCacheDiscardableAllocator(SkResourceCache::DiscardableFactory factory) {
        SkASSERT(factory);
        fFactory = factory;
    }

    virtual bool allocPixelRef(SkBitmap*, SkColorTable*) SK_OVERRIDE;

private:
    SkResourceCache::DiscardableFactory fFactory;
};

bool SkResourceCacheDiscardableAllocator::allocPixelRef(SkBitmap* bitmap, SkColorTable* ctable) {
    size_t size = bitmap->getSize();
    uint64_t size64 = bitmap->computeSize64();
    if (0 == size || size64 > (uint64_t)size) {
        return false;
    }

    SkDiscardableMemory* dm = fFactory(size);
    if (NULL == dm) {
        return false;
    }

    // can we relax this?
    if (kN32_SkColorType != bitmap->colorType()) {
        return false;
    }

    SkImageInfo info = bitmap->info();
    bitmap->setPixelRef(SkNEW_ARGS(SkOneShotDiscardablePixelRef,
                                   (info, dm, bitmap->rowBytes())))->unref();
    bitmap->lockPixels();
    return bitmap->readyToDraw();
}

SkResourceCache::SkResourceCache(DiscardableFactory factory) {
    this->init();
    fDiscardableFactory = factory;

    fAllocator = SkNEW_ARGS(SkResourceCacheDiscardableAllocator, (factory));
}

SkResourceCache::SkResourceCache(size_t byteLimit) {
    this->init();
    fTotalByteLimit = byteLimit;
}

SkResourceCache::~SkResourceCache() {
    SkSafeUnref(fAllocator);

    Rec* rec = fHead;
    while (rec) {
        Rec* next = rec->fNext;
        SkDELETE(rec);
        rec = next;
    }
    delete fHash;
}

////////////////////////////////////////////////////////////////////////////////

const SkResourceCache::Rec* SkResourceCache::findAndLock(const Key& key) {
#ifdef USE_HASH
    Rec* rec = fHash->find(key);
#else
    Rec* rec = find_rec_in_list(fHead, key);
#endif
    if (rec) {
        this->moveToHead(rec);  // for our LRU
        rec->fLockCount += 1;
    }
    return rec;
}

const SkResourceCache::Rec* SkResourceCache::addAndLock(Rec* rec) {
    SkASSERT(rec);
    // See if we already have this key (racy inserts, etc.)
    const Rec* existing = this->findAndLock(rec->getKey());
    if (existing) {
        SkDELETE(rec);
        return existing;
    }

    this->addToHead(rec);
    SkASSERT(1 == rec->fLockCount);
#ifdef USE_HASH
    SkASSERT(fHash);
    fHash->add(rec);
#endif
    // We may (now) be overbudget, so see if we need to purge something.
    this->purgeAsNeeded();
    return rec;
}

void SkResourceCache::add(Rec* rec) {
    SkASSERT(rec);
    // See if we already have this key (racy inserts, etc.)
    const Rec* existing = this->findAndLock(rec->getKey());
    if (existing) {
        SkDELETE(rec);
        this->unlock(existing);
        return;
    }
    
    this->addToHead(rec);
    SkASSERT(1 == rec->fLockCount);
#ifdef USE_HASH
    SkASSERT(fHash);
    fHash->add(rec);
#endif
    this->unlock(rec);
}

void SkResourceCache::unlock(SkResourceCache::ID id) {
    SkASSERT(id);

#ifdef SK_DEBUG
    {
        bool found = false;
        Rec* rec = fHead;
        while (rec != NULL) {
            if (rec == id) {
                found = true;
                break;
            }
            rec = rec->fNext;
        }
        SkASSERT(found);
    }
#endif
    const Rec* rec = id;
    SkASSERT(rec->fLockCount > 0);
    // We're under our lock, and we're the only possible mutator, so unconsting is fine.
    const_cast<Rec*>(rec)->fLockCount -= 1;

    // we may have been over-budget, but now have released something, so check
    // if we should purge.
    if (0 == rec->fLockCount) {
        this->purgeAsNeeded();
    }
}

void SkResourceCache::remove(Rec* rec) {
    SkASSERT(0 == rec->fLockCount);

    size_t used = rec->bytesUsed();
    SkASSERT(used <= fTotalBytesUsed);

    this->detach(rec);
#ifdef USE_HASH
    fHash->remove(rec->getKey());
#endif

    SkDELETE(rec);

    fTotalBytesUsed -= used;
    fCount -= 1;
}

void SkResourceCache::purgeAsNeeded(bool forcePurge) {
    size_t byteLimit;
    int    countLimit;

    if (fDiscardableFactory) {
        countLimit = SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT;
        byteLimit = SK_MaxU32;  // no limit based on bytes
    } else {
        countLimit = SK_MaxS32; // no limit based on count
        byteLimit = fTotalByteLimit;
    }

    Rec* rec = fTail;
    while (rec) {
        if (!forcePurge && fTotalBytesUsed < byteLimit && fCount < countLimit) {
            break;
        }

        Rec* prev = rec->fPrev;
        if (0 == rec->fLockCount) {
            this->remove(rec);
        }
        rec = prev;
    }
}

size_t SkResourceCache::setTotalByteLimit(size_t newLimit) {
    size_t prevLimit = fTotalByteLimit;
    fTotalByteLimit = newLimit;
    if (newLimit < prevLimit) {
        this->purgeAsNeeded();
    }
    return prevLimit;
}

///////////////////////////////////////////////////////////////////////////////

void SkResourceCache::detach(Rec* rec) {
    Rec* prev = rec->fPrev;
    Rec* next = rec->fNext;

    if (!prev) {
        SkASSERT(fHead == rec);
        fHead = next;
    } else {
        prev->fNext = next;
    }

    if (!next) {
        fTail = prev;
    } else {
        next->fPrev = prev;
    }

    rec->fNext = rec->fPrev = NULL;
}

void SkResourceCache::moveToHead(Rec* rec) {
    if (fHead == rec) {
        return;
    }

    SkASSERT(fHead);
    SkASSERT(fTail);

    this->validate();

    this->detach(rec);

    fHead->fPrev = rec;
    rec->fNext = fHead;
    fHead = rec;

    this->validate();
}

void SkResourceCache::addToHead(Rec* rec) {
    this->validate();

    rec->fPrev = NULL;
    rec->fNext = fHead;
    if (fHead) {
        fHead->fPrev = rec;
    }
    fHead = rec;
    if (!fTail) {
        fTail = rec;
    }
    fTotalBytesUsed += rec->bytesUsed();
    fCount += 1;

    this->validate();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkResourceCache::validate() const {
    if (NULL == fHead) {
        SkASSERT(NULL == fTail);
        SkASSERT(0 == fTotalBytesUsed);
        return;
    }

    if (fHead == fTail) {
        SkASSERT(NULL == fHead->fPrev);
        SkASSERT(NULL == fHead->fNext);
        SkASSERT(fHead->bytesUsed() == fTotalBytesUsed);
        return;
    }

    SkASSERT(NULL == fHead->fPrev);
    SkASSERT(fHead->fNext);
    SkASSERT(NULL == fTail->fNext);
    SkASSERT(fTail->fPrev);

    size_t used = 0;
    int count = 0;
    const Rec* rec = fHead;
    while (rec) {
        count += 1;
        used += rec->bytesUsed();
        SkASSERT(used <= fTotalBytesUsed);
        rec = rec->fNext;
    }
    SkASSERT(fCount == count);

    rec = fTail;
    while (rec) {
        SkASSERT(count > 0);
        count -= 1;
        SkASSERT(used >= rec->bytesUsed());
        used -= rec->bytesUsed();
        rec = rec->fPrev;
    }

    SkASSERT(0 == count);
    SkASSERT(0 == used);
}
#endif

void SkResourceCache::dump() const {
    this->validate();

    const Rec* rec = fHead;
    int locked = 0;
    while (rec) {
        locked += rec->fLockCount > 0;
        rec = rec->fNext;
    }

    SkDebugf("SkResourceCache: count=%d bytes=%d locked=%d %s\n",
             fCount, fTotalBytesUsed, locked,
             fDiscardableFactory ? "discardable" : "malloc");
}

size_t SkResourceCache::setSingleAllocationByteLimit(size_t newLimit) {
    size_t oldLimit = fSingleAllocationByteLimit;
    fSingleAllocationByteLimit = newLimit;
    return oldLimit;
}

size_t SkResourceCache::getSingleAllocationByteLimit() const {
    return fSingleAllocationByteLimit;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkThread.h"

SK_DECLARE_STATIC_MUTEX(gMutex);
static SkResourceCache* gResourceCache = NULL;
static void cleanup_gResourceCache() {
    // We'll clean this up in our own tests, but disable for clients.
    // Chrome seems to have funky multi-process things going on in unit tests that
    // makes this unsafe to delete when the main process atexit()s.
    // SkLazyPtr does the same sort of thing.
#if SK_DEVELOPER
    SkDELETE(gResourceCache);
#endif
}

/** Must hold gMutex when calling. */
static SkResourceCache* get_cache() {
    // gMutex is always held when this is called, so we don't need to be fancy in here.
    gMutex.assertHeld();
    if (NULL == gResourceCache) {
#ifdef SK_USE_DISCARDABLE_SCALEDIMAGECACHE
        gResourceCache = SkNEW_ARGS(SkResourceCache, (SkDiscardableMemory::Create));
#else
        gResourceCache = SkNEW_ARGS(SkResourceCache, (SK_DEFAULT_IMAGE_CACHE_LIMIT));
#endif
        atexit(cleanup_gResourceCache);
    }
    return gResourceCache;
}

void SkResourceCache::Unlock(SkResourceCache::ID id) {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->unlock(id);

//    get_cache()->dump();
}

void SkResourceCache::Remove(SkResourceCache::ID id) {
    SkAutoMutexAcquire am(gMutex);
    SkASSERT(id);

#ifdef SK_DEBUG
    {
        bool found = false;
        Rec* rec = get_cache()->fHead;
        while (rec != NULL) {
            if (rec == id) {
                found = true;
                break;
            }
            rec = rec->fNext;
        }
        SkASSERT(found);
    }
#endif
    const Rec* rec = id;
    get_cache()->remove(const_cast<Rec*>(rec));
}

size_t SkResourceCache::GetTotalBytesUsed() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getTotalBytesUsed();
}

size_t SkResourceCache::GetTotalByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getTotalByteLimit();
}

size_t SkResourceCache::SetTotalByteLimit(size_t newLimit) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->setTotalByteLimit(newLimit);
}

SkBitmap::Allocator* SkResourceCache::GetAllocator() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->allocator();
}

void SkResourceCache::Dump() {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->dump();
}

size_t SkResourceCache::SetSingleAllocationByteLimit(size_t size) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->setSingleAllocationByteLimit(size);
}

size_t SkResourceCache::GetSingleAllocationByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getSingleAllocationByteLimit();
}

void SkResourceCache::PurgeAll() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->purgeAll();
}

const SkResourceCache::Rec* SkResourceCache::FindAndLock(const Key& key) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->findAndLock(key);
}

const SkResourceCache::Rec* SkResourceCache::AddAndLock(Rec* rec) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->addAndLock(rec);
}

void SkResourceCache::Add(Rec* rec) {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->add(rec);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGraphics.h"

size_t SkGraphics::GetResourceCacheTotalBytesUsed() {
    return SkResourceCache::GetTotalBytesUsed();
}

size_t SkGraphics::GetResourceCacheTotalByteLimit() {
    return SkResourceCache::GetTotalByteLimit();
}

size_t SkGraphics::SetResourceCacheTotalByteLimit(size_t newLimit) {
    return SkResourceCache::SetTotalByteLimit(newLimit);
}

size_t SkGraphics::GetResourceCacheSingleAllocationByteLimit() {
    return SkResourceCache::GetSingleAllocationByteLimit();
}

size_t SkGraphics::SetResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return SkResourceCache::SetSingleAllocationByteLimit(newLimit);
}

void SkGraphics::PurgeResourceCache() {
    return SkResourceCache::PurgeAll();
}

