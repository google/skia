/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkSpan.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "src/sksl/SkSLUtil.h"

#include "tools/graphite/GraphiteTestContext.h"

#include <cstdint>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

// These tests run the SkSL subgroup builtins and intrinsics on the GPU. They're skipped when the
// context doesn't report SkSL::ShaderCaps::fSubgroupSupport.
//
// Subgroup size and layout are implementation-defined. The size can change between dispatches,
// invocations aren't guaranteed to be assigned to subgroups contiguously by
// sk_LocalInvocationIndex, and a subgroup may be partially filled. So every invocation records its
// sk_SubgroupID, sk_SubgroupInvocationID and sk_SubgroupSize next to its result, and the expected
// values are computed from the subgroups as the GPU actually formed them.
//
// Every subgroup operation is called from uniform control flow. Without maximal reconvergence,
// which invocations are active under divergence is implementation-defined, so it isn't tested.

namespace {

// A single workgroup, so sk_LocalInvocationIndex identifies an invocation and sk_SubgroupID
// identifies a subgroup across the whole dispatch. 128 is the minimum
// maxComputeWorkGroupInvocations on Vulkan.
constexpr uint32_t kWorkgroupSize = 128;

enum class ValueType { kInt, kUInt, kFloat };

const char* sksl_input_block(ValueType type) {
    // ComputeStep::ResourceDesc keeps a raw pointer to its SkSL, so these must be string literals.
    switch (type) {
        case ValueType::kInt:   return "inputBlock { int inData[]; }";
        case ValueType::kUInt:  return "inputBlock { uint inData[]; }";
        case ValueType::kFloat: return "inputBlock { float inData[]; }";
    }
    SkUNREACHABLE;
}

const char* type_name(ValueType type) {
    switch (type) {
        case ValueType::kInt:   return "int";
        case ValueType::kUInt:  return "uint";
        case ValueType::kFloat: return "float";
    }
    SkUNREACHABLE;
}

// Returns SkSL that converts `expr`, of `type`, to a uint with the same bit pattern, so results of
// every type can be compared exactly.
std::string sksl_bits(ValueType type, const std::string& expr) {
    return type == ValueType::kFloat ? "floatBitsToUint(" + expr + ")" : "uint(" + expr + ")";
}

// Returns the bit pattern of a pseudo-random test value in [-125, 125] (shifted to [0, 250] for
// uint). The float values are multiples of 1/256, so they're exact.
uint32_t test_value_bits(ValueType type, uint32_t index) {
    const int32_t v = (int32_t)((index * 17 + 1) % 251) - 125;
    switch (type) {
        case ValueType::kInt:
            return (uint32_t)v;
        case ValueType::kUInt:
            return (uint32_t)(v + 125);
        case ValueType::kFloat: {
            const float f = v / 256.0f;
            uint32_t bits;
            memcpy(&bits, &f, sizeof(bits));
            return bits;
        }
    }
    SkUNREACHABLE;
}

std::vector<uint32_t> test_values(ValueType type) {
    std::vector<uint32_t> values(kWorkgroupSize);
    for (uint32_t i = 0; i < kWorkgroupSize; ++i) {
        values[i] = test_value_bits(type, i);
    }
    return values;
}

// One entry per invocation, matching `uint4 outData[]`.
struct InvocationResult {
    uint32_t fSubgroupID;
    uint32_t fInvocationID;
    uint32_t fSubgroupSize;
    uint32_t fResult;
};
static_assert(sizeof(InvocationResult) == 4 * sizeof(uint32_t));

// A subgroup as the GPU formed it.
struct Subgroup {
    uint32_t fSize = 0;
    // Maps sk_SubgroupInvocationID to sk_LocalInvocationIndex for each invocation in the subgroup.
    std::map<uint32_t, uint32_t> fInvocations;
};

class SubgroupTestStep final : public ComputeStep {
public:
    // `body` must assign the uint `result`. It may read `inData[i]`, which holds `input[i]`.
    SubgroupTestStep(ValueType type, std::vector<uint32_t> input, std::string body)
            : ComputeStep(
                      /*name=*/"SubgroupTestStep",
                      /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                      /*resources=*/
                      {{
                              /*type=*/ResourceType::kStorageBuffer,
                              /*flow=*/DataFlow::kPrivate,
                              /*policy=*/ResourcePolicy::kMapped,
                              /*sksl=*/sksl_input_block(type),
                       },
                       {
                               /*type=*/ResourceType::kStorageBuffer,
                               /*flow=*/DataFlow::kShared,
                               /*policy=*/ResourcePolicy::kMapped,  // mappable for read-back
                               /*slot=*/0,
                               /*sksl=*/"outputBlock { uint4 outData[]; }",
                       }})
            , fInput(std::move(input))
            , fBody(std::move(body)) {
        SkASSERT(fInput.size() == kWorkgroupSize);
    }

