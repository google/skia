/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ProxyCache_DEFINED
#define skgpu_graphite_ProxyCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkTHash.h"

class SkBitmap;

namespace skgpu {
    enum class Mipmapped : bool;
    class UniqueKey;
}

namespace skgpu::graphite {

class Recorder;
class TextureProxy;

// This class encapsulates the _internal_ Recorder-local caching of utility proxies.
// TODO: Add IDChangeListeners to the Bitmaps
//       Add mailboxes to receive change messages
//       Add a message processing call to remove newly unreachable proxies
//       Add a removeUniquelyHeld entry point and link into purging system
//       Unit test all that
class ProxyCache {
public:
    ~ProxyCache();

    sk_sp<TextureProxy> findOrCreateCachedProxy(Recorder*, const SkBitmap&, Mipmapped);

private:

    struct UniqueKeyHash {
        uint32_t operator()(const skgpu::UniqueKey& key) const;
    };
    typedef skia_private::THashMap<skgpu::UniqueKey, sk_sp<TextureProxy>, UniqueKeyHash>
            UniqueKeyProxyHash;

    UniqueKeyProxyHash fCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ProxyCache_DEFINED
