/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrResourceAllocator.h"

#include "src/gpu/GrDeinstantiateProxyTracker.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrOpsTask.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxy.h"

#if GR_TRACK_INTERVAL_CREATION
    #include <atomic>

    uint32_t GrResourceAllocator::Interval::CreateUniqueID() {
        static std::atomic<uint32_t> nextID{1};
        uint32_t id;
        do {
            id = nextID++;
        } while (id == SK_InvalidUniqueID);
        return id;
    }
#endif

void GrResourceAllocator::Interval::assign(sk_sp<GrSurface> s) {
    SkASSERT(!fAssignedSurface);
    fAssignedSurface = s;
    fProxy->priv().assign(std::move(s));
}

void GrResourceAllocator::determineRecyclability() {
    for (Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        if (cur->proxy()->canSkipResourceAllocator()) {
            // These types of proxies can slip in here if they require a stencil buffer
            continue;
        }

        if (cur->uses() >= cur->proxy()->priv().getProxyRefCnt()) {
            // All the refs on the proxy are known to the resource allocator thus no one
            // should be holding onto it outside of Ganesh.
            SkASSERT(cur->uses() == cur->proxy()->priv().getProxyRefCnt());
            cur->markAsRecyclable();
        }
    }
}

void GrResourceAllocator::markEndOfOpsTask(int opsTaskIndex) {
    SkASSERT(!fAssigned);      // We shouldn't be adding any opsTasks after (or during) assignment

    SkASSERT(fEndOfOpsTaskOpIndices.count() == opsTaskIndex);
    if (!fEndOfOpsTaskOpIndices.empty()) {
        SkASSERT(fEndOfOpsTaskOpIndices.back() < this->curOp());
    }

    // This is the first op index of the next opsTask
    fEndOfOpsTaskOpIndices.push_back(this->curOp());
    SkASSERT(fEndOfOpsTaskOpIndices.count() <= fNumOpsTasks);
}

GrResourceAllocator::~GrResourceAllocator() {
    SkASSERT(fIntvlList.empty());
    SkASSERT(fActiveIntvls.empty());
    SkASSERT(!fIntvlHash.count());
}

void GrResourceAllocator::addInterval(GrSurfaceProxy* proxy, unsigned int start, unsigned int end,
                                      ActualUse actualUse
                                      SkDEBUGCODE(, bool isDirectDstRead)) {

    if (proxy->canSkipResourceAllocator()) {
        // If the proxy is still not instantiated at this point but will need stencil, it will
        // attach its own stencil buffer upon onFlush instantiation.
        if (proxy->isInstantiated()) {
            int minStencilSampleCount = (proxy->asRenderTargetProxy())
                    ? proxy->asRenderTargetProxy()->numStencilSamples()
                    : 0;
            if (minStencilSampleCount) {
                if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(
                        fResourceProvider, proxy->peekSurface(), minStencilSampleCount)) {
                    SkDebugf("WARNING: failed to attach stencil buffer. "
                             "Rendering may be incorrect.\n");
                }
            }
        }
        return;
    }

    SkASSERT(!proxy->priv().ignoredByResourceAllocator());

    SkASSERT(start <= end);
    SkASSERT(!fAssigned);      // We shouldn't be adding any intervals after (or during) assignment

    // If a proxy is read only it must refer to a texture with specific content that cannot be
    // recycled. We don't need to assign a texture to it and no other proxy can be instantiated
    // with the same texture.
    if (proxy->readOnly()) {
        // Since we aren't going to add an interval we won't revisit this proxy in assign(). So it
        // must already be instantiated or it must be a lazy proxy that we will instantiate below.
        SkASSERT(proxy->isInstantiated() ||
                 GrSurfaceProxy::LazyState::kNot != proxy->lazyInstantiationState());
    } else {
        if (Interval* intvl = fIntvlHash.find(proxy->uniqueID().asUInt())) {
            // Revise the interval for an existing use
#ifdef SK_DEBUG
            if (0 == start && 0 == end) {
                // This interval is for the initial upload to a deferred proxy. Due to the vagaries
                // of how deferred proxies are collected they can appear as uploads multiple times
                // in a single opsTasks' list and as uploads in several opsTasks.
                SkASSERT(0 == intvl->start());
            } else if (isDirectDstRead) {
                // Direct reads from the render target itself should occur w/in the existing
                // interval
                SkASSERT(intvl->start() <= start && intvl->end() >= end);
            } else {
                SkASSERT(intvl->end() <= start && intvl->end() <= end);
            }
#endif
            if (ActualUse::kYes == actualUse) {
                intvl->addUse();
            }
            intvl->extendEnd(end);
            return;
        }
        Interval* newIntvl;
        if (fFreeIntervalList) {
            newIntvl = fFreeIntervalList;
            fFreeIntervalList = newIntvl->next();
            newIntvl->setNext(nullptr);
            newIntvl->resetTo(proxy, start, end);
        } else {
            newIntvl = fIntervalAllocator.make<Interval>(proxy, start, end);
        }

        if (ActualUse::kYes == actualUse) {
            newIntvl->addUse();
        }
        fIntvlList.insertByIncreasingStart(newIntvl);
        fIntvlHash.add(newIntvl);
    }

    // Because readOnly proxies do not get a usage interval we must instantiate them here (since it
    // won't occur in GrResourceAllocator::assign)
    if (proxy->readOnly()) {
        // FIXME: remove this once we can do the lazy instantiation from assign instead.
        if (GrSurfaceProxy::LazyState::kNot != proxy->lazyInstantiationState()) {
            if (proxy->priv().doLazyInstantiation(fResourceProvider)) {
                if (proxy->priv().lazyInstantiationType() ==
                    GrSurfaceProxy::LazyInstantiationType::kDeinstantiate) {
                    fDeinstantiateTracker->addProxy(proxy);
                }
            } else {
                fLazyInstantiationError = true;
            }
        }
    }
}

