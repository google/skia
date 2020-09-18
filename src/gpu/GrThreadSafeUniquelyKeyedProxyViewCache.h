/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrThreadSafeUniquelyKeyedProxyViewCache_DEFINED
#define GrThreadSafeUniquelyKeyedProxyViewCache_DEFINED

#include "include/private/SkSpinlock.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkTDynamicHash.h"
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
//----------------------------------------------------------------------------------------
//
// For testing:
//    Create DDLs all needing the same texture - check that only one wins - and that there is some duplicate work
//    Create a gpu version & check that all DDLs use it - with no duplicate work
//    Create a texture w/ a DDL and then make a live use of it - check that the DDL version wins
//       - in the above, each mask should have the correct # of refs for the # of DDLs
//
//    Need to test out the flushing behavior:
//      generate a bunch of masks to fill up memory
//      flush the resource cache and ensure the corresponding entries in the cache are cleared
//
//      do the above but have some locked up in DDLs - so they survive the flush
//
//      do the above but have them all non-instantiated (in DDLs) so they all survive the flush
//
// Concern:
//    It seems like the interaction between the resource cache and the thread-safe view cache
//    is going to be very fraught. In particular, when the resource cache is purging we have to
//    ensure that there are no race conditions w/ any recording threads.
//
//    In that case it seems like we need to figure out we're going to nuke a resource, then
//    try to nuke it's entry here first. If that succeeds then we can delete the resource otherwise
//    some thread locked it in the interim so we need to let the resource live. (Test this case!)
//
// Qs:
//    It seems like the proxies in this cache shouldn't appear in any of the DDL nor Direct
//    contexts' proxy-caches. That way there will only be one source of truth. A related question
//    is, if that is the case, should all the uniquely keyed proxies just be stored here? It does
//    seem like that would have some problems w/ the higgledy-piggledy nature of the unique keys in
//    the normal caches and w/ the lack of a 1-1 correspondence between the proxy and the actual
//    resource (i.e., there can be unique-ly keyed resources that lack a proxy).
//
// More:
//    Add gpu stats about SW vs. HW mask generation
//    Make sure we're not reffing/un-reffing too much by passing GrSurfaceProxyViews around
//
class GrThreadSafeUniquelyKeyedProxyViewCache {
public:
    GrThreadSafeUniquelyKeyedProxyViewCache();
    ~GrThreadSafeUniquelyKeyedProxyViewCache();

#if GR_TEST_UTILS
    int numEntries() const  SK_EXCLUDES(fSpinLock);
    int count() const  SK_EXCLUDES(fSpinLock);
#endif

    void dropAllRefs()  SK_EXCLUDES(fSpinLock);
    void dropAllUniqueRefs()  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView find(const GrUniqueKey&)  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView add(const GrUniqueKey&, const GrSurfaceProxyView&)  SK_EXCLUDES(fSpinLock);

private:
    struct Entry {
        Entry(const GrUniqueKey& key, const GrSurfaceProxyView& view, bool) : fKey(key), fView(view) {}

        // Note: the unique key is stored here bc it is never attached to a proxy or a GrTexture
        GrUniqueKey        fKey;
        GrSurfaceProxyView fView;
        Entry*             fNext = nullptr;

        // for SkTDynamicHash
        static const GrUniqueKey& GetKey(const Entry& e) { return e.fKey; }
        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };

    Entry* getEntry(const GrUniqueKey& key, const GrSurfaceProxyView& view) {
        Entry* newEntry;
        if (fFreeEntryList) {
            newEntry = fFreeEntryList;
            fFreeEntryList = newEntry->fNext;
            newEntry->fNext = nullptr;

            newEntry->fKey = key;
            newEntry->fView = view;
        } else {
            newEntry = fEntryAllocator.make<Entry>(key, view, true);
        }

        return newEntry;
    }

    void recycleEntry(Entry* dead) {
        dead->fKey.reset();
        dead->fView.reset();
        dead->fNext = fFreeEntryList;
        fFreeEntryList = dead;
    }

    GrSurfaceProxyView internalAdd(const GrUniqueKey&,
                                   const GrSurfaceProxyView&)  SK_REQUIRES(fSpinLock);

    mutable SkSpinlock fSpinLock;

    SkTDynamicHash<Entry, GrUniqueKey> fUniquelyKeyedProxyViews  SK_GUARDED_BY(fSpinLock);

    // TODO: empirically determine this from the skps
    static const int kInitialArenaSize = 64 * sizeof(Entry);

    char                         fStorage[kInitialArenaSize];
    SkArenaAlloc                 fEntryAllocator{fStorage, kInitialArenaSize, kInitialArenaSize};
    Entry*                       fFreeEntryList = nullptr;

};

#endif // GrThreadSafeUniquelyKeyedProxyViewCache_DEFINED
