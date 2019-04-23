/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkResourceCache.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/private/SkMessageBus.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTo.h"
#include "src/core/SkDiscardableMemory.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkOpts.h"

#include <stddef.h>
#include <stdlib.h>

DECLARE_SKMESSAGEBUS_MESSAGE(SkResourceCache::PurgeSharedIDMessage)

static inline bool SkShouldPostMessageToBus(
        const SkResourceCache::PurgeSharedIDMessage&, uint32_t) {
    // SkResourceCache is typically used as a singleton and we don't label Inboxes so all messages
    // go to all inboxes.
    return true;
}

// This can be defined by the caller's build system
//#define SK_USE_DISCARDABLE_SCALEDIMAGECACHE

#ifndef SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT
#   define SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT   1024
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
    #define SK_DEFAULT_IMAGE_CACHE_LIMIT     (32 * 1024 * 1024)
#endif

void SkResourceCache::Key::init(void* nameSpace, uint64_t sharedID, size_t dataSize) {
    SkASSERT(SkAlign4(dataSize) == dataSize);

    // fCount32 and fHash are not hashed
    static const int kUnhashedLocal32s = 2; // fCache32 + fHash
    static const int kSharedIDLocal32s = 2; // fSharedID_lo + fSharedID_hi
    static const int kHashedLocal32s = kSharedIDLocal32s + (sizeof(fNamespace) >> 2);
    static const int kLocal32s = kUnhashedLocal32s + kHashedLocal32s;

    static_assert(sizeof(Key) == (kLocal32s << 2), "unaccounted_key_locals");
    static_assert(sizeof(Key) == offsetof(Key, fNamespace) + sizeof(fNamespace),
                 "namespace_field_must_be_last");

    fCount32 = SkToS32(kLocal32s + (dataSize >> 2));
    fSharedID_lo = (uint32_t)(sharedID & 0xFFFFFFFF);
    fSharedID_hi = (uint32_t)(sharedID >> 32);
    fNamespace = nameSpace;
    // skip unhashed fields when computing the hash
    fHash = SkOpts::hash(this->as32() + kUnhashedLocal32s,
                         (fCount32 - kUnhashedLocal32s) << 2);
}

#include "include/private/SkTHash.h"

namespace {
    struct HashTraits {
        static uint32_t Hash(const SkResourceCache::Key& key) { return key.hash(); }
        static const SkResourceCache::Key& GetKey(const SkResourceCache::Rec* rec) {
            return rec->getKey();
        }
    };
}

class SkResourceCache::Hash :
    public SkTHashTable<SkResourceCache::Rec*, SkResourceCache::Key, HashTraits> {};


///////////////////////////////////////////////////////////////////////////////

void SkResourceCache::init() {
    fHead = nullptr;
    fTail = nullptr;
    fHash = new Hash;
    fTotalBytesUsed = 0;
    fCount = 0;
    fSingleAllocationByteLimit = 0;

    // One of these should be explicit set by the caller after we return.
    fTotalByteLimit = 0;
    fDiscardableFactory = nullptr;
}

SkResourceCache::SkResourceCache(DiscardableFactory factory) {
    this->init();
    fDiscardableFactory = factory;
}

SkResourceCache::SkResourceCache(size_t byteLimit) {
    this->init();
    fTotalByteLimit = byteLimit;
}

SkResourceCache::~SkResourceCache() {
    Rec* rec = fHead;
    while (rec) {
        Rec* next = rec->fNext;
        delete rec;
        rec = next;
    }
    delete fHash;
}

////////////////////////////////////////////////////////////////////////////////

bool SkResourceCache::find(const Key& key, FindVisitor visitor, void* context) {
    this->checkMessages();

    if (auto found = fHash->find(key)) {
        Rec* rec = *found;
        if (visitor(*rec, context)) {
            this->moveToHead(rec);  // for our LRU
            return true;
        } else {
            this->remove(rec);  // stale
            return false;
        }
    }
    return false;
}

static void make_size_str(size_t size, SkString* str) {
    const char suffix[] = { 'b', 'k', 'm', 'g', 't', 0 };
    int i = 0;
    while (suffix[i] && (size > 1024)) {
        i += 1;
        size >>= 10;
    }
    str->printf("%zu%c", size, suffix[i]);
}

static bool gDumpCacheTransactions;

