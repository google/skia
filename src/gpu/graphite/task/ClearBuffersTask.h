/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_ClearBuffersTask_DEFINED
#define skgpu_graphite_task_ClearBuffersTask_DEFINED

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/task/Task.h"

namespace skgpu::graphite {

/**
 * Task that clears a region of a list of buffers to 0.
 */
class ClearBuffersTask final : public Task {
public:
    static sk_sp<ClearBuffersTask> Make(skia_private::TArray<ClearBufferInfo>);
    ~ClearBuffersTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            const RuntimeEffectDictionary*) override {
        return Status::kSuccess;
    }

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    explicit ClearBuffersTask(skia_private::TArray<ClearBufferInfo> clearList)
            : fClearList(std::move(clearList)) {}

    skia_private::TArray<ClearBufferInfo> fClearList;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_task_ClearBuffersTask_DEFINED