    std::string computeSkSL() const override {
        return "void main() {\n"
               "    uint i = sk_LocalInvocationIndex;\n"
               "    uint result = 0;\n" +
               fBody +
               "\n"
               "    outData[i] = uint4(sk_SubgroupID, sk_SubgroupInvocationID, sk_SubgroupSize,"
               " result);\n"
               "}\n";
    }

    size_t calculateBufferSize(int index, const ResourceDesc&) const override {
        return index == 0 ? kWorkgroupSize * sizeof(uint32_t)
                          : kWorkgroupSize * sizeof(InvocationResult);
    }

    void prepareStorageBuffer(int resourceIndex,
                              const ResourceDesc&,
                              skgpu::BufferWriter&& writer) const override {
        if (resourceIndex == 0) {
            writer.write(fInput.data(), fInput.size() * sizeof(uint32_t));
        }
    }

    WorkgroupSize calculateGlobalDispatchSize() const override { return WorkgroupSize(1, 1, 1); }

private:
    std::vector<uint32_t> fInput;
    std::string fBody;
};

void* map_buffer(Context* context, GraphiteTestContext* testContext, Buffer* buffer,
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
    if (recorder->priv().caps()->drawBufferCanBeMappedForReadback()) {
        // `buffer` can be mapped directly, but it may still need an API-level synchronization
        // (e.g. a managed buffer in Metal), which SynchronizeToCpuTask handles.
        recorder->priv().add(SynchronizeToCpuTask::Make(sk_ref_sp(buffer)));
        return sk_ref_sp(buffer);
    }

    // The backend requires a transfer buffer for CPU read-back.
    sk_sp<Buffer> xferBuffer = recorder->priv().resourceProvider()->findOrCreateNonShareableBuffer(
            buffer->size(),
            BufferType::kXferGpuToCpu,
            AccessPattern::kHostVisible,
            "SubgroupComputeTest_TransferToCpu");
    if (!xferBuffer) {
        return nullptr;
    }
    recorder->priv().add(CopyBufferToBufferTask::Make(buffer,
                                                      /*srcOffset=*/0,
                                                      xferBuffer,
                                                      /*dstOffset=*/0,
                                                      buffer->size()));
    return xferBuffer;
}

// Dispatches `step` and returns one result per invocation, or nothing on failure.
std::vector<InvocationResult> dispatch(skiatest::Reporter* reporter,
                                       Context* context,
                                       GraphiteTestContext* testContext,
                                       SubgroupTestStep* step) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    DispatchGroup::Builder builder(recorder.get());
    if (!builder.appendStep(step)) {
        ERRORF(reporter, "Failed to add ComputeStep to DispatchGroup");
        return {};
    }
    BindBufferInfo outputInfo = builder.getSharedBufferResource(0);
    if (!outputInfo) {
        ERRORF(reporter, "Failed to allocate an output buffer at slot 0");
        return {};
    }

    ComputeTask::DispatchGroupList groups;
    groups.push_back(builder.finalize());
    recorder->priv().add(ComputeTask::Make(std::move(groups)));

    // Keep a ref to the read-back buffer until its contents have been copied out.
    sk_sp<Buffer> readback = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
    if (!readback) {
        ERRORF(reporter, "Failed to create a read-back buffer");
        return {};
    }

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return {};
    }
    InsertRecordingInfo insertInfo;
    insertInfo.fRecording = recording.get();
    if (!context->insertRecording(insertInfo)) {
        ERRORF(reporter, "Failed to insert recording");
        return {};
    }
    testContext->syncedSubmit(context);

    // The shared buffer may be suballocated, so honor its offset.
    const auto* data = static_cast<const InvocationResult*>(
            map_buffer(context, testContext, readback.get(), outputInfo.fOffset));
    std::vector<InvocationResult> results(data, data + kWorkgroupSize);
    readback->unmap();
    return results;
}

