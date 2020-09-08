/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMagicCache_DEFINED
#define GrMagicCache_DEFINED

#include "include/private/SkSpinLock.h"
//#include "src/core/SkTDynamicHash.h"

class GrSurfaceProxyView;
class GrUniqueKey;

// A magical cache - full of magic
class GrMagicCache {
public:
    GrMagicCache();

    void freeAll()  SK_EXCLUDES(fSpinLock) {}

    // parallels purgeStaleBlobs
    void purgeStale()  SK_EXCLUDES(fSpinLock) {}

    size_t usedBytes() const  SK_EXCLUDES(fSpinLock) { return 0; }

    GrSurfaceProxyView find(const GrUniqueKey&) SK_EXCLUDES(fSpinLock);

    GrSurfaceProxyView add(const GrUniqueKey&, GrSurfaceProxyView) SK_EXCLUDES(fSpinLock);

private:
#if 0
    struct UniquelyKeyedProxyHashTraits {
        static const GrUniqueKey& GetKey(const GrTextureProxy& p) { return p.getUniqueKey(); }

        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };
    typedef SkTDynamicHash<GrTextureProxy, GrUniqueKey, UniquelyKeyedProxyHashTraits> UniquelyKeyedProxyHash;
#endif

    mutable SkSpinlock fSpinLock;

    //UniquelyKeyedProxyHash fUniquelyKeyedProxies;
};

#endif // GrMagicCache_DEFINED
