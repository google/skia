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
#include "include/private/base/SkDeque.h"

#include <memory>
#include <vector>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class GpuWorkSubmission;
struct InsertRecordingInfo;
class ResourceProvider;
class SharedContext;
class Task;

class QueueManager {
public:
    virtual ~QueueManager();

    // Adds the commands from the passed in Recording to the current CommandBuffer
    bool SK_WARN_UNUSED_RESULT addRecording(const InsertRecordingInfo&, Context*);

    // Adds the commands from the passed in Task to the current CommandBuffer
    bool SK_WARN_UNUSED_RESULT addTask(Task*, Context*);

    // Adds the commands from the passed in Task to the current CommandBuffer
    bool SK_WARN_UNUSED_RESULT addFinishInfo(const InsertFinishInfo&, ResourceProvider*);

    bool SK_WARN_UNUSED_RESULT submitToGpu();
    void checkForFinishedWork(SyncToCpu);

#if GRAPHITE_TEST_UTILS
    virtual void startCapture() {}
    virtual void stopCapture() {}
#endif

    void returnCommandBuffer(std::unique_ptr<CommandBuffer>);

protected:
    QueueManager(const SharedContext* sharedContext);

    using OutstandingSubmission = std::unique_ptr<GpuWorkSubmission>;

    const SharedContext* fSharedContext;
    std::unique_ptr<CommandBuffer> fCurrentCommandBuffer;

private:
    virtual std::unique_ptr<CommandBuffer> getNewCommandBuffer(ResourceProvider*) = 0;
    virtual OutstandingSubmission onSubmitToGpu() = 0;

    bool setupCommandBuffer(ResourceProvider*);

    SkDeque fOutstandingSubmissions;

    std::vector<std::unique_ptr<CommandBuffer>> fAvailableCommandBuffers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_QueueManager_DEFINED
