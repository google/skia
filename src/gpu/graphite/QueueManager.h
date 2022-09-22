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
class GpuWorkSubmission;
struct InsertRecordingInfo;
class ResourceProvider;
class SharedContext;

class QueueManager {
public:
    virtual ~QueueManager();

    // Adds the commands from the passed in Recording to the current CommandBuffer
    void addRecording(const InsertRecordingInfo&, ResourceProvider*);

    bool submitToGpu();
    void checkForFinishedWork(SyncToCpu);

#if GRAPHITE_TEST_UTILS
    virtual void startCapture() {}
    virtual void endCapture() {}
#endif

protected:
    QueueManager(const SharedContext* sharedContext);

    using OutstandingSubmission = std::unique_ptr<GpuWorkSubmission>;

    const SharedContext* fSharedContext;
    sk_sp<CommandBuffer> fCurrentCommandBuffer;

private:
    virtual sk_sp<CommandBuffer> getNewCommandBuffer(ResourceProvider*) = 0;
    virtual OutstandingSubmission onSubmitToGpu() = 0;

    SkDeque fOutstandingSubmissions;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_QueueManager_DEFINED
