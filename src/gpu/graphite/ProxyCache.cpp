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

void make_bitmap_key(skgpu::UniqueKey* key, const SkBitmap& bm, skgpu::Mipmapped mipmapped) {
    SkASSERT(key);

    SkIPoint origin = bm.pixelRefOrigin();
    SkIRect subset = SkIRect::MakePtSize(origin, bm.dimensions());

    static const skgpu::UniqueKey::Domain kProxyCacheDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey::Builder builder(key, kProxyCacheDomain, 6, "ProxyCache");
    builder[0] = bm.pixelRef()->getGenerationID();
    builder[1] = subset.fLeft;
    builder[2] = subset.fTop;
    builder[3] = subset.fRight;
    builder[4] = subset.fBottom;
    builder[5] = SkToBool(mipmapped);
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

uint32_t ProxyCache::UniqueKeyHash::operator()(const skgpu::UniqueKey& key) const {
    return key.hash();
}

sk_sp<TextureProxy> ProxyCache::findOrCreateCachedProxy(Recorder* recorder,
                                                        const SkBitmap& bitmap,
                                                        Mipmapped mipmapped) {
    this->processInvalidKeyMsgs();

    if (bitmap.dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    skgpu::UniqueKey key;

    if (mipmapped == Mipmapped::kNo) {
        make_bitmap_key(&key, bitmap, Mipmapped::kYes);

        if (sk_sp<TextureProxy>* cached = fCache.find(key)) {
            if (Resource* resource = (*cached)->texture(); resource) {
                resource->updateAccessTime();
            }
            return *cached;
        }
    }

    make_bitmap_key(&key, bitmap, mipmapped);

    if (sk_sp<TextureProxy>* cached = fCache.find(key)) {
        if (Resource* resource = (*cached)->texture(); resource) {
            resource->updateAccessTime();
        }
        return *cached;
    }

    auto [ view, ct ] = MakeBitmapProxyView(recorder, bitmap, nullptr,
                                            mipmapped, skgpu::Budgeted::kYes);
    if (view) {
        auto listener = make_unique_key_invalidation_listener(key, recorder->priv().recorderID());
        bitmap.pixelRef()->addGenIDChangeListener(std::move(listener));

        fCache.set(key, view.refProxy());
    }
    return view.refProxy();
}

void ProxyCache::purgeAll() {
    fCache.reset();
}

void ProxyCache::processInvalidKeyMsgs() {
    TArray<skgpu::UniqueKeyInvalidatedMsg_Graphite> invalidKeyMsgs;
    fInvalidUniqueKeyInbox.poll(&invalidKeyMsgs);

    if (!invalidKeyMsgs.empty()) {
        for (int i = 0; i < invalidKeyMsgs.size(); ++i) {
            fCache.remove(invalidKeyMsgs[i].key());
        }
    }
}

void ProxyCache::freeUniquelyHeld() {
    this->processInvalidKeyMsgs();

    std::vector<skgpu::UniqueKey> toRemove;

    fCache.foreach([&](const skgpu::UniqueKey& key, const sk_sp<TextureProxy>* proxy) {
        if ((*proxy)->unique()) {
            toRemove.push_back(key);
        }
    });

    for (const skgpu::UniqueKey& k : toRemove) {
        fCache.remove(k);
    }
}

void ProxyCache::purgeProxiesNotUsedSince(const skgpu::StdSteadyClock::time_point* purgeTime) {
    this->processInvalidKeyMsgs();

    std::vector<skgpu::UniqueKey> toRemove;

    fCache.foreach([&](const skgpu::UniqueKey& key, const sk_sp<TextureProxy>* proxy) {
        if (Resource* resource = (*proxy)->texture();
            resource &&
            (!purgeTime || resource->lastAccessTime() < *purgeTime)) {
            resource->setDeleteASAP();
            toRemove.push_back(key);
        }
    });

    for (const skgpu::UniqueKey& k : toRemove) {
        fCache.remove(k);
    }
}

#if defined(GRAPHITE_TEST_UTILS)
int ProxyCache::numCached() const {
    return fCache.count();
}

sk_sp<TextureProxy> ProxyCache::find(const SkBitmap& bitmap, Mipmapped mipmapped) {

    skgpu::UniqueKey key;

    make_bitmap_key(&key, bitmap, mipmapped);

    if (sk_sp<TextureProxy>* cached = fCache.find(key)) {
        return *cached;
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

#endif // defined(GRAPHITE_TEST_UTILS)

} // namespace skgpu::graphite
