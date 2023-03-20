/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DispatchGroup_DEFINED
#define skgpu_graphite_DispatchGroup_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

class CommandBuffer;
class ComputePipeline;
class ComputeStep;
class Recorder;
class ResourceProvider;

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
        ComputePassDesc fParams;
        SkTArray<ResourceBinding> fBindings;
        int fPipelineIndex = 0;
    };

    ~DispatchGroup();

    const SkTArray<Dispatch>& dispatches() const { return fDispatchList; }

    const ComputePipeline* getPipeline(size_t index) const { return fPipelines[index].get(); }

    bool prepareResources(ResourceProvider*);
    void addResourceRefs(CommandBuffer*) const;

private:
    friend class DispatchGroupBuilder;

    DispatchGroup() = default;

    // Disallow copy and move.
    DispatchGroup(const DispatchGroup&) = delete;
    DispatchGroup(DispatchGroup&&) = delete;

    SkTArray<Dispatch> fDispatchList;

    // Pipelines are referenced by index by each Dispatch in `fDispatchList`. They are stored as a
    // pipeline description until instantiated in `prepareResources()`.
    SkTArray<ComputePipelineDesc> fPipelineDescs;

    // Resources instantiated by `prepareResources()`
    SkTArray<sk_sp<ComputePipeline>> fPipelines;
};

class DispatchGroup::Builder final {
public:
    // Contains the resource handles assigned to the outputs of the most recently inserted
    // ComputeStep.
    // TODO(b/259564970): Support TextureProxy slot entries.
    struct OutputTable {
        // Draw buffers that can be forwarded to a DrawPass
        BindBufferInfo fVertexBuffer;
        BindBufferInfo fIndexBuffer;
        BindBufferInfo fInstanceBuffer;
        BindBufferInfo fIndirectDrawBuffer;

        BindBufferInfo fSharedSlots[kMaxComputeDataFlowSlots];

        OutputTable() = default;

        bool hasDrawBuffers() const {
            return fVertexBuffer || fIndexBuffer || fInstanceBuffer || fIndirectDrawBuffer;
        }

        void reset() { *this = {}; }
    };

    explicit Builder(Recorder*);

    const OutputTable& outputTable() const { return fOutputTable; }

    // Add a new compute step to the dispatch group and assign its resources. Any output resources
    // currently present in the OutputTable will be forwarded to the corresponding input slots of
    // the new ComputeStep. If the ComputeStep specifies a geometry input resource, it will be
    // prompted to populate it using the draw parameters. All other resources will be assigned
    // dynamically.
    bool appendStep(const ComputeStep*, const DrawParams&, int ssboIndex);

    // Finalize and return the constructed DispatchGroup. The Builder can be used to construct a new
    // DispatchGroup after this method returns.
    std::unique_ptr<DispatchGroup> finalize();

private:
    // The object under construction.
    std::unique_ptr<DispatchGroup> fObj;

    Recorder* fRecorder;
    OutputTable fOutputTable;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DispatchGroup_DEFINED
