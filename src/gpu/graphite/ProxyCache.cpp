/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ProxyCache.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixelRef.h"
#include "include/gpu/GpuTypes.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureUtils.h"

using namespace skia_private;

DECLARE_SKMESSAGEBUS_MESSAGE(skgpu::UniqueKeyInvalidatedMsg_Graphite, uint32_t,
                             /* AllowCopyableMessage= */ true)

namespace {

void make_bitmap_key(skgpu::UniqueKey* key, const SkBitmap& bm) {
    SkASSERT(key);

    SkIPoint origin = bm.pixelRefOrigin();
    SkIRect subset = SkIRect::MakePtSize(origin, bm.dimensions());

    static const skgpu::UniqueKey::Domain kProxyCacheDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey::Builder builder(key, kProxyCacheDomain, 5, "ProxyCache");
    builder[0] = bm.pixelRef()->getGenerationID();
    builder[1] = subset.fLeft;
    builder[2] = subset.fTop;
    builder[3] = subset.fRight;
    builder[4] = subset.fBottom;
}

sk_sp<SkIDChangeListener> make_unique_key_invalidation_listener(const skgpu::UniqueKey& key,
                                                                uint32_t recorderID) {
    class Listener : public SkIDChangeListener {
    public:
        Listener(const skgpu::UniqueKey& key, uint32_t recorderUniqueID)
                : fMsg(key, recorderUniqueID) {}

        void changed() override {
            SkMessageBus<skgpu::UniqueKeyInvalidatedMsg_Graphite, uint32_t>::Post(fMsg);
        }

    private:
        skgpu::UniqueKeyInvalidatedMsg_Graphite fMsg;
    };

    return sk_make_sp<Listener>(key, recorderID);
}

} // anonymous namespace

