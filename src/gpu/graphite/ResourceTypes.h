/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ResourceTypes_DEFINED
#define skgpu_graphite_ResourceTypes_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkEnumBitMask.h"

namespace skgpu::graphite {

class Buffer;

enum class DepthStencilFlags : int {
    kNone = 0b000,
    kDepth = 0b001,
    kStencil = 0b010,
    kDepthStencil = kDepth | kStencil,
};
SK_MAKE_BITMASK_OPS(DepthStencilFlags);

/**
 * What a GPU buffer will be used for
 */
enum class BufferType : int {
    kVertex,
    kIndex,
    kXferCpuToGpu,
    kXferGpuToCpu,
    kUniform,
    kStorage,

    // GPU-only buffer types
    kIndirect,
    kVertexStorage,
    kIndexStorage,

    kLast = kIndexStorage,
};
static const int kBufferTypeCount = static_cast<int>(BufferType::kLast) + 1;

/**
 * Data layout requirements on host-shareable buffer contents.
 */
enum class Layout {
    kInvalid = 0,
    kStd140,
    kStd430,
    kMetal,
};

/**
 * Indicates the intended access pattern over resource memory. This is used to select the most
 * efficient memory type during resource creation based on the capabilities of the platform.
 *
 * This is only a hint and the actual memory type will be determined based on the resource type and
 * backend capabilities.
 */
enum class AccessPattern : int {
    // GPU-only memory does not need to support reads/writes from the CPU. GPU-private memory will
    // be preferred if the backend supports an efficient private memory type.
    kGpuOnly,

    // The resource needs to be CPU visible, e.g. for read-back or as a copy/upload source.
    kHostVisible,
};

/**
 * Determines whether the contents of a GPU buffer sub-allocation gets cleared to 0 before being
 * used in a GPU command submission.
 */
enum class ClearBuffer : bool {
    kNo = false,
    kYes = true,
};

/**
 * Must the contents of the Resource be preserved af a render pass or can a more efficient
 * representation be chosen when supported by hardware.
 */
enum class Discardable : bool {
    kNo = false,
    kYes = true
};

enum class Ownership {
    kOwned,
    kWrapped,
};

/** Uniquely identifies the type of resource that is cached with a GraphiteResourceKey. */
using ResourceType = uint32_t;

/**
 * Can the resource be held by multiple users at the same time?
 * For example, stencil buffers, pipelines, etc.
 */
enum class Shareable : bool {
    kNo = false,
    kYes = true,
};

/**
 * This enum is used to notify the ResourceCache which type of ref just dropped to zero on a
 * Resource.
 */
enum class LastRemovedRef {
    kUsage,
    kCommandBuffer,
    kCache,
};

/*
 * Struct that can be passed into bind buffer calls on the CommandBuffer. The ownership of the
 * buffer and its usage in command submission must be tracked by the caller (e.g. as with
 * buffers created by DrawBufferManager).
 */
struct BindBufferInfo {
    const Buffer* fBuffer = nullptr;
    size_t fOffset = 0;

    operator bool() const { return SkToBool(fBuffer); }

    bool operator==(const BindBufferInfo& o) const {
        return fBuffer == o.fBuffer && (!fBuffer || fOffset == o.fOffset);
    }
    bool operator!=(const BindBufferInfo& o) const { return !(*this == o); }
};

/**
 * Represents a buffer region that should be cleared to 0. A ClearBuffersTask does not take an
 * owning reference to the buffer it clears. A higher layer is responsible for managing the lifetime
 * and usage refs of the buffer.
 */
struct ClearBufferInfo {
    const Buffer* fBuffer = nullptr;
    size_t fOffset = 0;
    size_t fSize = 0;

    operator bool() const { return SkToBool(fBuffer); }
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceTypes_DEFINED
