/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/DispatchGroup.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

DispatchGroup::~DispatchGroup() = default;

bool DispatchGroup::prepareResources(ResourceProvider* resourceProvider) {
    fPipelines.reserve_back(fPipelineDescs.size());
    for (const ComputePipelineDesc& desc : fPipelineDescs) {
        auto pipeline = resourceProvider->findOrCreateComputePipeline(desc);
        if (!pipeline) {
            SKGPU_LOG_W("Failed to create ComputePipeline for dispatch group. Dropping group!");
            return false;
        }
        fPipelines.push_back(std::move(pipeline));
    }

    // The DispatchGroup may be long lived on a Recording and we no longer need ComputePipelineDescs
    // once we've created pipelines.
    fPipelineDescs.clear();

    return true;
}

void DispatchGroup::addResourceRefs(CommandBuffer* commandBuffer) const {
    for (int i = 0; i < fPipelines.size(); ++i) {
        commandBuffer->trackResource(fPipelines[i]);
    }
}

using Builder = DispatchGroup::Builder;

Builder::Builder(Recorder* recorder) : fObj(new DispatchGroup()), fRecorder(recorder) {
    SkASSERT(fRecorder);
}

bool Builder::appendStep(const ComputeStep* step, const DrawParams& params, int ssboIndex) {
    SkASSERT(fObj);
    SkASSERT(step);

    Dispatch dispatch;

    // Process the step's resources.
    DrawBufferManager* bufferMgr = fRecorder->priv().drawBufferManager();

    auto resources = step->resources();
    dispatch.fBindings.reserve(resources.size());

    int nextIndex = 0;
    for (const ComputeStep::ResourceDesc& r : resources) {
        SkASSERT(r.fSlot == -1 || (r.fSlot >= 0 && r.fSlot < kMaxComputeDataFlowSlots));
        int index = nextIndex++;

        // TODO(b/259564970): Support textures
        // TODO(b/259564970): Support populating uniform buffers
        SkASSERT(r.fType == ComputeStep::ResourceType::kStorageBuffer);
        BindBufferInfo info;

        size_t bufferSize = step->calculateResourceSize(params, index, r);
        SkASSERT(bufferSize);

        using DataFlow = ComputeStep::DataFlow;
        using ResourcePolicy = ComputeStep::ResourcePolicy;
        switch (r.fFlow) {
            case DataFlow::kVertexOutput:
                // Multiple steps in a sequence are currently not allowed to output the same type of
                // geometry.
                SkASSERT(!fOutputTable.fVertexBuffer);
                info = bufferMgr->getVertexStorage(bufferSize);
                fOutputTable.fVertexBuffer = info;
                break;
            case DataFlow::kIndexOutput:
                // Multiple steps in a sequence are currently not allowed to output the same type of
                // geometry.
                SkASSERT(!fOutputTable.fIndexBuffer);
                info = bufferMgr->getIndexStorage(bufferSize);
                fOutputTable.fIndexBuffer = info;
                break;
            case DataFlow::kInstanceOutput:
                // Multiple steps in a sequence are currently not allowed to output the same type of
                // geometry.
                SkASSERT(!fOutputTable.fInstanceBuffer);
                info = bufferMgr->getVertexStorage(bufferSize);
                fOutputTable.fInstanceBuffer = info;
                break;
            case DataFlow::kIndirectDrawOutput:
                // Multiple steps in a sequence are currently not allowed to write to an indirect
                // draw buffer.
                SkASSERT(!fOutputTable.fIndirectDrawBuffer);
                info = bufferMgr->getIndirectStorage(bufferSize);
                fOutputTable.fIndirectDrawBuffer = info;
                break;
            case DataFlow::kPrivate:
                if (r.fPolicy == ResourcePolicy::kMapped) {
                    auto [ptr, bufInfo] = bufferMgr->getMappedStorage(bufferSize);
                    // Allocation failures are handled below.
                    if (ptr) {
                        step->prepareBuffer(params, ssboIndex, index, r, ptr, bufferSize);
                    }
                    info = bufInfo;
                } else {
                    info = bufferMgr->getStorage(bufferSize);
                }
                break;
            case DataFlow::kShared:
                SkASSERT(r.fSlot >= 0);
                // Allocate a new buffer only if the shared slot is empty.
                info = fOutputTable.fSharedSlots[r.fSlot];
                if (!info) {
                    if (r.fPolicy == ResourcePolicy::kMapped) {
                        auto [ptr, bufInfo] = bufferMgr->getMappedStorage(bufferSize);
                        // Allocation failures are handled below.
                        if (ptr) {
                            step->prepareBuffer(params, ssboIndex, index, r, ptr, bufferSize);
                        }
                        info = bufInfo;
                    } else {
                        info = bufferMgr->getStorage(bufferSize);
                    }
                    fOutputTable.fSharedSlots[r.fSlot] = info;
                }
                break;
        }

        if (!info) {
            SKGPU_LOG_W("Failed to allocate buffer slice for compute dispatch");
            return false;
        }
        dispatch.fBindings.push_back({static_cast<BindingIndex>(index), info});
    }

    // We need to switch pipelines if this step uses a different pipeline from the previous step.
    if (fObj->fPipelineDescs.empty() ||
        fObj->fPipelineDescs.back().uniqueID() != step->uniqueID()) {
        fObj->fPipelineDescs.push_back(ComputePipelineDesc(step));
    }

    dispatch.fPipelineIndex = fObj->fPipelineDescs.size() - 1;
    dispatch.fParams.fGlobalDispatchSize = step->calculateGlobalDispatchSize(params);
    dispatch.fParams.fLocalDispatchSize = step->localDispatchSize();

    fObj->fDispatchList.push_back(std::move(dispatch));

    return true;
}

std::unique_ptr<DispatchGroup> Builder::finalize() {
    auto obj = std::move(fObj);
    fOutputTable.reset();
    return obj;
}

}  // namespace skgpu::graphite
