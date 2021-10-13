/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypesPriv_DEFINED
#define skgpu_GraphiteTypesPriv_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"
#include "include/core/SkMath.h"

namespace skgpu {

/**
 *  align up to a power of 2
 */
static inline constexpr size_t AlignTo(size_t x, size_t alignment) {
    SkASSERT(alignment && SkIsPow2(alignment));
    return (x + alignment - 1) & ~(alignment - 1);
}

/**
 * Is the Texture renderable or not
 */
enum class Renderable : bool {
    kNo = false,
    kYes = true,
};

enum class DepthStencilType {
    kDepthOnly,
    kStencilOnly,
    kDepthStencil,
};

/**
 * What a GPU buffer will be used for
 */
enum class BufferType {
    kVertex,
    kIndex,
    kXferCpuToGpu,
    kXferGpuToCpu,
    kUniform,
};
static const int kBufferTypeCount = static_cast<int>(BufferType::kUniform) + 1;

/**
 * When creating the memory for a resource should we use a memory type that prioritizes the
 * effeciency of GPU reads even if it involves extra work to write CPU data to it. For example, we
 * would want this for buffers that we cache to read the same data many times on the GPU.
 */
enum class PrioritizeGpuReads : bool {
    kNo = false,
    kYes = true,
};

} // namespace skgpu

#endif // skgpu_GraphiteTypesPriv_DEFINED
