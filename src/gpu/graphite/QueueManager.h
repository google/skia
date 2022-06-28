/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_QueueManager_DEFINED
#define skgpu_graphite_QueueManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/SkDeque.h"

namespace skgpu::graphite {

class CommandBuffer;
class Gpu;
class GpuWorkSubmission;
class Recording;

class QueueManager {
public:
    virtual ~QueueManager();

    // Adds the commands from the passed in Recording to the current CommandBuffer
    // TODO: Implement
    bool addRecording(const Recording*) {
        SkASSERT(false);
        return false;
    }

    bool submitToGpu();
    void checkForFinishedWork(SyncToCpu);

    // TODO: This is a temporary api until we move Recordings to not make/hold CommandBuffers
    // themselves. This will be removed soon.
    void setCurrentCommandBuffer(sk_sp<CommandBuffer>);

protected:
    QueueManager(Gpu* gpu);

    using OutstandingSubmission = std::unique_ptr<GpuWorkSubmission>;

    Gpu* fGpu;
    sk_sp<CommandBuffer> fCurrentCommandBuffer;

private:
    virtual sk_sp<CommandBuffer> getNewCommandBuffer() = 0;
    virtual OutstandingSubmission onSubmitToGpu() = 0;

    SkDeque fOutstandingSubmissions;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_QueueManager_DEFINED
