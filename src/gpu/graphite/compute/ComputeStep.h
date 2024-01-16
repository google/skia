/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_ComputeStep_DEFINED
#define skgpu_graphite_compute_ComputeStep_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/gpu/graphite/ComputeTypes.h"

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace skgpu::graphite {

class UniformManager;

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
        kReadOnlyStorageBuffer,

        // An indirect buffer is a storage buffer populated by this ComputeStep to determine the
        // global dispatch size of a subsequent ComputeStep within the same DispatchGroup. The
        // contents of the buffer must be laid out according to the `IndirectDispatchArgs` struct
        // definition declared in ComputeTypes.h.
        kIndirectBuffer,

        kWriteOnlyStorageTexture,
        kReadOnlyTexture,
        kSampledTexture,
    };

    enum class ResourcePolicy {
        kNone,

        // The memory of the resource will be initialized to 0
        kClear,

        // The ComputeStep will be asked to initialize the memory on the CPU via
        // `ComputeStep::prepareStorageBuffer` or `ComputeStep::prepareUniformBuffer` prior to
        // pipeline execution. This may incur a transfer cost on platforms that do not allow buffers
        // to be mapped in shared memory.
        //
        // If multiple ComputeSteps in a DispatchGroup declare a mapped resource with the same
        // shared slot number, only the first ComputeStep in the group will receive a call to
        // prepare the buffer.
        //
        // This only has meaning for buffer resources. A resource with the `kUniformBuffer` resource
        // type must specify the `kMapped` resource policy.
        kMapped,
    };

    struct ResourceDesc final {
        ResourceType fType;
        DataFlow fFlow;
        ResourcePolicy fPolicy;

        // This field only has meaning (and must have a non-negative value) if `fFlow` is
        // `DataFlow::kShared`.
        int fSlot;

        // The SkSL variable declaration code excluding the layout and type definitions. This field
        // is ignored for a ComputeStep that supports native shader source.
        const char* fSkSL = "";

        constexpr ResourceDesc(ResourceType type,
                               DataFlow flow,
                               ResourcePolicy policy,
                               int slot = -1)
                : fType(type), fFlow(flow), fPolicy(policy), fSlot(slot) {}

        constexpr ResourceDesc(ResourceType type,
                               DataFlow flow,
                               ResourcePolicy policy,
                               int slot,
                               const char* sksl)
                : fType(type), fFlow(flow), fPolicy(policy), fSlot(slot), fSkSL(sksl) {}

        constexpr ResourceDesc(ResourceType type,
                               DataFlow flow,
                               ResourcePolicy policy,
                               const char* sksl)
                : fType(type), fFlow(flow), fPolicy(policy), fSlot(-1), fSkSL(sksl) {}
    };

    // On platforms that support late bound workgroup shared resources (e.g. Metal) a ComputeStep
    // can optionally provide a list of memory sizes and binding indices.
    struct WorkgroupBufferDesc {
        // The buffer size in bytes.
        size_t size;
        size_t index;
    };

    virtual ~ComputeStep() = default;

    // Returns a complete SkSL compute program. The returned SkSL must constitute a complete compute
    // program and declare all resource bindings starting at `nextBindingIndex` in the order in
    // which they are enumerated by `ComputeStep::resources()`.
    //
    // If this ComputeStep supports native shader source then it must override
    // `nativeShaderSource()` instead.
    virtual std::string computeSkSL() const;

    // A ComputeStep that supports native shader source then then it must implement
    // `nativeShaderSource()` and return the shader source in the requested format. This is intended
    // to instantiate a compute pipeline from a pre-compiled shader module. The returned source must
    // constitute a shader module that contains at least one compute entry-point function that
    // matches the specified name.
    enum class NativeShaderFormat {
        kWGSL,
        kMSL,
    };
    struct NativeShaderSource {
        std::string_view fSource;
        std::string fEntryPoint;
    };
    virtual NativeShaderSource nativeShaderSource(NativeShaderFormat) const;

    // This method will be called for buffer entries in the ComputeStep's resource list to
    // determine the required allocation size. The ComputeStep must return a non-zero value.
    //
    // TODO(b/279955342): Provide a context object, e.g. a type a associated with
    // DispatchGroup::Builder, to aid the ComputeStep in its buffer size calculations.
    virtual size_t calculateBufferSize(int resourceIndex, const ResourceDesc&) const;

    // This method will be called for storage texture entries in the ComputeStep's resource list to
    // determine the required dimensions and color type. The ComputeStep must return a non-zero
    // value for the size and a valid color type.
    virtual std::tuple<SkISize, SkColorType> calculateTextureParameters(int resourceIndex,
                                                                        const ResourceDesc&) const;

    // This method will be called for sampler entries in the ComputeStep's resource list to
    // determine the sampling and tile mode options.
    virtual SamplerDesc calculateSamplerParameters(int resourceIndex, const ResourceDesc&) const;

    // Return the global dispatch size (aka "workgroup count") for this step based on the draw
    // parameters. The default value is a workgroup count of (1, 1, 1)
    //
    // TODO(b/279955342): Provide a context object, e.g. a type a associated with
    // DispatchGroup::Builder, to aid the ComputeStep in its buffer size calculations.
    virtual WorkgroupSize calculateGlobalDispatchSize() const;

    // Populates a storage buffer resource which was specified as "mapped". This method will only be
    // called once for a resource right after its allocation and before pipeline execution. For
    // shared resources, only the first ComputeStep in a DispatchGroup will be asked to prepare the
    // buffer.
    //
    // `resourceIndex` matches the order in which `resource` was enumerated by
    // `ComputeStep::resources()`.
    virtual void prepareStorageBuffer(int resourceIndex,
                                      const ResourceDesc& resource,
                                      void* buffer,
                                      size_t bufferSize) const;

    // Populates a uniform buffer resource. This method will be called once for a resource right
    // after its allocation and before pipeline execution. For shared resources, only the first
    // ComputeStep in a DispatchGroup will be asked to prepare the buffer.
    //
    // `resourceIndex` matches the order in which `resource` was enumerated by
    // `ComputeStep::resources()`.
    //
    // The implementation must use the provided `UniformManager` to populate the buffer. On debug
    // builds, the implementation must validate the buffer layout by setting up an expectation, for
    // example:
    //
    //     SkDEBUGCODE(mgr->setExpectedUniforms({{"foo", SkSLType::kFloat}}));
    //
    // TODO(b/279955342): Provide a context object, e.g. a type a associated with
    // DispatchGroup::Builder, to aid the ComputeStep in its buffer size calculations.
    virtual void prepareUniformBuffer(int resourceIndex,
                                      const ResourceDesc&,
                                      UniformManager*) const;

    SkSpan<const ResourceDesc> resources() const { return SkSpan(fResources); }
    SkSpan<const WorkgroupBufferDesc> workgroupBuffers() const { return SkSpan(fWorkgroupBuffers); }

    // Identifier that can be used as part of a unique key for a compute pipeline state object
    // associated with this `ComputeStep`.
    uint32_t uniqueID() const { return fUniqueID; }

    // Returns a debug name for the subclass implementation.
    const char* name() const { return fName.c_str(); }

    // The size of the workgroup for this ComputeStep's entry point function. This value is hardware
    // dependent. On Metal, this value should be used when invoking the dispatch API call. On all
    // other backends, this value will be baked into the pipeline.
    WorkgroupSize localDispatchSize() const { return fLocalDispatchSize; }

    bool supportsNativeShader() const { return SkToBool(fFlags & Flags::kSupportsNativeShader); }

