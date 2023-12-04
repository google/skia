/*
 * Copyright 2022 Google LLC
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
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ComputeTask.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/UploadTask.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;

namespace {

void* map_buffer(Context* context,
                 skiatest::graphite::GraphiteTestContext* testContext,
                 Buffer* buffer,
                 size_t offset) {
    SkASSERT(buffer);
    if (context->priv().caps()->bufferMapsAreAsync()) {
        buffer->asyncMap();
        while (!buffer->isMapped()) {
            testContext->tick();
        }
    }
    std::byte* ptr = static_cast<std::byte*>(buffer->map());
    SkASSERT(ptr);

    return ptr + offset;
}

sk_sp<Buffer> sync_buffer_to_cpu(Recorder* recorder, const Buffer* buffer) {
    if (recorder->priv().caps()->drawBufferCanBeMapped()) {
        // `buffer` can be mapped directly, however it may still require a synchronization step
        // by the underlying API (e.g. a managed buffer in Metal). SynchronizeToCpuTask
        // automatically handles this for us.
        recorder->priv().add(SynchronizeToCpuTask::Make(sk_ref_sp(buffer)));
        return sk_ref_sp(buffer);
    }

    // The backend requires a transfer buffer for CPU read-back
    auto xferBuffer = recorder->priv().resourceProvider()->findOrCreateBuffer(
            buffer->size(), BufferType::kXferGpuToCpu, AccessPattern::kHostVisible);
    SkASSERT(xferBuffer);

    recorder->priv().add(CopyBufferToBufferTask::Make(buffer,
                                                      /*srcOffset=*/0,
                                                      xferBuffer,
                                                      /*dstOffset=*/0,
                                                      buffer->size()));
    return xferBuffer;
}

bool is_dawn_or_metal_context_type(skiatest::GpuContextType ctxType) {
    return skiatest::IsDawnContextType(ctxType) || skiatest::IsMetalContextType(ctxType);
}

}  // namespace

#define DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(            \
        name, reporter, graphite_context, test_context)           \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name,                          \
                                   is_dawn_or_metal_context_type, \
                                   reporter,                      \
                                   graphite_context,              \
                                   test_context,                  \
                                   CtsEnforcement::kNever)

// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_SingleDispatchTest,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 512;
    constexpr float kFactor = 4.f;

    // The ComputeStep packs kProblemSize floats into kProblemSize / 4 vectors and each thread
    // processes 1 vector at a time.
    constexpr uint32_t kWorkgroupSize = kProblemSize / 4;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    class TestComputeStep : public ComputeStep {
    public:
        // TODO(skia:40045541): SkSL doesn't support std430 layout well, so the buffers
        // below all pack their data into vectors to be compatible with SPIR-V/WGSL.
        TestComputeStep() : ComputeStep(
                /*name=*/"TestArrayMultiply",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    // Input buffer:
                    {
                        // TODO(b/299979165): Declare this binding as read-only.
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kPrivate,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*sksl=*/"inputBlock {\n"
                            "    float factor;\n"
                            "    layout(offset=16) float4 in_data[];\n"
                            "}",
                    },
                    // Output buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,  // shared to allow us to access it from the
                                                     // Builder
                        /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                        /*slot=*/0,
                        /*sksl=*/"outputBlock { float4 out_data[]; }",
                    }
                }) {}
        ~TestComputeStep() override = default;

        // A kernel that multiplies a large array of floats by a supplied factor.
        std::string computeSkSL() const override {
            return R"(
                void main() {
                    out_data[sk_GlobalInvocationID.x] = in_data[sk_GlobalInvocationID.x] * factor;
                }
            )";
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            if (index == 0) {
                SkASSERT(r.fFlow == DataFlow::kPrivate);
                return sizeof(float) * (kProblemSize + 4);
            }
            SkASSERT(index == 1);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(float) * kProblemSize;
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            // Only initialize the input buffer.
            if (resourceIndex != 0) {
                return;
            }
            SkASSERT(r.fFlow == DataFlow::kPrivate);

            size_t dataCount = sizeof(float) * (kProblemSize + 4);
            SkASSERT(bufferSize == dataCount);
            SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
            inData[0] = kFactor;
            for (unsigned int i = 0; i < kProblemSize; ++i) {
                inData[i + 4] = i + 1;
            }
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    if (!builder.appendStep(&step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return;
    }

    // The output buffer should have been placed in the right output slot.
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    if (!outputInfo) {
        ERRORF(reporter, "Failed to allocate an output buffer at slot 0");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);

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
    float* outData = static_cast<float*>(
            map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
    SkASSERT(outputBuffer->isMapped() && outData != nullptr);
    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const float expected = (i + 1) * kFactor;
        const float found = outData[i];
        REPORTER_ASSERT(reporter, expected == found, "expected '%f', found '%f'", expected, found);
    }
}

// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_DispatchGroupTest,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 512;
    constexpr float kFactor1 = 4.f;
    constexpr float kFactor2 = 3.f;

    // The ComputeStep packs kProblemSize floats into kProblemSize / 4 vectors and each thread
    // processes 1 vector at a time.
    constexpr uint32_t kWorkgroupSize = kProblemSize / 4;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // Define two steps that perform two multiplication passes over the same input.

    class TestComputeStep1 : public ComputeStep {
    public:
        // TODO(skia:40045541): SkSL doesn't support std430 layout well, so the buffers
        // below all pack their data into vectors to be compatible with SPIR-V/WGSL.
        TestComputeStep1() : ComputeStep(
                /*name=*/"TestArrayMultiplyFirstPass",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    // Input buffer:
                    {
                        // TODO(b/299979165): Declare this binding as read-only.
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kPrivate,
                        /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                        /*sksl=*/"inputBlock {\n"
                            "    float factor;\n"
                            "    layout(offset=16) float4 in_data[];\n"
                            "}",
                    },
                    // Output buffers:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,  // GPU-only, read by second step
                        /*slot=*/0,
                        /*sksl=*/"outputBlock1 { float4 forward_data[]; }",
                    },
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                        /*slot=*/1,
                        /*sksl=*/"outputBlock2 { float2 extra_data; }",
                    }
                }) {}
        ~TestComputeStep1() override = default;

        // A kernel that multiplies a large array of floats by a supplied factor.
        std::string computeSkSL() const override {
            return R"(
                void main() {
                    uint idx = sk_GlobalInvocationID.x;
                    forward_data[idx] = in_data[idx] * factor;
                    if (idx == 0) {
                        extra_data.x = factor;
                        extra_data.y = 2 * factor;
                    }
                }
            )";
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            if (index == 0) {
                SkASSERT(r.fFlow == DataFlow::kPrivate);
                return sizeof(float) * (kProblemSize + 4);
            }
            if (index == 1) {
                SkASSERT(r.fFlow == DataFlow::kShared);
                SkASSERT(r.fSlot == 0);
                return sizeof(float) * kProblemSize;
            }

            SkASSERT(index == 2);
            SkASSERT(r.fSlot == 1);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return 2 * sizeof(float);
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            if (resourceIndex != 0) {
                return;
            }

            size_t dataCount = sizeof(float) * (kProblemSize + 4);
            SkASSERT(bufferSize == dataCount);
            SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
            inData[0] = kFactor1;
            for (unsigned int i = 0; i < kProblemSize; ++i) {
                inData[i + 4] = i + 1;
            }
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step1;

    class TestComputeStep2 : public ComputeStep {
    public:
        TestComputeStep2() : ComputeStep(
                /*name=*/"TestArrayMultiplySecondPass",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    // Input buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,  // GPU-only
                        /*slot=*/0, // this is the output from the first step
                        /*sksl=*/"inputBlock { float4 in_data[]; }",
                    },
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kPrivate,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*sksl=*/"factorBlock { float factor; }"
                    },
                    // Output buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                        /*slot=*/2,
                        /*sksl=*/"outputBlock { float4 out_data[]; }",
                    }
                }) {}
        ~TestComputeStep2() override = default;

        // A kernel that multiplies a large array of floats by a supplied factor.
        std::string computeSkSL() const override {
            return R"(
                void main() {
                    out_data[sk_GlobalInvocationID.x] = in_data[sk_GlobalInvocationID.x] * factor;
                }
            )";
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            SkASSERT(index != 0);
            if (index == 1) {
                SkASSERT(r.fFlow == DataFlow::kPrivate);
                return sizeof(float) * 4;
            }
            SkASSERT(index == 2);
            SkASSERT(r.fSlot == 2);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(float) * kProblemSize;
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            if (resourceIndex != 1) {
                return;
            }
            SkASSERT(r.fFlow == DataFlow::kPrivate);
            *static_cast<float*>(buffer) = kFactor2;
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step2;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step1);
    builder.appendStep(&step2);

    // Slots 0, 1, and 2 should all contain shared buffers. Slot 1 contains the extra output buffer
    // from step 1 while slot 2 contains the result of the second multiplication pass from step 1.
    // Slot 0 is not mappable.
    REPORTER_ASSERT(reporter,
                    std::holds_alternative<BufferView>(builder.outputTable().fSharedSlots[0]),
                    "shared resource at slot 0 is missing");
    BindBufferInfo outputInfo = builder.getSharedBufferResource(2);
    if (!outputInfo) {
        ERRORF(reporter, "Failed to allocate an output buffer at slot 0");
        return;
    }

    // Extra output buffer from step 1 (corresponding to 'outputBlock2')
    BindBufferInfo extraOutputInfo = builder.getSharedBufferResource(1);
    if (!extraOutputInfo) {
        ERRORF(reporter, "shared resource at slot 1 is missing");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffers get synchronized to the CPU once the GPU submission has finished.
    auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    auto extraOutputBuffer = sync_buffer_to_cpu(recorder.get(), extraOutputInfo.fBuffer);

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

    // Verify the contents of the output buffer from step 2
    float* outData = static_cast<float*>(
            map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
    SkASSERT(outputBuffer->isMapped() && outData != nullptr);
    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const float expected = (i + 1) * kFactor1 * kFactor2;
        const float found = outData[i];
        REPORTER_ASSERT(reporter, expected == found, "expected '%f', found '%f'", expected, found);
    }

    // Verify the contents of the extra output buffer from step 1
    float* extraOutData = static_cast<float*>(
            map_buffer(context, testContext, extraOutputBuffer.get(), extraOutputInfo.fOffset));
    SkASSERT(extraOutputBuffer->isMapped() && extraOutData != nullptr);
    REPORTER_ASSERT(reporter,
                    kFactor1 == extraOutData[0],
                    "expected '%f', found '%f'",
                    kFactor1,
                    extraOutData[0]);
    REPORTER_ASSERT(reporter,
                    2 * kFactor1 == extraOutData[1],
                    "expected '%f', found '%f'",
                    2 * kFactor2,
                    extraOutData[1]);
}

// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_UniformBufferTest,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 512;
    constexpr float kFactor = 4.f;

    // The ComputeStep packs kProblemSize floats into kProblemSize / 4 vectors and each thread
    // processes 1 vector at a time.
    constexpr uint32_t kWorkgroupSize = kProblemSize / 4;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestArrayMultiply",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    // Uniform buffer:
                    {
                        /*type=*/ResourceType::kUniformBuffer,
                        /*flow=*/DataFlow::kPrivate,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*sksl=*/"uniformBlock { float factor; }"
                    },
                    // Input buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kPrivate,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*sksl=*/"inputBlock { float4 in_data[]; }",
                    },
                    // Output buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,  // shared to allow us to access it from the
                                                     // Builder
                        /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                        /*slot=*/0,
                        /*sksl=*/"outputBlock { float4 out_data[]; }",
                    }
                }) {}
        ~TestComputeStep() override = default;

        // A kernel that multiplies a large array of floats by a supplied factor.
        std::string computeSkSL() const override {
            return R"(
                void main() {
                    out_data[sk_GlobalInvocationID.x] = in_data[sk_GlobalInvocationID.x] * factor;
                }
            )";
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            if (index == 0) {
                SkASSERT(r.fFlow == DataFlow::kPrivate);
                return sizeof(float);
            }
            if (index == 1) {
                SkASSERT(r.fFlow == DataFlow::kPrivate);
                return sizeof(float) * kProblemSize;
            }
            SkASSERT(index == 2);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(float) * kProblemSize;
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            // Only initialize the input storage buffer.
            if (resourceIndex != 1) {
                return;
            }
            SkASSERT(r.fFlow == DataFlow::kPrivate);
            size_t dataCount = sizeof(float) * kProblemSize;
            SkASSERT(bufferSize == dataCount);
            SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
            for (unsigned int i = 0; i < kProblemSize; ++i) {
                inData[i] = i + 1;
            }
        }

        void prepareUniformBuffer(int resourceIndex,
                                  const ResourceDesc&,
                                  UniformManager* mgr) const override {
            SkASSERT(resourceIndex == 0);
            SkDEBUGCODE(
                const Uniform uniforms[] = {{"factor", SkSLType::kFloat}};
                mgr->setExpectedUniforms(uniforms);
            )
            mgr->write(kFactor);
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    if (!builder.appendStep(&step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return;
    }

    // The output buffer should have been placed in the right output slot.
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    if (!outputInfo) {
        ERRORF(reporter, "Failed to allocate an output buffer at slot 0");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);

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
    float* outData = static_cast<float*>(
            map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
    SkASSERT(outputBuffer->isMapped() && outData != nullptr);
    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const float expected = (i + 1) * kFactor;
        const float found = outData[i];
        REPORTER_ASSERT(reporter, expected == found, "expected '%f', found '%f'", expected, found);
    }
}

// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_ExternallyAssignedBuffer,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 512;
    constexpr float kFactor = 4.f;

    // The ComputeStep packs kProblemSize floats into kProblemSize / 4 vectors and each thread
    // processes 1 vector at a time.
    constexpr uint32_t kWorkgroupSize = kProblemSize / 4;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"ExternallyAssignedBuffer",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    // Input buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kPrivate,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*sksl=*/"inputBlock {\n"
                                 "    float factor;\n"
                                 "    layout(offset = 16) float4 in_data[];\n"
                                 "}\n",
                    },
                    // Output buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,  // shared to allow us to access it from the
                                                     // Builder
                        /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                        /*slot=*/0,
                        /*sksl=*/"outputBlock { float4 out_data[]; }",
                    }
                }) {}
        ~TestComputeStep() override = default;

        // A kernel that multiplies a large array of floats by a supplied factor.
        std::string computeSkSL() const override {
            return R"(
                void main() {
                    out_data[sk_GlobalInvocationID.x] = in_data[sk_GlobalInvocationID.x] * factor;
                }
            )";
        }

        size_t calculateBufferSize(int resourceIndex, const ResourceDesc& r) const override {
            SkASSERT(resourceIndex == 0);
            SkASSERT(r.fFlow == DataFlow::kPrivate);
            return sizeof(float) * (kProblemSize + 4);
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            SkASSERT(resourceIndex == 0);
            SkASSERT(r.fFlow == DataFlow::kPrivate);

            size_t dataCount = sizeof(float) * (kProblemSize + 4);
            SkASSERT(bufferSize == dataCount);
            SkSpan<float> inData(static_cast<float*>(buffer), dataCount);
            inData[0] = kFactor;
            for (unsigned int i = 0; i < kProblemSize; ++i) {
                inData[i + 4] = i + 1;
            }
        }
    } step;

    // We allocate a buffer and directly assign it to the DispatchGroup::Builder. The ComputeStep
    // will not participate in the creation of this buffer.
    auto [_, outputInfo] =
            recorder->priv().drawBufferManager()->getStoragePointer(sizeof(float) * kProblemSize);
    REPORTER_ASSERT(reporter, outputInfo, "Failed to allocate output buffer");

    DispatchGroup::Builder builder(recorder.get());
    builder.assignSharedBuffer({outputInfo, sizeof(float) * kProblemSize}, 0);

    // Initialize the step with a pre-determined global size
    if (!builder.appendStep(&step, {WorkgroupSize(1, 1, 1)})) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);

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
    float* outData = static_cast<float*>(
            map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
    SkASSERT(outputBuffer->isMapped() && outData != nullptr);
    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const float expected = (i + 1) * kFactor;
        const float found = outData[i];
        REPORTER_ASSERT(reporter, expected == found, "expected '%f', found '%f'", expected, found);
    }
}

