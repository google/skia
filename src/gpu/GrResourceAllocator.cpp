/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceAllocator.h"

#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"

void GrResourceAllocator::addInterval(GrSurfaceProxy* proxy,
                                      unsigned int start, unsigned int end) {
    SkASSERT(start <= end);
    SkASSERT(!fAssigned);      // We shouldn't be adding any intervals after (or during) assignment

    if (Interval* intvl = fIntvlHash.find(proxy->uniqueID().asUInt())) {
        // Revise the interval for an existing use
        //SkASSERT(intvl->fEnd <= end);
        intvl->fEnd = end;
        return;
    }

    Interval* newIntvl;
    if (fFreeIntervalList) {
        newIntvl = fFreeIntervalList;
        fFreeIntervalList = newIntvl->fNext;
        newIntvl->resetTo(proxy, start, end);
    } else {
        newIntvl = fIntervalAllocator.make<Interval>(proxy, start, end);
    }

    fIntvlList.insertByIncreasingStart(newIntvl);
    fIntvlHash.add(newIntvl);
}

GrResourceAllocator::Interval* GrResourceAllocator::IntervalList::popHead() {
    Interval* temp = fHead;
    if (temp) {
        fHead = temp->fNext;
    }
    return temp;
}

// TODO: fuse this with insertByIncreasingEnd
void GrResourceAllocator::IntervalList::insertByIncreasingStart(Interval* intvl) {
    if (!fHead) {
        intvl->fNext = nullptr;
        fHead = intvl;
    } else if (intvl->fStart <= fHead->fStart) {
        intvl->fNext = fHead;
        fHead = intvl;
    } else {
        Interval* prev = fHead;
        Interval* next = prev->fNext;
        for (; next && intvl->fStart > next->fStart; prev = next, next = next->fNext) {
        }
        intvl->fNext = next;
        prev->fNext = intvl;
    }
}

// TODO: fuse this with insertByIncreasingStart
void GrResourceAllocator::IntervalList::insertByIncreasingEnd(Interval* intvl) {
    if (!fHead) {
        intvl->fNext = nullptr;
        fHead = intvl;
    } else if (intvl->fEnd <= fHead->fEnd) {
        intvl->fNext = fHead;
        fHead = intvl;
    } else {
        Interval* prev = fHead;
        Interval* next = prev->fNext;
        for (; next && intvl->fEnd > next->fEnd; prev = next, next = next->fNext) {
        }
        intvl->fNext = next;
        prev->fNext = intvl;
    }
}

// 'surface' can be reused. Add it back to the free pool.
void GrResourceAllocator::freeUpSurface(GrSurface* surface) {
    const GrScratchKey &key = surface->resourcePriv().getScratchKey();

    if (!key.isValid()) {
        return; // can't do it w/o a valid scratch key
    }

    // TODO: fix this insertion so we get a more LRU-ish behavior
    fFreePool.insert(key, surface);
}

// First try to reuse one of the recently allocated/used GrSurfaces in the free pool.
// If we can't find a useable one, create a new one.
// TODO: handle being overbudget
sk_sp<GrSurface> GrResourceAllocator::findSurfaceFor(const GrSurfaceProxy* proxy) {
    // First look in the free pool
    GrScratchKey key;

    proxy->priv().computeScratchKey(&key);

    GrSurface* surface = fFreePool.find(key);
    if (surface) {
        return sk_ref_sp(surface);
    }

    // Failing that, try to grab a new one from the resource cache
    return proxy->priv().createSurface(fResourceProvider);
}

// Remove any intervals that end before the current index. Return their GrSurfaces
// to the free pool.
void GrResourceAllocator::expire(unsigned int curIndex) {
    while (!fActiveIntvls.empty() && fActiveIntvls.peekHead()->fEnd < curIndex) {
        Interval* temp = fActiveIntvls.popHead();
        this->freeUpSurface(temp->fProxy->priv().peekSurface());

        // Add temp to the free interval list so it can be reused
        temp->fNext = fFreeIntervalList;
        fFreeIntervalList = temp;
    }
}

void GrResourceAllocator::assign() {
    fIntvlHash.reset(); // we don't need this anymore
    SkDEBUGCODE(fAssigned = true;)

    while (Interval* cur = fIntvlList.popHead()) {
        this->expire(cur->fStart);

        if (cur->fProxy->priv().isInstantiated()) {
            fActiveIntvls.insertByIncreasingEnd(cur);
            continue;
        }

        // TODO: add over budget handling here?
        sk_sp<GrSurface> surface = this->findSurfaceFor(cur->fProxy);
        if (surface) {
            cur->fProxy->priv().assign(std::move(surface));
        }
        // TODO: handle resouce allocation failure upstack
        fActiveIntvls.insertByIncreasingEnd(cur);
    }
}

#ifdef SK_DEBUG
void GrResourceAllocator::dump() {
    unsigned int min = fNumOps+1;
    unsigned int max = 0;
    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->fNext) {
        SkDebugf("{ %d,%d }: [%d, %d]\n",
                 cur->fProxy->uniqueID().asUInt(), cur->fProxy->underlyingUniqueID().asUInt(),
                 cur->fStart, cur->fEnd);
        if (min > cur->fStart) {
            min = cur->fStart;
        }
        if (max < cur->fEnd) {
            max = cur->fEnd;
        }
    }

    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->fNext) {
        SkDebugf("{ %3d,%3d }: ",
                 cur->fProxy->uniqueID().asUInt(), cur->fProxy->underlyingUniqueID().asUInt());
        for (unsigned int i = min; i <= max; ++i) {
            if (i >= cur->fStart && i <= cur->fEnd) {
                SkDebugf("x");
            } else {
                SkDebugf(" ");
            }
        }
        SkDebugf("\n");
    }
}
#endif

