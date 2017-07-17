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
                                      unsigned int start, unsigned int end,
                                      const char* msg) {
    SkASSERT(start <= end);
    SkASSERT(!fAssigned);      // We shouldn't be adding any intervals after (or during) assignment

    unsigned int proxyID = proxy->uniqueID().asUInt();
    unsigned int underlyingID = proxy->underlyingUniqueID().asUInt();

    if (Interval* intvl = fIntvlHash.find(proxyID)) {
        // Revise the interval for an existing use
        SkASSERT(intvl->fEnd < start);
        SkDebugf("revising interval for %s { %d,%d } from [%d, %d] to [%d, %d]\n",
                 msg,
                 proxyID, underlyingID,
                 intvl->fStart, intvl->fEnd,
                 intvl->fStart, end);
        intvl->fEnd = end;
        return;
    }

    SkDebugf("adding new interval for { %d,%d }: [ %d, %d ]\n",
             proxyID, underlyingID, start, end);
    Interval* newIntvl = new Interval(proxy, start, end);

    fIntvlList.insertByIncreasingStart(newIntvl);
    fIntvlHash.add(newIntvl);
}

GrResourceAllocator::Interval* GrResourceAllocator::IntervalList::popStart() {
    Interval* temp = fHead;
    if (temp) {
        fHead = temp->fNext;
    }
    return temp;
}

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
    return proxy->foo(fResourceProvider);
}

// Remove any intervals that end before the current index. Return their GrSurfaces
// to the free pool.
void GrResourceAllocator::expire(unsigned int curIndex) {
    while (!fActiveIntvls.empty() && fActiveIntvls.peekHead()->fEnd < curIndex) {
        Interval* temp = fActiveIntvls.popStart();
        this->freeUpSurface(temp->fProxy->priv().peekSurface());
        delete temp;
    }
}

void GrResourceAllocator::assign() {
    fIntvlHash.reset(); // we don't need this anymore
    SkDEBUGCODE(fAssigned = true;)

    while (Interval* cur = fIntvlList.popStart()) {
        this->expire(cur->fStart);
        // TODO: add over budget handling here?
        sk_sp<GrSurface> surface = this->findSurfaceFor(cur->fProxy);
        if (surface) {
            cur->fProxy->assign(std::move(surface));
        }
        // TODO: handle resouce allocation failure upstack
        fActiveIntvls.insertByIncreasingEnd(cur);
    }
}
