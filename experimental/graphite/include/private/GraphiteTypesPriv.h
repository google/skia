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

/**
 * Types of shader-language-specific boxed variables we can create.
 */
enum class SLType {
    kVoid,
    kBool,
    kBool2,
    kBool3,
    kBool4,
    kByte,
    kByte2,
    kByte3,
    kByte4,
    kUByte,
    kUByte2,
    kUByte3,
    kUByte4,
    kShort,
    kShort2,
    kShort3,
    kShort4,
    kUShort,
    kUShort2,
    kUShort3,
    kUShort4,
    kFloat,
    kFloat2,
    kFloat3,
    kFloat4,
    kFloat2x2,
    kFloat3x3,
    kFloat4x4,
    kHalf,
    kHalf2,
    kHalf3,
    kHalf4,
    kHalf2x2,
    kHalf3x3,
    kHalf4x4,
    kInt,
    kInt2,
    kInt3,
    kInt4,
    kUint,
    kUint2,
    kUint3,
    kUint4,
    kTexture2DSampler,
    kTextureExternalSampler,
    kTexture2DRectSampler,
    kTexture2D,
    kSampler,
    kInput,

    kLast = kInput
};

enum class CType : unsigned {
    // Any float/half, vector of floats/half, or matrices of floats/halfs are a tightly
    // packed array of floats. Similarly, any bool/shorts/ints are a tightly packed array
    // of int32_t.
    kDefault,
    // Can be used with kFloat3x3 or kHalf3x3
    kSkMatrix,

    kLast = kSkMatrix
};

/**
 * This enum is used to specify the load operation to be used when a RenderPass begins execution
 */
enum class LoadOp : uint8_t {
    kLoad,
    kClear,
    kDiscard,

    kLast = kDiscard
};
inline static constexpr int kLoadOpCount = (int)(LoadOp::kLast) + 1;

/**
 * This enum is used to specify the store operation to be used when a RenderPass ends execution.
 */
enum class StoreOp : uint8_t {
    kStore,
    kDiscard,

    kLast = kDiscard
};
inline static constexpr int kStoreOpCount = (int)(StoreOp::kLast) + 1;

/**
 * Geometric primitives used for drawing.
 */
enum class PrimitiveType : uint8_t {
    kTriangles,
    kTriangleStrip,
    kPoints,
};

} // namespace skgpu

#endif // skgpu_GraphiteTypesPriv_DEFINED
