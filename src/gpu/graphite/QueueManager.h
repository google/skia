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
#include "include/private/base/SkTArray.h"
#include "src/core/SkTHash.h"

#include <memory>
#include <vector>

namespace skgpu::graphite {

class Buffer;
class CommandBuffer;
class Context;
class GpuWorkSubmission;
struct InsertRecordingInfo;
class ResourceProvider;
class SharedContext;
class Task;
class UploadBufferManager;

/**
 * QueueManager class manages all our command buffers and making sure they are submitted to the GPU
 * in the correct order. Each backend subclasses this class in order to specialize how it gets
 * new command buffers and how they are submitted
 *
 * The class also supports sending commands to either a protected command buffer or a non-protected
 * command buffer. When we are in a non-protected Context, all commands will use non-protected
 * command buffers. When we are in a protected Context, the majority of commands will all go to a
 * protected command buffer (e.g. everything coming in via addRecording). However, there are cases
 * where we need to do some commands in a non-protected command buffer. One specific example of this
 * is when uploading data to a buffer that is read in the vertex shader. Protected memory is not
 * allowed to be accessed in a vertex shader, so all resources must be non-protected. That means if
 * we need to copy data into these resources, those copy operations must occur in a non-protected
 * command buffer. The only way to current request a command buffer that does not match the
 * protectedness of the Context is to call addTask directly here. We do not support intermixing
 * calls to a protected and non-protected command buffer without calling submit beforehand. If you
 * want to switch which type of command is being recorded, you must make sure to call submitToGpu
 * before recording the new commands.
 */
class QueueManager {
public:
    virtual ~QueueManager();

    // Adds the commands from the passed in Recording to the current CommandBuffer
    [[nodiscard]] bool addRecording(const InsertRecordingInfo&, Context*);

    // Adds the commands from the passed in Task to the current CommandBuffer
    [[nodiscard]] bool addTask(Task*, Context*, Protected);

    // Adds a proc that will be called when the current CommandBuffer is submitted and finishes
    [[nodiscard]] bool addFinishInfo(const InsertFinishInfo&,
                                     ResourceProvider*,
                                     SkSpan<const sk_sp<Buffer>> buffersToAsyncMap = {});

    [[nodiscard]] bool submitToGpu();
    [[nodiscard]] bool hasUnfinishedGpuWork();
    void checkForFinishedWork(SyncToCpu);

#if defined(GPU_TEST_UTILS)
    virtual void startCapture() {}
    virtual void stopCapture() {}
#endif

    void returnCommandBuffer(std::unique_ptr<CommandBuffer>);

    virtual void tick() const {}

    void addUploadBufferManagerRefs(UploadBufferManager*);

protected:
    QueueManager(const SharedContext* sharedContext);

    using OutstandingSubmission = std::unique_ptr<GpuWorkSubmission>;

    const SharedContext* fSharedContext;
    std::unique_ptr<CommandBuffer> fCurrentCommandBuffer;

private:
    virtual std::unique_ptr<CommandBuffer> getNewCommandBuffer(ResourceProvider*, Protected) = 0;
    virtual OutstandingSubmission onSubmitToGpu() = 0;

    bool setupCommandBuffer(ResourceProvider*, Protected);

    std::vector<std::unique_ptr<CommandBuffer>>* getAvailableCommandBufferList(Protected);

    SkDeque fOutstandingSubmissions;

    std::vector<std::unique_ptr<CommandBuffer>> fAvailableCommandBuffers;
    std::vector<std::unique_ptr<CommandBuffer>> fAvailableProtectedCommandBuffers;

    skia_private::THashMap<uint32_t, uint32_t> fLastAddedRecordingIDs;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_QueueManager_DEFINED
