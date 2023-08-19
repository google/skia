/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/DispatchGroup.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UniformManager.h"

namespace skgpu::graphite {

DispatchGroup::~DispatchGroup() = default;

bool DispatchGroup::prepareResources(ResourceProvider* resourceProvider) {
    fPipelines.reserve(fPipelines.size() + fPipelineDescs.size());
    for (const ComputePipelineDesc& desc : fPipelineDescs) {
        auto pipeline = resourceProvider->findOrCreateComputePipeline(desc);
        if (!pipeline) {
            SKGPU_LOG_W("Failed to create ComputePipeline for dispatch group. Dropping group!");
            return false;
        }
        fPipelines.push_back(std::move(pipeline));
    }

    for (int i = 0; i < fTextures.size(); ++i) {
        if (!fTextures[i]->textureInfo().isValid()) {
            SKGPU_LOG_W("Failed to validate bound texture. Dropping dispatch group!");
            return false;
        }
        if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fTextures[i].get())) {
            SKGPU_LOG_W("Failed to instantiate bound texture. Dropping dispatch group!");
            return false;
        }
    }

    for (const SamplerDesc& desc : fSamplerDescs) {
        sk_sp<Sampler> sampler = resourceProvider->findOrCreateCompatibleSampler(
                desc.samplingOptions(), desc.tileModeX(), desc.tileModeY());
        if (!sampler) {
            SKGPU_LOG_W("Failed to create sampler. Dropping dispatch group!");
            return false;
        }
        fSamplers.push_back(std::move(sampler));
    }

    // The DispatchGroup may be long lived on a Recording and we no longer need the descriptors
    // once we've created pipelines.
    fPipelineDescs.clear();
    fSamplerDescs.clear();

    return true;
}

void DispatchGroup::addResourceRefs(CommandBuffer* commandBuffer) const {
    for (int i = 0; i < fPipelines.size(); ++i) {
        commandBuffer->trackResource(fPipelines[i]);
    }
    for (int i = 0; i < fTextures.size(); ++i) {
        commandBuffer->trackResource(fTextures[i]->refTexture());
    }
}

const Texture* DispatchGroup::getTexture(size_t index) const {
    SkASSERT(index < SkToSizeT(fTextures.size()));
    SkASSERT(fTextures[index]);
    SkASSERT(fTextures[index]->texture());
    return fTextures[index]->texture();
}

const Sampler* DispatchGroup::getSampler(size_t index) const {
    SkASSERT(index < SkToSizeT(fSamplers.size()));
    SkASSERT(fSamplers[index]);
    return fSamplers[index].get();
}

using Builder = DispatchGroup::Builder;

Builder::Builder(Recorder* recorder) : fObj(new DispatchGroup()), fRecorder(recorder) {
    SkASSERT(fRecorder);
}

