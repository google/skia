/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeStep_DEFINED
#define skgpu_graphite_ComputeStep_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkEnumBitMask.h"
#include "src/gpu/graphite/ComputeTypes.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace skgpu::graphite {

class DrawParams;
struct ResourceBindingRequirements;

/**
 * A `ComputeStep` represents a compute pass within a wider draw operation. A `ComputeStep`
 * implementation describes an invocation of a compute program and its data binding layout.
 *
 * A `ComputeStep` can perform arbitrary operations on the GPU over various types of data, including
 * geometry and image processing. The data processed by a `ComputeStep` can be inputs (textures or
 * buffers) populated on the CPU, data forwarded to and from other `ComputeStep` invocations (via
 * "slots"), transient storage buffers/textures that are only used within an individual dispatch,
 * geometry attribute (vertex/index/instance) and indirect draw parameters of a subsequent raster
 * pipeline stage, as well as texture outputs.
 *
 * The data flow between sequential `ComputeStep` invocations within a DispatchGroup is achieved by
 * operating over a shared "resource table". `ComputeStep`s can declare a resource with a slot
 * number. Multiple `ComputeStep`s in a group that declare a resource with the same slot number will
 * have access to the same backing resource object through that slot:
 *
 *      _______________                _______________
 *     |               |              |               |
 *     |                ---[Slot 0]---                |
 *     |               |              |               |
 *     |                ---[Slot 1]---                |
 *     | ComputeStep 1 |              | ComputeStep 2 |
 *     |                ---[Slot 2]   |               |
 *     |               |              |               |
 *     |               |   [Slot 3]---                |
 *     |               |              |               |
 *      ---------------                ---------------
 *
 * In the example above, slots 0 and 1 are accessed by both ComputeSteps, while slots 2 and 3 are
 * exclusively accessed by ComputeStep 1 and 2 respectively. Alternately, slots 2 and 3 could be
 * declared as "private" resources which are visible to a single ComputeStep.
 *
 * Similarly, raster stage geometry buffers that are specified as the output of a ComputeStep can be
 * used to assign the draw buffers of a RenderStep.
 *
 * It is the responsibility of the owning entity (e.g. a RendererProvider) to ensure that a chain of
 * ComputeStep and RenderStep invocations have a compatible resource and data-flow layout.
 */
class ComputeStep {
public:
    enum class DataFlow {
        // A set of writable Buffer bindings that the `ComputeStep` will write vertex and instance
        // attributes to. If present, these buffers can be used to encode the draw command for a
        // subsequent `RenderStep`.
        kVertexOutput,
        kIndexOutput,
        kInstanceOutput,
        kIndirectDrawOutput,

        // A private binding is a resource that is only visible to a single ComputeStep invocation.
        kPrivate,

        // Bindings with a slot number that can be used to forward data between a series of
        // `ComputeStep`s. This DataFlow type is accompanied with a "slot number" that can be
        // shared by multiple `ComputeStep`s in a group.
        kShared,
    };

    enum class ResourceType {
        kUniformBuffer,
        kStorageBuffer,

        // TODO(b/238794438): Support sampled and storage texture types.
    };

    enum class ResourcePolicy {
        kNone,

        // The memory of the resource will be initialized to 0
        kClear,

        // The ComputeStep will be asked to initialize the memory on the CPU via
        // `ComputeStep::prepareBuffer` prior to pipeline execution. This may incur a transfer cost
        // on platforms that do not allow buffers to be mapped in shared memory.
        //
        // If multiple ComputeSteps in a DispatchGroup declare a mapped resource with the same
        // shared slot number, only the first ComputeStep in the series will receive a call to
        // `ComputeStep::prepareBuffer`.
        kMapped,
    };

    struct ResourceDesc final {
        ResourceType fType;
        DataFlow fFlow;
        ResourcePolicy fPolicy;

        // This field only has meaning (and must have a non-negative value) if `fFlow` is
        // `DataFlow::kShared`.
        int fSlot = -1;

        constexpr ResourceDesc(ResourceType type,
                               DataFlow flow,
                               ResourcePolicy policy,
                               int slot = -1)
                : fType(type), fFlow(flow), fPolicy(policy), fSlot(slot) {}
    };

    virtual ~ComputeStep() = default;

    // Returns a complete SkSL compute program. The returned SkSL must declare all resoure bindings
    // starting at `nextBindingIndex` in the order in which they are enumerated by
    // `ComputeStep::resources()`.
    virtual std::string computeSkSL(const ResourceBindingRequirements&,
                                    int nextBindingIndex) const = 0;

