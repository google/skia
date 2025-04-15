/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_ClearBuffersTask_DEFINED
#define skgpu_graphite_task_ClearBuffersTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/task/Task.h"

#include <utility>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;

/**
 * Task that clears a region of a list of buffers to 0.
 */
class ClearBuffersTask final : public Task {
public:
    static sk_sp<ClearBuffersTask> Make(skia_private::TArray<BindBufferInfo>);
    ~ClearBuffersTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            const RuntimeEffectDictionary*) override {
        return Status::kSuccess;
    }

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    explicit ClearBuffersTask(skia_private::TArray<BindBufferInfo> clearList)
            : fClearList(std::move(clearList)) {}

    skia_private::TArray<BindBufferInfo> fClearList;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_task_ClearBuffersTask_DEFINED