bool Builder::appendStep(const ComputeStep* step, std::optional<WorkgroupSize> globalSize) {
    SkASSERT(fObj);
    SkASSERT(step);

    Dispatch dispatch;

    // Process the step's resources.
    auto resources = step->resources();
    dispatch.fBindings.reserve(resources.size());

    // `nextIndex` matches the declaration order of resources as specified by the ComputeStep.
    int nextIndex = 0;

    // We assign buffer, texture, and sampler indices from separate ranges. This is compatible with
    // how Graphite assigns indices on Metal, as these map directly to the buffer/texture/sampler
    // index ranges. On Dawn/Vulkan buffers and textures/samplers are allocated from separate bind
    // groups/descriptor sets but texture and sampler indices need to not overlap.
    //
    // TODO(armansito): Count the indices based on
    // `ResourceBindingRequirements::fDistinctIndexRanges` obtained from Caps.
    //
    // TODO(armansito): The Metal backend index binding scheme happens to be compatible with the
    // vello_shaders crate's WGSL->MSL translation (see
    // https://github.com/linebender/vello/blob/main/crates/shaders/src/compile/msl.rs#L10).
    // However, Vello's WGSL shaders assign all resources to the same bind group (at index 0) which
    // differs from how Graphite binds textures and samplers (at index 1). We can handle this by
    // having ComputeStep resources define a bind group index explicitly and assigning them to the
    // specified bind group during command encoding.
    int bufferIndex = 0;
    int texIndex = 0;
    int samplerIndex = 0;
    for (const ComputeStep::ResourceDesc& r : resources) {
        SkASSERT(r.fSlot == -1 || (r.fSlot >= 0 && r.fSlot < kMaxComputeDataFlowSlots));
        int index = nextIndex++;

        DispatchResourceOptional maybeResource;

        using DataFlow = ComputeStep::DataFlow;
        switch (r.fFlow) {
            case DataFlow::kPrivate:
                maybeResource = this->allocateResource(step, r, index);
                break;
            case DataFlow::kShared: {
                // TODO: Support allocating a scratch texture
                SkASSERT(r.fSlot >= 0);
                // Allocate a new buffer only if the shared slot is empty.
                DispatchResourceOptional* slot = &fOutputTable.fSharedSlots[r.fSlot];
                if (std::holds_alternative<std::monostate>(*slot)) {
                    maybeResource = this->allocateResource(step, r, index);
                    *slot = maybeResource;
                } else {
                    SkDEBUGCODE(using Type = ComputeStep::ResourceType;) SkASSERT(
                            (r.fType == Type::kStorageBuffer &&
                             std::holds_alternative<BufferView>(*slot)) ||
                            ((r.fType == Type::kTexture || r.fType == Type::kStorageTexture) &&
                             std::holds_alternative<TextureIndex>(*slot)));
                    maybeResource = *slot;
                }
                break;
            }
        }

        int bindingIndex = 0;
        DispatchResource dispatchResource;
        if (const BufferView* buffer = std::get_if<BufferView>(&maybeResource)) {
            dispatchResource = *buffer;
            bindingIndex = bufferIndex++;
        } else if (const TextureIndex* texIdx = std::get_if<TextureIndex>(&maybeResource)) {
            dispatchResource = *texIdx;
            bindingIndex = texIndex++;
        } else if (const SamplerIndex* samplerIdx = std::get_if<SamplerIndex>(&maybeResource)) {
            dispatchResource = *samplerIdx;
            bindingIndex = samplerIndex++;
        } else {
            SKGPU_LOG_W("Failed to allocate resource for compute dispatch");
            return false;
        }
        dispatch.fBindings.push_back({static_cast<BindingIndex>(bindingIndex), dispatchResource});
    }

    auto wgBufferDescs = step->workgroupBuffers();
    if (!wgBufferDescs.empty()) {
        dispatch.fWorkgroupBuffers.push_back_n(wgBufferDescs.size(), wgBufferDescs.data());
    }

    // We need to switch pipelines if this step uses a different pipeline from the previous step.
    if (fObj->fPipelineDescs.empty() ||
        fObj->fPipelineDescs.back().uniqueID() != step->uniqueID()) {
        fObj->fPipelineDescs.push_back(ComputePipelineDesc(step));
    }

    dispatch.fPipelineIndex = fObj->fPipelineDescs.size() - 1;
    dispatch.fParams.fGlobalDispatchSize =
            globalSize ? *globalSize : step->calculateGlobalDispatchSize();
    dispatch.fParams.fLocalDispatchSize = step->localDispatchSize();

    fObj->fDispatchList.push_back(std::move(dispatch));

    return true;
}

void Builder::assignSharedBuffer(BufferView buffer, unsigned int slot) {
    SkASSERT(fObj);
    SkASSERT(buffer.fInfo);
    SkASSERT(buffer.fSize);

    fOutputTable.fSharedSlots[slot] = buffer;
}

void Builder::assignSharedTexture(sk_sp<TextureProxy> texture, unsigned int slot) {
    SkASSERT(fObj);
    SkASSERT(texture);

    fObj->fTextures.push_back(std::move(texture));
    fOutputTable.fSharedSlots[slot] = TextureIndex{fObj->fTextures.size() - 1u};
}

std::unique_ptr<DispatchGroup> Builder::finalize() {
    auto obj = std::move(fObj);
    fOutputTable.reset();
    return obj;
}

