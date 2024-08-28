/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrResourceAllocator.h"

#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrTexture.h"  // IWYU pragma: keep

#include <cstddef>
#include <limits>
#include <utility>

#ifdef SK_DEBUG
#include <atomic>

uint32_t GrResourceAllocator::Interval::CreateUniqueID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return id;
}

uint32_t GrResourceAllocator::Register::CreateUniqueID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return id;
}
#endif

GrResourceAllocator::~GrResourceAllocator() {
    SkASSERT(fFailedInstantiation || fIntvlList.empty());
    SkASSERT(fActiveIntvls.empty());
    SkASSERT(!fIntvlHash.count());
}

void GrResourceAllocator::addInterval(GrSurfaceProxy* proxy, unsigned int start, unsigned int end,
                                      ActualUse actualUse, AllowRecycling allowRecycling
                                      SkDEBUGCODE(, bool isDirectDstRead)) {
    SkASSERT(start <= end);
    SkASSERT(!fAssigned);  // We shouldn't be adding any intervals after (or during) assignment

    if (proxy->canSkipResourceAllocator()) {
        return;
    }

    // If a proxy is read only it must refer to a texture with specific content that cannot be
    // recycled. We don't need to assign a texture to it and no other proxy can be instantiated
    // with the same texture.
    if (proxy->readOnly()) {
        auto resourceProvider = fDContext->priv().resourceProvider();
        if (proxy->isLazy() && !proxy->priv().doLazyInstantiation(resourceProvider)) {
            fFailedInstantiation = true;
        } else {
            // Since we aren't going to add an interval we won't revisit this proxy in assign(). So
            // must already be instantiated or it must be a lazy proxy that we instantiated above.
            SkASSERT(proxy->isInstantiated());
        }
        return;
    }
    uint32_t proxyID = proxy->uniqueID().asUInt();
    if (Interval** intvlPtr = fIntvlHash.find(proxyID)) {
        // Revise the interval for an existing use
        Interval* intvl = *intvlPtr;
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
        if (allowRecycling == AllowRecycling::kNo) {
            // In this case, a preexisting interval is made non-reuseable since its proxy is sampled
            // into a secondary command buffer.
            intvl->disallowRecycling();
        }
        intvl->extendEnd(end);
        return;
    }
    Interval* newIntvl = fInternalAllocator.make<Interval>(proxy, start, end);

    if (ActualUse::kYes == actualUse) {
        newIntvl->addUse();
    }
    if (allowRecycling == AllowRecycling::kNo) {
        newIntvl->disallowRecycling();
    }
    fIntvlList.insertByIncreasingStart(newIntvl);
    fIntvlHash.set(proxyID, newIntvl);
}

// Tragically we have cases where we always have to make new textures.
static bool can_proxy_use_scratch(const GrCaps& caps, GrSurfaceProxy* proxy) {
    return caps.reuseScratchTextures() || proxy->asRenderTargetProxy();
}

GrResourceAllocator::Register::Register(GrSurfaceProxy* originatingProxy,
                                        skgpu::ScratchKey scratchKey,
                                        GrResourceProvider* provider)
        : fOriginatingProxy(originatingProxy)
        , fScratchKey(std::move(scratchKey)) {
    SkASSERT(originatingProxy);
    SkASSERT(!originatingProxy->isInstantiated());
    SkASSERT(!originatingProxy->isLazy());
    SkDEBUGCODE(fUniqueID = CreateUniqueID();)
    if (fScratchKey.isValid()) {
        if (can_proxy_use_scratch(*provider->caps(), originatingProxy)) {
            fExistingSurface = provider->findAndRefScratchTexture(
                    fScratchKey, /*label=*/"ResourceAllocatorRegister");
        }
    } else {
        SkASSERT(this->uniqueKey().isValid());
        fExistingSurface = provider->findByUniqueKey<GrSurface>(this->uniqueKey());
    }
}

bool GrResourceAllocator::Register::isRecyclable(const GrCaps& caps,
                                                 GrSurfaceProxy* proxy,
                                                 int knownUseCount,
                                                 AllowRecycling allowRecycling) const {
    if (allowRecycling == AllowRecycling::kNo) {
        return false;
    }

    if (!can_proxy_use_scratch(caps, proxy)) {
        return false;
    }

    if (!this->scratchKey().isValid()) {
        return false; // no scratch key, no free pool
    }
    if (this->uniqueKey().isValid()) {
        return false; // rely on the resource cache to hold onto uniquely-keyed surfaces.
    }
    // If all the refs on the proxy are known to the resource allocator then no one
    // should be holding onto it outside of Ganesh.
    return !proxy->refCntGreaterThan(knownUseCount);
}

