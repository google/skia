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


/**
 * Returns the size of the attrib type in bytes.
 */
static constexpr inline size_t VertexAttribTypeSize(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return sizeof(float);
        case VertexAttribType::kFloat2:
            return 2 * sizeof(float);
        case VertexAttribType::kFloat3:
            return 3 * sizeof(float);
        case VertexAttribType::kFloat4:
            return 4 * sizeof(float);
        case VertexAttribType::kHalf:
            return sizeof(uint16_t);
        case VertexAttribType::kHalf2:
            return 2 * sizeof(uint16_t);
        case VertexAttribType::kHalf4:
            return 4 * sizeof(uint16_t);
        case VertexAttribType::kInt2:
            return 2 * sizeof(int32_t);
        case VertexAttribType::kInt3:
            return 3 * sizeof(int32_t);
        case VertexAttribType::kInt4:
            return 4 * sizeof(int32_t);
        case VertexAttribType::kByte:
            return 1 * sizeof(char);
        case VertexAttribType::kByte2:
            return 2 * sizeof(char);
        case VertexAttribType::kByte4:
            return 4 * sizeof(char);
        case VertexAttribType::kUByte:
            return 1 * sizeof(char);
        case VertexAttribType::kUByte2:
            return 2 * sizeof(char);
        case VertexAttribType::kUByte4:
            return 4 * sizeof(char);
        case VertexAttribType::kUByte_norm:
            return 1 * sizeof(char);
        case VertexAttribType::kUByte4_norm:
            return 4 * sizeof(char);
        case VertexAttribType::kShort2:
            return 2 * sizeof(int16_t);
        case VertexAttribType::kShort4:
            return 4 * sizeof(int16_t);
        case VertexAttribType::kUShort2: [[fallthrough]];
        case VertexAttribType::kUShort2_norm:
            return 2 * sizeof(uint16_t);
        case VertexAttribType::kInt:
            return sizeof(int32_t);
        case VertexAttribType::kUInt:
            return sizeof(uint32_t);
        case VertexAttribType::kUShort_norm:
            return sizeof(uint16_t);
        case VertexAttribType::kUShort4_norm:
            return 4 * sizeof(uint16_t);
    }
}

/*
 * Struct returned by the DrawBufferManager that can be passed into bind buffer calls on the
 * CommandBuffer.
 */
struct BindBufferInfo {
    const Buffer* fBuffer = nullptr;
    size_t fOffset = 0;

    operator bool() const { return SkToBool(fBuffer); }

    bool operator==(const BindBufferInfo& o) const {
        return fBuffer == o.fBuffer && (!fBuffer || fOffset == o.fOffset);
    }
    bool operator!=(const BindBufferInfo& o) const {
        return !(*this == o);
    }
};

/*
 * Depth and stencil settings
 */
enum class CompareOp : uint8_t {
    kAlways,
    kNever,
    kGreater,
    kGEqual,
    kLess,
    kLEqual,
    kEqual,
    kNotEqual
};
static constexpr int kCompareOpCount = 1 + (int)CompareOp::kNotEqual;

enum class StencilOp : uint8_t {
    kKeep,
    kZero,
    kReplace, // Replace stencil value with reference (only the bits enabled in fWriteMask).
    kInvert,
    kIncWrap,
    kDecWrap,
    // NOTE: clamping occurs before the write mask. So if the MSB is zero and masked out, stencil
    // values will still wrap when using clamping ops.
    kIncClamp,
    kDecClamp
};
static constexpr int kStencilOpCount = 1 + (int)StencilOp::kDecClamp;

struct DepthStencilSettings {
    // Per-face settings for stencil
    struct Face {
        StencilOp fStencilFailOp = StencilOp::kKeep;
        StencilOp fDepthFailOp = StencilOp::kKeep;
        StencilOp fDepthStencilPassOp = StencilOp::kKeep;
        CompareOp fCompareOp = CompareOp::kAlways;
        uint32_t fReadMask = 0xffffffff;
        uint32_t fWriteMask = 0xffffffff;

        bool operator==(const Face& that) const {
            return this->fStencilFailOp == that.fStencilFailOp &&
                   this->fDepthFailOp == that.fDepthFailOp &&
                   this->fDepthStencilPassOp == that.fDepthStencilPassOp &&
                   this->fCompareOp == that.fCompareOp &&
                   this->fReadMask == that.fReadMask &&
                   this->fWriteMask == that.fWriteMask;
        }
    };

    bool operator==(const DepthStencilSettings& that) const {
        return this->fFrontStencil == that.fFrontStencil &&
               this->fBackStencil == that.fBackStencil &&
               this->fStencilReferenceValue == that.fStencilReferenceValue &&
               this->fDepthCompareOp == that.fDepthCompareOp &&
               this->fStencilTestEnabled == that.fStencilTestEnabled &&
               this->fDepthTestEnabled == that.fDepthTestEnabled &&
               this->fDepthWriteEnabled == that.fDepthWriteEnabled;
    }

    Face fFrontStencil;
    Face fBackStencil;
    uint32_t fStencilReferenceValue = 0;
    CompareOp fDepthCompareOp = CompareOp::kAlways;
    bool fStencilTestEnabled = false;
    bool fDepthTestEnabled = false;
    bool fDepthWriteEnabled = false;
};

};  // namespace skgpu

#endif // skgpu_DrawTypes_DEFINED
