/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrThreadSafeCache.h"

#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"  // IWYU pragma: keep
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrTexture.h"

#include <chrono>
#include <functional>

class GrResourceProvider;
class SkData;
enum class SkBackingFit;

GrThreadSafeCache::VertexData::~VertexData () {
    this->reset();
}

GrThreadSafeCache::GrThreadSafeCache()
    : fFreeEntryList(nullptr) {
}

GrThreadSafeCache::~GrThreadSafeCache() {
    this->dropAllRefs();
}

#if defined(GPU_TEST_UTILS)
int GrThreadSafeCache::numEntries() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedEntryMap.count();
}

size_t GrThreadSafeCache::approxBytesUsedForHash() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedEntryMap.approxBytesUsed();
}
#endif

void GrThreadSafeCache::dropAllRefs() {
    SkAutoSpinlock lock{fSpinLock};

    fUniquelyKeyedEntryMap.reset();
    while (auto tmp = fUniquelyKeyedEntryList.head()) {
        fUniquelyKeyedEntryList.remove(tmp);
        this->recycleEntry(tmp);
    }
    // TODO: should we empty out the fFreeEntryList and reset fEntryAllocator?
}

// TODO: If iterating becomes too expensive switch to using something like GrIORef for the
// GrSurfaceProxy
void GrThreadSafeCache::dropUniqueRefs(GrResourceCache* resourceCache) {
    SkAutoSpinlock lock{fSpinLock};

    // Iterate from LRU to MRU
    Entry* cur = fUniquelyKeyedEntryList.tail();
    Entry* prev = cur ? cur->fPrev : nullptr;

    while (cur) {
        if (resourceCache && !resourceCache->overBudget()) {
            return;
        }

        if (cur->uniquelyHeld()) {
            fUniquelyKeyedEntryMap.remove(cur->key());
            fUniquelyKeyedEntryList.remove(cur);
            this->recycleEntry(cur);
        }

        cur = prev;
        prev = cur ? cur->fPrev : nullptr;
    }
}

void GrThreadSafeCache::dropUniqueRefsOlderThan(skgpu::StdSteadyClock::time_point purgeTime) {
    SkAutoSpinlock lock{fSpinLock};

    // Iterate from LRU to MRU
    Entry* cur = fUniquelyKeyedEntryList.tail();
    Entry* prev = cur ? cur->fPrev : nullptr;

    while (cur) {
        if (cur->fLastAccess >= purgeTime) {
            // This entry and all the remaining ones in the list will be newer than 'purgeTime'
            return;
        }

        if (cur->uniquelyHeld()) {
            fUniquelyKeyedEntryMap.remove(cur->key());
            fUniquelyKeyedEntryList.remove(cur);
            this->recycleEntry(cur);
        }

        cur = prev;
        prev = cur ? cur->fPrev : nullptr;
    }
}

