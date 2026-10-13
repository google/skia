/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GpuWorkSubmission_DEFINED
#define skgpu_graphite_GpuWorkSubmission_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/SkTArray.h"

#include <memory>
#include <optional>

namespace skgpu {
class RefCntedCallback;
}

namespace skgpu::graphite {
class CommandBuffer;
class QueueManager;
class SharedContext;

class GpuWorkSubmission {
public:
    virtual ~GpuWorkSubmission();

    bool isFinished(const SharedContext* sharedContext);
    void waitUntilFinished(const SharedContext* sharedContext);

    void addFinishedProc(sk_sp<RefCntedCallback> finishedProc);
    void addFinishedProcs(skia_private::TArray<sk_sp<RefCntedCallback>> finishedProcs);
    void callFinishedProcs(bool success);

    std::optional<GpuStats> gpuStats() const;

protected:
    CommandBuffer* commandBuffer() const { return fCommandBuffer.get(); }
    CommandBuffer* protectedCommandBuffer() const { return fProtectedCommandBuffer.get(); }

    GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                      std::unique_ptr<CommandBuffer> protectedCmdBuffer,
                      QueueManager* queueManager);
    GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer, QueueManager* queueManager);

private:
    virtual bool onIsFinished(const SharedContext* sharedContext) = 0;
    virtual void onWaitUntilFinished(const SharedContext* sharedContext) = 0;

    std::unique_ptr<CommandBuffer> fCommandBuffer;
    std::unique_ptr<CommandBuffer> fProtectedCommandBuffer;
    skia_private::TArray<sk_sp<RefCntedCallback>> fFinishedProcs;
    sk_sp<SkRefCnt> fOutstandingAsyncMapCounter;
    QueueManager* fQueueManager;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GpuWorkSubmission_DEFINED
