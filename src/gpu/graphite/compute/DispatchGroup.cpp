/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/DispatchGroup.h"

#include "include/core/SkColorType.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/task/ClearBuffersTask.h"

#include <utility>

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
        sk_sp<Sampler> sampler = resourceProvider->findOrCreateCompatibleSampler(desc);
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
        commandBuffer->trackCommandBufferResource(fTextures[i]->refTexture());
    }
}

sk_sp<Task> DispatchGroup::snapChildTask() {
    if (fClearList.empty()) {
        return nullptr;
    }
    return ClearBuffersTask::Make(std::move(fClearList));
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
    return this->appendStepInternal(step,
                                    globalSize ? *globalSize : step->calculateGlobalDispatchSize());
}

bool Builder::appendStepIndirect(const ComputeStep* step, BindBufferInfo indirectBuffer) {
    return this->appendStepInternal(step, indirectBuffer);
}

bool Builder::appendStepInternal(
        const ComputeStep* step,
        const std::variant<WorkgroupSize, BindBufferInfo>& globalSizeOrIndirect) {
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
    const auto& bindingReqs = fRecorder->priv().caps()->resourceBindingRequirements();
    const bool separateSampler = bindingReqs.fSeparateTextureAndSamplerBinding;
    const bool texturesUseDistinctIdxRanges = bindingReqs.fComputeUsesDistinctIdxRangesForTextures;
    // Some binding index determination logic relies upon the fact that we do not expect to
    // encounter a backend that both uses separate sampler bindings AND requires separate index
    // ranges for textures.
    SkASSERT(!(separateSampler && texturesUseDistinctIdxRanges));

    int bufferOrGlobalIndex = 0;
    int texIndex = 0;
    // NOTE: SkSL Metal codegen always assigns the same binding index to a texture and its sampler.
    // TODO: This could cause sampler indices to not be tightly packed if the sampler2D declaration
    // comes after 1 or more storage texture declarations (which don't have samplers).
    for (const ComputeStep::ResourceDesc& r : resources) {
        SkASSERT(r.fSlot == -1 || (r.fSlot >= 0 && r.fSlot < kMaxComputeDataFlowSlots));
        const int index = nextIndex++;

        DispatchResourceOptional maybeResource;

        using DataFlow = ComputeStep::DataFlow;
        using Type = ComputeStep::ResourceType;
        switch (r.fFlow) {
            case DataFlow::kPrivate:
                // A sampled or fetched-type readonly texture must either get assigned via
                // `assignSharedTexture()` or internally allocated as a storage texture of a
                // preceding step. Such a texture always has a data slot.
                SkASSERT(r.fType != Type::kReadOnlyTexture);
                SkASSERT(r.fType != Type::kSampledTexture);
                maybeResource = this->allocateResource(step, r, index);
                break;
            case DataFlow::kShared: {
                SkASSERT(r.fSlot >= 0);
                // Allocate a new resource only if the shared slot is empty (except for a
                // SampledTexture which needs its sampler to be allocated internally).
                DispatchResourceOptional* slot = &fOutputTable.fSharedSlots[r.fSlot];
                if (std::holds_alternative<std::monostate>(*slot)) {
                    SkASSERT(r.fType != Type::kReadOnlyTexture);
                    SkASSERT(r.fType != Type::kSampledTexture);
                    maybeResource = this->allocateResource(step, r, index);
                    *slot = maybeResource;
                } else {
                    SkASSERT(((r.fType == Type::kUniformBuffer ||
                               r.fType == Type::kStorageBuffer ||
                               r.fType == Type::kReadOnlyStorageBuffer ||
                               r.fType == Type::kIndirectBuffer) &&
                              std::holds_alternative<BindBufferInfo>(*slot)) ||
                             ((r.fType == Type::kReadOnlyTexture ||
                               r.fType == Type::kSampledTexture ||
                               r.fType == Type::kWriteOnlyStorageTexture) &&
                              std::holds_alternative<TextureIndex>(*slot)));
#ifdef SK_DEBUG
                    // Ensure that the texture has the right format if it was assigned via
                    // `assignSharedTexture()`.
                    const TextureIndex* texIdx = std::get_if<TextureIndex>(slot);
                    if (texIdx && r.fType == Type::kWriteOnlyStorageTexture) {
                        const TextureProxy* t = fObj->fTextures[texIdx->fValue].get();
                        SkASSERT(t);
                        auto [_, colorType] = step->calculateTextureParameters(index, r);
                        SkASSERT(t->textureInfo().canBeFulfilledBy(
                                fRecorder->priv().caps()->getDefaultStorageTextureInfo(colorType)));
                    }
#endif  // SK_DEBUG

                    maybeResource = *slot;

                    if (r.fType == Type::kSampledTexture) {
                        // The shared slot holds the texture part of the sampled texture but we
                        // still need to allocate the sampler.
                        SkASSERT(std::holds_alternative<TextureIndex>(*slot));
                        auto samplerResource = this->allocateResource(step, r, index);
                        const SamplerIndex* samplerIdx =
                                std::get_if<SamplerIndex>(&samplerResource);
                        SkASSERT(samplerIdx);
                        int bindingIndex = texturesUseDistinctIdxRanges ? texIndex :
                                                        separateSampler ? bufferOrGlobalIndex++
                                                                        : bufferOrGlobalIndex;
                        dispatch.fBindings.push_back(
                                {static_cast<BindingIndex>(bindingIndex), *samplerIdx});
                    }
                }
                break;
            }
        }

        int bindingIndex = 0;
        DispatchResource dispatchResource;
        if (const BindBufferInfo* buffer = std::get_if<BindBufferInfo>(&maybeResource)) {
            dispatchResource = *buffer;
            bindingIndex = bufferOrGlobalIndex++;
        } else if (const TextureIndex* texIdx = std::get_if<TextureIndex>(&maybeResource)) {
            dispatchResource = *texIdx;
            bindingIndex = texturesUseDistinctIdxRanges ? texIndex++ : bufferOrGlobalIndex++;
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
    dispatch.fLocalSize = step->localDispatchSize();
    dispatch.fGlobalSizeOrIndirect = globalSizeOrIndirect;

    fObj->fDispatchList.push_back(std::move(dispatch));

    return true;
}

void Builder::assignSharedBuffer(BindBufferInfo buffer, unsigned int slot, ClearBuffer cleared) {
    SkASSERT(fObj);
    SkASSERT(buffer);
    SkASSERT(buffer.fSize);

    fOutputTable.fSharedSlots[slot] = buffer;
    if (cleared == ClearBuffer::kYes) {
        fObj->fClearList.push_back(buffer);
    }
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

#if defined(GPU_TEST_UTILS)
void Builder::reset() {
    fOutputTable.reset();
    fObj.reset(new DispatchGroup);
}
#endif

BindBufferInfo Builder::getSharedBufferResource(unsigned int slot) const {
    SkASSERT(fObj);

    BindBufferInfo info;
    if (const BindBufferInfo* slotValue =
                std::get_if<BindBufferInfo>(&fOutputTable.fSharedSlots[slot])) {
        info = *slotValue;
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
    SkASSERT(fObj);
    using Type = ComputeStep::ResourceType;
    using ResourcePolicy = ComputeStep::ResourcePolicy;

    DrawBufferManager* bufferMgr = fRecorder->priv().drawBufferManager();
    DispatchResourceOptional result;
    switch (resource.fType) {
        case Type::kReadOnlyStorageBuffer:
        case Type::kStorageBuffer: {
            size_t bufferSize = step->calculateBufferSize(resourceIdx, resource);
            SkASSERT(bufferSize);
            if (resource.fPolicy == ResourcePolicy::kMapped) {
                auto [ptr, bufInfo] = bufferMgr->getStoragePointer(bufferSize);
                if (ptr) {
                    step->prepareStorageBuffer(resourceIdx, resource, ptr, bufferSize);
                    result = bufInfo;
                }
            } else {
                auto bufInfo = bufferMgr->getStorage(bufferSize,
                                                     resource.fPolicy == ResourcePolicy::kClear
                                                             ? ClearBuffer::kYes
                                                             : ClearBuffer::kNo);
                if (bufInfo) {
                    result = bufInfo;
                }
            }
            break;
        }
        case Type::kIndirectBuffer: {
            SkASSERT(resource.fPolicy != ResourcePolicy::kMapped);

            size_t bufferSize = step->calculateBufferSize(resourceIdx, resource);
            SkASSERT(bufferSize);
            auto bufInfo = bufferMgr->getIndirectStorage(bufferSize,
                                                         resource.fPolicy == ResourcePolicy::kClear
                                                                 ? ClearBuffer::kYes
                                                                 : ClearBuffer::kNo);
            if (bufInfo) {
                result = bufInfo;
            }
            break;
        }
        case Type::kUniformBuffer: {
            SkASSERT(resource.fPolicy == ResourcePolicy::kMapped);

            const auto& resourceReqs = fRecorder->priv().caps()->resourceBindingRequirements();
            UniformManager uboMgr(resourceReqs.fUniformBufferLayout);
            step->prepareUniformBuffer(resourceIdx, resource, &uboMgr);

            auto dataBlock = uboMgr.finish();
            SkASSERT(!dataBlock.empty());

            auto [writer, bufInfo] = bufferMgr->getUniformWriter(/*count=*/1, dataBlock.size());
            if (bufInfo) {
                writer.write(dataBlock.data(), dataBlock.size());
                result = bufInfo;
            }
            break;
        }
        case Type::kWriteOnlyStorageTexture: {
            auto [size, colorType] = step->calculateTextureParameters(resourceIdx, resource);
            SkASSERT(!size.isEmpty());
            SkASSERT(colorType != kUnknown_SkColorType);

            auto textureInfo = fRecorder->priv().caps()->getDefaultStorageTextureInfo(colorType);
            sk_sp<TextureProxy> texture = TextureProxy::Make(
                    fRecorder->priv().caps(), fRecorder->priv().resourceProvider(),
                    size, textureInfo, "DispatchWriteOnlyStorageTexture", skgpu::Budgeted::kYes);
            if (texture) {
                fObj->fTextures.push_back(std::move(texture));
                result = TextureIndex{fObj->fTextures.size() - 1u};
            }
            break;
        }
        case Type::kReadOnlyTexture:
            // This resource type is meant to be populated externally (e.g. by an upload or a render
            // pass) and only read/sampled by a ComputeStep. It's not meaningful to allocate an
            // internal texture for a DispatchGroup if none of the ComputeSteps will write to it.
            //
            // Instead of using internal allocation, this texture must be assigned explicitly to a
            // slot by calling the Builder::assignSharedTexture() method.
            //
            // Note: A ComputeStep is allowed to read/sample from a storage texture that a previous
            // ComputeStep has written to.
            SK_ABORT("a readonly texture must be externally assigned to a ComputeStep");
            break;
        case Type::kSampledTexture: {
            fObj->fSamplerDescs.push_back(step->calculateSamplerParameters(resourceIdx, resource));
            result = SamplerIndex{fObj->fSamplerDescs.size() - 1u};
            break;
        }
    }
    return result;
}

}  // namespace skgpu::graphite
