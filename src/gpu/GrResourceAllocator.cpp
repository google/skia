/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceAllocator.h"

#include "GrGpuResourcePriv.h"
#include "GrOpList.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSurfacePriv.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureProxy.h"

void GrResourceAllocator::Interval::assign(sk_sp<GrSurface> s) {
    SkASSERT(!fAssignedSurface);
    fAssignedSurface = s;
    fProxy->priv().assign(std::move(s));
}


void GrResourceAllocator::markEndOfOpList(int opListIndex) {
    SkASSERT(!fAssigned);      // We shouldn't be adding any opLists after (or during) assignment

    SkASSERT(fEndOfOpListOpIndices.count() == opListIndex);
    if (!fEndOfOpListOpIndices.empty()) {
        SkASSERT(fEndOfOpListOpIndices.back() < this->curOp());
    }

    fEndOfOpListOpIndices.push_back(this->curOp()); // This is the first op index of the next opList
}

GrResourceAllocator::~GrResourceAllocator() {
    SkASSERT(fIntvlList.empty());
    SkASSERT(fActiveIntvls.empty());
    SkASSERT(!fIntvlHash.count());
}

void GrResourceAllocator::addInterval(GrSurfaceProxy* proxy, unsigned int start, unsigned int end
                                      SkDEBUGCODE(, bool isDirectDstRead)) {
    SkASSERT(start <= end);
    SkASSERT(!fAssigned);      // We shouldn't be adding any intervals after (or during) assignment

    if (Interval* intvl = fIntvlHash.find(proxy->uniqueID().asUInt())) {
        // Revise the interval for an existing use
#ifdef SK_DEBUG
        if (0 == start && 0 == end) {
            // This interval is for the initial upload to a deferred proxy. Due to the vagaries
            // of how deferred proxies are collected they can appear as uploads multiple times in a
            // single opLists' list and as uploads in several opLists.
            SkASSERT(0 == intvl->start());
        } else if (isDirectDstRead) {
            // Direct reads from the render target itself should occur w/in the existing interval
            SkASSERT(intvl->start() <= start && intvl->end() >= end);
        } else {
            SkASSERT(intvl->end() <= start && intvl->end() <= end);
        }
#endif
        intvl->extendEnd(end);
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

    if (!fResourceProvider->explicitlyAllocateGPUResources()) {
        // FIXME: remove this once we can do the lazy instantiation from assign instead.
        if (GrSurfaceProxy::LazyState::kNot != proxy->lazyInstantiationState()) {
            proxy->priv().doLazyInstantiation(fResourceProvider);
        }
    }
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


 GrResourceAllocator::Interval* GrResourceAllocator::IntervalList::detachAll() {
    Interval* tmp = fHead;
    fHead = nullptr;
    return tmp;
}

// 'surface' can be reused. Add it back to the free pool.
void GrResourceAllocator::freeUpSurface(sk_sp<GrSurface> surface) {
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
    fFreePool.insert(key, surface.release());
}

// First try to reuse one of the recently allocated/used GrSurfaces in the free pool.
// If we can't find a useable one, create a new one.
sk_sp<GrSurface> GrResourceAllocator::findSurfaceFor(const GrSurfaceProxy* proxy,
                                                     bool needsStencil) {
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

        GrSurfaceProxyPriv::AttachStencilIfNeeded(fResourceProvider, surface.get(), needsStencil);
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

        if (temp->wasAssignedSurface()) {
            this->freeUpSurface(temp->detachSurface());
        }

        // Add temp to the free interval list so it can be reused
        temp->setNext(fFreeIntervalList);
        fFreeIntervalList = temp;
    }
}

bool GrResourceAllocator::assign(int* startIndex, int* stopIndex, AssignError* outError) {
    SkASSERT(outError);
    *outError = AssignError::kNoError;

    fIntvlHash.reset(); // we don't need the interval hash anymore
    if (fIntvlList.empty()) {
        return false;          // nothing to render
    }

    *startIndex = fCurOpListIndex;
    *stopIndex = fEndOfOpListOpIndices.count();

    if (!fResourceProvider->explicitlyAllocateGPUResources()) {
        fIntvlList.detachAll(); // arena allocator will clean these up for us
        return true;
    }

    SkDEBUGCODE(fAssigned = true;)

    while (Interval* cur = fIntvlList.popHead()) {
        if (fEndOfOpListOpIndices[fCurOpListIndex] < cur->start()) {
            fCurOpListIndex++;
        }

        this->expire(cur->start());

        bool needsStencil = cur->proxy()->asRenderTargetProxy()
                                            ? cur->proxy()->asRenderTargetProxy()->needsStencil()
                                            : false;

        if (cur->proxy()->priv().isInstantiated()) {
            GrSurfaceProxyPriv::AttachStencilIfNeeded(fResourceProvider,
                                                      cur->proxy()->priv().peekSurface(),
                                                      needsStencil);

            fActiveIntvls.insertByIncreasingEnd(cur);

            if (fResourceProvider->overBudget()) {
                // Only force intermediate draws on opList boundaries
                if (!fIntvlList.empty() &&
                    fEndOfOpListOpIndices[fCurOpListIndex] < fIntvlList.peekHead()->start()) {
                    *stopIndex = fCurOpListIndex+1;
                    return true;
                }
            }

            continue;
        }

        if (GrSurfaceProxy::LazyState::kNot != cur->proxy()->lazyInstantiationState()) {
            if (!cur->proxy()->priv().doLazyInstantiation(fResourceProvider)) {
                *outError = AssignError::kFailedProxyInstantiation;
            }
        } else if (sk_sp<GrSurface> surface = this->findSurfaceFor(cur->proxy(), needsStencil)) {
            // TODO: make getUniqueKey virtual on GrSurfaceProxy
            GrTextureProxy* tex = cur->proxy()->asTextureProxy();
            if (tex && tex->getUniqueKey().isValid()) {
                fResourceProvider->assignUniqueKeyToResource(tex->getUniqueKey(), surface.get());
                SkASSERT(surface->getUniqueKey() == tex->getUniqueKey());
            }

            cur->assign(std::move(surface));
        } else {
            SkASSERT(!cur->proxy()->priv().isInstantiated());
            *outError = AssignError::kFailedProxyInstantiation;
        }

        fActiveIntvls.insertByIncreasingEnd(cur);

        if (fResourceProvider->overBudget()) {
            // Only force intermediate draws on opList boundaries
            if (!fIntvlList.empty() &&
                fEndOfOpListOpIndices[fCurOpListIndex] < fIntvlList.peekHead()->start()) {
                *stopIndex = fCurOpListIndex+1;
                return true;
            }
        }
    }

    // expire all the remaining intervals to drain the active interval list
    this->expire(std::numeric_limits<unsigned int>::max());
    return true;
}