namespace skgpu::graphite {

ProxyCache::ProxyCache(uint32_t recorderID) : fInvalidUniqueKeyInbox(recorderID) {
    SkASSERT(recorderID != SK_InvalidGenID);
}

ProxyCache::~ProxyCache() {}

uint32_t ProxyCache::UniqueKeyHash::operator()(const UniqueKey& key) const {
    return key.hash();
}

sk_sp<TextureProxy> ProxyCache::findOrCreateCachedProxy(Recorder* recorder,
                                                        const SkBitmap& bitmap,
                                                        std::string_view label) {

    skgpu::UniqueKey key;
    make_bitmap_key(&key, bitmap);
    return this->findOrCreateCachedProxy(
            recorder, key, &bitmap,
            [](const void* context) { return *static_cast<const SkBitmap*>(context); },
            label);
}

sk_sp<TextureProxy> ProxyCache::findOrCreateCachedProxy(Recorder* recorder,
                                                        const UniqueKey& key,
                                                        BitmapGeneratorContext context,
                                                        BitmapGeneratorFn generator,
                                                        std::string_view label) {
    this->processInvalidKeyMsgs();

    if (CacheEntry* cached = fCache.find(key)) {
        SkASSERT(cached->fProxy);
        if (Resource* resource = cached->fProxy->texture()) {
            resource->updateAccessTime();
        }
        return cached->fProxy;
    }

    SkBitmap bitmap = generator(context);
    if (bitmap.empty()) {
        return nullptr;
    }
    auto [ view, ct ] = MakeBitmapProxyView(recorder, bitmap, nullptr, Mipmapped::kNo,
                                            Budgeted::kYes, label.empty() ? key.tag() : label);
    if (view) {
        // Since if the bitmap is held by more than just this function call (e.g. it likely came
        // from findOrCreateCachedProxy() that takes an existing SkBitmap), it's worth adding a
        // listener to remove them from the cache automatically when no one holds on to it anymore.
        // NOTE: We add listeners even if the bitmap is immutable because the listener triggers when
        // the bitmap is destroyed. We avoid leaking listeners when the proxy cache is purged due
        // to unuse by marking old listeners for cleanup.
        sk_sp<SkIDChangeListener> listener = nullptr;
        const bool addListener = !bitmap.pixelRef()->unique();
        if (addListener) {
            listener = make_unique_key_invalidation_listener(key, recorder->priv().uniqueID());
            bitmap.pixelRef()->addGenIDChangeListener(listener);
        }
        fCache.set(key, {view.refProxy(), std::move(listener)});
    }
    return view.refProxy();
}

void ProxyCache::purgeAll() {
    // removeEntriesAndListeners() without having to copy out all of the keys
    fCache.foreach([](const skgpu::UniqueKey&, const CacheEntry* entry) {
        if (entry->fListener) {
            entry->fListener->markShouldDeregister();
        }
    });
    fCache.reset();
}

void ProxyCache::processInvalidKeyMsgs() {
    TArray<skgpu::UniqueKeyInvalidatedMsg_Graphite> invalidKeyMsgs;
    fInvalidUniqueKeyInbox.poll(&invalidKeyMsgs);

    if (!invalidKeyMsgs.empty()) {
        for (int i = 0; i < invalidKeyMsgs.size(); ++i) {
            // NOTE(crbug.com/1480570): A change listener is only invoked once, so we shouldn't see
            // an invalid message added twice for the same entry. However, we can remove the entry
            // due to other reasons while the bitmap listener owner is still alive. While we mark
            // the listener to de-register itself, there is still a race where we could decide to
            // remove the entry on one thread but haven't de-registered it yet, then another thread
            // cleans up the bitmap and posts a message, then the first thread removes the entry the
            // posted message also wants to remove.
            if (fCache.find(invalidKeyMsgs[i].key())) {
                fCache.remove(invalidKeyMsgs[i].key());
            }
        }
    }
}

void ProxyCache::removeEntriesAndListeners(SkSpan<const UniqueKey> toRemove) {
    // This assumes that the entry removal is coming from not polling the invalid key
    // messages, so it's necessary to mark the listeners as done. Removing the listeners also means
    // we don't leak change listeners if the bitmap is ever re-cached.
    for (const UniqueKey& k : toRemove) {
        CacheEntry* e = fCache.find(k);
        if (e->fListener) {
            e->fListener->markShouldDeregister();
        }
        fCache.remove(k);
    }
}

void ProxyCache::freeUniquelyHeld() {
    this->processInvalidKeyMsgs();

    skia_private::TArray<skgpu::UniqueKey> toRemove;

    fCache.foreach([&](const skgpu::UniqueKey& key, const CacheEntry* entry) {
        SkASSERT(entry->fProxy);
        if (entry->fProxy->unique()) {
            toRemove.push_back(key);
        }
    });

    this->removeEntriesAndListeners(toRemove);
}

void ProxyCache::purgeProxiesNotUsedSince(const skgpu::StdSteadyClock::time_point* purgeTime) {
    this->processInvalidKeyMsgs();

    skia_private::TArray<skgpu::UniqueKey> toRemove;

    fCache.foreach([&](const skgpu::UniqueKey& key, const CacheEntry* entry) {
        SkASSERT(entry->fProxy);
        if (Resource* resource = entry->fProxy->texture();
            resource &&
            (!purgeTime || resource->lastAccessTime() < *purgeTime)) {
            toRemove.push_back(key);
        }
    });

    this->removeEntriesAndListeners(toRemove);
}

#if defined(GPU_TEST_UTILS)
int ProxyCache::numCached() const {
    return fCache.count();
}

sk_sp<TextureProxy> ProxyCache::find(const SkBitmap& bitmap) {

    skgpu::UniqueKey key;

    make_bitmap_key(&key, bitmap);

    if (CacheEntry* cached = fCache.find(key)) {
        SkASSERT(cached->fProxy);
        return cached->fProxy;
    }

    return nullptr;
}

void ProxyCache::forceProcessInvalidKeyMsgs() {
    this->processInvalidKeyMsgs();
}

void ProxyCache::forceFreeUniquelyHeld() {
    this->freeUniquelyHeld();
}

void ProxyCache::forcePurgeProxiesNotUsedSince(skgpu::StdSteadyClock::time_point purgeTime) {
    this->purgeProxiesNotUsedSince(&purgeTime);
}

#endif // defined(GPU_TEST_UTILS)

} // namespace skgpu::graphite
