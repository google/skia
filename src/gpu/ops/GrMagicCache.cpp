/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrMagicCache.h"

#include "src/gpu/GrSurfaceProxyView.h"

GrThreadSafeUniquelyKeyedProxyViewCache::GrThreadSafeUniquelyKeyedProxyViewCache() {}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::find(const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    GrSurfaceProxyView* tmp = fUniquelyKeyedProxyViews.find(key.hash());
    if (tmp) {
        return *tmp;
    }

    return {};
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::internalAdd(const GrUniqueKey& key,
                                                                        GrSurfaceProxyView view) {
    GrSurfaceProxyView* tmp = fUniquelyKeyedProxyViews.find(key.hash());
    if (tmp) {
        return *tmp;
    }

    return *fUniquelyKeyedProxyViews.set(key.hash(), std::move(view));
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::add(const GrUniqueKey& key,
                                                                GrSurfaceProxyView view) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAdd(key, std::move(view));
}