// Tests the storage texture binding for a compute dispatch that writes the same color to every
// pixel of a storage texture.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_StorageTexture,
                                              reporter,
                                              context,
                                              testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // For this test we allocate a 16x16 tile which is written to by a single workgroup of the same
    // size.
    constexpr uint32_t kDim = 16;

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestStorageTexture",
                /*localDispatchSize=*/{kDim, kDim, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kWriteOnlyStorageTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/0,
                        /*sksl=*/"dst",
                    }
                }) {}
        ~TestComputeStep() override = default;

        std::string computeSkSL() const override {
            return R"(
                void main() {
                    textureWrite(dst, sk_LocalInvocationID.xy, half4(0.0, 1.0, 0.0, 1.0));
                }
            )";
        }

        std::tuple<SkISize, SkColorType> calculateTextureParameters(
                int index, const ResourceDesc& r) const override {
            return {{kDim, kDim}, kRGBA_8888_SkColorType};
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    if (!builder.appendStep(&step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return;
    }

    sk_sp<TextureProxy> texture = builder.getSharedTextureResource(0);
    if (!texture) {
        ERRORF(reporter, "Shared resource at slot 0 is missing");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

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

    SkBitmap bitmap;
    SkImageInfo imgInfo =
            SkImageInfo::Make(kDim, kDim, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    bitmap.allocPixels(imgInfo);

    SkPixmap pixels;
    bool peekPixelsSuccess = bitmap.peekPixels(&pixels);
    REPORTER_ASSERT(reporter, peekPixelsSuccess);

    bool readPixelsSuccess = context->priv().readPixels(pixels, texture.get(), imgInfo, 0, 0);
    REPORTER_ASSERT(reporter, readPixelsSuccess);

    for (uint32_t x = 0; x < kDim; ++x) {
        for (uint32_t y = 0; y < kDim; ++y) {
            SkColor4f expected = SkColor4f::FromColor(SK_ColorGREEN);
            SkColor4f color = pixels.getColor4f(x, y);
            REPORTER_ASSERT(reporter, expected == color,
                            "At position {%u, %u}, "
                            "expected {%.1f, %.1f, %.1f, %.1f}, "
                            "found {%.1f, %.1f, %.1f, %.1f}",
                            x, y,
                            expected.fR, expected.fG, expected.fB, expected.fA,
                            color.fR, color.fG, color.fB, color.fA);
        }
    }
}

// Tests the readonly texture binding for a compute dispatch that random-access reads from a
// CPU-populated texture and copies it to a storage texture.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_StorageTextureReadAndWrite,
                                              reporter,
                                              context,
                                              testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // For this test we allocate a 16x16 tile which is written to by a single workgroup of the same
    // size.
    constexpr uint32_t kDim = 16;

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestStorageTextureReadAndWrite",
                /*localDispatchSize=*/{kDim, kDim, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kReadOnlyTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/0,
                        /*sksl=*/"src",
                    },
                    {
                        /*type=*/ResourceType::kWriteOnlyStorageTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/1,
                        /*sksl=*/"dst",
                    }
                }) {}
        ~TestComputeStep() override = default;

        std::string computeSkSL() const override {
            return R"(
                void main() {
                    half4 color = textureRead(src, sk_LocalInvocationID.xy);
                    textureWrite(dst, sk_LocalInvocationID.xy, color);
                }
            )";
        }

        std::tuple<SkISize, SkColorType> calculateTextureParameters(
                int index, const ResourceDesc& r) const override {
            SkASSERT(index == 1);
            return {{kDim, kDim}, kRGBA_8888_SkColorType};
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step;

    // Create and populate an input texture.
    SkBitmap srcBitmap;
    SkImageInfo srcInfo =
            SkImageInfo::Make(kDim, kDim, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    srcBitmap.allocPixels(srcInfo);
    SkPixmap srcPixels;
    bool srcPeekPixelsSuccess = srcBitmap.peekPixels(&srcPixels);
    REPORTER_ASSERT(reporter, srcPeekPixelsSuccess);
    for (uint32_t x = 0; x < kDim; ++x) {
        for (uint32_t y = 0; y < kDim; ++y) {
            *srcPixels.writable_addr32(x, y) =
                    SkColorSetARGB(255, x * 256 / kDim, y * 256 / kDim, 0);
        }
    }

    sk_sp<TextureProxy> srcProxy = TextureProxy::Make(context->priv().caps(),
                                                      {kDim, kDim},
                                                      kRGBA_8888_SkColorType,
                                                      skgpu::Mipmapped::kNo,
                                                      skgpu::Protected::kNo,
                                                      skgpu::Renderable::kNo,
                                                      skgpu::Budgeted::kNo);
    MipLevel mipLevel;
    mipLevel.fPixels = srcPixels.addr();
    mipLevel.fRowBytes = srcPixels.rowBytes();
    UploadInstance upload = UploadInstance::Make(recorder.get(),
                                                 srcProxy,
                                                 srcPixels.info().colorInfo(),
                                                 srcPixels.info().colorInfo(),
                                                 {mipLevel},
                                                 SkIRect::MakeWH(kDim, kDim),
                                                 std::make_unique<ImageUploadContext>());
    if (!upload.isValid()) {
        ERRORF(reporter, "Could not create UploadInstance");
        return;
    }
    recorder->priv().add(UploadTask::Make(std::move(upload)));

    DispatchGroup::Builder builder(recorder.get());

    // Assign the input texture to slot 0. This corresponds to the ComputeStep's "src" texture
    // binding.
    builder.assignSharedTexture(std::move(srcProxy), 0);

    if (!builder.appendStep(&step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return;
    }

    sk_sp<TextureProxy> dst = builder.getSharedTextureResource(1);
    if (!dst) {
        ERRORF(reporter, "shared resource at slot 1 is missing");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

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

    SkBitmap bitmap;
    SkImageInfo imgInfo =
            SkImageInfo::Make(kDim, kDim, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    bitmap.allocPixels(imgInfo);

    SkPixmap pixels;
    bool peekPixelsSuccess = bitmap.peekPixels(&pixels);
    REPORTER_ASSERT(reporter, peekPixelsSuccess);

    bool readPixelsSuccess = context->priv().readPixels(pixels, dst.get(), imgInfo, 0, 0);
    REPORTER_ASSERT(reporter, readPixelsSuccess);

    for (uint32_t x = 0; x < kDim; ++x) {
        for (uint32_t y = 0; y < kDim; ++y) {
            SkColor4f expected = SkColor4f::FromBytes_RGBA(
                    SkColorSetARGB(255, x * 256 / kDim, y * 256 / kDim, 0));
            SkColor4f color = pixels.getColor4f(x, y);
            REPORTER_ASSERT(reporter, expected == color,
                            "At position {%u, %u}, "
                            "expected {%.1f, %.1f, %.1f, %.1f}, "
                            "found {%.1f, %.1f, %.1f, %.1f}",
                            x, y,
                            expected.fR, expected.fG, expected.fB, expected.fA,
                            color.fR, color.fG, color.fB, color.fA);
        }
    }
}

// Tests that a texture written by one compute step can be sampled by a subsequent step.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_StorageTextureMultipleComputeSteps,
                                              reporter,
                                              context,
                                              testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // For this test we allocate a 16x16 tile which is written to by a single workgroup of the same
    // size.
    constexpr uint32_t kDim = 16;

    // Writes to a texture in slot 0.
    class TestComputeStep1 : public ComputeStep {
    public:
        TestComputeStep1() : ComputeStep(
                /*name=*/"TestStorageTexturesFirstPass",
                /*localDispatchSize=*/{kDim, kDim, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kWriteOnlyStorageTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/0,
                        /*sksl=*/"dst",
                    }
                }) {}
        ~TestComputeStep1() override = default;

        std::string computeSkSL() const override {
            return R"(
                void main() {
                    textureWrite(dst, sk_LocalInvocationID.xy, half4(0.0, 1.0, 0.0, 1.0));
                }
            )";
        }

        std::tuple<SkISize, SkColorType> calculateTextureParameters(
                int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            return {{kDim, kDim}, kRGBA_8888_SkColorType};
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step1;

    // Reads from the texture in slot 0 and writes it to another texture in slot 1.
    class TestComputeStep2 : public ComputeStep {
    public:
        TestComputeStep2() : ComputeStep(
                /*name=*/"TestStorageTexturesSecondPass",
                /*localDispatchSize=*/{kDim, kDim, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kReadOnlyTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/0,
                        /*sksl=*/"src",
                    },
                    {
                        /*type=*/ResourceType::kWriteOnlyStorageTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/1,
                        /*sksl=*/"dst",
                    }
                }) {}
        ~TestComputeStep2() override = default;

        std::string computeSkSL() const override {
            return R"(
                void main() {
                    half4 color = textureRead(src, sk_LocalInvocationID.xy);
                    textureWrite(dst, sk_LocalInvocationID.xy, color);
                }
            )";
        }

        std::tuple<SkISize, SkColorType> calculateTextureParameters(
                int index, const ResourceDesc& r) const override {
            SkASSERT(index == 1);
            return {{kDim, kDim}, kRGBA_8888_SkColorType};
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step2;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step1);
    builder.appendStep(&step2);

    sk_sp<TextureProxy> dst = builder.getSharedTextureResource(1);
    if (!dst) {
        ERRORF(reporter, "shared resource at slot 1 is missing");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

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

    SkBitmap bitmap;
    SkImageInfo imgInfo =
            SkImageInfo::Make(kDim, kDim, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    bitmap.allocPixels(imgInfo);

    SkPixmap pixels;
    bool peekPixelsSuccess = bitmap.peekPixels(&pixels);
    REPORTER_ASSERT(reporter, peekPixelsSuccess);

    bool readPixelsSuccess = context->priv().readPixels(pixels, dst.get(), imgInfo, 0, 0);
    REPORTER_ASSERT(reporter, readPixelsSuccess);

    for (uint32_t x = 0; x < kDim; ++x) {
        for (uint32_t y = 0; y < kDim; ++y) {
            SkColor4f expected = SkColor4f::FromColor(SK_ColorGREEN);
            SkColor4f color = pixels.getColor4f(x, y);
            REPORTER_ASSERT(reporter, expected == color,
                            "At position {%u, %u}, "
                            "expected {%.1f, %.1f, %.1f, %.1f}, "
                            "found {%.1f, %.1f, %.1f, %.1f}",
                            x, y,
                            expected.fR, expected.fG, expected.fB, expected.fA,
                            color.fR, color.fG, color.fB, color.fA);
        }
    }
}

// Tests that a texture can be sampled by a compute step using a sampler.
// TODO(armansito): Once the previous TODO is done, add additional tests that exercise mixed use of
// texture, buffer, and sampler bindings.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_SampledTexture,
                                              reporter,
                                              context,
                                              testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // The first ComputeStep initializes a 16x16 texture with a checkerboard pattern of alternating
    // red and black pixels. The second ComputeStep downsamples this texture into a 4x4 using
    // bilinear filtering at pixel borders, intentionally averaging the values of each 4x4 tile in
    // the source texture, and writes the result to the destination texture.
    constexpr uint32_t kSrcDim = 16;
    constexpr uint32_t kDstDim = 4;

    class TestComputeStep1 : public ComputeStep {
    public:
        TestComputeStep1() : ComputeStep(
                /*name=*/"Test_SampledTexture_Init",
                /*localDispatchSize=*/{kSrcDim, kSrcDim, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kWriteOnlyStorageTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/0,
                        /*sksl=*/"dst",
                    }
                }) {}
        ~TestComputeStep1() override = default;

        std::string computeSkSL() const override {
            return R"(
                void main() {
                    uint2 c = sk_LocalInvocationID.xy;
                    uint checkerBoardColor = (c.x + (c.y % 2)) % 2;
                    textureWrite(dst, c, half4(checkerBoardColor, 0, 0, 1));
                }
            )";
        }

        std::tuple<SkISize, SkColorType> calculateTextureParameters(
                int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            return {{kSrcDim, kSrcDim}, kRGBA_8888_SkColorType};
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step1;

    class TestComputeStep2 : public ComputeStep {
    public:
        TestComputeStep2() : ComputeStep(
                /*name=*/"Test_SampledTexture_Sample",
                /*localDispatchSize=*/{kDstDim, kDstDim, 1},
                /*resources=*/{
                    // Declare the storage texture before the sampled texture. This tests that
                    // binding index assignment works consistently across all backends when a
                    // sampler-less texture and a texture+sampler pair are intermixed and sampler
                    // bindings aren't necessarily contiguous when the ranges are distinct.
                    {
                        /*type=*/ResourceType::kWriteOnlyStorageTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/1,
                        /*sksl=*/"dst",
                    },
                    {
                        /*type=*/ResourceType::kSampledTexture,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kNone,
                        /*slot=*/0,
                        /*sksl=*/"src",
                    }
                }) {}
        ~TestComputeStep2() override = default;

        std::string computeSkSL() const override {
            return R"(
                void main() {
                    // Normalize the 4x4 invocation indices and sample the source texture using
                    // that.
                    uint2 dstCoord = sk_LocalInvocationID.xy;
                    const float2 dstSizeInv = float2(0.25, 0.25);
                    float2 unormCoord = float2(dstCoord) * dstSizeInv;

                    // Use explicit LOD, as quad derivatives are not available to a compute shader.
                    half4 color = sampleLod(src, unormCoord, 0);
                    textureWrite(dst, dstCoord, color);
                }
            )";
        }

        std::tuple<SkISize, SkColorType> calculateTextureParameters(
                int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0 || index == 1);
            return {{kDstDim, kDstDim}, kRGBA_8888_SkColorType};
        }

        SamplerDesc calculateSamplerParameters(int index, const ResourceDesc&) const override {
            SkASSERT(index == 1);
            // Use the repeat tile mode to sample an infinite checkerboard.
            constexpr SkTileMode kTileModes[2] = {SkTileMode::kRepeat, SkTileMode::kRepeat};
            return {SkFilterMode::kLinear, kTileModes};
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step2;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step1);
    builder.appendStep(&step2);

    sk_sp<TextureProxy> dst = builder.getSharedTextureResource(1);
    if (!dst) {
        ERRORF(reporter, "shared resource at slot 1 is missing");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

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

    SkBitmap bitmap;
    SkImageInfo imgInfo =
            SkImageInfo::Make(kDstDim, kDstDim, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    bitmap.allocPixels(imgInfo);

    SkPixmap pixels;
    bool peekPixelsSuccess = bitmap.peekPixels(&pixels);
    REPORTER_ASSERT(reporter, peekPixelsSuccess);

    bool readPixelsSuccess = context->priv().readPixels(pixels, dst.get(), imgInfo, 0, 0);
    REPORTER_ASSERT(reporter, readPixelsSuccess);

    for (uint32_t x = 0; x < kDstDim; ++x) {
        for (uint32_t y = 0; y < kDstDim; ++y) {
            SkColor4f color = pixels.getColor4f(x, y);
            REPORTER_ASSERT(reporter, color.fR > 0.49 && color.fR < 0.51,
                            "At position {%u, %u}, "
                            "expected red channel in range [0.49, 0.51], "
                            "found {%.3f}",
                            x, y, color.fR);
        }
    }
}

// TODO(b/260622403): The shader tested here is identical to
// `resources/sksl/compute/AtomicsOperations.compute`. It would be nice to be able to exercise SkSL
// features like this as part of SkSLTest.cpp instead of as a graphite test.
// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_AtomicOperationsTest,
                                              reporter,
                                              context,
                                              testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    constexpr uint32_t kWorkgroupCount = 32;
    constexpr uint32_t kWorkgroupSize = 256;

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestAtomicOperations",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*slot=*/0,
                        /*sksl=*/"ssbo { atomicUint globalCounter; }",
                    }
                }) {}
        ~TestComputeStep() override = default;

        // A kernel that increments a global (device memory) counter across multiple workgroups.
        // Each workgroup maintains its own independent tally in a workgroup-shared counter which
        // is then added to the global count.
        //
        // This exercises atomic store/load/add and coherent reads and writes over memory in storage
        // and workgroup address spaces.
        std::string computeSkSL() const override {
            return R"(
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
            )";
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(uint32_t);
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(kWorkgroupCount, 1, 1);
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            SkASSERT(resourceIndex == 0);
            *static_cast<uint32_t*>(buffer) = 0;
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);

    BindBufferInfo info = builder.getSharedBufferResource(0);
    if (!info) {
        ERRORF(reporter, "shared resource at slot 0 is missing");
        return;
    }

    // Record the compute pass task.
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto buffer = sync_buffer_to_cpu(recorder.get(), info.fBuffer);

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
    constexpr uint32_t kExpectedCount = kWorkgroupCount * kWorkgroupSize;
    const uint32_t result = static_cast<const uint32_t*>(
            map_buffer(context, testContext, buffer.get(), info.fOffset))[0];
    REPORTER_ASSERT(reporter,
                    result == kExpectedCount,
                    "expected '%d', found '%d'",
                    kExpectedCount,
                    result);
}