void SkResourceCache::add(Rec* rec, void* payload) {
    this->checkMessages();

    SkASSERT(rec);
    // See if we already have this key (racy inserts, etc.)
    if (Rec** preexisting = fHash->find(rec->getKey())) {
        Rec* prev = *preexisting;
        if (prev->canBePurged()) {
            // if it can be purged, the install may fail, so we have to remove it
            this->remove(prev);
        } else {
            // if it cannot be purged, we reuse it and delete the new one
            prev->postAddInstall(payload);
            delete rec;
            return;
        }
    }

    this->addToHead(rec);
    fHash->set(rec);
    rec->postAddInstall(payload);

    if (gDumpCacheTransactions) {
        SkString bytesStr, totalStr;
        make_size_str(rec->bytesUsed(), &bytesStr);
        make_size_str(fTotalBytesUsed, &totalStr);
        SkDebugf("RC:    add %5s %12p key %08x -- total %5s, count %d\n",
                 bytesStr.c_str(), rec, rec->getHash(), totalStr.c_str(), fCount);
    }

    // since the new rec may push us over-budget, we perform a purge check now
    this->purgeAsNeeded();
}

void SkResourceCache::remove(Rec* rec) {
    SkASSERT(rec->canBePurged());
    size_t used = rec->bytesUsed();
    SkASSERT(used <= fTotalBytesUsed);

    this->release(rec);
    fHash->remove(rec->getKey());

    fTotalBytesUsed -= used;
    fCount -= 1;

    //SkDebugf("-RC count [%3d] bytes %d\n", fCount, fTotalBytesUsed);

    if (gDumpCacheTransactions) {
        SkString bytesStr, totalStr;
        make_size_str(used, &bytesStr);
        make_size_str(fTotalBytesUsed, &totalStr);
        SkDebugf("RC: remove %5s %12p key %08x -- total %5s, count %d\n",
                 bytesStr.c_str(), rec, rec->getHash(), totalStr.c_str(), fCount);
    }

    delete rec;
}

void SkResourceCache::purgeAsNeeded(bool forcePurge) {
    size_t byteLimit;
    int    countLimit;

    if (fDiscardableFactory) {
        countLimit = SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT;
        byteLimit = UINT32_MAX;  // no limit based on bytes
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
        if (rec->canBePurged()) {
            this->remove(rec);
        }
        rec = prev;
    }
}

//#define SK_TRACK_PURGE_SHAREDID_HITRATE

#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
static int gPurgeCallCounter;
static int gPurgeHitCounter;
#endif

void SkResourceCache::purgeSharedID(uint64_t sharedID) {
    if (0 == sharedID) {
        return;
    }

#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
    gPurgeCallCounter += 1;
    bool found = false;
#endif
    // go backwards, just like purgeAsNeeded, just to make the code similar.
    // could iterate either direction and still be correct.
    Rec* rec = fTail;
    while (rec) {
        Rec* prev = rec->fPrev;
        if (rec->getKey().getSharedID() == sharedID) {
            // even though the "src" is now dead, caches could still be in-flight, so
            // we have to check if it can be removed.
            if (rec->canBePurged()) {
                this->remove(rec);
            }
#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
            found = true;
#endif
        }
        rec = prev;
    }

#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
    if (found) {
        gPurgeHitCounter += 1;
    }

    SkDebugf("PurgeShared calls=%d hits=%d rate=%g\n", gPurgeCallCounter, gPurgeHitCounter,
             gPurgeHitCounter * 100.0 / gPurgeCallCounter);
#endif
}

