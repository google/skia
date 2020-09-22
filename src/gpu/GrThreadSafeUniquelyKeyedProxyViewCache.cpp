/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrThreadSafeUniquelyKeyedProxyViewCache.h"

#include "src/gpu/GrResourceCache.h"

GrThreadSafeUniquelyKeyedProxyViewCache::GrThreadSafeUniquelyKeyedProxyViewCache()
    : fFreeEntryList(nullptr) {
}

GrThreadSafeUniquelyKeyedProxyViewCache::~GrThreadSafeUniquelyKeyedProxyViewCache() {
    this->dropAllRefs();
}

#if GR_TEST_UTILS
int GrThreadSafeUniquelyKeyedProxyViewCache::numEntries() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViewMap.count();
}

size_t GrThreadSafeUniquelyKeyedProxyViewCache::approxBytesUsedForHash() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViewMap.approxBytesUsed();
}
#endif

void GrThreadSafeUniquelyKeyedProxyViewCache::dropAllRefs() {
    SkAutoSpinlock lock{fSpinLock};

    fUniquelyKeyedProxyViewMap.reset();
    while (auto tmp = fUniquelyKeyedProxyViewList.head()) {
        fUniquelyKeyedProxyViewList.remove(tmp);
        this->recycleEntry(tmp);
    }
    // TODO: should we empty out the fFreeEntryList and reset fEntryAllocator?
}

void GrThreadSafeUniquelyKeyedProxyViewCache::dropAllUniqueRefs(GrResourceCache* resourceCache) {
    SkAutoSpinlock lock{fSpinLock};

    // Iterate from LRU to MRU
    Entry* cur = fUniquelyKeyedProxyViewList.tail();
    Entry* prev = cur ? cur->fPrev : nullptr;

    while (cur) {
        if (resourceCache && !resourceCache->overBudget()) {
            return;
        }

        if (cur->fView.proxy()->unique()) {
            fUniquelyKeyedProxyViewMap.remove(cur->fKey);
            fUniquelyKeyedProxyViewList.remove(cur);
            this->recycleEntry(cur);
        }

        cur = prev;
        prev = cur ? cur->fPrev : nullptr;
    }
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::find(const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    Entry* tmp = fUniquelyKeyedProxyViewMap.find(key);
    if (tmp) {
        SkASSERT(fUniquelyKeyedProxyViewList.isInList(tmp));
        // make the sought out entry the MRU
        fUniquelyKeyedProxyViewList.remove(tmp);
        fUniquelyKeyedProxyViewList.addToHead(tmp);
        return tmp->fView;
    }

    return {};
}

GrThreadSafeUniquelyKeyedProxyViewCache::Entry*
GrThreadSafeUniquelyKeyedProxyViewCache::getEntry(const GrUniqueKey& key,
                                                  const GrSurfaceProxyView& view) {
    Entry* entry;

    if (fFreeEntryList) {
        entry = fFreeEntryList;
        fFreeEntryList = entry->fNext;
        entry->fNext = nullptr;

        entry->fKey = key;
        entry->fView = view;
    } else {
        entry = fEntryAllocator.make<Entry>(key, view);
    }

    fUniquelyKeyedProxyViewList.addToHead(entry);  // make 'entry' the MRU
    fUniquelyKeyedProxyViewMap.add(entry);
    return entry;
}

void GrThreadSafeUniquelyKeyedProxyViewCache::recycleEntry(Entry* dead) {
    SkASSERT(!dead->fPrev && !dead->fNext && !dead->fList);

    dead->fKey.reset();
    dead->fView.reset();

    dead->fNext = fFreeEntryList;
    fFreeEntryList = dead;
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::internalAdd(
                                                                const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    Entry* tmp = fUniquelyKeyedProxyViewMap.find(key);
    if (!tmp) {
        tmp = this->getEntry(key, view);

        SkASSERT(fUniquelyKeyedProxyViewMap.find(key));
    }

    return tmp->fView;
}

GrSurfaceProxyView GrThreadSafeUniquelyKeyedProxyViewCache::add(const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAdd(key, view);
}
