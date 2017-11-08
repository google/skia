/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceAllocator.h"

#include "GrGpuResourcePriv.h"
#include "GrResourceProvider.h"
#include "GrSurfacePriv.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureProxy.h"

void GrResourceAllocator::addInterval(GrSurfaceProxy* proxy,
                                      unsigned int start, unsigned int end) {
    SkASSERT(start <= end);
    SkASSERT(!fAssigned);      // We shouldn't be adding any intervals after (or during) assignment

    if (Interval* intvl = fIntvlHash.find(proxy->uniqueID().asUInt())) {
        // Revise the interval for an existing use
        // TODO: this assert is failing on the copy_on_write_retain GM!
        //SkASSERT(intvl->end() <= start);
        if (intvl->end() < end) {
            intvl->extendEnd(end);
        }
        return;
    }

    Interval* newIntvl;
    if (fFreeIntervalList) {
        newIntvl = fFreeIntervalList;
        fFreeIntervalList = newIntvl->next();
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

    // TODO: fix this insertion so we get a more LRU-ish behavior
    fFreePool.insert(key, SkRef(surface));
}

// First try to reuse one of the recently allocated/used GrSurfaces in the free pool.
// If we can't find a useable one, create a new one.
// TODO: handle being overbudget
sk_sp<GrSurface> GrResourceAllocator::findSurfaceFor(const GrSurfaceProxy* proxy) {
    // First look in the free pool
    GrScratchKey key;

    proxy->priv().computeScratchKey(&key);

    auto filter = [&] (const GrSurface* s) {
        return !proxy->priv().requiresNoPendingIO() || !s->surfacePriv().hasPendingIO();
    };
    sk_sp<GrSurface> surface(fFreePool.findAndRemove(key, filter));
    if (surface) {
        if (SkBudgeted::kYes == proxy->isBudgeted() &&
            SkBudgeted::kNo == surface->resourcePriv().isBudgeted()) {
            // This gets the job done but isn't quite correct. It would be better to try to
            // match budgeted proxies w/ budgeted surface and unbudgeted w/ unbudgeted.
            surface->resourcePriv().makeBudgeted();
        }

        return surface;
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
                fResourceProvider->assignUniqueKeyToResource(tex->getUniqueKey(), surface.get());
                SkASSERT(surface->getUniqueKey() == tex->getUniqueKey());
            }

            cur->proxy()->priv().assign(std::move(surface));
        }
        // TODO: handle resouce allocation failure upstack
        fActiveIntvls.insertByIncreasingEnd(cur);
    }
}
