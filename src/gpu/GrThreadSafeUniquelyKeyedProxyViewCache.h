/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrThreadSafeUniquelyKeyedProxyViewCache_DEFINED
#define GrThreadSafeUniquelyKeyedProxyViewCache_DEFINED

#include "include/private/SkSpinLock.h"
#include "src/core/SkTDynamicHash.h"
#include "src/gpu/GrSurfaceProxyView.h"

// Ganesh creates a lot of utility textures (e.g., blurred-rrect masks) that need to be shared
// between the direct context and all the DDL recording contexts. This thread-safe cache
// allows this sharing.
//
// In operation each thread will first check if the threaded cache possesses the required texture.
//
// If a DDL thread doesn't find a needed texture it will go off and create it on the cpu and then
// attempt to add it to the cache. If another thread had added it in the interim the losing thread
// will discard its work and use the texture the winning thread had created.
//
// If the thread in possession of the direct context doesn't find the needed texture ...
//  [Fill in - the gpu-thread should put a place holder in the cache and then queue the work]
//
// For this cache we should have the invariant that:
//   if there is a uniquely keyed version in the resource cache there should be an entry in
//   this table
//   if there is an entry here that doesn't guarantee there is a live entry in the resource cache
//      - but this entry should be able to generate that entry (i.e., be a lazy proxy)
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
// More:
//    Add gpu stats about SW vs. HW mask generation
//
class GrThreadSafeUniquelyKeyedProxyViewCache {
public:
    GrThreadSafeUniquelyKeyedProxyViewCache();

    void freeAll()  SK_EXCLUDES(fSpinLock) {}

    // parallels purgeStaleBlobs
    void purgeStale()  SK_EXCLUDES(fSpinLock) {}

    size_t usedBytes() const  SK_EXCLUDES(fSpinLock) { return 0; }

    GrSurfaceProxyView find(const GrUniqueKey&)  SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView add(const GrUniqueKey&, GrSurfaceProxyView)  SK_EXCLUDES(fSpinLock);

private:
    GrSurfaceProxyView internalAdd(const GrUniqueKey&, GrSurfaceProxyView)  SK_REQUIRES(fSpinLock);

    mutable SkSpinlock fSpinLock;

    SkTHashMap<uint32_t, GrSurfaceProxyView> fUniquelyKeyedProxyViews  SK_GUARDED_BY(fSpinLock);
};

#endif // GrThreadSafeUniquelyKeyedProxyViewCache_DEFINED
