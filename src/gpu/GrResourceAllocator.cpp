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
        SkASSERT(intvl->fEnd < start);
        intvl->fEnd = end;
        return;
    }

    // TODO: given the usage pattern an arena allocation scheme would work well here
    Interval* newIntvl = new Interval(proxy, start, end);

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
    // TODO: add free pool
}

// First try to reuse one of the recently allocated/used GrSurfaces in the free pool.
// If we can't find a useable one, create a new one.
// TODO: handle being overbudget
sk_sp<GrSurface> GrResourceAllocator::findSurfaceFor(GrSurfaceProxy* proxy) {
    // TODO: add free pool

    // Try to grab one from the resource cache
    return proxy->priv().createSurface(fResourceProvider);
}

// Remove any intervals that end before the current index. Return their GrSurfaces
// to the free pool.
void GrResourceAllocator::expire(unsigned int curIndex) {
    while (!fActiveIntvls.empty() && fActiveIntvls.peekHead()->fEnd < curIndex) {
        Interval* temp = fActiveIntvls.popHead();
        this->freeUpSurface(temp->fProxy->priv().peekSurface());
        delete temp;
    }
}

void GrResourceAllocator::assign() {
    fIntvlHash.reset(); // we don't need this anymore
    SkDEBUGCODE(fAssigned = true;)

    while (Interval* cur = fIntvlList.popHead()) {
        this->expire(cur->fStart);
        // TODO: add over budget handling here?
        sk_sp<GrSurface> surface = this->findSurfaceFor(cur->fProxy);
        if (surface) {
            cur->fProxy->priv().assign(std::move(surface));
        }
        // TODO: handle resouce allocation failure upstack
        fActiveIntvls.insertByIncreasingEnd(cur);
    }
}