GrResourceAllocator::Interval* GrResourceAllocator::IntervalList::popHead() {
    SkDEBUGCODE(this->validate());

    Interval* temp = fHead;
    if (temp) {
        fHead = temp->next();
        if (!fHead) {
            fTail = nullptr;
        }
        temp->setNext(nullptr);
    }

    SkDEBUGCODE(this->validate());
    return temp;
}

// TODO: fuse this with insertByIncreasingEnd
void GrResourceAllocator::IntervalList::insertByIncreasingStart(Interval* intvl) {
    SkDEBUGCODE(this->validate());
    SkASSERT(!intvl->next());

    if (!fHead) {
        // 14%
        fHead = fTail = intvl;
    } else if (intvl->start() <= fHead->start()) {
        // 3%
        intvl->setNext(fHead);
        fHead = intvl;
    } else if (fTail->start() <= intvl->start()) {
        // 83%
        fTail->setNext(intvl);
        fTail = intvl;
    } else {
        // almost never
        Interval* prev = fHead;
        Interval* next = prev->next();
        for (; intvl->start() > next->start(); prev = next, next = next->next()) {
        }

        SkASSERT(next);
        intvl->setNext(next);
        prev->setNext(intvl);
    }

    SkDEBUGCODE(this->validate());
}

// TODO: fuse this with insertByIncreasingStart
void GrResourceAllocator::IntervalList::insertByIncreasingEnd(Interval* intvl) {
    SkDEBUGCODE(this->validate());
    SkASSERT(!intvl->next());

    if (!fHead) {
        // 14%
        fHead = fTail = intvl;
    } else if (intvl->end() <= fHead->end()) {
        // 64%
        intvl->setNext(fHead);
        fHead = intvl;
    } else if (fTail->end() <= intvl->end()) {
        // 3%
        fTail->setNext(intvl);
        fTail = intvl;
    } else {
        // 19% but 81% of those land right after the list's head
        Interval* prev = fHead;
        Interval* next = prev->next();
        for (; intvl->end() > next->end(); prev = next, next = next->next()) {
        }

        SkASSERT(next);
        intvl->setNext(next);
        prev->setNext(intvl);
    }

    SkDEBUGCODE(this->validate());
}

#ifdef SK_DEBUG
void GrResourceAllocator::IntervalList::validate() const {
    SkASSERT(SkToBool(fHead) == SkToBool(fTail));

    Interval* prev = nullptr;
    for (Interval* cur = fHead; cur; prev = cur, cur = cur->next()) {
    }

    SkASSERT(fTail == prev);
}
#endif

 GrResourceAllocator::Interval* GrResourceAllocator::IntervalList::detachAll() {
    Interval* tmp = fHead;
    fHead = nullptr;
    fTail = nullptr;
    return tmp;
}

// 'surface' can be reused. Add it back to the free pool.
void GrResourceAllocator::recycleSurface(sk_sp<GrSurface> surface) {
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

#if GR_ALLOCATION_SPEW
    SkDebugf("putting surface %d back into pool\n", surface->uniqueID().asUInt());
#endif
    // TODO: fix this insertion so we get a more LRU-ish behavior
    fFreePool.insert(key, surface.release());
}

