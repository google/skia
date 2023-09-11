/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeTypes_DEFINED
#define skgpu_graphite_ComputeTypes_DEFINED

#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

// The maximum number of shared resource binding slots permitted for ComputeSteps of a DispatchGroup
constexpr int kMaxComputeDataFlowSlots = 25;

/**
 * Defines the space that a compute shader operates on. A problem space is logically divided into
 * abstract "work groups" (or "thread groups" in Metal/D3D12).
 *
 * The "work group count" or "global size" of the work group is a 3-dimensional number that defines
 * the size of the problem space. The user must provide the global size to define the number of
 * work groups that execute as part of a dispatch.
 *
 * The local size of a work group defines the number of parallel execution units that run in that
 * group. The local group size is defined in terms of the "raw number of threads" that run within
 * the group.
 *
 * A local group is further divided into fixed-sized SIMD units called "subgroups" (in Vulkan
 * terminology - these are referred to as "SIMD groups"/"threads" in Metal, "wavefronts" in OpenCL,
 * "warps" in CUDA).
 *
 * The local size is defined in 3 dimensions and must be determined based on hardware limitations,
 * which can be queried via Caps::maxComputeWorkgroupSize() (for each individual dimension) and
 * Caps::maxComputeInvocationsPerWorkgroup().
 *
 * The WorkgroupSize type is used to represent both global size and local size.
 */
struct WorkgroupSize {
    WorkgroupSize() = default;
    WorkgroupSize(uint32_t width, uint32_t height, uint32_t depth)
            : fWidth(width)
            , fHeight(height)
            , fDepth(depth) {}

    uint32_t scalarSize() const { return fWidth * fHeight * fDepth; }

    uint32_t fWidth = 1;
    uint32_t fHeight = 1;
    uint32_t fDepth = 1;
};

struct ComputePassDesc {
    WorkgroupSize fGlobalDispatchSize;

    // TODO(b/240615224): On OpenGL D3D, and Vulkan 1.0, the local work group size is expressed with
    // literals in the shading language and is tied to the pipeline state. On those platforms, we
    // could either:
    //
    //     1. Defer pipeline creation until `fLocalDispatchSize` is known for a particular compute
    //     pass and build the shader text using this value, or
    //     2. Hard-code reasonable defaults within GPU capabilities and disregard
    //     `fLocalDispatchSize`.
    //
    // The local size is a function of both the number of supported hardware threads AND how the
    // problem is divided between the global and local sizes. Which approach is more optimal depends
    // on the problem.
    WorkgroupSize fLocalDispatchSize;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputeTypes_DEFINED