bool GrResourceAllocator::Register::instantiateSurface(GrSurfaceProxy* proxy,
                                                       GrResourceProvider* resourceProvider) {
    SkASSERT(!proxy->peekSurface());

    sk_sp<GrSurface> newSurface;
    if (!fExistingSurface) {
        if (proxy == fOriginatingProxy) {
            newSurface = proxy->priv().createSurface(resourceProvider);
        } else {
            newSurface = sk_ref_sp(fOriginatingProxy->peekSurface());
        }
    }
    if (!fExistingSurface && !newSurface) {
        return false;
    }

    GrSurface* surface = newSurface ? newSurface.get() : fExistingSurface.get();
    // Make surface budgeted if this proxy is budgeted.
    if (skgpu::Budgeted::kYes == proxy->isBudgeted() &&
        GrBudgetedType::kBudgeted != surface->resourcePriv().budgetedType()) {
        // This gets the job done but isn't quite correct. It would be better to try to
        // match budgeted proxies w/ budgeted surfaces and unbudgeted w/ unbudgeted.
        surface->resourcePriv().makeBudgeted();
    }

    // Propagate the proxy unique key to the surface if we have one.
    if (const auto& uniqueKey = proxy->getUniqueKey(); uniqueKey.isValid()) {
        if (!surface->getUniqueKey().isValid()) {
            resourceProvider->assignUniqueKeyToResource(uniqueKey, surface);
        }
        SkASSERT(surface->getUniqueKey() == uniqueKey);
    }
    proxy->priv().assign(fExistingSurface ? fExistingSurface : std::move(newSurface));
    return true;
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

// First try to reuse one of the recently allocated/used registers in the free pool.
GrResourceAllocator::Register* GrResourceAllocator::findOrCreateRegisterFor(GrSurfaceProxy* proxy) {
    auto resourceProvider = fDContext->priv().resourceProvider();
    // Handle uniquely keyed proxies
    if (const auto& uniqueKey = proxy->getUniqueKey(); uniqueKey.isValid()) {
        if (auto p = fUniqueKeyRegisters.find(uniqueKey)) {
            return *p;
        }
        // No need for a scratch key. These don't go in the free pool.
        Register* r = fInternalAllocator.make<Register>(proxy,
                                                        skgpu::ScratchKey(),
                                                        resourceProvider);
        fUniqueKeyRegisters.set(uniqueKey, r);
        return r;
    }

    // Then look in the free pool
    skgpu::ScratchKey scratchKey;
    proxy->priv().computeScratchKey(*fDContext->priv().caps(), &scratchKey);

    auto filter = [] (const Register* r) {
        return true;
    };
    if (Register* r = fFreePool.findAndRemove(scratchKey, filter)) {
        return r;
    }

    return fInternalAllocator.make<Register>(proxy, std::move(scratchKey), resourceProvider);
}

// Remove any intervals that end before the current index. Add their registers
// to the free pool if possible.
void GrResourceAllocator::expire(unsigned int curIndex) {
    while (!fActiveIntvls.empty() && fActiveIntvls.peekHead()->end() < curIndex) {
        Interval* intvl = fActiveIntvls.popHead();
        SkASSERT(!intvl->next());

        Register* r = intvl->getRegister();
        if (r && r->isRecyclable(*fDContext->priv().caps(), intvl->proxy(), intvl->uses(),
                                 intvl->allowRecycling())) {
#if GR_ALLOCATION_SPEW
            SkDebugf("putting register %d back into pool\n", r->uniqueID());
#endif
            // TODO: fix this insertion so we get a more LRU-ish behavior
            fFreePool.insert(r->scratchKey(), r);
        }
        fFinishedIntvls.insertByIncreasingStart(intvl);
    }
}

bool GrResourceAllocator::planAssignment() {
    fIntvlHash.reset(); // we don't need the interval hash anymore

    SkASSERT(!fPlanned && !fAssigned);
    SkDEBUGCODE(fPlanned = true;)

#if GR_ALLOCATION_SPEW
    SkDebugf("assigning %d ops\n", fNumOps);
    this->dumpIntervals();
#endif

    auto resourceProvider = fDContext->priv().resourceProvider();
    while (Interval* cur = fIntvlList.popHead()) {
        this->expire(cur->start());
        fActiveIntvls.insertByIncreasingEnd(cur);

        // Already-instantiated proxies and lazy proxies don't use registers.
        if (cur->proxy()->isInstantiated()) {
            continue;
        }

        // Instantiate fully-lazy proxies immediately. Ignore other lazy proxies at this stage.
        if (cur->proxy()->isLazy()) {
            if (cur->proxy()->isFullyLazy()) {
                fFailedInstantiation = !cur->proxy()->priv().doLazyInstantiation(resourceProvider);
                if (fFailedInstantiation) {
                    break;
                }
            }
            continue;
        }

        Register* r = this->findOrCreateRegisterFor(cur->proxy());
#if GR_ALLOCATION_SPEW
        SkDebugf("Assigning register %d to %d\n",
             r->uniqueID(),
             cur->proxy()->uniqueID().asUInt());
#endif
        SkASSERT(!cur->proxy()->peekSurface());
        cur->setRegister(r);
    }

    // expire all the remaining intervals to drain the active interval list
    this->expire(std::numeric_limits<unsigned int>::max());
    return !fFailedInstantiation;
}

bool GrResourceAllocator::makeBudgetHeadroom() {
    SkASSERT(fPlanned);
    SkASSERT(!fFailedInstantiation);
    size_t additionalBytesNeeded = 0;
    for (Interval* cur = fFinishedIntvls.peekHead(); cur; cur = cur->next()) {
        GrSurfaceProxy* proxy = cur->proxy();
        if (skgpu::Budgeted::kNo == proxy->isBudgeted() || proxy->isInstantiated()) {
            continue;
        }

        // N.B Fully-lazy proxies were already instantiated in planAssignment
        if (proxy->isLazy()) {
            additionalBytesNeeded += proxy->gpuMemorySize();
        } else {
            Register* r = cur->getRegister();
            SkASSERT(r);
            if (!r->accountedForInBudget() && !r->existingSurface()) {
                additionalBytesNeeded += proxy->gpuMemorySize();
            }
            r->setAccountedForInBudget();
        }
    }
    return fDContext->priv().getResourceCache()->purgeToMakeHeadroom(additionalBytesNeeded);
}

void GrResourceAllocator::reset() {
    // NOTE: We do not reset the failedInstantiation flag because we currently do not attempt
    // to recover from failed instantiations. The user is responsible for checking this flag and
    // bailing early.
    SkDEBUGCODE(fPlanned = false;)
    SkDEBUGCODE(fAssigned = false;)
    SkASSERT(fActiveIntvls.empty());
    fFinishedIntvls = IntervalList();
    fIntvlList = IntervalList();
    fIntvlHash.reset();
    fUniqueKeyRegisters.reset();
    fFreePool.reset();
    fInternalAllocator.reset();
}

bool GrResourceAllocator::assign() {
    if (fFailedInstantiation) {
        return false;
    }
    SkASSERT(fPlanned && !fAssigned);
    SkDEBUGCODE(fAssigned = true;)
    auto resourceProvider = fDContext->priv().resourceProvider();
    while (Interval* cur = fFinishedIntvls.popHead()) {
        if (fFailedInstantiation) {
            break;
        }
        if (cur->proxy()->isInstantiated()) {
            continue;
        }
        if (cur->proxy()->isLazy()) {
            fFailedInstantiation = !cur->proxy()->priv().doLazyInstantiation(resourceProvider);
            continue;
        }
        Register* r = cur->getRegister();
        SkASSERT(r);
        fFailedInstantiation = !r->instantiateSurface(cur->proxy(), resourceProvider);
    }
    return !fFailedInstantiation;
}

#if GR_ALLOCATION_SPEW
void GrResourceAllocator::dumpIntervals() {
    // Print all the intervals while computing their range
    SkDebugf("------------------------------------------------------------\n");
    unsigned int min = std::numeric_limits<unsigned int>::max();
    unsigned int max = 0;
    for(const Interval* cur = fIntvlList.peekHead(); cur; cur = cur->next()) {
        SkDebugf("{ %3d,%3d }: [%2d, %2d] - refProxys:%d surfaceRefs:%d\n",
                 cur->proxy()->uniqueID().asUInt(),
                 cur->proxy()->isInstantiated() ? cur->proxy()->underlyingUniqueID().asUInt() : -1,
                 cur->start(),
                 cur->end(),
                 cur->proxy()->priv().getProxyRefCnt(),
                 cur->proxy()->testingOnly_getBackingRefCnt());
        min = std::min(min, cur->start());
        max = std::max(max, cur->end());
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