constexpr bool is_pow2(uint32_t x) { return x != 0 && (x & (x - 1)) == 0; }

// Groups the invocations by subgroup and checks that the subgroup builtins are consistent.
std::map<uint32_t, Subgroup> group_by_subgroup(skiatest::Reporter* reporter,
                                               const std::vector<InvocationResult>& results) {
    std::map<uint32_t, Subgroup> subgroups;
    for (uint32_t i = 0; i < results.size(); ++i) {
        const InvocationResult& r = results[i];
        Subgroup& subgroup = subgroups[r.fSubgroupID];
        if (subgroup.fInvocations.empty()) {
            subgroup.fSize = r.fSubgroupSize;
        }
        // Vulkan, Metal and WGSL all guarantee a power-of-two subgroup size; WGSL caps it at 128.
        REPORTER_ASSERT(reporter, is_pow2(r.fSubgroupSize) && r.fSubgroupSize <= 128,
                        "invocation %u: bad sk_SubgroupSize %u", i, r.fSubgroupSize);
        REPORTER_ASSERT(reporter, r.fSubgroupSize == subgroup.fSize,
                        "invocation %u: sk_SubgroupSize %u differs within subgroup %u (%u)",
                        i, r.fSubgroupSize, r.fSubgroupID, subgroup.fSize);
        REPORTER_ASSERT(reporter, r.fInvocationID < r.fSubgroupSize,
                        "invocation %u: sk_SubgroupInvocationID %u >= sk_SubgroupSize %u",
                        i, r.fInvocationID, r.fSubgroupSize);
        const bool inserted = subgroup.fInvocations.emplace(r.fInvocationID, i).second;
        REPORTER_ASSERT(reporter, inserted,
                        "invocation %u: sk_SubgroupInvocationID %u repeats in subgroup %u",
                        i, r.fInvocationID, r.fSubgroupID);
    }
    return subgroups;
}

// Returns the expected result for one invocation, or nothing if its result is undefined.
using ExpectedFn =
        std::function<std::optional<uint32_t>(const Subgroup&, uint32_t invocationID)>;

void run_subgroup_test(skiatest::Reporter* reporter,
                       Context* context,
                       GraphiteTestContext* testContext,
                       ValueType type,
                       const std::vector<uint32_t>& input,
                       const std::string& body,
                       const ExpectedFn& expected) {
    if (!context->priv().caps()->shaderCaps()->fSubgroupSupport) {
        return;
    }
    SubgroupTestStep step(type, input, body);
    std::vector<InvocationResult> results = dispatch(reporter, context, testContext, &step);
    if (results.empty()) {
        return;
    }
    for (const auto& [subgroupID, subgroup] : group_by_subgroup(reporter, results)) {
        for (const auto& [invocationID, index] : subgroup.fInvocations) {
            std::optional<uint32_t> want = expected(subgroup, invocationID);
            const uint32_t got = results[index].fResult;
            if (want && *want != got) {
                ERRORF(reporter,
                       "%s, %s: subgroup %u (size %u), invocation %u: expected 0x%08x, got 0x%08x",
                       body.c_str(), type_name(type), subgroupID, subgroup.fSize, invocationID,
                       *want, got);
            }
        }
    }
}

bool is_compute_supported_context_type(skiatest::GpuContextType ctxType) {
    return skiatest::IsDawnContextType(ctxType) || skiatest::IsMetalContextType(ctxType) ||
           skiatest::IsVulkanContextType(ctxType);
}

constexpr ValueType kAllTypes[] = {ValueType::kInt, ValueType::kUInt, ValueType::kFloat};

}  // anonymous namespace

#define DEF_SUBGROUP_TEST(name, reporter, context, testContext) \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name,                         \
                                   is_compute_supported_context_type, \
                                   reporter,                     \
                                   context,                      \
                                   testContext,                  \
                                   CtsEnforcement::kNever)

// Checks only the builtins, which run_subgroup_test validates for every test.
DEF_SUBGROUP_TEST(SubgroupBuiltinsTest, reporter, context, testContext) {
    run_subgroup_test(reporter, context, testContext, ValueType::kUInt,
                      test_values(ValueType::kUInt), "",
                      [](const Subgroup&, uint32_t) -> std::optional<uint32_t> { return 0; });
}

