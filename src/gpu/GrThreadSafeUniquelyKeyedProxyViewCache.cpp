/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrThreadSafeUniquelyKeyedProxyViewCache.h"

GrThreadSafeUniquelyKeyedProxyViewCache::GrThreadSafeUniquelyKeyedProxyViewCache() {}

GrThreadSafeUniquelyKeyedProxyViewCache::~GrThreadSafeUniquelyKeyedProxyViewCache() {
    this->dropAllRefs();
}

#if GR_TEST_UTILS
int GrThreadSafeUniquelyKeyedProxyViewCache::numEntries() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViewMap.count();
}

// sigh - need to be approxBytes
int GrThreadSafeUniquelyKeyedProxyViewCache::count() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViewMap.count();
}
#endif

void GrThreadSafeUniquelyKeyedProxyViewCache::dropAllRefs() {
    SkAutoSpinlock lock{fSpinLock};

    while (auto tmp = fUniquelyKeyedProxyViewList.head()) {
        fUniquelyKeyedProxyViewList.remove(tmp);
        this->recycleEntry(tmp);
    }
    fUniquelyKeyedProxyViewMap.reset();
}

void GrThreadSafeUniquelyKeyedProxyViewCache::dropAllUniqueRefs() {
    SkAutoSpinlock lock{fSpinLock};

    while (auto tmp = fUniquelyKeyedProxyViewList.head()) {
        if (tmp->fView.proxy()->unique()) {
            fUniquelyKeyedProxyViewList.remove(tmp);
            fUniquelyKeyedProxyViewMap.remove(tmp->fKey);
            this->recycleEntry(tmp);
        }
    }
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::find(const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    Entry* tmp = fUniquelyKeyedProxyViewMap.find(key);
    if (tmp) {
        return tmp->fView;
    }

    return {};
}

GrThreadSafeUniquelyKeyedProxyViewCache::Entry*
GrThreadSafeUniquelyKeyedProxyViewCache::createEntry(const GrUniqueKey& key,
                                                     const GrSurfaceProxyView& view) {
    Entry* newEntry;
    if (fFreeEntryList) {
        newEntry = fFreeEntryList;
        fFreeEntryList = newEntry->fNext;
        newEntry->fNext = nullptr;

        SkASSERT(!newEntry->fPrev);
        newEntry->fKey = key;
        newEntry->fView = view;
    } else {
        newEntry = fEntryAllocator.make<Entry>(key, view);
    }

    fUniquelyKeyedProxyViewList.addToHead(newEntry);
    return newEntry;
}

void GrThreadSafeUniquelyKeyedProxyViewCache::recycleEntry(Entry* dead) {
    dead->fKey.reset();
    dead->fView.reset();
    dead->fPrev = nullptr;
    dead->fNext = fFreeEntryList;
    fFreeEntryList = dead;
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::internalAdd(
                                                                const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    Entry* tmp = fUniquelyKeyedProxyViewMap.find(key);
    if (!tmp) {
        tmp = this->createEntry(key, view);
        fUniquelyKeyedProxyViewList.addToHead(tmp);
    }

    return tmp->fView;
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::add(const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAdd(key, view);
}
