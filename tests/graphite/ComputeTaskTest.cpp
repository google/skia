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
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputePassTask.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"

using namespace skgpu::graphite;

// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(ComputeTaskTest, reporter, context) {
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
    size_t inputSize = SkAlignTo(sizeof(float) * (kProblemSize + 1),
                                 recorder->priv().caps()->requiredStorageBufferAlignment());
    sk_sp<Buffer> inputBuffer = provider->findOrCreateBuffer(
            inputSize, BufferType::kStorage, PrioritizeGpuReads::kNo);
    size_t outputSize = SkAlignTo(sizeof(float) * kProblemSize,
                                  recorder->priv().caps()->requiredStorageBufferAlignment());
    sk_sp<Buffer> outputBuffer = provider->findOrCreateBuffer(
            outputSize, BufferType::kStorage, PrioritizeGpuReads::kNo);

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

// TODO(b/260622403): The shader tested here is identical to
// `resources/sksl/compute/AtomicsOperations.compute`. It would be nice to be able to exercise SkSL
// features like this as part of SkSLTest.cpp instead of as a graphite test.
// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(ComputeShaderAtomicOperationsTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // Construct a kernel that increments a global (device memory) counter across multiple
    // workgroups. Each workgroup maintains its own independent tally in a workgroup-shared counter
    // which is then added to the global count.
    //
    // This exercises atomic store/load/add and coherent reads and writes over memory in storage and
    // workgroup address spaces.
    ComputePipelineDesc pipelineDesc;
    pipelineDesc.setProgram(
            R"(
                layout(metal, binding = 0) buffer ssbo {
                    atomicUint globalCounter;
                };

                workgroup atomicUint localCounter;

                void main() {
                    // Initialize the local counter.
                    if (sk_LocalInvocationID.x == 0) {
                        atomicStore(localCounter, 0);
                    }

                    // Synchronize the threads in the workgroup so they all see the initial value.
                    workgroupBarrier();

                    // All threads increment the counter.
                    atomicAdd(localCounter, 1);

                    // Synchronize the threads again to ensure they have all executed the increment
                    // and the following load reads the same value across all threads in the
                    // workgroup.
                    workgroupBarrier();

                    // Add the workgroup-only tally to the global counter.
                    if (sk_LocalInvocationID.x == 0) {
                        atomicAdd(globalCounter, atomicLoad(localCounter));
                    }
                }
            )",
            "TestAtomicOperations");

    ResourceProvider* provider = recorder->priv().resourceProvider();
    size_t minSize = SkAlignTo(sizeof(uint32_t),
                               recorder->priv().caps()->requiredStorageBufferAlignment());
    sk_sp<Buffer> ssbo = provider->findOrCreateBuffer(
            minSize, BufferType::kStorage, PrioritizeGpuReads::kNo);

    std::vector<ResourceBinding> bindings;
    bindings.push_back({/*index=*/0, {ssbo.get(), /*offset=*/0}});

    // Initialize the global counter to 0.
    {
        uint32_t* ssboData = static_cast<uint32_t*>(ssbo->map());
        ssboData[0] = 0;
        ssbo->unmap();
    }

    constexpr uint32_t kWorkgroupCount = 32;
    constexpr uint32_t kWorkgroupSize = 1024;

    ComputePassDesc desc;
    desc.fGlobalDispatchSize = WorkgroupSize(kWorkgroupCount, 1, 1);
    desc.fLocalDispatchSize = WorkgroupSize(kWorkgroupSize, 1, 1);

    // Record the compute pass task.
    recorder->priv().add(ComputePassTask::Make(std::move(bindings), pipelineDesc, desc));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    recorder->priv().add(SynchronizeToCpuTask::Make(ssbo));

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
        constexpr uint32_t kExpectedCount = kWorkgroupCount * kWorkgroupSize;
        const uint32_t result = static_cast<const uint32_t*>(ssbo->map())[0];
        REPORTER_ASSERT(reporter,
                        result == kExpectedCount,
                        "expected '%d', found '%d'",
                        kExpectedCount, result);
        ssbo->unmap();
    }
}