// First try to reuse one of the recently allocated/used GrSurfaces in the free pool.
// If we can't find a useable one, create a new one.
sk_sp<GrSurface> GrResourceAllocator::findSurfaceFor(const GrSurfaceProxy* proxy,
                                                     int minStencilSampleCount) {

    if (proxy->asTextureProxy() && proxy->asTextureProxy()->getUniqueKey().isValid()) {
        // First try to reattach to a cached version if the proxy is uniquely keyed
        sk_sp<GrSurface> surface = fResourceProvider->findByUniqueKey<GrSurface>(
                                                        proxy->asTextureProxy()->getUniqueKey());
        if (surface) {
            if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(fResourceProvider, surface.get(),
                                                           minStencilSampleCount)) {
                return nullptr;
            }

            return surface;
        }
    }

    // First look in the free pool
    GrScratchKey key;

    proxy->priv().computeScratchKey(&key);

    auto filter = [] (const GrSurface* s) {
        return true;
    };
    sk_sp<GrSurface> surface(fFreePool.findAndRemove(key, filter));
    if (surface) {
        if (SkBudgeted::kYes == proxy->isBudgeted() &&
            GrBudgetedType::kBudgeted != surface->resourcePriv().budgetedType()) {
            // This gets the job done but isn't quite correct. It would be better to try to
            // match budgeted proxies w/ budgeted surfaces and unbudgeted w/ unbudgeted.
            surface->resourcePriv().makeBudgeted();
        }

        if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(fResourceProvider, surface.get(),
                                                       minStencilSampleCount)) {
            return nullptr;
        }
        SkASSERT(!surface->getUniqueKey().isValid());
        return surface;
    }

    // Failing that, try to grab a new one from the resource cache
    return proxy->priv().createSurface(fResourceProvider);
}

// Remove any intervals that end before the current index. Return their GrSurfaces
// to the free pool if possible.
void GrResourceAllocator::expire(unsigned int curIndex) {
    while (!fActiveIntvls.empty() && fActiveIntvls.peekHead()->end() < curIndex) {
        Interval* temp = fActiveIntvls.popHead();
        SkASSERT(!temp->next());

        if (temp->wasAssignedSurface()) {
            sk_sp<GrSurface> surface = temp->detachSurface();

            if (temp->isRecyclable()) {
                this->recycleSurface(std::move(surface));
            }
        }

        // Add temp to the free interval list so it can be reused
        SkASSERT(!temp->wasAssignedSurface()); // it had better not have a ref on a surface
        temp->setNext(fFreeIntervalList);
        fFreeIntervalList = temp;
    }
}

bool GrResourceAllocator::onOpsTaskBoundary() const {
    if (fIntvlList.empty()) {
        SkASSERT(fCurOpsTaskIndex+1 <= fNumOpsTasks);
        // Although technically on an opsTask boundary there is no need to force an
        // intermediate flush here
        return false;
    }

    const Interval* tmp = fIntvlList.peekHead();
    return fEndOfOpsTaskOpIndices[fCurOpsTaskIndex] <= tmp->start();
}

void GrResourceAllocator::forceIntermediateFlush(int* stopIndex) {
    *stopIndex = fCurOpsTaskIndex+1;

    // This is interrupting the allocation of resources for this flush. We need to
    // proactively clear the active interval list of any intervals that aren't
    // guaranteed to survive the partial flush lest they become zombies (i.e.,
    // holding a deleted surface proxy).
    const Interval* tmp = fIntvlList.peekHead();
    SkASSERT(fEndOfOpsTaskOpIndices[fCurOpsTaskIndex] <= tmp->start());

    fCurOpsTaskIndex++;
    SkASSERT(fCurOpsTaskIndex < fNumOpsTasks);

    this->expire(tmp->start());
}

