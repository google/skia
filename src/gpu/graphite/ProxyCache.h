/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ProxyCache_DEFINED
#define skgpu_graphite_ProxyCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkTHash.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/ResourceKey.h"

class SkBitmap;

namespace skgpu {
    enum class Mipmapped : bool;
    class UniqueKey;
}

namespace skgpu::graphite {

class Recorder;
class TextureProxy;

// This class encapsulates the _internal_ Recorder-local caching of utility proxies.
// TODO:
//   Add purgeProxiesNotUsedSince method
//   Link into Context purging system
//   Add unit tests
class ProxyCache {
public:
    ProxyCache(uint32_t recorderID);
    ~ProxyCache();

    sk_sp<TextureProxy> findOrCreateCachedProxy(Recorder*, const SkBitmap&, Mipmapped);

    void purgeAll();

#if defined(GRAPHITE_TEST_UTILS)
    int numCached() const;
    sk_sp<TextureProxy> find(const SkBitmap&, Mipmapped);
    void forceProcessInvalidKeyMsgs();
    void forceFreeUniquelyHeld();
    void forcePurgeProxiesNotUsedSince(skgpu::StdSteadyClock::time_point purgeTime);
#endif

private:
    friend class ResourceCache; // for freeUniquelyHeld

    void processInvalidKeyMsgs();
    void freeUniquelyHeld();
    void purgeProxiesNotUsedSince(const skgpu::StdSteadyClock::time_point* purgeTime);

    typedef SkMessageBus<skgpu::UniqueKeyInvalidatedMsg_Graphite, uint32_t>::Inbox InvalidKeyInbox;

    InvalidKeyInbox fInvalidUniqueKeyInbox;

    struct UniqueKeyHash {
        uint32_t operator()(const skgpu::UniqueKey& key) const;
    };
    typedef skia_private::THashMap<skgpu::UniqueKey, sk_sp<TextureProxy>, UniqueKeyHash>
            UniqueKeyProxyHash;

    UniqueKeyProxyHash fCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ProxyCache_DEFINED
