/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrThreadSafeCache_DEFINED
#define GrThreadSafeCache_DEFINED

#include "include/private/SkSpinlock.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkTDynamicHash.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/GrSurfaceProxyView.h"

// Ganesh creates a lot of utility textures (e.g., blurred-rrect masks) that need to be shared
// between the direct context and all the DDL recording contexts. This thread-safe cache
// allows this sharing.
//
// In operation, each thread will first check if the threaded cache possesses the required texture.
//
// If a DDL thread doesn't find a needed texture it will go off and create it on the cpu and then
// attempt to add it to the cache. If another thread had added it in the interim, the losing thread
// will discard its work and use the texture the winning thread had created.
//
// If the thread in possession of the direct context doesn't find the needed texture it should
// add a place holder view and then queue up the draw calls to complete it. In this way the
// gpu-thread has precedence over the recording threads.
//
// The invariants for this cache differ a bit from those of the proxy and resource caches.
// For this cache:
//
//   only this cache knows the unique key - neither the proxy nor backing resource should
//              be discoverable in any other cache by the unique key
//   if a backing resource resides in the resource cache then there should be an entry in this
//              cache
//   an entry in this cache, however, doesn't guarantee that there is a corresponding entry in
//              the resource cache - although the entry here should be able to generate that entry
//              (i.e., be a lazy proxy)
//
// Wrt interactions w/ GrContext/GrResourceCache purging, we have:
//
//    Both GrContext::abandonContext and GrContext::releaseResourcesAndAbandonContext will cause
//    all the refs held in this cache to be dropped prior to clearing out the resource cache.
//
//    For the size_t-variant of GrContext::purgeUnlockedResources, after an initial attempt
//    to purge the requested amount of resources fails, uniquely held resources in this cache
//    will be dropped in LRU to MRU order until the cache is under budget. Note that this
//    prioritizes the survival of resources in this cache over those just in the resource cache.
//
//    For the 'scratchResourcesOnly' variant of GrContext::purgeUnlockedResources, this cache
//    won't be modified in the scratch-only case unless the resource cache is over budget (in
//    which case it will purge uniquely-held resources in LRU to MRU order to get
//    back under budget). In the non-scratch-only case, all uniquely held resources in this cache
//    will be released prior to the resource cache being cleared out.
//
//    For GrContext::setResourceCacheLimit, if an initial pass through the resource cache doesn't
//    reach the budget, uniquely held resources in this cache will be released in LRU to MRU order.
//
//    For GrContext::performDeferredCleanup, any uniquely held resources that haven't been accessed
//    w/in 'msNotUsed' will be released from this cache prior to the resource cache being cleaned.
class GrThreadSafeCache {
public:
    GrThreadSafeCache();
    ~GrThreadSafeCache();

#if GR_TEST_UTILS
    int numEntries() const  SK_EXCLUDES(fSpinLock);

    size_t approxBytesUsedForHash() const  SK_EXCLUDES(fSpinLock);
#endif

    void dropAllRefs()  SK_EXCLUDES(fSpinLock);

    // Drop uniquely held refs until under the resource cache's budget.
    // A null parameter means drop all uniquely held refs.
    void dropUniqueRefs(GrResourceCache* resourceCache)  SK_EXCLUDES(fSpinLock);

    // Drop uniquely held refs that were last accessed before 'purgeTime'
    void dropUniqueRefsOlderThan(GrStdSteadyClock::time_point purgeTime)  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView find(const GrUniqueKey&)  SK_EXCLUDES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> findWithData(
                                                      const GrUniqueKey&)  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView add(const GrUniqueKey&, const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> addWithData(
                            const GrUniqueKey&, const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView findOrAdd(const GrUniqueKey&,
                                 const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> findOrAddWithData(
                            const GrUniqueKey&, const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);

    void remove(const GrUniqueKey&)  SK_EXCLUDES(fSpinLock);

    // To allow gpu-created resources to have priority, we pre-emptively place a lazy proxy
    // in the thread-safe cache (with findOrAdd). The Trampoline object allows that lazy proxy to
    // be instantiated with some later generated rendering result.
    class Trampoline : public SkRefCnt {
    public:
        sk_sp<GrTextureProxy> fProxy;
    };

    static std::tuple<GrSurfaceProxyView, sk_sp<Trampoline>> CreateLazyView(GrDirectContext*,
                                                                            GrColorType,
                                                                            SkISize dimensions,
                                                                            GrSurfaceOrigin,
                                                                            SkBackingFit);
private:
    struct Entry {
        Entry(const GrUniqueKey& key, const GrSurfaceProxyView& view) : fKey(key), fView(view) {}

        // Note: the unique key is stored here bc it is never attached to a proxy or a GrTexture
        GrUniqueKey                  fKey;
        GrSurfaceProxyView           fView;
        GrStdSteadyClock::time_point fLastAccess;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Entry);

        // for SkTDynamicHash
        static const GrUniqueKey& GetKey(const Entry& e) { return e.fKey; }
        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };

    Entry* getEntry(const GrUniqueKey&, const GrSurfaceProxyView&) SK_REQUIRES(fSpinLock);
    void recycleEntry(Entry*)  SK_REQUIRES(fSpinLock);

    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> internalFind(
                                                       const GrUniqueKey&)  SK_REQUIRES(fSpinLock);
    std::tuple<GrSurfaceProxyView, sk_sp<SkData>> internalAdd(
                            const GrUniqueKey&, const GrSurfaceProxyView&)  SK_REQUIRES(fSpinLock);

    mutable SkSpinlock fSpinLock;

    SkTDynamicHash<Entry, GrUniqueKey> fUniquelyKeyedProxyViewMap  SK_GUARDED_BY(fSpinLock);
    // The head of this list is the MRU
    SkTInternalLList<Entry>            fUniquelyKeyedProxyViewList  SK_GUARDED_BY(fSpinLock);

    // TODO: empirically determine this from the skps
    static const int kInitialArenaSize = 64 * sizeof(Entry);

    char                         fStorage[kInitialArenaSize];
    SkArenaAlloc                 fEntryAllocator{fStorage, kInitialArenaSize, kInitialArenaSize};
    Entry*                       fFreeEntryList  SK_GUARDED_BY(fSpinLock);
};

#endif // GrThreadSafeCache_DEFINED