// TODO(b/260622403): The shader tested here is identical to
// `resources/sksl/compute/AtomicsOperationsOverArrayAndStruct.compute`. It would be nice to be able
// to exercise SkSL features like this as part of SkSLTest.cpp instead of as a graphite test.
// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(ComputeShaderAtomicOperationsOverArrayAndStructTest,
                                    reporter,
                                    context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // Construct a kernel that increments a two global (device memory) counters across multiple
    // workgroups. Each workgroup maintains its own independent tallies in workgroup-shared counters
    // which are then added to the global counts.
    //
    // This exercises atomic store/load/add and coherent reads and writes over memory in storage and
    // workgroup address spaces.
    ComputePipelineDesc pipelineDesc;
    pipelineDesc.setProgram(
            R"(
                const uint WORKGROUP_SIZE = 1024;

                struct GlobalCounts {
                    atomicUint firstHalfCount;
                    atomicUint secondHalfCount;
                };
                layout(metal, binding = 0) buffer ssbo {
                    GlobalCounts globalCounts;
                };

                workgroup atomicUint localCounts[2];

                void main() {
                    // Initialize the local counts.
                    if (sk_LocalInvocationID.x == 0) {
                        atomicStore(localCounts[0], 0);
                        atomicStore(localCounts[1], 0);
                    }

                    // Synchronize the threads in the workgroup so they all see the initial value.
                    workgroupBarrier();

                    // Each thread increments one of the local counters based on its invocation
                    // index.
                    uint idx = sk_LocalInvocationID.x < (WORKGROUP_SIZE / 2) ? 0 : 1;
                    atomicAdd(localCounts[idx], 1);

                    // Synchronize the threads again to ensure they have all executed the increments
                    // and the following load reads the same value across all threads in the
                    // workgroup.
                    workgroupBarrier();

                    // Add the workgroup-only tally to the global counter.
                    if (sk_LocalInvocationID.x == 0) {
                        atomicAdd(globalCounts.firstHalfCount, atomicLoad(localCounts[0]));
                        atomicAdd(globalCounts.secondHalfCount, atomicLoad(localCounts[1]));
                    }
                }
            )",
            "TestAtomicOperationsOverArrayAndStruct");

    ResourceProvider* provider = recorder->priv().resourceProvider();
    size_t minSize = SkAlignTo(2*sizeof(uint32_t),
                               recorder->priv().caps()->requiredStorageBufferAlignment());
    sk_sp<Buffer> ssbo = provider->findOrCreateBuffer(
            minSize, BufferType::kStorage, PrioritizeGpuReads::kNo);

    std::vector<ResourceBinding> bindings;
    bindings.push_back({/*index=*/0, {ssbo.get(), /*offset=*/0}});

    // Initialize the global counter to 0.
    {
        uint32_t* ssboData = static_cast<uint32_t*>(ssbo->map());
        ssboData[0] = 0;
        ssboData[1] = 0;
        ssbo->unmap();
    }

    constexpr uint32_t kWorkgroupCount = 32;
    constexpr uint32_t kWorkgroupSize = 1024;

    ComputePassDesc desc;
    desc.fGlobalDispatchSize = WorkgroupSize(kWorkgroupCount, 1, 1);
    desc.fLocalDispatchSize = WorkgroupSize(kWorkgroupSize, 1, 1);

    // Record the compute pass task.
    recorder->priv().add(ComputePassTask::Make(std::move(bindings), pipelineDesc, desc));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    recorder->priv().add(SynchronizeToCpuTask::Make(ssbo));

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
        constexpr uint32_t kExpectedCount = kWorkgroupCount * kWorkgroupSize / 2;

        const uint32_t* ssboData = static_cast<const uint32_t*>(ssbo->map());
        const uint32_t firstHalfCount = ssboData[0];
        const uint32_t secondHalfCount = ssboData[1];
        REPORTER_ASSERT(reporter,
                        firstHalfCount == kExpectedCount,
                        "expected '%d', found '%d'",
                        kExpectedCount, firstHalfCount);
        REPORTER_ASSERT(reporter,
                        secondHalfCount == kExpectedCount,
                        "expected '%d', found '%d'",
                        kExpectedCount, secondHalfCount);
        ssbo->unmap();
    }
}
