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

// A magical cache - full of magic
//
// For this cache we should have the invariant that:
//   if there is a uniquely keyed version in the resource cache there should be an entry in this table
//   if there is an entry here that doesn't guarantee there is a live entry in the resource cache - but this entry should be able to generate that entry
//
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

    SkTHashMap<uint32_t, GrSurfaceProxyView> fUniquelyKeyedProxyViews SK_GUARDED_BY(fSpinLock);
};

#endif // GrThreadSafeUniquelyKeyedProxyViewCache_DEFINED
