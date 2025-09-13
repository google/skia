/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecordingPriv_DEFINED
#define skgpu_graphite_RecordingPriv_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"  // IWYU pragma: keep
#include "include/gpu/graphite/Recording.h"

#include <cstdint>
#include <memory>

struct SkIRect;

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class Resource;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;
class Surface;
class TaskList;
class Texture;
class TextureProxy;

class RecordingPriv {
public:
    TextureProxy* deferredTargetProxy();
    const Texture* setupDeferredTarget(ResourceProvider*,
                                       Surface* targetSurface,
                                       SkIVector targetTranslation,
                                       SkIRect targetClip);

    bool hasVolatileLazyProxies() const;
    bool instantiateVolatileLazyProxies(ResourceProvider*);
    void deinstantiateVolatileLazyProxies();

    bool hasNonVolatileLazyProxies() const;
    bool instantiateNonVolatileLazyProxies(ResourceProvider*);

    void setFailureResultForFinishedProcs();

    bool prepareResources(ResourceProvider*,
                          ScratchResourceManager*,
                          sk_sp<const RuntimeEffectDictionary>);

    bool addCommands(Context*,
                     CommandBuffer*,
                     const Texture* replayTarget,
                     SkIVector targetTranslation,
                     SkIRect targetClip);
    // This will eventually lead to adding a Usage Ref on the CommandBuffer. For now that is fine
    // since the only Resource's we are reffing here are Buffers. However, if we ever want to track
    // Textures or GPU only Buffers as well, we should keep a second list for Refs that we want to
    // put CommandBuffer refs on.
    void addResourceRef(sk_sp<Resource> resource);

    TaskList* taskList() { return fRecording->fRootTaskList.get(); }

    uint32_t recorderID() const { return fRecording->fRecorderID; }
    uint32_t uniqueID() const { return fRecording->fUniqueID; }

#if defined(GPU_TEST_UTILS)
    bool isTargetProxyInstantiated() const;
    int numVolatilePromiseImages() const;
    int numNonVolatilePromiseImages() const;
    bool hasTasks() const;
#endif

private:
    explicit RecordingPriv(Recording* recorder) : fRecording(recorder) {}
    RecordingPriv& operator=(const RecordingPriv&) = delete;

    // No taking addresses of this type.
    const RecordingPriv* operator&() const = delete;
    RecordingPriv* operator&() = delete;

    Recording* fRecording;

    friend class Recording;  // to construct/copy this type.
};

inline RecordingPriv Recording::priv() {
    return RecordingPriv(this);
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_RecordingPriv_DEFINED