void SkResourceCache::visitAll(Visitor visitor, void* context) {
    // go backwards, just like purgeAsNeeded, just to make the code similar.
    // could iterate either direction and still be correct.
    Rec* rec = fTail;
    while (rec) {
        visitor(*rec, context);
        rec = rec->fPrev;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkResourceCache::setTotalByteLimit(size_t newLimit) {
    size_t prevLimit = fTotalByteLimit;
    fTotalByteLimit = newLimit;
    if (newLimit < prevLimit) {
        this->purgeAsNeeded();
    }
    return prevLimit;
}

SkCachedData* SkResourceCache::newCachedData(size_t bytes) {
    this->checkMessages();

    if (fDiscardableFactory) {
        SkDiscardableMemory* dm = fDiscardableFactory(bytes);
        return dm ? new SkCachedData(bytes, dm) : nullptr;
    } else {
        return new SkCachedData(sk_malloc_throw(bytes), bytes);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkResourceCache::release(Rec* rec) {
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

    rec->fNext = rec->fPrev = nullptr;
}

void SkResourceCache::moveToHead(Rec* rec) {
    if (fHead == rec) {
        return;
    }

    SkASSERT(fHead);
    SkASSERT(fTail);

    this->validate();

    this->release(rec);

    fHead->fPrev = rec;
    rec->fNext = fHead;
    fHead = rec;

    this->validate();
}

void SkResourceCache::addToHead(Rec* rec) {
    this->validate();

    rec->fPrev = nullptr;
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
    if (nullptr == fHead) {
        SkASSERT(nullptr == fTail);
        SkASSERT(0 == fTotalBytesUsed);
        return;
    }

    if (fHead == fTail) {
        SkASSERT(nullptr == fHead->fPrev);
        SkASSERT(nullptr == fHead->fNext);
        SkASSERT(fHead->bytesUsed() == fTotalBytesUsed);
        return;
    }

    SkASSERT(nullptr == fHead->fPrev);
    SkASSERT(fHead->fNext);
    SkASSERT(nullptr == fTail->fNext);
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

    SkDebugf("SkResourceCache: count=%d bytes=%d %s\n",
             fCount, fTotalBytesUsed, fDiscardableFactory ? "discardable" : "malloc");
}

size_t SkResourceCache::setSingleAllocationByteLimit(size_t newLimit) {
    size_t oldLimit = fSingleAllocationByteLimit;
    fSingleAllocationByteLimit = newLimit;
    return oldLimit;
}

size_t SkResourceCache::getSingleAllocationByteLimit() const {
    return fSingleAllocationByteLimit;
}

size_t SkResourceCache::getEffectiveSingleAllocationByteLimit() const {
    // fSingleAllocationByteLimit == 0 means the caller is asking for our default
    size_t limit = fSingleAllocationByteLimit;

    // if we're not discardable (i.e. we are fixed-budget) then cap the single-limit
    // to our budget.
    if (nullptr == fDiscardableFactory) {
        if (0 == limit) {
            limit = fTotalByteLimit;
        } else {
            limit = SkTMin(limit, fTotalByteLimit);
        }
    }
    return limit;
}

void SkResourceCache::checkMessages() {
    SkTArray<PurgeSharedIDMessage> msgs;
    fPurgeSharedIDInbox.poll(&msgs);
    for (int i = 0; i < msgs.count(); ++i) {
        this->purgeSharedID(msgs[i].fSharedID);
    }
}

///////////////////////////////////////////////////////////////////////////////

SK_DECLARE_STATIC_MUTEX(gMutex);
static SkResourceCache* gResourceCache = nullptr;

/** Must hold gMutex when calling. */
static SkResourceCache* get_cache() {
    // gMutex is always held when this is called, so we don't need to be fancy in here.
    gMutex.assertHeld();
    if (nullptr == gResourceCache) {
#ifdef SK_USE_DISCARDABLE_SCALEDIMAGECACHE
        gResourceCache = new SkResourceCache(SkDiscardableMemory::Create);
#else
        gResourceCache = new SkResourceCache(SK_DEFAULT_IMAGE_CACHE_LIMIT);
#endif
    }
    return gResourceCache;
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

SkResourceCache::DiscardableFactory SkResourceCache::GetDiscardableFactory() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->discardableFactory();
}

SkCachedData* SkResourceCache::NewCachedData(size_t bytes) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->newCachedData(bytes);
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

size_t SkResourceCache::GetEffectiveSingleAllocationByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getEffectiveSingleAllocationByteLimit();
}

void SkResourceCache::PurgeAll() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->purgeAll();
}

bool SkResourceCache::Find(const Key& key, FindVisitor visitor, void* context) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->find(key, visitor, context);
}

void SkResourceCache::Add(Rec* rec, void* payload) {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->add(rec, payload);
}

void SkResourceCache::VisitAll(Visitor visitor, void* context) {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->visitAll(visitor, context);
}

void SkResourceCache::PostPurgeSharedID(uint64_t sharedID) {
    if (sharedID) {
        SkMessageBus<PurgeSharedIDMessage>::Post(PurgeSharedIDMessage(sharedID));
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "include/core/SkGraphics.h"
#include "include/core/SkImageFilter.h"

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
    SkImageFilter::PurgeCache();
    return SkResourceCache::PurgeAll();
}

/////////////

static void dump_visitor(const SkResourceCache::Rec& rec, void*) {
    SkDebugf("RC: %12s bytes %9lu  discardable %p\n",
             rec.getCategory(), rec.bytesUsed(), rec.diagnostic_only_getDiscardable());
}

void SkResourceCache::TestDumpMemoryStatistics() {
    VisitAll(dump_visitor, nullptr);
}

static void sk_trace_dump_visitor(const SkResourceCache::Rec& rec, void* context) {
    SkTraceMemoryDump* dump = static_cast<SkTraceMemoryDump*>(context);
    SkString dumpName = SkStringPrintf("skia/sk_resource_cache/%s_%p", rec.getCategory(), &rec);
    SkDiscardableMemory* discardable = rec.diagnostic_only_getDiscardable();
    if (discardable) {
        dump->setDiscardableMemoryBacking(dumpName.c_str(), *discardable);

        // The discardable memory size will be calculated by dumper, but we also dump what we think
        // the size of object in memory is irrespective of whether object is live or dead.
        dump->dumpNumericValue(dumpName.c_str(), "discardable_size", "bytes", rec.bytesUsed());
    } else {
        dump->dumpNumericValue(dumpName.c_str(), "size", "bytes", rec.bytesUsed());
        dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
    }
}

void SkResourceCache::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    // Since resource could be backed by malloc or discardable, the cache always dumps detailed
    // stats to be accurate.
    VisitAll(sk_trace_dump_visitor, dump);
}
