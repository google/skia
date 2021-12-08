/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceTypes_DEFINED
#define skgpu_ResourceTypes_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/src/EnumBitMask.h"

namespace skgpu {

/**
 * Is the Texture renderable or not
 */
enum class Renderable : bool {
    kNo = false,
    kYes = true,
};

enum class DepthStencilFlags : int {
    kNone = 0b000,
    kDepth = 0b001,
    kStencil = 0b010,
    kDepthStencil = kDepth | kStencil,
};
SKGPU_MAKE_MASK_OPS(DepthStencilFlags);

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

enum class Ownership {
    kOwned,
    kWrapped,
};

};  // namespace skgpu

#endif // skgpu_ResourceTypes_DEFINED
