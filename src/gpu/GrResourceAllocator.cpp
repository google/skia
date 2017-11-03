/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceAllocator.h"

#include "GrGpuResourcePriv.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureProxy.h"

static void emit_tabs(int num) {
    for (int i = 0; i < num; ++i) {
        SkDebugf("    ");
    }
}

void GrResourceAllocator::addInterval(GrSurfaceProxy* proxy,
                                      unsigned int start, unsigned int end, int tabs) {
    SkASSERT(start <= end);
    SkASSERT(!fAssigned);      // We shouldn't be adding any intervals after (or during) assignment

    unsigned int proxyID = proxy->uniqueID().asUInt();
    int underlyingID = proxy->priv().isInstantiated() ? proxy->underlyingUniqueID().asUInt() : -1;

    if (Interval* intvl = fIntvlHash.find(proxy->uniqueID().asUInt())) {
        // Revise the interval for an existing use
        SkASSERT(intvl->fEnd <= start);
        if (intvl->fEnd < end) {
            emit_tabs(tabs);
            SkDebugf("revising interval { rtpID %d, rtID %d } from [op# %d, op# %d] to [op# %d, op# %d]\n",
                     proxyID, underlyingID,
                     intvl->fStart, intvl->fEnd,
                     intvl->fStart, end);
            intvl->fEnd = end;
        }
        return;
    }

    emit_tabs(tabs);
    SkDebugf("adding new interval for { rtpID %d, rtID %d }: [ op# %d, op# %d ]\n",
             proxyID, underlyingID, start, end);
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
    const GrScratchKey &key = surface->resourcePriv().getScratchKey();

    if (!key.isValid()) {
        return; // can't do it w/o a valid scratch key
    }

    if (surface->getUniqueKey().isValid()) {
        // If the surface has a unique key we throw it back into the resource cache.
        // If things get really tight 'findSurfaceFor' may pull it back out but there is
        // no need to have it in tight rotation.
        return;
    }

    SkDebugf("putting surface %d back into pool\n", surface->uniqueID().asUInt());
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
        if (SkBudgeted::kYes == proxy->isBudgeted() &&
            SkBudgeted::kNo == surface->resourcePriv().isBudgeted()) {
            // This gets the job done but isn't quite correct. It would be better to try to
            // match budgeted proxies w/ budgeted surface and unbudgeted w/ unbudgeted.
            surface->resourcePriv().makeBudgeted();
        }

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
        delete temp;
    }
}

void GrResourceAllocator::assign() {
    fIntvlHash.reset(); // we don't need this anymore
    SkDEBUGCODE(fAssigned = true;)

    this->dump();
    while (Interval* cur = fIntvlList.popHead()) {
        this->expire(cur->fStart);

        if (cur->fProxy->priv().isInstantiated()) {
            fActiveIntvls.insertByIncreasingEnd(cur);
            continue;
        }

        // TODO: add over budget handling here?
        sk_sp<GrSurface> surface = this->findSurfaceFor(cur->fProxy);
        if (surface) {
            // TODO: make getUniqueKey virtual on GrSurfaceProxy
            GrTextureProxy* tex = cur->fProxy->asTextureProxy();
            if (tex && tex->getUniqueKey().isValid()) {
                SkDebugf("pushing proxy %d's unique key onto surface %d\n",
                         tex->uniqueID().asUInt(), surface->uniqueID().asUInt());
                fResourceProvider->assignUniqueKeyToResource(tex->getUniqueKey(), surface.get());
                SkASSERT(surface->getUniqueKey() == tex->getUniqueKey());
            }

            cur->fProxy->priv().assign1(std::move(surface));
            cur->fProxy->fIsOkayToBeInstantiated = true;  // This dude assigned it so it should be okay
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
        SkDebugf("{ %3d,%3d }: [%2d, %2d] - pRef:%d rRef:%d R:%d W:%d\n",
                 cur->fProxy->uniqueID().asUInt(),
                 cur->fProxy->priv().isInstantiated() ? cur->fProxy->underlyingUniqueID().asUInt() : -1,
                 cur->fStart, cur->fEnd,
                 cur->fProxy->getProxyRefCnt_TestOnly(),
                 cur->fProxy->getBackingRefCnt_TestOnly(),
                 cur->fProxy->getPendingReadCnt_TestOnly(),
                 cur->fProxy->getPendingWriteCnt_TestOnly());
        if (min > cur->fStart) {
            min = cur->fStart;
        }
        if (max < cur->fEnd) {
            max = cur->fEnd;
        }
    }

    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->fNext) {
        SkDebugf("{ %3d,%3d }: ",
                 cur->fProxy->uniqueID().asUInt(),
                 cur->fProxy->priv().isInstantiated() ? cur->fProxy->underlyingUniqueID().asUInt() : -1);
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

