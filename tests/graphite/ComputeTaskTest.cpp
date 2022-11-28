/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePassTask.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ComputeTaskTest, reporter, context) {
    constexpr uint32_t kProblemSize = 512;
    constexpr float kFactor = 4.f;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // Construct a kernel that multiplies a large array of floats by a supplied factor.
    ComputePipelineDesc pipelineDesc;
    pipelineDesc.setProgram(
            "layout(set=0, binding=0) readonly buffer inputBlock"
            "{"
            "    float in_factor;"
            "    float in_data[];"
            "};"
            "layout(set=0, binding=1) buffer outputBlock"
            "{"
            "    float out_data[];"
            "};"
            "void main() {"
            "    out_data[sk_GlobalInvocationID.x] = in_data[sk_GlobalInvocationID.x] * in_factor;"
            "}",
            "TestArrayMultiply");

    ResourceProvider* provider = recorder->priv().resourceProvider();
    sk_sp<Buffer> inputBuffer = provider->findOrCreateBuffer(
            sizeof(float) * (kProblemSize + 1), BufferType::kStorage, PrioritizeGpuReads::kNo);
    sk_sp<Buffer> outputBuffer = provider->findOrCreateBuffer(
            sizeof(float) * kProblemSize, BufferType::kStorage, PrioritizeGpuReads::kNo);

    std::vector<ResourceBinding> bindings;
    bindings.push_back({/*index=*/0, {inputBuffer.get(), /*offset=*/0}});
    bindings.push_back({/*index=*/1, {outputBuffer.get(), /*offset=*/0}});

    // Initialize "in_data" to contain an ascending sequence of integers.
    // Initialize "out_data" to "-1"s.
    {
        float* inData = static_cast<float*>(inputBuffer->map());
        float* outData = static_cast<float*>(outputBuffer->map());
        SkASSERT(inputBuffer->isMapped() && inData != nullptr);
        SkASSERT(outputBuffer->isMapped() && outData != nullptr);

        inData[0] = kFactor;  // "in_factor"
        for (unsigned int i = 0; i < kProblemSize; ++i) {
            inData[i + 1] = i + 1;
            outData[i] = -1;
        }
        inputBuffer->unmap();
        outputBuffer->unmap();
    }

    ComputePassDesc desc;
    desc.fLocalDispatchSize = WorkgroupSize(kProblemSize, 1, 1);

    // Record the compute pass task.
    recorder->priv().add(ComputePassTask::Make(std::move(bindings), pipelineDesc, desc));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    recorder->priv().add(SynchronizeToCpuTask::Make(outputBuffer));

    // Submit the work and wait for it to complete.
    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }

    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    context->insertRecording(insertInfo);
    context->submit(SyncToCpu::kYes);

    // Verify the contents of the output buffer.
    {
        float* inData = static_cast<float*>(inputBuffer->map());
        float* outData = static_cast<float*>(outputBuffer->map());
        SkASSERT(inputBuffer->isMapped() && inData != nullptr);
        SkASSERT(outputBuffer->isMapped() && outData != nullptr);
        for (unsigned int i = 0; i < kProblemSize; ++i) {
            const float expected = inData[i + 1] * kFactor;
            const float found = outData[i];
            REPORTER_ASSERT(
                    reporter, expected == found, "expected '%f', found '%f'", expected, found);
        }
        inputBuffer->unmap();
        outputBuffer->unmap();
    }
}
