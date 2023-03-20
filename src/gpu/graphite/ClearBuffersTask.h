/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ClearBuffersTask_DEFINED
#define skgpu_graphite_ClearBuffersTask_DEFINED

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Task.h"

#include <vector>

namespace skgpu::graphite {

class Buffer;

/**
 * Represents a buffer region that will be cleared. A ClearBuffersTask does not take a reference to
 * the buffer it clears. A higher layer is responsible for managing the lifetime and usage refs of
 * the buffer.
 */
struct ClearBufferInfo {
    const Buffer* fBuffer;
    size_t fOffset = 0;
    size_t fSize = 0;
};

/**
 * Task that clears a region of a list of buffers to 0.
 */
class ClearBuffersTask final : public Task {
public:
    static sk_sp<ClearBuffersTask> Make(std::vector<ClearBufferInfo>);
    ~ClearBuffersTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override {
        return true;
    }

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    explicit ClearBuffersTask(std::vector<ClearBufferInfo> clearList)
            : fClearList(std::move(clearList)) {}

    std::vector<ClearBufferInfo> fClearList;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ClearBuffersTask_DEFINED
