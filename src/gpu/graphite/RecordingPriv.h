/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RecordingPriv_DEFINED
#define skgpu_graphite_RecordingPriv_DEFINED

#include "include/gpu/graphite/Recording.h"

namespace skgpu::graphite {

class Context;
class Task;
class Surface;

class RecordingPriv {
public:
    bool hasVolatileLazyProxies() const;
    bool instantiateVolatileLazyProxies(ResourceProvider*);
    void deinstantiateVolatileLazyProxies();

    bool hasNonVolatileLazyProxies() const;
    bool instantiateNonVolatileLazyProxies(ResourceProvider*);

#if GRAPHITE_TEST_UTILS
    int numVolatilePromiseImages() const;
    int numNonVolatilePromiseImages() const;
    bool hasTasks() const;
#endif

    bool addCommands(Context*, CommandBuffer*, Surface* replaySurface, SkIVector replayTranslation);
    void addResourceRef(sk_sp<Resource> resource);
    void addTask(sk_sp<Task> task);

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