// TODO(b/260622403): The shader tested here is identical to
// `resources/sksl/compute/AtomicsOperationsOverArrayAndStruct.compute`. It would be nice to be able
// to exercise SkSL features like this as part of SkSLTest.cpp instead of as a graphite test.
// TODO(b/262427430, b/262429132): Enable this test on other backends once they all support
// compute programs.
DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_AtomicOperationsOverArrayAndStructTest,
                                              reporter,
                                              context,
                                              testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    constexpr uint32_t kWorkgroupCount = 32;
    constexpr uint32_t kWorkgroupSize = 256;

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestAtomicOperationsOverArrayAndStruct",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*slot=*/0,
                        /*sksl=*/"ssbo {\n"
                            "   atomicUint globalCountsFirstHalf;\n"
                            "   atomicUint globalCountsSecondHalf;\n"
                            "}\n"
                    }
                }) {}
        ~TestComputeStep() override = default;

        // Construct a kernel that increments a two global (device memory) counters across multiple
        // workgroups. Each workgroup maintains its own independent tallies in workgroup-shared
        // counters which are then added to the global counts.
        //
        // This exercises atomic store/load/add and coherent reads and writes over memory in storage
        // and workgroup address spaces.
        std::string computeSkSL() const override {
            return R"(
                const uint WORKGROUP_SIZE = 256;

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
                        atomicAdd(globalCountsFirstHalf, atomicLoad(localCounts[0]));
                        atomicAdd(globalCountsSecondHalf, atomicLoad(localCounts[1]));
                    }
                }
            )";
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return 2 * sizeof(uint32_t);
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(kWorkgroupCount, 1, 1);
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            SkASSERT(resourceIndex == 0);
            uint32_t* data = static_cast<uint32_t*>(buffer);
            data[0] = 0;
            data[1] = 0;
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);

    BindBufferInfo info = builder.getSharedBufferResource(0);
    if (!info) {
        ERRORF(reporter, "shared resource at slot 0 is missing");
        return;
    }

    // Record the compute pass task.
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto buffer = sync_buffer_to_cpu(recorder.get(), info.fBuffer);

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
    constexpr uint32_t kExpectedCount = kWorkgroupCount * kWorkgroupSize / 2;

    const uint32_t* ssboData = static_cast<const uint32_t*>(
            map_buffer(context, testContext, buffer.get(), info.fOffset));
    const uint32_t firstHalfCount = ssboData[0];
    const uint32_t secondHalfCount = ssboData[1];
    REPORTER_ASSERT(reporter,
                    firstHalfCount == kExpectedCount,
                    "expected '%d', found '%d'",
                    kExpectedCount,
                    firstHalfCount);
    REPORTER_ASSERT(reporter,
                    secondHalfCount == kExpectedCount,
                    "expected '%d', found '%d'",
                    kExpectedCount,
                    secondHalfCount);
}

