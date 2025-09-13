/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Recording_DEFINED
#define skgpu_graphite_Recording_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkTArray.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>

struct SkISize;

namespace skgpu {
class RefCntedCallback;
}

namespace skgpu::graphite {

class Caps;
class CommandBuffer;
class RecordingPriv;
class Resource;
class ResourceProvider;
class TaskList;
class Texture;
class TextureInfo;
class TextureProxy;

class SK_API Recording final {
public:
    ~Recording();

    RecordingPriv priv();

private:
    friend class Recorder;  // for ctor and LazyProxyData
    friend class RecordingPriv;

    // LazyProxyData is used if this recording should be replayed to a target that is provided on
    // replay, and it handles the target proxy's instantiation with the provided target.
    class LazyProxyData {
    public:
        LazyProxyData(const Caps*, SkISize dimensions, const TextureInfo&);
        ~LazyProxyData();

        TextureProxy* lazyProxy();
        sk_sp<TextureProxy> refLazyProxy();

        bool lazyInstantiate(ResourceProvider*, sk_sp<Texture>);

    private:
        sk_sp<Texture> fTarget;
        sk_sp<TextureProxy> fTargetProxy;
    };

    struct ProxyHash {
        std::size_t operator()(const sk_sp<TextureProxy>& proxy) const;
    };

    Recording(uint32_t uniqueID,
              uint32_t recorderID,
              std::unique_ptr<LazyProxyData> targetProxyData,
              skia_private::TArray<sk_sp<RefCntedCallback>>&& finishedProcs);

    void addResourceRef(sk_sp<Resource>);

    // Used to verify ordering if recorder ID is not SK_InvalidGenID
    uint32_t fUniqueID;
    uint32_t fRecorderID;

    // This is held by a pointer instead of being inline to allow TaskList to be forward declared.
    std::unique_ptr<TaskList> fRootTaskList;
    // We don't always take refs to all resources used by specific Tasks (e.g. a common buffer used
    // for uploads). Instead we'll just hold onto one ref for those Resources outside the Tasks.
    // Those refs are stored in the array here and will eventually be passed onto a CommandBuffer
    // when the Recording adds its commands.
    std::vector<sk_sp<Resource>> fExtraResourceRefs;

    std::unordered_set<sk_sp<TextureProxy>, ProxyHash> fNonVolatileLazyProxies;
    std::unordered_set<sk_sp<TextureProxy>, ProxyHash> fVolatileLazyProxies;

    std::unique_ptr<LazyProxyData> fTargetProxyData;

    skia_private::TArray<sk_sp<RefCntedCallback>> fFinishedProcs;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Recording_DEFINED
