/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Recording_DEFINED
#define skgpu_graphite_Recording_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkChecksum.h"

#include <memory>
#include <unordered_set>
#include <vector>

namespace skgpu::graphite {

class CommandBuffer;
class RecordingPriv;
class Resource;
class ResourceProvider;
class TaskGraph;
class TextureProxy;

class Recording final {
public:
    ~Recording();

    RecordingPriv priv();

private:
    friend class Recorder; // for ctor
    friend class RecordingPriv;

    struct ProxyHash {
        std::size_t operator()(const sk_sp<TextureProxy>& proxy) const {
            return SkGoodHash()(proxy.get());
        }
    };

    Recording(std::unique_ptr<TaskGraph>,
              std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& nonVolatileLazyProxies,
              std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& volatileLazyProxies);

    bool addCommands(CommandBuffer*, ResourceProvider*);
    void addResourceRef(sk_sp<Resource>);

    std::unique_ptr<TaskGraph> fGraph;
    // We don't always take refs to all resources used by specific Tasks (e.g. a common buffer used
    // for uploads). Instead we'll just hold onto one ref for those Resources outside the Tasks.
    // Those refs are stored in the array here and will eventually be passed onto a CommandBuffer
    // when the Recording adds its commands.
    std::vector<sk_sp<Resource>> fExtraResourceRefs;

    std::unordered_set<sk_sp<TextureProxy>, ProxyHash> fNonVolatileLazyProxies;
    std::unordered_set<sk_sp<TextureProxy>, ProxyHash> fVolatileLazyProxies;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Recording_DEFINED
