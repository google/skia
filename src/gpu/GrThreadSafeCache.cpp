/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrThreadSafeCache.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrResourceCache.h"

GrThreadSafeCache::GrThreadSafeCache()
    : fFreeEntryList(nullptr) {
}

GrThreadSafeCache::~GrThreadSafeCache() {
    this->dropAllRefs();
}

#if GR_TEST_UTILS
int GrThreadSafeCache::numEntries() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViewMap.count();
}

size_t GrThreadSafeCache::approxBytesUsedForHash() const {
    SkAutoSpinlock lock{fSpinLock};

    return fUniquelyKeyedProxyViewMap.approxBytesUsed();
}
#endif

void GrThreadSafeCache::dropAllRefs() {
    SkAutoSpinlock lock{fSpinLock};

    fUniquelyKeyedProxyViewMap.reset();
    while (auto tmp = fUniquelyKeyedProxyViewList.head()) {
        fUniquelyKeyedProxyViewList.remove(tmp);
        this->recycleEntry(tmp);
    }
    // TODO: should we empty out the fFreeEntryList and reset fEntryAllocator?
}

// TODO: If iterating becomes too expensive switch to using something like GrIORef for the
// GrSurfaceProxy
void GrThreadSafeCache::dropUniqueRefs(GrResourceCache* resourceCache) {
    SkAutoSpinlock lock{fSpinLock};

    // Iterate from LRU to MRU
    Entry* cur = fUniquelyKeyedProxyViewList.tail();
    Entry* prev = cur ? cur->fPrev : nullptr;

    while (cur) {
        if (resourceCache && !resourceCache->overBudget()) {
            return;
        }

        if (cur->fView.proxy()->unique()) {
            fUniquelyKeyedProxyViewMap.remove(cur->fKey);
            fUniquelyKeyedProxyViewList.remove(cur);
            this->recycleEntry(cur);
        }

        cur = prev;
        prev = cur ? cur->fPrev : nullptr;
    }
}

void GrThreadSafeCache::dropUniqueRefsOlderThan(GrStdSteadyClock::time_point purgeTime) {
    SkAutoSpinlock lock{fSpinLock};

    // Iterate from LRU to MRU
    Entry* cur = fUniquelyKeyedProxyViewList.tail();
    Entry* prev = cur ? cur->fPrev : nullptr;

    while (cur) {
        if (cur->fLastAccess >= purgeTime) {
            // This entry and all the remaining ones in the list will be newer than 'purgeTime'
            return;
        }

        if (cur->fView.proxy()->unique()) {
            fUniquelyKeyedProxyViewMap.remove(cur->fKey);
            fUniquelyKeyedProxyViewList.remove(cur);
            this->recycleEntry(cur);
        }

        cur = prev;
        prev = cur ? cur->fPrev : nullptr;
    }
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::internalFind(
                                                       const GrUniqueKey& key) {
    Entry* tmp = fUniquelyKeyedProxyViewMap.find(key);
    if (tmp) {
        SkASSERT(fUniquelyKeyedProxyViewList.isInList(tmp));
        // make the sought out entry the MRU
        tmp->fLastAccess = GrStdSteadyClock::now();
        fUniquelyKeyedProxyViewList.remove(tmp);
        fUniquelyKeyedProxyViewList.addToHead(tmp);
        return { tmp->fView, tmp->fKey.refCustomData() };
    }

    return {};
}

GrSurfaceProxyView GrThreadSafeCache::find(const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    GrSurfaceProxyView view;
    std::tie(view, std::ignore) = this->internalFind(key);
    return view;
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::findWithData(
                                                                        const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalFind(key);
}

GrThreadSafeCache::Entry* GrThreadSafeCache::getEntry(const GrUniqueKey& key,
                                                      const GrSurfaceProxyView& view) {
    Entry* entry;

    if (fFreeEntryList) {
        entry = fFreeEntryList;
        fFreeEntryList = entry->fNext;
        entry->fNext = nullptr;

        entry->fKey = key;
        entry->fView = view;
    } else {
        entry = fEntryAllocator.make<Entry>(key, view);
    }

    // make 'entry' the MRU
    entry->fLastAccess = GrStdSteadyClock::now();
    fUniquelyKeyedProxyViewList.addToHead(entry);
    fUniquelyKeyedProxyViewMap.add(entry);
    return entry;
}

void GrThreadSafeCache::recycleEntry(Entry* dead) {
    SkASSERT(!dead->fPrev && !dead->fNext && !dead->fList);

    dead->fKey.reset();
    dead->fView.reset();

    dead->fNext = fFreeEntryList;
    fFreeEntryList = dead;
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::internalAdd(
                                                                const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    Entry* tmp = fUniquelyKeyedProxyViewMap.find(key);
    if (!tmp) {
        tmp = this->getEntry(key, view);

        SkASSERT(fUniquelyKeyedProxyViewMap.find(key));
    }

    return { tmp->fView, tmp->fKey.refCustomData() };
}

GrSurfaceProxyView GrThreadSafeCache::add(const GrUniqueKey& key, const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    GrSurfaceProxyView newView;
    std::tie(newView, std::ignore) = this->internalAdd(key, view);
    return newView;
}

std::tuple<GrSurfaceProxyView, sk_sp<SkData>> GrThreadSafeCache::addWithData(
                                                                const GrUniqueKey& key,
                                                                const GrSurfaceProxyView& view) {
    SkAutoSpinlock lock{fSpinLock};

    return this->internalAdd(key, view);
}

GrSurfaceProxyView GrThreadSafeCache::findOrAdd(const GrUniqueKey& key,
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
                                                                      const GrUniqueKey& key,
                                                                      const GrSurfaceProxyView& v) {
    SkAutoSpinlock lock{fSpinLock};

    auto [view, data] = this->internalFind(key);
    if (view) {
        return { std::move(view), std::move(data) };
    }

    return this->internalAdd(key, v);
}

void GrThreadSafeCache::remove(const GrUniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};

    Entry* tmp = fUniquelyKeyedProxyViewMap.find(key);
    if (tmp) {
        fUniquelyKeyedProxyViewMap.remove(key);
        fUniquelyKeyedProxyViewList.remove(tmp);
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

    constexpr int kSampleCnt = 1;
    auto [newCT, format] = GrRenderTargetContext::GetFallbackColorTypeAndFormat(
            dContext, origCT, kSampleCnt);

    if (newCT == GrColorType::kUnknown) {
        return {GrSurfaceProxyView(nullptr), nullptr};
    }

    sk_sp<Trampoline> trampoline(new Trampoline);

    GrProxyProvider::TextureInfo texInfo{ GrMipMapped::kNo, GrTextureType::k2D };

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
            SkBudgeted::kYes,
            GrProtected::kNo,
            /* wrapsVkSecondaryCB */ false,
            GrSurfaceProxy::UseAllocator::kYes);

    // TODO: It seems like this 'newCT' usage should be 'origCT' but this is
    // what GrRenderTargetContext::MakeWithFallback does
    GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(format, newCT);

    return {{std::move(proxy), origin, swizzle}, std::move(trampoline)};
}
