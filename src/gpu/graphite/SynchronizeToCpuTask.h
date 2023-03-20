/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SynchronizeToCpuTask_DEFINED
#define skgpu_graphite_SynchronizeToCpuTask_DEFINED

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Task.h"

namespace skgpu::graphite {

class Buffer;

/**
 * Task that synchronizes the contents of a buffer from the GPU to the CPU. This task ensures that
 * all modifications to the buffer made the GPU are visible from the CPU. This task may not result
 * in any work if the underlying buffer does not require synchronization (e.g. a shared memory
 * buffer).
 */
class SynchronizeToCpuTask final : public Task {
public:
    static sk_sp<SynchronizeToCpuTask> Make(sk_sp<Buffer>);
    ~SynchronizeToCpuTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override {
        return true;
    }

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    explicit SynchronizeToCpuTask(sk_sp<Buffer> buffer) : fBuffer(std::move(buffer)) {}

    sk_sp<Buffer> fBuffer;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SynchronizeToCpuTask_DEFINED
