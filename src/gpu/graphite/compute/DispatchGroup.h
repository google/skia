/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_DispatchGroup_DEFINED
#define skgpu_graphite_compute_DispatchGroup_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/compute/ComputeStep.h"

#include <variant>

namespace skgpu::graphite {

class CommandBuffer;
class ComputePipeline;
class Recorder;
class ResourceProvider;

using BindingIndex = uint32_t;
struct TextureIndex { uint32_t fValue; };
struct SamplerIndex { uint32_t fValue; };

struct BufferView {
    BindBufferInfo fInfo;
    size_t fSize;
};

using DispatchResource = std::variant<BufferView, TextureIndex, SamplerIndex>;
using DispatchResourceOptional =
        std::variant<std::monostate, BufferView, TextureIndex, SamplerIndex>;

struct ResourceBinding {
    BindingIndex fIndex;
    DispatchResource fResource;
};

/**
 * DispatchGroup groups a series of compute pipeline dispatches that need to execute sequentially
 * (i.e. with a barrier). Dispatches are stored in the order that they will be encoded
 * in the eventual command buffer.
 *
 * A DispatchGroup can be constructed from a series of ComputeSteps using a Builder. The Builder
 * verifies that the data flow specification between successive ComputeSteps are compatible.
 * The resources required by a ComputeStep (such as Buffers and TextureProxies) are created by
 * the Builder as they get added.
 *
 * Once a DispatchGroup is finalized, it is immutable. It contains the complete ResourceBinding list
 * for each dispatch. A list of finalized DispatchGroups can be submitted to the command buffer in a
 * ComputeTask.
 */
class DispatchGroup final {
public:
    class Builder;

    struct Dispatch {
        WorkgroupSize fLocalSize;
        std::variant<WorkgroupSize, BufferView> fGlobalSizeOrIndirect;

        std::optional<WorkgroupSize> fGlobalDispatchSize;
        skia_private::TArray<ResourceBinding> fBindings;
        skia_private::TArray<ComputeStep::WorkgroupBufferDesc> fWorkgroupBuffers;
        int fPipelineIndex = 0;
    };

    ~DispatchGroup();

    const skia_private::TArray<Dispatch>& dispatches() const { return fDispatchList; }

    const ComputePipeline* getPipeline(size_t index) const { return fPipelines[index].get(); }
    const Texture* getTexture(size_t index) const;
    const Sampler* getSampler(size_t index) const;

    bool prepareResources(ResourceProvider*);
    void addResourceRefs(CommandBuffer*) const;

    // Returns a single tasks that must execute before this DispatchGroup or nullptr if the group
    // has no task dependencies.
    sk_sp<Task> snapChildTask();

private:
    friend class DispatchGroupBuilder;

    DispatchGroup() = default;

    // Disallow copy and move.
    DispatchGroup(const DispatchGroup&) = delete;
    DispatchGroup(DispatchGroup&&) = delete;

    skia_private::TArray<Dispatch> fDispatchList;

    // The list of all buffers that must be cleared before the dispatches.
    skia_private::TArray<ClearBufferInfo> fClearList;

    // Pipelines are referenced by index by each Dispatch in `fDispatchList`. They are stored as a
    // pipeline description until instantiated in `prepareResources()`.
    skia_private::TArray<ComputePipelineDesc> fPipelineDescs;
    skia_private::TArray<SamplerDesc> fSamplerDescs;

    // Resources instantiated by `prepareResources()`
    skia_private::TArray<sk_sp<ComputePipeline>> fPipelines;
    skia_private::TArray<sk_sp<TextureProxy>> fTextures;
    skia_private::TArray<sk_sp<Sampler>> fSamplers;
};

class DispatchGroup::Builder final {
public:
    // Contains the resource handles assigned to the outputs of the most recently inserted
    // ComputeStep.
    struct OutputTable {
        // Contains the std::monostate variant if the slot is uninitialized
        DispatchResourceOptional fSharedSlots[kMaxComputeDataFlowSlots];

        OutputTable() = default;

        void reset() { *this = {}; }
    };

    explicit Builder(Recorder*);

    const OutputTable& outputTable() const { return fOutputTable; }

    // Add a new compute step to the dispatch group and initialize its required resources if
    // necessary.
    //
    // If the global dispatch size (i.e. workgroup count) is known ahead of time it can be
    // optionally provided here while appending a step. If provided, the ComputeStep will not
    // receive a call to `calculateGlobalDispatchSize`.
    bool appendStep(const ComputeStep*, std::optional<WorkgroupSize> globalSize = std::nullopt);

    // Add a new compute step to the dispatch group with an indirectly specified global dispatch
    // size. Initialize the required resources if necessary.
    //
    // The global dispatch size is determined by the GPU by reading the entries in `indirectBuffer`.
    // The contents of this buffer must conform to the layout of the `IndirectDispatchArgs`
    // structure declared in ComputeTypes.h.
    //
    // The ComputeStep will not receive a call to `calculateGlobalDispatchSize`.
    bool appendStepIndirect(const ComputeStep*, BufferView indirectBuffer);

    // Directly assign a buffer range to a shared slot. ComputeSteps that are appended after this
    // call will use this resouce if they reference the given `slot` index. Builder will not
    // allocate the resource internally and ComputeSteps will not receive calls to
    // `calculateBufferSize`.
    //
    // If the slot is already assigned a buffer, it will be overwritten. Calling this method does
    // not have any effect on previously appended ComputeSteps that were already bound that
    // resource.
    //
    // If `cleared` is kYes, the contents of the given view will be cleared to 0 before the current
    // DispatchGroup gets submitted.
    void assignSharedBuffer(BufferView buffer,
                            unsigned int slot,
                            ClearBuffer cleared = ClearBuffer::kNo);

    // Directly assign a texture to a shared slot. ComputeSteps that are appended after this call
    // will use this resource if they reference the given `slot` index. Builder will not allocate
    // the resource internally and ComputeSteps will not receive calls to
    // `calculateTextureParameters`.
    //
    // If the slot is already assigned a texture, it will be overwritten. Calling this method does
    // not have any effect on previously appended ComputeSteps that were already bound that
    // resource.
    void assignSharedTexture(sk_sp<TextureProxy> texture, unsigned int slot);

    // Finalize and return the constructed DispatchGroup.
    //
    // The Builder can be used to construct a new DispatchGroup by calling "reset()" after this
    // method returns.
    std::unique_ptr<DispatchGroup> finalize();

#if defined(GRAPHITE_TEST_UTILS)
    // Clear old state and start a new DispatchGroup.
    void reset();
#endif

    // Returns the buffer resource assigned to the shared slot with the given index, if any.
    BindBufferInfo getSharedBufferResource(unsigned int slot) const;

    // Returns the texture resource assigned to the shared slot with the given index, if any.
    sk_sp<TextureProxy> getSharedTextureResource(unsigned int slot) const;

private:
    bool appendStepInternal(const ComputeStep*, const std::variant<WorkgroupSize, BufferView>&);

    // Allocate a resource that can be assigned to the shared or private data flow slots. Returns a
    // std::monostate if allocation fails.
    DispatchResourceOptional allocateResource(const ComputeStep* step,
                                              const ComputeStep::ResourceDesc& resource,
                                              int resourceIdx);

    // The object under construction.
    std::unique_ptr<DispatchGroup> fObj;

    Recorder* fRecorder;
    OutputTable fOutputTable;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_DispatchGroup_DEFINED