    // This method will be called for entries in the ComputeStep's resource list to determine the
    // required allocation sizes. The ComputeStep should return the minimum allocation size for the
    // resource.
    //
    // TODO(armansito): The only piece of information that the ComputeStep currently uses to make
    // this determination is the draw parameters. This approach particularly doesn't address (and
    // likely needs to be reworked) for intermediate ComputeSteps in a chain of invocations, where
    // the effective data sizes may not be known on the CPU.
    //
    // For now, we assume that there will be a strict data contract between chained ComputeSteps.
    // The buffer sizes are an estimate based on the DrawParams. This is generic enough to allow
    // different schemes (such as dynamic allocations and buffer pools) but may not be easily
    // validated on the CPU.
    virtual size_t calculateResourceSize(const DrawParams&,
                                         int resourceIndex,
                                         const ResourceDesc&) const {
        return 0u;
    }

    // Return the global dispatch size (aka "workgroup count") for this step based on the draw
    // parameters. The default value is a workgroup count of (1, 1, 1)
    //
    // TODO(armansito): The only piece of information that the ComputeStep currently gets to make
    // this determination is the draw parameters. There might be other inputs to this calculation
    // for intermediate compute stages that may not be known on the CPU. One way to address this is
    // to drive the workgroup dimensions via an indirect dispatch.
    virtual WorkgroupSize calculateGlobalDispatchSize(const DrawParams&) const {
        return WorkgroupSize();
    }

    // Populates a buffer resource which was specified as "mapped". This method will only be called
    // once for a resource right after its allocation and before pipeline execution. For shared
    // resources, only the first ComputeStep in a DispatchGroup will be asked to prepare the buffer.
    //
    // `resourceIndex` matches the order in which `resource` was enumerated by
    // `ComputeStep::resources()`.
    virtual void prepareBuffer(const DrawParams&,
                               int ssboIndex,
                               int resourceIndex,
                               const ResourceDesc& resource,
                               void* buffer,
                               size_t bufferSize) const;

    SkSpan<const ResourceDesc> resources() const { return SkSpan(fResources); }

    // Identifier that can be used as part of a unique key for a compute pipeline state object
    // associated with this `ComputeStep`.
    uint32_t uniqueID() const { return fUniqueID; }

    // Returns a debug name for the subclass implementation.
    const char* name() const { return fName.c_str(); }

    // The size of the workgroup for this ComputeStep's entry point function. This value is hardware
    // dependent. On Metal, this value should be used when invoking the dispatch API call. On all
    // other backends, this value will be baked into the pipeline.
    WorkgroupSize localDispatchSize() const { return fLocalDispatchSize; }

    // Data flow behavior queries:
    bool outputsVertices() const { return fFlags & Flags::kOutputsVertexBuffer; }
    bool outputsIndices() const { return fFlags & Flags::kOutputsIndexBuffer; }
    bool outputsInstances() const { return fFlags & Flags::kOutputsInstanceBuffer; }
    bool writesIndirectDraw() const { return fFlags & Flags::kOutputsIndirectDrawBuffer; }

protected:
    ComputeStep(std::string_view name,
                WorkgroupSize localDispatchSize,
                SkSpan<const ResourceDesc> resources);

private:
    enum class Flags : uint8_t {
        kNone                      = 0b0000,
        kOutputsVertexBuffer       = 0b0001,
        kOutputsIndexBuffer        = 0b0010,
        kOutputsInstanceBuffer     = 0b0100,
        kOutputsIndirectDrawBuffer = 0b1000,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(Flags);

    // Disallow copy and move
    ComputeStep(const ComputeStep&) = delete;
    ComputeStep(ComputeStep&&)      = delete;

    uint32_t fUniqueID;
    SkEnumBitMask<Flags> fFlags;
    std::string fName;
    std::vector<ResourceDesc> fResources;

    // TODO(b/240615224): Subclasses should simply specify the workgroup size that they need.
    // The ComputeStep constructor should check and reduce that number based on the maximum
    // supported workgroup size stored in Caps. In Metal, we'll pass this number directly to the
    // dispatch API call. On other backends, we'll use this value to generate the right SkSL
    // workgroup size declaration to avoid any validation failures.
    WorkgroupSize fLocalDispatchSize;
};
SK_MAKE_BITMASK_OPS(ComputeStep::Flags);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputeStep_DEFINED