BindBufferInfo Builder::getSharedBufferResource(unsigned int slot) const {
    SkASSERT(fObj);

    BindBufferInfo info;
    if (const BufferView* slotValue = std::get_if<BufferView>(&fOutputTable.fSharedSlots[slot])) {
        info = slotValue->fInfo;
    }
    return info;
}

sk_sp<TextureProxy> Builder::getSharedTextureResource(unsigned int slot) const {
    SkASSERT(fObj);

    const TextureIndex* idx = std::get_if<TextureIndex>(&fOutputTable.fSharedSlots[slot]);
    if (!idx) {
        return nullptr;
    }

    SkASSERT(idx->fValue < SkToSizeT(fObj->fTextures.size()));
    return fObj->fTextures[idx->fValue];
}

DispatchResourceOptional Builder::allocateResource(const ComputeStep* step,
                                                   const ComputeStep::ResourceDesc& resource,
                                                   int resourceIdx) {
    SkASSERT(step);
    using Type = ComputeStep::ResourceType;
    using ResourcePolicy = ComputeStep::ResourcePolicy;

    DrawBufferManager* bufferMgr = fRecorder->priv().drawBufferManager();
    DispatchResourceOptional result;
    switch (resource.fType) {
        case Type::kStorageBuffer: {
            size_t bufferSize = step->calculateBufferSize(resourceIdx, resource);
            SkASSERT(bufferSize);
            if (resource.fPolicy == ResourcePolicy::kMapped) {
                auto [ptr, bufInfo] = bufferMgr->getStoragePointer(bufferSize);
                if (ptr) {
                    step->prepareStorageBuffer(resourceIdx, resource, ptr, bufferSize);
                    result = BufferView{bufInfo, bufferSize};
                }
            } else {
                auto bufInfo = bufferMgr->getStorage(bufferSize,
                                                     resource.fPolicy == ResourcePolicy::kClear
                                                             ? ClearBuffer::kYes
                                                             : ClearBuffer::kNo);
                if (bufInfo) {
                    result = BufferView{bufInfo, bufferSize};
                }
            }
            break;
        }
        case Type::kUniformBuffer: {
            SkASSERT(resource.fPolicy == ResourcePolicy::kMapped);

            const auto& resourceReqs = fRecorder->priv().caps()->resourceBindingRequirements();
            UniformManager uboMgr(resourceReqs.fUniformBufferLayout);
            step->prepareUniformBuffer(resourceIdx, resource, &uboMgr);

            auto dataBlock = uboMgr.finishUniformDataBlock();
            SkASSERT(dataBlock.size());

            auto [writer, bufInfo] = bufferMgr->getUniformWriter(dataBlock.size());
            if (bufInfo) {
                writer.write(dataBlock.data(), dataBlock.size());
                result = BufferView{bufInfo, dataBlock.size()};
            }
            break;
        }
        case Type::kStorageTexture: {
            auto [size, colorType] = step->calculateTextureParameters(resourceIdx, resource);
            SkASSERT(!size.isEmpty());
            SkASSERT(colorType != kUnknown_SkColorType);

            sk_sp<TextureProxy> texture = TextureProxy::MakeStorage(
                    fRecorder->priv().caps(), size, colorType, skgpu::Budgeted::kYes);
            if (texture) {
                fObj->fTextures.push_back(std::move(texture));
                result = TextureIndex{fObj->fTextures.size() - 1u};
            }
            break;
        }
        case Type::kTexture:
            // This resource type is meant to be populated externally (e.g. by an upload or a render
            // pass) and only read/sampled by a ComputeStep. It's not meaningful to allocate an
            // internal texture for a DispatchGroup if none of the ComputeSteps will write to it.
            //
            // Instead of using internal allocation, this texture must be assigned explicitly to a
            // slot by calling the Builder::assignSharedTexture() method.
            //
            // Note: A ComputeStep is allowed to read/sample from a storage texture that a previous
            // ComputeStep has written to.
            SK_ABORT("a sampled texture must be externally assigned to a ComputeStep");
            break;
        case Type::kSampler: {
            fObj->fSamplerDescs.push_back(step->calculateSamplerParameters(resourceIdx, resource));
            result = SamplerIndex{fObj->fSamplerDescs.size() - 1u};
            break;
        }
    }
    return result;
}

}  // namespace skgpu::graphite