protected:
    enum class Flags : uint8_t {
        kNone                 = 0b00000,
        kSupportsNativeShader = 0b00010,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(Flags);

    ComputeStep(std::string_view name,
                WorkgroupSize localDispatchSize,
                SkSpan<const ResourceDesc> resources,
                SkSpan<const WorkgroupBufferDesc> workgroupBuffers = {},
                Flags baseFlags = Flags::kNone);

private:
    // Disallow copy and move
    ComputeStep(const ComputeStep&) = delete;
    ComputeStep(ComputeStep&&)      = delete;

    uint32_t fUniqueID;
    SkEnumBitMask<Flags> fFlags;
    std::string fName;
    skia_private::TArray<ResourceDesc> fResources;
    skia_private::TArray<WorkgroupBufferDesc> fWorkgroupBuffers;

    // TODO(b/240615224): Subclasses should simply specify the workgroup size that they need.
    // The ComputeStep constructor should check and reduce that number based on the maximum
    // supported workgroup size stored in Caps. In Metal, we'll pass this number directly to the
    // dispatch API call. On other backends, we'll use this value to generate the right SkSL
    // workgroup size declaration to avoid any validation failures.
    WorkgroupSize fLocalDispatchSize;
};
SK_MAKE_BITMASK_OPS(ComputeStep::Flags)

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_ComputeStep_DEFINED
