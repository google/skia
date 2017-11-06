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

uint32_t GrResourceAllocator::Interval::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

void GrResourceAllocator::addInterval(GrSurfaceProxy* proxy,
                                      unsigned int start1, unsigned int end, int tabs) {
    SkASSERT(start1 <= end);
    SkASSERT(!fAssigned);      // We shouldn't be adding any intervals after (or during) assignment

    unsigned int proxyID = proxy->uniqueID().asUInt();
    int underlyingID = proxy->priv().isInstantiated() ? proxy->underlyingUniqueID().asUInt() : -1;

    if (Interval* intvl = fIntvlHash.find(proxy->uniqueID().asUInt())) {
        // Revise the interval for an existing use
        SkASSERT(intvl->end() <= start1);
        if (intvl->end() < end) {  // How can fEnd be >= end ??
            emit_tabs(tabs);
            SkDebugf("revising interval { rtpID %d, rtID %d } from [op# %d, op# %d] to [op# %d, op# %d]\n",
                     proxyID, underlyingID,
                     intvl->start(), intvl->end(),
                     intvl->start(), end);
            intvl->extendEnd(end);
        }
        return;
    }

    emit_tabs(tabs);
    SkDebugf("adding new interval for { rtpID %d, rtID %d }: [ op# %d, op# %d ]\n",
             proxyID, underlyingID, start1, end);

    Interval* newIntvl;
    if (fFreeIntervalList) {
        newIntvl = fFreeIntervalList;
        fFreeIntervalList = newIntvl->next();
        newIntvl->resetTo(proxy, start1, end);
    } else {
        newIntvl = fIntervalAllocator.make<Interval>(proxy, start1, end);
    }

    fIntvlList.insertByIncreasingStart(newIntvl);
    fIntvlHash.add(newIntvl);
}

GrResourceAllocator::Interval* GrResourceAllocator::IntervalList::popHead() {
    Interval* temp = fHead;
    if (temp) {
        fHead = temp->next();
    }
    return temp;
}

// TODO: fuse this with insertByIncreasingEnd
void GrResourceAllocator::IntervalList::insertByIncreasingStart(Interval* intvl) {
    if (!fHead) {
        intvl->setNext(nullptr);
        fHead = intvl;
    } else if (intvl->start() <= fHead->start()) {
        intvl->setNext(fHead);
        fHead = intvl;
    } else {
        Interval* prev = fHead;
        Interval* next = prev->next();
        for (; next && intvl->start() > next->start(); prev = next, next = next->next()) {
        }
        intvl->setNext(next);
        prev->setNext(intvl);
    }
}

