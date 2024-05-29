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
// For simplicity it does not support generating mipmapped cached proxies. Internal utility data
// does not typically require mipmapping, and unlike Ganesh, the internal proxy cache is not used
// for uploaded client-provided bitmaps (which may require generating mipmaps).
class ProxyCache {
public:
    ProxyCache(uint32_t recorderID);
    ~ProxyCache();

    sk_sp<TextureProxy> findOrCreateCachedProxy(Recorder*,
                                                const SkBitmap&,
                                                std::string_view label);

    void purgeAll();

#if defined(GRAPHITE_TEST_UTILS)
    int numCached() const;
    sk_sp<TextureProxy> find(const SkBitmap&);
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
