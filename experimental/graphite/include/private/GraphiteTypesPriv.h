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

class Buffer;

// Use familiar type names from SkSL.
template<int N> using vec = skvx::Vec<N, float>;
using float2 = vec<2>;
using float4 = vec<4>;

template<int N> using ivec = skvx::Vec<N, int32_t>;
using int2 = ivec<2>;
using int4 = ivec<4>;

template<int N> using uvec = skvx::Vec<N, uint32_t>;
using uint2 = uvec<2>;
using uint4 = uvec<4>;

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
    kUint,

    kUShort_norm,

    kUShort4_norm, // vector of 4 unsigned shorts. 0 -> 0.0f, 65535 -> 1.0f.

    kLast = kUShort4_norm
};
static const int kVertexAttribTypeCount = (int)(VertexAttribType::kLast) + 1;

/**
 * Wraps an enum that is used for flags, and enables masking with type safety. Example:
 *
 *   enum class MyFlags {
 *       kNone = 0,
 *       kA = 1,
 *       kB = 2,
 *       kC = 4,
 *   };
 *
 *   SKGPU_MAKE_MASK_OPS(MyFlags)
 *
 *   ...
 *
 *       Mask<MyFlags> flags = MyFlags::kA | MyFlags::kB;
 *
 *       if (flags & MyFlags::kB) {}
 *
 *   ...
 */
template<typename E>
class Mask {
public:
    SK_ALWAYS_INLINE constexpr Mask(E e) : Mask((int)e) {}

    SK_ALWAYS_INLINE constexpr operator bool() const { return fValue; }

    SK_ALWAYS_INLINE bool operator==(Mask m) const { return fValue == m.fValue; }
    SK_ALWAYS_INLINE bool operator!=(Mask m) const { return fValue != m.fValue; }

    SK_ALWAYS_INLINE constexpr Mask operator|(Mask m) const { return Mask(fValue | m.fValue); }
    SK_ALWAYS_INLINE constexpr Mask operator&(Mask m) const { return Mask(fValue & m.fValue); }
    SK_ALWAYS_INLINE constexpr Mask operator^(Mask m) const { return Mask(fValue ^ m.fValue); }
    SK_ALWAYS_INLINE constexpr Mask operator~() const { return Mask(~fValue); }

    SK_ALWAYS_INLINE Mask& operator|=(Mask m) { return *this = *this | m; }
    SK_ALWAYS_INLINE Mask& operator&=(Mask m) { return *this = *this & m; }
    SK_ALWAYS_INLINE Mask& operator^=(Mask m) { return *this = *this ^ m; }

private:
    SK_ALWAYS_INLINE constexpr explicit Mask(int value) : fValue(value) {}

    int fValue;
};

/**
 * Defines functions that make it possible to use bitwise operators on an enum.
 */
#define SKGPU_MAKE_MASK_OPS(E) \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator|(E a, E b) { return skgpu::Mask<E>(a) | b; } \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator&(E a, E b) { return skgpu::Mask<E>(a) & b; } \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator^(E a, E b) { return skgpu::Mask<E>(a) ^ b; } \
    SK_MAYBE_UNUSED constexpr skgpu::Mask<E> operator~(E e) { return ~skgpu::Mask<E>(e); } \

#define SKGPU_DECL_MASK_OPS_FRIENDS(E) \
    friend constexpr skgpu::Mask<E> operator|(E, E); \
    friend constexpr skgpu::Mask<E> operator&(E, E); \
    friend constexpr skgpu::Mask<E> operator^(E, E); \
    friend constexpr skgpu::Mask<E> operator~(E); \

/*
 * Struct returned by the DrawBufferManager that can be passed into bind buffer calls on the
 * CommandBuffer.
 */
struct BindBufferInfo {
    Buffer* fBuffer = nullptr;
    size_t fOffset = 0;
};

} // namespace skgpu

#endif // skgpu_GraphiteTypesPriv_DEFINED