DEF_GRAPHITE_TEST_FOR_DAWN_AND_METAL_CONTEXTS(Compute_ClearedBuffer,
                                              reporter,
                                              context,
                                              testContext) {
    constexpr uint32_t kProblemSize = 512;

    // The ComputeStep packs kProblemSize floats into kProblemSize / 4 vectors and each thread
    // processes 1 vector at a time.
    constexpr uint32_t kWorkgroupSize = kProblemSize / 4;

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // The ComputeStep requests an unmapped buffer that is zero-initialized. It writes the output to
    // a mapped buffer which test verifies.
    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestClearedBuffer",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    // Zero initialized input buffer
                    {
                        // TODO(b/299979165): Declare this binding as read-only.
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kPrivate,
                        /*policy=*/ResourcePolicy::kClear,
                        /*sksl=*/"inputBlock { uint4 in_data[]; }\n",
                    },
                    // Output buffer:
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,  // shared to allow us to access it from the
                                                     // Builder
                        /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                        /*slot=*/0,
                        /*sksl=*/"outputBlock { uint4 out_data[]; }\n",
                    }
                }) {}
        ~TestComputeStep() override = default;

        std::string computeSkSL() const override {
            return R"(
                void main() {
                    out_data[sk_GlobalInvocationID.x] = in_data[sk_GlobalInvocationID.x];
                }
            )";
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            return sizeof(uint32_t) * kProblemSize;
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            // Should receive this call only for the mapped buffer.
            SkASSERT(resourceIndex == 1);
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(1, 1, 1);
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    if (!builder.appendStep(&step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return;
    }

    // The output buffer should have been placed in the right output slot.
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    if (!outputInfo) {
        ERRORF(reporter, "Failed to allocate an output buffer at slot 0");
        return;
    }

    // Record the compute task
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);

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
    uint32_t* outData = static_cast<uint32_t*>(
            map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
    SkASSERT(outputBuffer->isMapped() && outData != nullptr);
    for (unsigned int i = 0; i < kProblemSize; ++i) {
        const uint32_t found = outData[i];
        REPORTER_ASSERT(reporter, 0u == found, "expected '0u', found '%u'", found);
    }
}

DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(Compute_NativeShaderSourceMetal,
                                    reporter,
                                    context,
                                    testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    constexpr uint32_t kWorkgroupCount = 32;
    constexpr uint32_t kWorkgroupSize = 1024;

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestAtomicOperationsMetal",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*slot=*/0,
                    }
                },
                /*workgroupBuffers=*/{},
                /*baseFlags=*/Flags::kSupportsNativeShader) {}
        ~TestComputeStep() override = default;

        NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
            SkASSERT(format == NativeShaderFormat::kMSL);
            static constexpr std::string_view kSource = R"(
                #include <metal_stdlib>

                using namespace metal;

                kernel void atomicCount(uint3 localId [[thread_position_in_threadgroup]],
                                        device atomic_uint& globalCounter [[buffer(0)]]) {
                    threadgroup atomic_uint localCounter;

                    // Initialize the local counter.
                    if (localId.x == 0u) {
                        atomic_store_explicit(&localCounter, 0u, memory_order_relaxed);
                    }

                    // Synchronize the threads in the workgroup so they all see the initial value.
                    threadgroup_barrier(mem_flags::mem_threadgroup);

                    // All threads increment the counter.
                    atomic_fetch_add_explicit(&localCounter, 1u, memory_order_relaxed);

                    // Synchronize the threads again to ensure they have all executed the increment
                    // and the following load reads the same value across all threads in the
                    // workgroup.
                    threadgroup_barrier(mem_flags::mem_threadgroup);

                    // Add the workgroup-only tally to the global counter.
                    if (localId.x == 0u) {
                        uint tally = atomic_load_explicit(&localCounter, memory_order_relaxed);
                        atomic_fetch_add_explicit(&globalCounter, tally, memory_order_relaxed);
                    }
                }
            )";
            return {kSource, "atomicCount"};
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(uint32_t);
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(kWorkgroupCount, 1, 1);
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            SkASSERT(resourceIndex == 0);
            *static_cast<uint32_t*>(buffer) = 0;
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);

    BindBufferInfo info = builder.getSharedBufferResource(0);
    if (!info) {
        ERRORF(reporter, "shared resource at slot 0 is missing");
        return;
    }

    // Record the compute pass task.
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto buffer = sync_buffer_to_cpu(recorder.get(), info.fBuffer);

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
    constexpr uint32_t kExpectedCount = kWorkgroupCount * kWorkgroupSize;
    const uint32_t result = static_cast<const uint32_t*>(
            map_buffer(context, testContext, buffer.get(), info.fOffset))[0];
    REPORTER_ASSERT(reporter,
                    result == kExpectedCount,
                    "expected '%d', found '%d'",
                    kExpectedCount,
                    result);
}

DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(Compute_WorkgroupBufferDescMetal,
                                    reporter,
                                    context,
                                    testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    constexpr uint32_t kWorkgroupCount = 32;
    constexpr uint32_t kWorkgroupSize = 1024;

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestAtomicOperationsMetal",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*slot=*/0,
                    }
                },
                /*workgroupBuffers=*/{
                    {
                        /*size=*/sizeof(uint32_t),
                        /*index=*/0u,
                    }
                },
                /*baseFlags=*/Flags::kSupportsNativeShader) {}
        ~TestComputeStep() override = default;

        // This is the same MSL kernel as in Compute_NativeShaderSourceMetal, except `localCounter`
        // is an entry-point parameter instead of a local variable. This forces the workgroup
        // binding to be encoded explicitly in the command encoder.
        NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
            SkASSERT(format == NativeShaderFormat::kMSL);
            static constexpr std::string_view kSource = R"(
                #include <metal_stdlib>

                using namespace metal;

                kernel void atomicCount(uint3 localId [[thread_position_in_threadgroup]],
                                        device atomic_uint& globalCounter [[buffer(0)]],
                                        threadgroup atomic_uint& localCounter [[threadgroup(0)]]) {
                    // Initialize the local counter.
                    if (localId.x == 0u) {
                        atomic_store_explicit(&localCounter, 0u, memory_order_relaxed);
                    }

                    // Synchronize the threads in the workgroup so they all see the initial value.
                    threadgroup_barrier(mem_flags::mem_threadgroup);

                    // All threads increment the counter.
                    atomic_fetch_add_explicit(&localCounter, 1u, memory_order_relaxed);

                    // Synchronize the threads again to ensure they have all executed the increment
                    // and the following load reads the same value across all threads in the
                    // workgroup.
                    threadgroup_barrier(mem_flags::mem_threadgroup);

                    // Add the workgroup-only tally to the global counter.
                    if (localId.x == 0u) {
                        uint tally = atomic_load_explicit(&localCounter, memory_order_relaxed);
                        atomic_fetch_add_explicit(&globalCounter, tally, memory_order_relaxed);
                    }
                }
            )";
            return {kSource, "atomicCount"};
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(uint32_t);
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(kWorkgroupCount, 1, 1);
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            SkASSERT(resourceIndex == 0);
            *static_cast<uint32_t*>(buffer) = 0;
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);

    BindBufferInfo info = builder.getSharedBufferResource(0);
    if (!info) {
        ERRORF(reporter, "shared resource at slot 0 is missing");
        return;
    }

    // Record the compute pass task.
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto buffer = sync_buffer_to_cpu(recorder.get(), info.fBuffer);

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
    constexpr uint32_t kExpectedCount = kWorkgroupCount * kWorkgroupSize;
    const uint32_t result = static_cast<const uint32_t*>(
            map_buffer(context, testContext, buffer.get(), info.fOffset))[0];
    REPORTER_ASSERT(reporter,
                    result == kExpectedCount,
                    "expected '%d', found '%d'",
                    kExpectedCount,
                    result);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(Compute_NativeShaderSourceWGSL, reporter, context, testContext) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    constexpr uint32_t kWorkgroupCount = 32;
    constexpr uint32_t kWorkgroupSize = 256;  // The WebGPU default workgroup size limit is 256

    class TestComputeStep : public ComputeStep {
    public:
        TestComputeStep() : ComputeStep(
                /*name=*/"TestAtomicOperationsWGSL",
                /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                /*resources=*/{
                    {
                        /*type=*/ResourceType::kStorageBuffer,
                        /*flow=*/DataFlow::kShared,
                        /*policy=*/ResourcePolicy::kMapped,
                        /*slot=*/0,
                    }
                },
                /*workgroupBuffers=*/{},
                /*baseFlags=*/Flags::kSupportsNativeShader) {}
        ~TestComputeStep() override = default;

        NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
            SkASSERT(format == NativeShaderFormat::kWGSL);
            static constexpr std::string_view kSource = R"(
                @group(0) @binding(0) var<storage, read_write> globalCounter: atomic<u32>;

                var<workgroup> localCounter: atomic<u32>;

                @compute @workgroup_size(256)
                fn atomicCount(@builtin(local_invocation_id) localId: vec3u) {
                    // Initialize the local counter.
                    if localId.x == 0u {
                        atomicStore(&localCounter, 0u);
                    }

                    // Synchronize the threads in the workgroup so they all see the initial value.
                    workgroupBarrier();

                    // All threads increment the counter.
                    atomicAdd(&localCounter, 1u);

                    // Synchronize the threads again to ensure they have all executed the increment
                    // and the following load reads the same value across all threads in the
                    // workgroup.
                    workgroupBarrier();

                    // Add the workgroup-only tally to the global counter.
                    if localId.x == 0u {
                        let tally = atomicLoad(&localCounter);
                        atomicAdd(&globalCounter, tally);
                    }
                }
            )";
            return {kSource, "atomicCount"};
        }

        size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
            SkASSERT(index == 0);
            SkASSERT(r.fSlot == 0);
            SkASSERT(r.fFlow == DataFlow::kShared);
            return sizeof(uint32_t);
        }

        WorkgroupSize calculateGlobalDispatchSize() const override {
            return WorkgroupSize(kWorkgroupCount, 1, 1);
        }

        void prepareStorageBuffer(int resourceIndex,
                                  const ResourceDesc& r,
                                  void* buffer,
                                  size_t bufferSize) const override {
            SkASSERT(resourceIndex == 0);
            *static_cast<uint32_t*>(buffer) = 0;
        }
    } step;

    DispatchGroup::Builder builder(recorder.get());
    builder.appendStep(&step);

    BindBufferInfo info = builder.getSharedBufferResource(0);
    if (!info) {
        ERRORF(reporter, "shared resource at slot 0 is missing");
        return;
    }

    // Record the compute pass task.
    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Ensure the output buffer is synchronized to the CPU once the GPU submission has finished.
    auto buffer = sync_buffer_to_cpu(recorder.get(), info.fBuffer);

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
    constexpr uint32_t kExpectedCount = kWorkgroupCount * kWorkgroupSize;
    const uint32_t result = static_cast<const uint32_t*>(
            map_buffer(context, testContext, buffer.get(), info.fOffset))[0];
    REPORTER_ASSERT(reporter,
                    result == kExpectedCount,
                    "expected '%d', found '%d'",
                    kExpectedCount,
                    result);
}
