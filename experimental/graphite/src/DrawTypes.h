/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawTypes_DEFINED
#define skgpu_DrawTypes_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

class Buffer;

/**
 * Types of shader-language-specific boxed variables we can create.
 */
enum class SLType {
    kVoid,
    kBool,
    kBool2,
    kBool3,
    kBool4,
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
    kUInt,
    kUInt2,
    kUInt3,
    kUInt4,
    kTexture2DSampler,
    kTextureExternalSampler,
    kTexture2DRectSampler,
    kTexture2D,
    kSampler,
    kInput,

    kLast = kInput
};
static const int kSLTypeCount = static_cast<int>(SLType::kLast) + 1;

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

/**
 * Types used to describe format of vertices in buffers.
 */
enum class VertexAttribType : uint8_t {
    kFloat = 0,
    kFloat2,
    kFloat3,
    kFloat4,
    kHalf,
    kHalf2,
    kHalf4,

    kInt2,   // vector of 2 32-bit ints
    kInt3,   // vector of 3 32-bit ints
    kInt4,   // vector of 4 32-bit ints

    kByte,  // signed byte
    kByte2, // vector of 2 8-bit signed bytes
    kByte4, // vector of 4 8-bit signed bytes
    kUByte,  // unsigned byte
    kUByte2, // vector of 2 8-bit unsigned bytes
    kUByte4, // vector of 4 8-bit unsigned bytes

    kUByte_norm,  // unsigned byte, e.g. coverage, 0 -> 0.0f, 255 -> 1.0f.
    kUByte4_norm, // vector of 4 unsigned bytes, e.g. colors, 0 -> 0.0f, 255 -> 1.0f.

    kShort2,       // vector of 2 16-bit shorts.
    kShort4,       // vector of 4 16-bit shorts.

    kUShort2,      // vector of 2 unsigned shorts. 0 -> 0, 65535 -> 65535.
    kUShort2_norm, // vector of 2 unsigned shorts. 0 -> 0.0f, 65535 -> 1.0f.

    kInt,
    kUInt,

    kUShort_norm,

    kUShort4_norm, // vector of 4 unsigned shorts. 0 -> 0.0f, 65535 -> 1.0f.

    kLast = kUShort4_norm
};
static const int kVertexAttribTypeCount = (int)(VertexAttribType::kLast) + 1;

/*
 * Struct returned by the DrawBufferManager that can be passed into bind buffer calls on the
 * CommandBuffer.
 */
struct BindBufferInfo {
    Buffer* fBuffer = nullptr;
    size_t fOffset = 0;
};

};  // namespace skgpu

#endif // skgpu_DrawTypes_DEFINED