// For both vote tests, the first half of the workgroup is all true and the second half
// alternates, so with contiguous subgroups (the common case) both outcomes are exercised.
DEF_SUBGROUP_TEST(SubgroupAllTest, reporter, context, testContext) {
    std::vector<uint32_t> input(kWorkgroupSize);
    for (uint32_t i = 0; i < kWorkgroupSize; ++i) {
        input[i] = (i < kWorkgroupSize / 2) ? 1 : (i % 2);
    }
    run_subgroup_test(reporter, context, testContext, ValueType::kInt, input,
                      "result = uint(subgroupAll(inData[i] != 0));",
                      [&](const Subgroup& subgroup, uint32_t) -> std::optional<uint32_t> {
                          bool all = true;
                          for (const auto& [_, index] : subgroup.fInvocations) {
                              all &= input[index] != 0;
                          }
                          return all;
                      });
}

DEF_SUBGROUP_TEST(SubgroupAnyTest, reporter, context, testContext) {
    std::vector<uint32_t> input(kWorkgroupSize);
    for (uint32_t i = 0; i < kWorkgroupSize; ++i) {
        input[i] = (i < kWorkgroupSize / 2) ? 0 : (i % 2);
    }
    run_subgroup_test(reporter, context, testContext, ValueType::kInt, input,
                      "result = uint(subgroupAny(inData[i] != 0));",
                      [&](const Subgroup& subgroup, uint32_t) -> std::optional<uint32_t> {
                          bool any = false;
                          for (const auto& [_, index] : subgroup.fInvocations) {
                              any |= input[index] != 0;
                          }
                          return any;
                      });
}

// Each invocation reports word (sk_SubgroupInvocationID % 4) of the ballot, so every word is
// checked, including the words above the subgroup size, which must be zero.
DEF_SUBGROUP_TEST(SubgroupBallotTest, reporter, context, testContext) {
    const std::vector<uint32_t> input = test_values(ValueType::kInt);
    run_subgroup_test(reporter, context, testContext, ValueType::kInt, input,
                      "uint4 ballot = subgroupBallot(inData[i] > 0);\n"
                      "result = ballot[sk_SubgroupInvocationID % 4];",
                      [&](const Subgroup& subgroup,
                          uint32_t invocationID) -> std::optional<uint32_t> {
                          uint32_t words[4] = {};
                          for (const auto& [id, index] : subgroup.fInvocations) {
                              if ((int32_t)input[index] > 0) {
                                  words[id / 32] |= 1u << (id % 32);
                              }
                          }
                          return words[invocationID % 4];
                      });
}

DEF_SUBGROUP_TEST(SubgroupBroadcastTest, reporter, context, testContext) {
    for (ValueType type : kAllTypes) {
        const std::vector<uint32_t> input = test_values(type);
        for (uint32_t lane : {0u, 3u}) {
            const std::string call =
                    "subgroupBroadcast(inData[i], " + std::to_string(lane) + "u)";
            run_subgroup_test(reporter, context, testContext, type, input,
                              "result = " + sksl_bits(type, call) + ";",
                              [&](const Subgroup& subgroup,
                                  uint32_t) -> std::optional<uint32_t> {
                                  // Broadcasting from an inactive or nonexistent invocation is
                                  // undefined.
                                  auto source = subgroup.fInvocations.find(lane);
                                  if (source == subgroup.fInvocations.end()) {
                                      return std::nullopt;
                                  }
                                  return input[source->second];
                              });
        }
    }
}

DEF_SUBGROUP_TEST(SubgroupBroadcastFirstTest, reporter, context, testContext) {
    for (ValueType type : kAllTypes) {
        const std::vector<uint32_t> input = test_values(type);
        run_subgroup_test(reporter, context, testContext, type, input,
                          "result = " + sksl_bits(type, "subgroupBroadcastFirst(inData[i])") + ";",
                          [&](const Subgroup& subgroup, uint32_t) -> std::optional<uint32_t> {
                              // In uniform control flow every invocation in the subgroup is
                              // active, so the first is the one with the lowest id.
                              return input[subgroup.fInvocations.begin()->second];
                          });
    }
}
