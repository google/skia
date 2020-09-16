/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrThreadSafeUniquelyKeyedProxyViewCache.h"

GrThreadSafeUniquelyKeyedProxyViewCache::GrThreadSafeUniquelyKeyedProxyViewCache() {}

GrThreadSafeUniquelyKeyedProxyViewCache::~GrThreadSafeUniquelyKeyedProxyViewCache() {
    fUniquelyKeyedProxyViews.foreach([](const GrUniqueKey&k, Entry** v) { delete *v; });
}

#if GR_TEST_UTILS
int GrThreadSafeUniquelyKeyedProxyViewCache::numEntries() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViews.count();
}

size_t GrThreadSafeUniquelyKeyedProxyViewCache::approxBytesUsed() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViews.approxBytesUsed();
}
#endif

void GrThreadSafeUniquelyKeyedProxyViewCache::dropAllRefs() {
    SkAutoSpinlock lock{fSpinLock};

    fUniquelyKeyedProxyViews.reset();
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::find(const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    Entry** tmp = fUniquelyKeyedProxyViews.find(const_cast<GrUniqueKey&>(key));
    if (tmp) {
        return (*tmp)->fView;
    }

    return {};
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::internalAdd(
                                                                const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    Entry** tmp = fUniquelyKeyedProxyViews.find(const_cast<GrUniqueKey&>(key));
    if (!tmp) {
        // TODO: block allocate here?
        Entry* newT = new Entry(key, view);
        tmp = fUniquelyKeyedProxyViews.set(newT->fKey, newT);
    }

    return (*tmp)->fView;
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::add(const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAdd(key, view);
}
