/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GpuWorkSubmission_DEFINED
#define skgpu_graphite_GpuWorkSubmission_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>

namespace skgpu::graphite {
class CommandBuffer;
class SharedContext;
class QueueManager;

class GpuWorkSubmission {
public:
    virtual ~GpuWorkSubmission();

    virtual bool isFinished() = 0;
    virtual void waitUntilFinished() = 0;

protected:
    CommandBuffer* commandBuffer() { return fCommandBuffer.get(); }

    GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer, QueueManager* queueManager);

private:
    std::unique_ptr<CommandBuffer> fCommandBuffer;
    QueueManager* fQueueManager;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GpuWorkSubmission_DEFINED
