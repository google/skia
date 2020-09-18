/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrThreadSafeUniquelyKeyedProxyViewCache.h"

GrThreadSafeUniquelyKeyedProxyViewCache::GrThreadSafeUniquelyKeyedProxyViewCache() {}

GrThreadSafeUniquelyKeyedProxyViewCache::~GrThreadSafeUniquelyKeyedProxyViewCache() {
    fUniquelyKeyedProxyViews.foreach([this](Entry* v) { this->recycleEntry(v); });
}

#if GR_TEST_UTILS
int GrThreadSafeUniquelyKeyedProxyViewCache::numEntries() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViews.count();
}

int GrThreadSafeUniquelyKeyedProxyViewCache::count() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViews.count();
}
#endif

void GrThreadSafeUniquelyKeyedProxyViewCache::dropAllRefs() {
    SkAutoSpinlock lock{fSpinLock};

    fUniquelyKeyedProxyViews.foreach([this](Entry* v) { this->recycleEntry(v); });
    fUniquelyKeyedProxyViews.reset();
}

void GrThreadSafeUniquelyKeyedProxyViewCache::dropAllUniqueRefs() {
    SkAutoSpinlock lock{fSpinLock};

    fUniquelyKeyedProxyViews.foreach([](Entry* v) {
                                        // problematic
                                    });
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::find(const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    Entry* tmp = fUniquelyKeyedProxyViews.find(key);
    if (tmp) {
        return tmp->fView;
    }

    return {};
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::internalAdd(
                                                                const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    Entry* tmp = fUniquelyKeyedProxyViews.find(key);
    if (!tmp) {
        tmp = this->getEntry(key, view);
        fUniquelyKeyedProxyViews.add(tmp);
    }

    return tmp->fView;
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::add(const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAdd(key, view);
}
