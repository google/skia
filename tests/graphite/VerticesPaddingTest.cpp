/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

std::unique_ptr<Recording> submit_recording(Context* context,
                                            GraphiteTestContext* testContext,
                                            Recorder* recorder) {
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        return nullptr;
    }

    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    testContext->syncedSubmit(context);

    return recording;
}

void* map_buffer(Context* context,
                 skiatest::graphite::GraphiteTestContext* testContext,
                 Buffer* bufferToMap,
                 size_t readOffset) {
    SkASSERT(bufferToMap);
    if (context->priv().caps()->bufferMapsAreAsync()) {
        bufferToMap->asyncMap();
        while (!bufferToMap->isMapped()) {
            testContext->tick();
        }
    }
    std::byte* mappedBytes = static_cast<std::byte*>(bufferToMap->map());
    SkASSERT(mappedBytes);

    return mappedBytes + readOffset;
}

sk_sp<Buffer> get_readback_buffer(Recorder* recorder, size_t copyBufferSize) {
    return recorder->priv().resourceProvider()->findOrCreateBuffer(copyBufferSize,
                                                                   BufferType::kXferGpuToCpu,
                                                                   AccessPattern::kHostVisible,
                                                                   "VerticesPaddingTest_Copy");
}

template<typename T>
bool copy_and_readback_buffer_sync(Context* context,
                                   GraphiteTestContext* testContext,
                                   Recorder* recorder,
                                   Buffer* sourceBuffer,
                                   sk_sp<Buffer> copyBuffer,
                                   skia_private::TArray<T>& readbackDst,
                                   uint32_t readbackSize) {
    SkASSERT(!copyBuffer->isMapped() && readbackSize <= copyBuffer->size());
    recorder->priv().add(CopyBufferToBufferTask::Make(sourceBuffer,
                                                      /*srcOffset=*/0,
                                                      copyBuffer,
                                                      /*dstOffset=*/0,
                                                      readbackSize));
    if (!submit_recording(context, testContext, recorder)) {
        return false;
    }
    T* read = static_cast<T*>(map_buffer(context, testContext, copyBuffer.get(), 0));
    if (!read) {
        return false;
    }
    readbackDst = skia_private::TArray<T>(read, readbackSize);
    copyBuffer->unmap();
    return true;
}

bool is_dawn_or_vulkan_context_type(skiatest::GpuContextType contextType) {
    return skiatest::IsDawnContextType(contextType) ||
           skiatest::IsVulkanContextType(contextType);
}

} // namespace

#define DEF_GRAPHITE_TEST_FOR_DAWN_AND_VULKAN_CONTEXTS(            \
        testName, reporterObj, graphiteContext, testContextObj)    \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(testName,                       \
                                   is_dawn_or_vulkan_context_type, \
                                   reporterObj,                    \
                                   graphiteContext,                \
                                   testContextObj,                 \
                                   CtsEnforcement::kNever)

DEF_GRAPHITE_TEST_FOR_DAWN_AND_VULKAN_CONTEXTS(StaticVerticesPaddingTest,
                                               reporter,
                                               context,
                                               testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    auto sharedContext = recorder->priv().sharedContext();

    auto staticVertexCopyRanges = sharedContext->globalCache()->getStaticVertexCopyRanges();
    if (staticVertexCopyRanges.empty()) {
        ERRORF(reporter, "Did not get the static vertex information (copy ranges)");
        return;
    }

    uint32_t vertexInfoElementCount = static_cast<uint32_t>(staticVertexCopyRanges.size());
    if (vertexInfoElementCount & 3) {
        ERRORF(reporter, "Initial static vert information was not correctly populated: "
                         "Element count was %u, expected multiple of 4.", vertexInfoElementCount);
        return;
    }

    auto staticGpuVertexBuffer = sharedContext->globalCache()->getStaticVertexBuffer();
    if (!staticGpuVertexBuffer){
        ERRORF(reporter, "Did not get a valid static vertex buffer.");
        return;
    }

    submit_recording(context, testContext, recorder.get());

    // Readback exact size of static data
    const uint32_t readbackSize = staticGpuVertexBuffer->size();
    auto copyBuff = get_readback_buffer(recorder.get(), readbackSize);
    skia_private::TArray<uint8_t> readbackData;
    if (!copy_and_readback_buffer_sync(context,
                                       testContext,
                                       recorder.get(),
                                       staticGpuVertexBuffer.get(),
                                       copyBuff,
                                       readbackData,
                                       readbackSize)) {
        ERRORF(reporter, "Bad readback, exiting early.");
        return;
    }

    const uint32_t numRegionsToCheck = vertexInfoElementCount / 4;
    for (uint32_t i = 0; i < numRegionsToCheck; ++i) {
        auto cr = staticVertexCopyRanges[i];
        const uint32_t regionEndOffset = cr.fOffset + cr.fSize;
        if (cr.fOffset % cr.fRequiredAlignment) {
            ERRORF(reporter,
                   "Offset was not safely aligned to a count 4 stride: "
                   "Count four stride: %zu Offset: %u Expected: %zu\n",
                   cr.fRequiredAlignment,
                   cr.fOffset,
                   SkAlignNonPow2(cr.fOffset, cr.fRequiredAlignment));
            return;
        }

        if (regionEndOffset % cr.fRequiredAlignment) {
            ERRORF(reporter,
                   "End was not safely aligned to a count 4 stride: "
                   "Count four stride: %zu End: %u Expected: %zu\n",
                   cr.fRequiredAlignment,
                   regionEndOffset,
                   SkAlignNonPow2(regionEndOffset, cr.fRequiredAlignment));
            return;
        }

        const uint32_t paddingStart = cr.fOffset + cr.fUnalignedSize;
        for (uint32_t byteIndex = paddingStart; byteIndex < regionEndOffset; ++byteIndex) {
            if (readbackData[byteIndex]) {
                ERRORF(reporter,
                       "Buffer was not safely padded at byte: "
                       "%u Unaligned End of Buffer: %u Aligned End of Buffer: %u\n",
                       byteIndex,
                       paddingStart,
                       regionEndOffset);
                return;
            }
        }
    }
}
