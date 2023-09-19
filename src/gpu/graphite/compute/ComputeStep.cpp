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
    static std::atomic<uint32_t> nextId{0};
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
    for (const ResourceDesc& r : fResources) {
        // Validate that slot assignments within a ComputeStep are unique.
        if (r.fFlow == DataFlow::kShared) {
            SkASSERT(r.fSlot > -1);
            SkASSERT(r.fSlot < kMaxComputeDataFlowSlots);
            auto [_, inserted] = slots.insert(r.fSlot);
            SkASSERT(inserted);
        }
    }
#endif  // SK_DEBUG
}

void ComputeStep::prepareStorageBuffer(int, const ResourceDesc&, void*, size_t) const {
    SK_ABORT("ComputeSteps that initialize a mapped storage buffer must override "
             "prepareStorageBuffer()");
}

void ComputeStep::prepareUniformBuffer(int, const ResourceDesc&, UniformManager*) const {
    SK_ABORT("ComputeSteps that initialize a uniform buffer must override prepareUniformBuffer()");
}

std::string ComputeStep::computeSkSL() const {
    SK_ABORT("ComputeSteps must override computeSkSL() unless they support native shader source");
    return "";
}

ComputeStep::NativeShaderSource ComputeStep::nativeShaderSource(NativeShaderFormat) const {
    SK_ABORT("ComputeSteps that support native shader source must override nativeShaderSource()");
    return {};
}

size_t ComputeStep::calculateBufferSize(int, const ResourceDesc&) const {
    SK_ABORT("ComputeSteps that initialize a storage buffer must override calculateBufferSize()");
    return 0u;
}

std::tuple<SkISize, SkColorType> ComputeStep::calculateTextureParameters(
        int, const ResourceDesc&) const {
    SK_ABORT("ComputeSteps that initialize a texture must override calculateTextureParameters()");
    return {SkISize::MakeEmpty(), kUnknown_SkColorType};
}

SamplerDesc ComputeStep::calculateSamplerParameters(int resourceIndex, const ResourceDesc&) const {
    SK_ABORT("ComputeSteps that initialize a sampler must override calculateSamplerParameters()");
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    return {{}, kTileModes};
}

WorkgroupSize ComputeStep::calculateGlobalDispatchSize() const {
    SK_ABORT("ComputeSteps must override calculateGlobalDispatchSize() if it participates "
             "in resource creation");
    return WorkgroupSize();
}

}  // namespace skgpu::graphite
