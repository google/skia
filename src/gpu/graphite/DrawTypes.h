/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawTypes_DEFINED
#define skgpu_graphite_DrawTypes_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"

#include "src/gpu/graphite/ResourceTypes.h"

#include <array>

namespace skgpu::graphite {

class Buffer;

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

/**
 * Struct used to describe how a Texture/TextureProxy/TextureProxyView is sampled.
 */
struct SamplerDesc {
    SkSamplingOptions fSamplingOptions;
    SkTileMode fTileModes[2];

    bool operator==(const SamplerDesc& o) const {
        return fSamplingOptions == o.fSamplingOptions &&
               fTileModes[0] == o.fTileModes[0] &&
               fTileModes[1] == o.fTileModes[1];
    }
    bool operator!=(const SamplerDesc& o) const {
        return !(*this == o);
    }

    uint32_t asKey() const {
        static_assert(kSkTileModeCount <= 4 && kSkFilterModeCount <= 2);
        // Cubic sampling is handled in a shader, with the actual texture sampled by with NN,
        // but that is what a cubic SkSamplingOptions is set to if you ignore 'cubic', which let's
        // us simplify how we construct SamplerDec's from the options passed to high-level draws.
        SkASSERT(!fSamplingOptions.useCubic || (fSamplingOptions.filter == SkFilterMode::kNearest &&
                                                fSamplingOptions.mipmap == SkMipmapMode::kNone));
        // TODO: Add support for anisotropic filtering
        return (static_cast<int>(fTileModes[0])           << 0) |
               (static_cast<int>(fTileModes[1])           << 2) |
               (static_cast<int>(fSamplingOptions.filter) << 4) |
               (static_cast<int>(fSamplingOptions.mipmap) << 5);
    }
};

enum class UniformSlot {
    // TODO: Want this?
    // Meant for uniforms that change rarely to never over the course of a render pass
    // kStatic,
    // Meant for uniforms that are defined and used by the RenderStep portion of the pipeline shader
    kRenderStep,
    // Meant for uniforms that are defined and used by the paint parameters (ie SkPaint subset)
    kPaint,
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
        constexpr Face() = default;
        constexpr Face(StencilOp stencilFail,
                       StencilOp depthFail,
                       StencilOp dsPass,
                       CompareOp compare,
                       uint32_t readMask,
                       uint32_t writeMask)
                : fStencilFailOp(stencilFail)
                , fDepthFailOp(depthFail)
                , fDepthStencilPassOp(dsPass)
                , fCompareOp(compare)
                , fReadMask(readMask)
                , fWriteMask(writeMask) {}

        StencilOp fStencilFailOp = StencilOp::kKeep;
        StencilOp fDepthFailOp = StencilOp::kKeep;
        StencilOp fDepthStencilPassOp = StencilOp::kKeep;
        CompareOp fCompareOp = CompareOp::kAlways;
        uint32_t fReadMask = 0xffffffff;
        uint32_t fWriteMask = 0xffffffff;

        constexpr bool operator==(const Face& that) const {
            return this->fStencilFailOp == that.fStencilFailOp &&
                   this->fDepthFailOp == that.fDepthFailOp &&
                   this->fDepthStencilPassOp == that.fDepthStencilPassOp &&
                   this->fCompareOp == that.fCompareOp &&
                   this->fReadMask == that.fReadMask &&
                   this->fWriteMask == that.fWriteMask;
        }
    };

    constexpr DepthStencilSettings() = default;
    constexpr DepthStencilSettings(Face front,
                                   Face back,
                                   uint32_t stencilRef,
                                   bool stencilTest,
                                   CompareOp depthCompare,
                                   bool depthTest,
                                   bool depthWrite)
            : fFrontStencil(front)
            , fBackStencil(back)
            , fStencilReferenceValue(stencilRef)
            , fDepthCompareOp(depthCompare)
            , fStencilTestEnabled(stencilTest)
            , fDepthTestEnabled(depthTest)
            , fDepthWriteEnabled(depthWrite) {}

    constexpr bool operator==(const DepthStencilSettings& that) const {
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

};  // namespace skgpu::graphite

#endif // skgpu_graphite_DrawTypes_DEFINED
