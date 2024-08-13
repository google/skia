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

    // Find or create a cached TextureProxy that's associated with an externally managed UniqueKey.
    // If there is not a cached proxy available, the bitmap generator function will be called with
    // the provided context argument. The successfully generated bitmap is then uploaded to a
    // a new texture proxy on the Recorder and cached. If the bitmap generation fails, null is
    // returned.
    //
    // The texture proxy's label defaults to the tag of the unique key if not otherwise provided.
    using BitmapGeneratorContext = const void*;
    using BitmapGeneratorFn = SkBitmap (*) (BitmapGeneratorContext);
    sk_sp<TextureProxy> findOrCreateCachedProxy(Recorder* recorder,
                                                const UniqueKey& key,
                                                BitmapGeneratorContext context,
                                                BitmapGeneratorFn fn,
                                                std::string_view label = {});

    void purgeAll();

#if defined(GPU_TEST_UTILS)
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
    struct UniqueKeyHash {
        uint32_t operator()(const UniqueKey& key) const;
    };

    skia_private::THashMap<UniqueKey, sk_sp<TextureProxy>, UniqueKeyHash> fCache;
    SkMessageBus<UniqueKeyInvalidatedMsg_Graphite, uint32_t>::Inbox fInvalidUniqueKeyInbox;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ProxyCache_DEFINED