// TODO: fuse this with insertByIncreasingStart
void GrResourceAllocator::IntervalList::insertByIncreasingEnd(Interval* intvl) {
    if (!fHead) {
        intvl->setNext(nullptr);
        fHead = intvl;
    } else if (intvl->end() <= fHead->end()) {
        intvl->setNext(fHead);
        fHead = intvl;
    } else {
        Interval* prev = fHead;
        Interval* next = prev->next();
        for (; next && intvl->end() > next->end(); prev = next, next = next->next()) {
        }
        intvl->setNext(next);
        prev->setNext(intvl);
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

class Baz {
public:
    Baz(bool rejectPendingIO) : fRejectPendingIO(rejectPendingIO) { }

    bool operator()(const GrSurface* s) const {
        return !fRejectPendingIO || !s->internalHasPendingIO();
    }

private:
    bool fRejectPendingIO;
};

// First try to reuse one of the recently allocated/used GrSurfaces in the free pool.
// If we can't find a useable one, create a new one.
// TODO: handle being overbudget
sk_sp<GrSurface> GrResourceAllocator::findSurfaceFor(const GrSurfaceProxy* proxy) {
    // First look in the free pool
    GrScratchKey key;

    proxy->priv().computeScratchKey(&key);

    GrSurface* surface = fFreePool.findAndRemove(key, Baz(proxy->priv().requiresNoPendingIO()));
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
    while (!fActiveIntvls.empty() && fActiveIntvls.peekHead()->end() < curIndex) {
        Interval* temp = fActiveIntvls.popHead();
        this->freeUpSurface(temp->proxy()->priv().peekSurface());

        // Add temp to the free interval list so it can be reused
        temp->setNext(fFreeIntervalList);
        fFreeIntervalList = temp;
    }
}

void GrResourceAllocator::assign() {
    fIntvlHash.reset(); // we don't need this anymore
    SkDEBUGCODE(fAssigned = true;)

    this->dumpBeforeAssign();
    while (Interval* cur = fIntvlList.popHead()) {
        this->expire(cur->start());

        if (cur->proxy()->priv().isInstantiated()) {
            fActiveIntvls.insertByIncreasingEnd(cur);
            continue;
        }

        // TODO: add over budget handling here?
        sk_sp<GrSurface> surface = this->findSurfaceFor(cur->proxy());
        if (surface) {
            // TODO: make getUniqueKey virtual on GrSurfaceProxy
            GrTextureProxy* tex = cur->proxy()->asTextureProxy();
            if (tex && tex->getUniqueKey().isValid()) {
                SkDebugf("pushing proxy %d's unique key onto surface %d\n",
                         tex->uniqueID().asUInt(), surface->uniqueID().asUInt());
                fResourceProvider->assignUniqueKeyToResource(tex->getUniqueKey(), surface.get());
                SkASSERT(surface->getUniqueKey() == tex->getUniqueKey());
            }

            cur->proxy()->priv().assign1(std::move(surface));
            cur->proxy()->fIsOkayToBeInstantiated = true;  // This dude assigned it so it should be okay
        }
        // TODO: handle resouce allocation failure upstack
        fActiveIntvls.insertByIncreasingEnd(cur);
    }
    this->dumpAfterAssign();
}

#ifdef SK_DEBUG
void GrResourceAllocator::dumpBeforeAssign() {
    unsigned int min = fNumOps+1;
    unsigned int max = 0;
    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        SkDebugf("{ %3d,%3d }: [%2d, %2d] - pRef:%d rRef:%d R:%d W:%d\n",
                 cur->proxy()->uniqueID().asUInt(),
                 cur->proxy()->priv().isInstantiated() ? cur->proxy()->underlyingUniqueID().asUInt() : -1,
                 cur->start(), cur->end(),
                 cur->proxy()->getProxyRefCnt_TestOnly(),
                 cur->proxy()->getBackingRefCnt_TestOnly(),
                 cur->proxy()->getPendingReadCnt_TestOnly(),
                 cur->proxy()->getPendingWriteCnt_TestOnly());
        if (min > cur->start()) {
            min = cur->start();
        }
        if (max < cur->end()) {
            max = cur->end();
        }
    }

    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        SkDebugf("{ %3d,%3d }: ",
                 cur->proxy()->uniqueID().asUInt(),
                 cur->proxy()->priv().isInstantiated() ? cur->proxy()->underlyingUniqueID().asUInt() : -1);
        for (unsigned int i = min; i <= max; ++i) {
            if (i >= cur->start() && i <= cur->end()) {
                SkDebugf("x");
            } else {
                SkDebugf(" ");
            }
        }
        SkDebugf("\n");
    }
}

void GrResourceAllocator::dumpAfterAssign() {
#if 0
    unsigned int min = fNumOps+1;
    unsigned int max = 0;
    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        if (min > cur->start()) {
            min = cur->start();
        }
        if (max < cur->end()) {
            max = cur->end();
        }
    }

    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        SkDebugf("{ %3d,%3d }: ",
                 cur->proxy()->uniqueID().asUInt(),
                 cur->proxy()->priv().isInstantiated() ? cur->proxy()->underlyingUniqueID().asUInt() : -1);
        for (unsigned int i = min; i <= max; ++i) {
            if (i >= cur->start() && i <= cur->end()) {
                SkDebugf("|%3d");
            } else {
                SkDebugf("|   ");
            }
        }
        SkDebugf("|\n");
    }
#endif
}
#endif