bool GrResourceAllocator::assign(int* startIndex, int* stopIndex, AssignError* outError) {
    SkASSERT(outError);
    *outError = fLazyInstantiationError ? AssignError::kFailedProxyInstantiation
                                        : AssignError::kNoError;

    SkASSERT(fNumOpsTasks == fEndOfOpsTaskOpIndices.count());

    fIntvlHash.reset(); // we don't need the interval hash anymore

    if (fCurOpsTaskIndex >= fEndOfOpsTaskOpIndices.count()) {
        return false; // nothing to render
    }

    *startIndex = fCurOpsTaskIndex;
    *stopIndex = fEndOfOpsTaskOpIndices.count();

    if (fIntvlList.empty()) {
        fCurOpsTaskIndex = fEndOfOpsTaskOpIndices.count();
        return true;          // no resources to assign
    }

#if GR_ALLOCATION_SPEW
    SkDebugf("assigning opsTasks %d through %d out of %d numOpsTasks\n",
             *startIndex, *stopIndex, fNumOpsTasks);
    SkDebugf("EndOfOpsTaskIndices: ");
    for (int i = 0; i < fEndOfOpsTaskOpIndices.count(); ++i) {
        SkDebugf("%d ", fEndOfOpsTaskOpIndices[i]);
    }
    SkDebugf("\n");
#endif

    SkDEBUGCODE(fAssigned = true;)

#if GR_ALLOCATION_SPEW
    this->dumpIntervals();
#endif
    while (Interval* cur = fIntvlList.popHead()) {
        if (fEndOfOpsTaskOpIndices[fCurOpsTaskIndex] <= cur->start()) {
            fCurOpsTaskIndex++;
            SkASSERT(fCurOpsTaskIndex < fNumOpsTasks);
        }

        this->expire(cur->start());

        int minStencilSampleCount = (cur->proxy()->asRenderTargetProxy())
                ? cur->proxy()->asRenderTargetProxy()->numStencilSamples()
                : 0;

        if (cur->proxy()->isInstantiated()) {
            if (!GrSurfaceProxyPriv::AttachStencilIfNeeded(
                        fResourceProvider, cur->proxy()->peekSurface(), minStencilSampleCount)) {
                *outError = AssignError::kFailedProxyInstantiation;
            }

            fActiveIntvls.insertByIncreasingEnd(cur);

            if (fResourceProvider->overBudget()) {
                // Only force intermediate draws on opsTask boundaries
                if (this->onOpsTaskBoundary()) {
                    this->forceIntermediateFlush(stopIndex);
                    return true;
                }
            }

            continue;
        }

        if (GrSurfaceProxy::LazyState::kNot != cur->proxy()->lazyInstantiationState()) {
            if (!cur->proxy()->priv().doLazyInstantiation(fResourceProvider)) {
                *outError = AssignError::kFailedProxyInstantiation;
            } else {
                if (GrSurfaceProxy::LazyInstantiationType::kDeinstantiate ==
                    cur->proxy()->priv().lazyInstantiationType()) {
                    fDeinstantiateTracker->addProxy(cur->proxy());
                }
            }
        } else if (sk_sp<GrSurface> surface = this->findSurfaceFor(
                cur->proxy(), minStencilSampleCount)) {
            // TODO: make getUniqueKey virtual on GrSurfaceProxy
            GrTextureProxy* texProxy = cur->proxy()->asTextureProxy();

            if (texProxy && texProxy->getUniqueKey().isValid()) {
                if (!surface->getUniqueKey().isValid()) {
                    fResourceProvider->assignUniqueKeyToResource(texProxy->getUniqueKey(),
                                                                 surface.get());
                }
                SkASSERT(surface->getUniqueKey() == texProxy->getUniqueKey());
            }

#if GR_ALLOCATION_SPEW
            SkDebugf("Assigning %d to %d\n",
                 surface->uniqueID().asUInt(),
                 cur->proxy()->uniqueID().asUInt());
#endif

            cur->assign(std::move(surface));
        } else {
            SkASSERT(!cur->proxy()->isInstantiated());
            *outError = AssignError::kFailedProxyInstantiation;
        }

        fActiveIntvls.insertByIncreasingEnd(cur);

        if (fResourceProvider->overBudget()) {
            // Only force intermediate draws on opsTask boundaries
            if (this->onOpsTaskBoundary()) {
                this->forceIntermediateFlush(stopIndex);
                return true;
            }
        }
    }

    // expire all the remaining intervals to drain the active interval list
    this->expire(std::numeric_limits<unsigned int>::max());
    return true;
}

#if GR_ALLOCATION_SPEW
void GrResourceAllocator::dumpIntervals() {
    // Print all the intervals while computing their range
    SkDebugf("------------------------------------------------------------\n");
    unsigned int min = std::numeric_limits<unsigned int>::max();
    unsigned int max = 0;
    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        SkDebugf("{ %3d,%3d }: [%2d, %2d] - proxyRefs:%d surfaceRefs:%d\n",
                 cur->proxy()->uniqueID().asUInt(),
                 cur->proxy()->isInstantiated() ? cur->proxy()->underlyingUniqueID().asUInt() : -1,
                 cur->start(),
                 cur->end(),
                 cur->proxy()->priv().getProxyRefCnt(),
                 cur->proxy()->testingOnly_getBackingRefCnt());
        min = SkTMin(min, cur->start());
        max = SkTMax(max, cur->end());
    }

    // Draw a graph of the useage intervals
    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        SkDebugf("{ %3d,%3d }: ",
                 cur->proxy()->uniqueID().asUInt(),
                 cur->proxy()->isInstantiated() ? cur->proxy()->underlyingUniqueID().asUInt() : -1);
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
#endif
