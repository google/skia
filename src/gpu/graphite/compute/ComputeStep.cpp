/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/ComputeStep.h"

#include "include/private/base/SkAssert.h"

#include <atomic>
#include <unordered_set>

namespace skgpu::graphite {
namespace {

static uint32_t next_id() {
    static std::atomic<int32_t> nextId{0};
    // Not worried about overflow since a Context isn't expected to have that many ComputeSteps.
    // Even if this it wraps around to 0, that ComputeStep will not be in the same Context as the
    // original 0.
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace

ComputeStep::ComputeStep(std::string_view name,
                         WorkgroupSize localDispatchSize,
                         SkSpan<const ResourceDesc> resources,
                         SkSpan<const WorkgroupBufferDesc> workgroupBuffers,
                         Flags baseFlags)
        : fUniqueID(next_id())
        , fFlags(baseFlags)
        , fName(name)
        , fResources(resources.data(), resources.size())
        , fWorkgroupBuffers(workgroupBuffers.data(), workgroupBuffers.size())
        , fLocalDispatchSize(localDispatchSize) {
#ifdef SK_DEBUG
    std::unordered_set<int> slots;
#endif
    for (const ResourceDesc& r : fResources) {
#ifdef SK_DEBUG
        // Validate that slot assignments within a ComputeStep are unique.
        if (r.fFlow == DataFlow::kShared) {
            SkASSERT(r.fSlot > -1);
            SkASSERT(r.fSlot < kMaxComputeDataFlowSlots);
            auto [_, inserted] = slots.insert(r.fSlot);
            SkASSERT(inserted);
        }
#endif  // SK_DEBUG
        switch (r.fFlow) {
            case DataFlow::kVertexOutput:
                SkASSERT(r.fType == ResourceType::kStorageBuffer);
                SkASSERTF(!(fFlags & Flags::kOutputsVertexBuffer),
                          "a ComputeStep cannot produce more than one vertex buffer");
                fFlags |= Flags::kOutputsVertexBuffer;
                break;
            case DataFlow::kIndexOutput:
                SkASSERT(r.fType == ResourceType::kStorageBuffer);
                SkASSERTF(!(fFlags & Flags::kOutputsIndexBuffer),
                          "a ComputeStep cannot produce more than one index buffer");
                fFlags |= Flags::kOutputsIndexBuffer;
                break;
            case DataFlow::kInstanceOutput:
                SkASSERT(r.fType == ResourceType::kStorageBuffer);
                SkASSERTF(!(fFlags & Flags::kOutputsInstanceBuffer),
                          "a ComputeStep cannot produce more than one instance buffer");
                fFlags |= Flags::kOutputsInstanceBuffer;
                break;
            case DataFlow::kIndirectDrawOutput:
                // More than one indirect buffer output cannot be specified.
                SkASSERTF(!(fFlags & Flags::kOutputsIndirectDrawBuffer),
                          "a ComputeStep cannot produce more than indirect buffer");
                fFlags |= Flags::kOutputsIndirectDrawBuffer;
                break;
            default:
                break;
        }
    }
}

void ComputeStep::prepareStorageBuffer(
        const DrawParams&, int, int, const ResourceDesc&, void*, size_t) const {
    SK_ABORT("ComputeSteps that initialize a mapped storage buffer must override "
             "prepareStorageBuffer()");
}

void ComputeStep::prepareUniformBuffer(const DrawParams&,
                                       int,
                                       const ResourceDesc&,
                                       UniformManager*) const {
    SK_ABORT("ComputeSteps that initialize a uniform buffer must override prepareUniformBuffer()");
}

std::string ComputeStep::computeSkSL(const ResourceBindingRequirements&, int) const {
    SK_ABORT("ComputeSteps must override computeSkSL() unless they support native shader source");
    return "";
}

ComputeStep::NativeShaderSource ComputeStep::nativeShaderSource(NativeShaderFormat) const {
    SK_ABORT("ComputeSteps that support native shader source must override nativeShaderSource()");
    return {};
}

size_t ComputeStep::calculateBufferSize(const DrawParams&, int, const ResourceDesc&) const {
    SK_ABORT("ComputeSteps that initialize a storage buffer must override calculateBufferSize()");
    return 0u;
}

std::tuple<SkISize, SkColorType> ComputeStep::calculateTextureParameters(
        const DrawParams&, int resourceIndex, const ResourceDesc&) const {
    SK_ABORT("ComputeSteps that initialize a texture must override calculateTextureParameters()");
    return {SkISize::MakeEmpty(), kUnknown_SkColorType};
}

SamplerDesc ComputeStep::calculateSamplerParameters(const DrawParams&,
                                                    int resourceIndex,
                                                    const ResourceDesc&) const {
    SK_ABORT("ComputeSteps that initialize a sampler must override calculateSamplerParameters()");
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    return {{}, kTileModes};
}

WorkgroupSize ComputeStep::calculateGlobalDispatchSize(const DrawParams&) const {
    SK_ABORT("ComputeSteps must override calculateGlobalDispatchSize() if it participates "
             "in resource creation");
    return WorkgroupSize();
}

}  // namespace skgpu::graphite
