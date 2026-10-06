/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawTypes_DEFINED
#define skgpu_graphite_DrawTypes_DEFINED

#include "include/private/SkAssert.h"
#include "include/private/SkEnumBitMask.h"
#include "include/private/SkTo.h"

#include <cstddef>
#include <cstdint>

namespace skgpu::graphite {

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
    kUInt2,  // vector of 2 32-bit unsigned ints

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

    kUShort_norm,  // unsigned short, e.g. depth, 0 -> 0.0f, 65535 -> 1.0f.

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
        case VertexAttribType::kUInt2:
            return 2 * sizeof(uint32_t);
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
    SkUNREACHABLE;
}

enum class UniformSlot {
    // Slot for paints and render step uniforms
    kCombinedUniforms,
    // Storage buffer slot.
    kStorage
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

// These barrier types are not utilized by all backends, but we define them at this level anyhow
// since it impacts the logic used to group & sort draws.
enum class BarrierType : uint8_t {
    kNone,
    kAdvancedNoncoherentBlend,
    kReadDstFromInput,
};

enum class DstUsage : uint8_t {
    // Prior values of dst pixels will have no effect on final written color for all uses of the
    // pipeline (e.g. not just specific to the current draw's alpha value).
    kNone                  = 0,
    // Prior values of dst pixels can have an effect on the final written color
    kDependsOnDst          = 0b0001,
    // The prior values of dst pixels must be available in the fragment shader
    kDstReadRequired       = 0b0010,
    // The final written color uses an advanced blend function, which may require barriers for HW
    kAdvancedBlend         = 0b0100,
    // The only reason for kDependsOnDst is because the Renderer has analytic coverage. Switching
    // to a Coverage::kNone Renderer would result in DstUsage::kNone for the same paint.
    kDstOnlyUsedByRenderer = 0b1000,
};
SK_MAKE_BITMASK_OPS(DstUsage)

enum class RenderStateFlags : uint8_t {
    kNone                   = 0b0000,
    kFixed                  = 0b0001,   // Uses explicit DrawWriter::draw functions
    kAppendVertices         = 0b0010,   // Appends vertices
    kAppendInstances        = 0b0100,   // Appends instances with static vertex count
    kAppendDynamicInstances = 0b1000,   // Appends instances with a flexible vertex count
};
SK_MAKE_BITMASK_OPS(RenderStateFlags)

struct StencilSettings {
    // Per-face settings for stencil
    struct Face {
        constexpr Face() = default;
        constexpr Face(StencilOp stencilFail,
                       StencilOp depthFail,
                       StencilOp dsPass,
                       CompareOp compare)
                : fStencilFailOp(stencilFail)
                , fDepthFailOp(depthFail)
                , fDepthStencilPassOp(dsPass)
                , fCompareOp(compare) {}

        StencilOp fStencilFailOp = StencilOp::kKeep;
        StencilOp fDepthFailOp = StencilOp::kKeep;
        StencilOp fDepthStencilPassOp = StencilOp::kKeep;
        CompareOp fCompareOp = CompareOp::kAlways;

        // Return true if not the default stencil settings, i.e. does something to or with the
        // stencil buffer.
        constexpr explicit operator bool() const { return !(*this == Face()); }

        constexpr bool operator==(const Face& that) const {
            return this->fStencilFailOp == that.fStencilFailOp &&
                   this->fDepthFailOp == that.fDepthFailOp &&
                   this->fDepthStencilPassOp == that.fDepthStencilPassOp &&
                   this->fCompareOp == that.fCompareOp;
        }
    };

    constexpr StencilSettings() = default;
    constexpr StencilSettings(Face front,
                              Face back,
                              uint32_t stencilRef,
                              uint32_t stencilReadMask,
                              uint32_t stencilWriteMask)
            : fFrontFace(front)
            , fBackFace(back)
            , fReferenceValue(stencilRef)
            , fReadMask(stencilReadMask)
            , fWriteMask(stencilWriteMask) {
        // If there's no non-default stencil face state, the shared stencil state should be default
        SkASSERT(this->enabled() || (stencilRef == 0 &&
                                     stencilReadMask == 0xffffffff &&
                                     stencilWriteMask == 0xffffffff));
    }

    // True means the render pass must have a stencil attachment
    constexpr bool enabled() const { return SkToBool(fFrontFace) || SkToBool(fBackFace); }

    constexpr explicit operator bool() const { return this->enabled(); }

    constexpr bool operator==(const StencilSettings& that) const {
        return this->fFrontFace == that.fFrontFace &&
               this->fBackFace == that.fBackFace &&
               this->fReferenceValue == that.fReferenceValue &&
               this->fReadMask == that.fReadMask &&
               this->fWriteMask == that.fWriteMask;
    }

    Face fFrontFace;
    Face fBackFace;
    uint32_t fReferenceValue = 0;
    uint32_t fReadMask = 0xffffffff;
    uint32_t fWriteMask = 0xffffffff;
};

struct DepthSettings {
    constexpr DepthSettings() = default;
    constexpr DepthSettings(CompareOp compare, bool write)
            : fCompareOp(compare), fWriteEnabled(write) {}

    // True means the render pass must have a depth attachment
    constexpr bool enabled() const {
        return fCompareOp != CompareOp::kAlways || fWriteEnabled;
    }

    constexpr explicit operator bool() const {  return this->enabled(); }

    constexpr bool operator==(const DepthSettings& that) const {
        return this->fCompareOp == that.fCompareOp &&
               this->fWriteEnabled == that.fWriteEnabled;
    }

    CompareOp fCompareOp = CompareOp::kAlways;
    bool fWriteEnabled = false;
};

using DepthStencilSettings = std::pair<DepthSettings, StencilSettings>;

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DrawTypes_DEFINED