void GrThreadSafeCache::makeExistingEntryMRU(Entry* entry) {
    SkASSERT(fUniquelyKeyedEntryList.isInList(entry));

    entry->fLastAccess = skgpu::StdSteadyClock::now();
    fUniquelyKeyedEntryList.remove(entry);
    fUniquelyKeyedEntryList.addToHead(entry);
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::internalFind(
                                                       const skgpu::UniqueKey& key) {
    Entry* tmp = fUniquelyKeyedEntryMap.find(key);
    if (tmp) {
        this->makeExistingEntryMRU(tmp);
        return { tmp->view(), tmp->refCustomData() };
    }

    return {};
}

#ifdef SK_DEBUG
bool GrThreadSafeCache::has(const skgpu::UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    Entry* tmp = fUniquelyKeyedEntryMap.find(key);
    return SkToBool(tmp);
}
#endif

GrSurfaceProxyView GrThreadSafeCache::find(const skgpu::UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    GrSurfaceProxyView view;
    std::tie(view, std::ignore) = this->internalFind(key);
    return view;
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::findWithData(
        const skgpu::UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalFind(key);
}

GrThreadSafeCache::Entry* GrThreadSafeCache::getEntry(const skgpu::UniqueKey& key,
                                                      const GrSurfaceProxyView& view) {
    Entry* entry;

    if (fFreeEntryList) {
        entry = fFreeEntryList;
        fFreeEntryList = entry->fNext;
        entry->fNext = nullptr;

        entry->set(key, view);
    } else {
        entry = fEntryAllocator.make<Entry>(key, view);
    }

    return this->makeNewEntryMRU(entry);
}

GrThreadSafeCache::Entry* GrThreadSafeCache::makeNewEntryMRU(Entry* entry) {
    entry->fLastAccess = skgpu::StdSteadyClock::now();
    fUniquelyKeyedEntryList.addToHead(entry);
    fUniquelyKeyedEntryMap.add(entry);
    return entry;
}

GrThreadSafeCache::Entry* GrThreadSafeCache::getEntry(const skgpu::UniqueKey& key,
                                                      sk_sp<VertexData> vertData) {
    Entry* entry;

    if (fFreeEntryList) {
        entry = fFreeEntryList;
        fFreeEntryList = entry->fNext;
        entry->fNext = nullptr;

        entry->set(key, std::move(vertData));
    } else {
        entry = fEntryAllocator.make<Entry>(key, std::move(vertData));
    }

    return this->makeNewEntryMRU(entry);
}

void GrThreadSafeCache::recycleEntry(Entry* dead) {
    SkASSERT(!dead->fPrev && !dead->fNext && !dead->fList);

    dead->makeEmpty();

    dead->fNext = fFreeEntryList;
    fFreeEntryList = dead;
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::internalAdd(
                                                                const skgpu::UniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    Entry* tmp = fUniquelyKeyedEntryMap.find(key);
    if (!tmp) {
        tmp = this->getEntry(key, view);

        SkASSERT(fUniquelyKeyedEntryMap.find(key));
    }

    return { tmp->view(), tmp->refCustomData() };
}

GrSurfaceProxyView GrThreadSafeCache::add(const skgpu::UniqueKey& key,
                                          const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    GrSurfaceProxyView newView;
    std::tie(newView, std::ignore) = this->internalAdd(key, view);
    return newView;
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::addWithData(
                                                                const skgpu::UniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAdd(key, view);
}

GrSurfaceProxyView GrThreadSafeCache::findOrAdd(const skgpu::UniqueKey& key,
                                                const GrSurfaceProxyView& v) {
    SkAutoSpinlock lock{fSpinLock};

    GrSurfaceProxyView view;
    std::tie(view, std::ignore) = this->internalFind(key);
    if (view) {
        return view;
    }

    std::tie(view, std::ignore) = this->internalAdd(key, v);
    return view;
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::findOrAddWithData(
                                                                      const skgpu::UniqueKey& key,
                                                                      const GrSurfaceProxyView& v) {
    SkAutoSpinlock lock{fSpinLock};

    auto [view, data] = this->internalFind(key);
    if (view) {
        return { std::move(view), std::move(data) };
    }

    return this->internalAdd(key, v);
}

sk_sp<GrThreadSafeCache::VertexData> GrThreadSafeCache::MakeVertexData(const void* vertices,
                                                                       int vertexCount,
                                                                       size_t vertexSize) {
    return sk_sp<VertexData>(new VertexData(vertices, vertexCount, vertexSize));
}

sk_sp<GrThreadSafeCache::VertexData> GrThreadSafeCache::MakeVertexData(sk_sp<GrGpuBuffer> buffer,
                                                                       int vertexCount,
                                                                       size_t vertexSize) {
    return sk_sp<VertexData>(new VertexData(std::move(buffer), vertexCount, vertexSize));
}

std::tuple<sk_sp<GrThreadSafeCache::VertexData>, sk_sp<SkData>>
        GrThreadSafeCache::internalFindVerts(const skgpu::UniqueKey& key) {
    Entry* tmp = fUniquelyKeyedEntryMap.find(key);
    if (tmp) {
        this->makeExistingEntryMRU(tmp);
        return { tmp->vertexData(), tmp->refCustomData() };
    }

    return {};
}

std::tuple<sk_sp<GrThreadSafeCache::VertexData>, sk_sp<SkData>>
        GrThreadSafeCache::findVertsWithData(const skgpu::UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalFindVerts(key);
}

std::tuple<sk_sp<GrThreadSafeCache::VertexData>, sk_sp<SkData>> GrThreadSafeCache::internalAddVerts(
                                                                    const skgpu::UniqueKey& key,
                                                                    sk_sp<VertexData> vertData,
                                                                    IsNewerBetter isNewerBetter) {
    Entry* tmp = fUniquelyKeyedEntryMap.find(key);
    if (!tmp) {
        tmp = this->getEntry(key, std::move(vertData));

        SkASSERT(fUniquelyKeyedEntryMap.find(key));
    } else if (isNewerBetter(tmp->getCustomData(), key.getCustomData())) {
        // This orphans any existing uses of the prior vertex data but ensures the best
        // version is in the cache.
        tmp->set(key, std::move(vertData));
    }

    return { tmp->vertexData(), tmp->refCustomData() };
}

std::tuple<sk_sp<GrThreadSafeCache::VertexData>, sk_sp<SkData>> GrThreadSafeCache::addVertsWithData(
                                                                    const skgpu::UniqueKey& key,
                                                                    sk_sp<VertexData> vertData,
                                                                    IsNewerBetter isNewerBetter) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAddVerts(key, std::move(vertData), isNewerBetter);
}

void GrThreadSafeCache::remove(const skgpu::UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    Entry* tmp = fUniquelyKeyedEntryMap.find(key);
    if (tmp) {
        fUniquelyKeyedEntryMap.remove(key);
        fUniquelyKeyedEntryList.remove(tmp);
        this->recycleEntry(tmp);
    }
}

std::tuple<GrSurfaceProxyView, sk_sp<GrThreadSafeCache::Trampoline>>
GrThreadSafeCache::CreateLazyView(GrDirectContext* dContext,
                                  GrColorType origCT,
                                  SkISize dimensions,
                                  GrSurfaceOrigin origin,
                                  SkBackingFit fit) {
    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    const GrCaps* caps = dContext->priv().caps();

    constexpr int kSampleCnt = 1;
    auto [newCT, format] = caps->getFallbackColorTypeAndFormat(origCT, kSampleCnt);

    if (newCT == GrColorType::kUnknown) {
        return {GrSurfaceProxyView(nullptr), nullptr};
    }

    sk_sp<Trampoline> trampoline(new Trampoline);

    GrProxyProvider::TextureInfo texInfo{skgpu::Mipmapped::kNo, GrTextureType::k2D};

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [trampoline](
                    GrResourceProvider* resourceProvider,
                    const GrSurfaceProxy::LazySurfaceDesc&) -> GrSurfaceProxy::LazyCallbackResult {
                if (!resourceProvider || !trampoline->fProxy ||
                    !trampoline->fProxy->isInstantiated()) {
                    return GrSurfaceProxy::LazyCallbackResult(nullptr, true);
                }

                SkASSERT(!trampoline->fProxy->peekTexture()->getUniqueKey().isValid());
                return GrSurfaceProxy::LazyCallbackResult(
                        sk_ref_sp(trampoline->fProxy->peekTexture()));
            },
            format,
            dimensions,
            kSampleCnt,
            GrInternalSurfaceFlags::kNone,
            &texInfo,
            GrMipmapStatus::kNotAllocated,
            fit,
            skgpu::Budgeted::kYes,
            GrProtected::kNo,
            /* wrapsVkSecondaryCB */ false,
            GrSurfaceProxy::UseAllocator::kYes);

    // TODO: It seems like this 'newCT' usage should be 'origCT' but this is
    // what skgpu::ganesh::SurfaceDrawContext::MakeWithFallback does
    skgpu::Swizzle swizzle = dContext->priv().caps()->getReadSwizzle(format, newCT);

    return {{std::move(proxy), origin, swizzle}, std::move(trampoline)};
}
