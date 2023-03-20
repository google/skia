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
                         SkSpan<const ResourceDesc> resources)
        : fUniqueID(next_id())
        , fFlags(Flags::kNone)
        , fName(name)
        , fResources(resources.begin(), resources.end())
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

void ComputeStep::prepareBuffer(
        const DrawParams&, int, int, const ResourceDesc&, void*, size_t) const {
    SK_ABORT("ComputeSteps using a mapped resource must override prepareBuffer()");
}

}  // namespace skgpu::graphite
