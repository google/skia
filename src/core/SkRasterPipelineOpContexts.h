/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipelineOpContexts_DEFINED
#define SkRasterPipelineOpContexts_DEFINED

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace SkSL { class TraceHook; }
// state shared by stack_checkpoint and stack_rewind
struct SkRasterPipelineStage;
enum class SkPerlinNoiseShaderType;

namespace SkRasterPipelineContexts {

// The largest number of pixels we handle at a time. We have a separate value for the largest number
// of pixels we handle in the highp pipeline. Many of the context structs in this file are only used
// by stages that have no lowp implementation. They can therefore use the (smaller) highp value to
// save memory in the arena.
inline static constexpr int kMaxStride = 16;
inline static constexpr int kMaxStride_highp = 16;

// How much space to allocate for each MemoryCtx scratch buffer, as part of tail-pixel handling.
inline static constexpr size_t kMaxScratchPerPatch =
        std::max(kMaxStride_highp * 16,  // 16 == largest highp bpp (RGBA_F32)
                 kMaxStride * 4);        // 4 == largest lowp bpp (RGBA_8888)

// These structs hold the context data for many of the Raster Pipeline ops.
struct MemoryCtx {
    void* pixels;
    int   stride;
};

// Raster Pipeline typically processes N (4, 8, 16) pixels at a time, in SIMT fashion. If the
// number of pixels in a row isn't evenly divisible by N, there will be leftover pixels; this is
// called the "tail". To avoid reading or writing past the end of any source or destination buffers
// when we reach the tail:
//
//   1) Source buffers have their tail contents copied to a scratch buffer that is at least N wide.
//      In practice, each scratch buffer uses kMaxScratchPerPatch bytes.
//   2) Each MemoryCtx in the pipeline is patched, such that access to them (at the current scanline
//      and x-offset) will land in the scratch buffer.
//   3) Pipeline is run as normal (with all memory access happening safely in the scratch buffers).
//   4) Destination buffers have their tail contents copied back from the scratch buffer.
//   5) Each MemoryCtx is "un-patched".
//
// To do all of this, the pipeline creates a MemoryCtxPatch for each unique MemoryCtx referenced by
// the pipeline.
struct MemoryCtxInfo {
    MemoryCtx* context;

    int bytesPerPixel;
    bool load;
    bool store;
};

// Some SIMD instructions operate faster if we read from aligned memory. 64 bytes (512 bits) is
// the widest we have (AVX-512), so if we have the scratch field be first and the whole struct
// aligned that way, the memory for our tail pixels should also be aligned to 64 bytes.
struct alignas(64) MemoryCtxPatch {
    std::byte scratch[kMaxScratchPerPatch];

    MemoryCtxInfo info;
    void* backup;  // Remembers context->pixels so we can restore it
};

struct GatherCtx {
    const void* pixels;
    int         stride;
    float       width;
    float       height;
    float       weights[16];  // for bicubic and bicubic_clamp_8888
    // Controls whether pixel i-1 or i is selected when floating point sample position is exactly i.
    bool        roundDownAtInteger = false;
};

// State shared by save_xy, accumulate, and bilinear_* / bicubic_*.
struct SamplerCtx {
    float      x[kMaxStride_highp];
    float      y[kMaxStride_highp];
    float     fx[kMaxStride_highp];
    float     fy[kMaxStride_highp];
    float scalex[kMaxStride_highp];
    float scaley[kMaxStride_highp];

    // for bicubic_[np][13][xy]
    float weights[16];
    float wx[4][kMaxStride_highp];
    float wy[4][kMaxStride_highp];
};

struct TileCtx {
    float scale;
    float invScale; // cache of 1/scale
    // When in the reflection portion of mirror tiling we need to snap the opposite direction
    // at integer sample points than when in the forward direction. This controls which way we bias
    // in the reflection. It should be 1 if GatherCtx::roundDownAtInteger is true
    // and otherwise -1.
    int   mirrorBiasDir = -1;
};

struct DecalTileCtx {
    uint32_t mask[kMaxStride];
    float    limit_x;
    float    limit_y;
    // These control which edge of the interval is included (i.e. closed interval at 0 or at limit).
    // They should be set to limit_x and limit_y if GatherCtx::roundDownAtInteger
    // is true and otherwise zero.
    float    inclusiveEdge_x = 0;
    float    inclusiveEdge_y = 0;
};

struct PerlinNoiseCtx {
    SkPerlinNoiseShaderType noiseType;
    float baseFrequencyX, baseFrequencyY;
    float stitchDataInX, stitchDataInY;
    bool stitching;
    int numOctaves;
    const uint8_t* latticeSelector;  // [256 values]
    const uint16_t* noiseData;       // [4 channels][256 elements][vector of 2]
};

// State used by mipmap_linear_*
struct MipmapCtx {
    // Original coords, saved before the base level logic
    float x[kMaxStride_highp];
    float y[kMaxStride_highp];

    // Base level color
    float r[kMaxStride_highp];
    float g[kMaxStride_highp];
    float b[kMaxStride_highp];
    float a[kMaxStride_highp];

    // Scale factors to transform base level coords to lower level coords
    float scaleX;
    float scaleY;

    float lowerWeight;
};

struct CoordClampCtx {
    float min_x, min_y;
    float max_x, max_y;
};

struct CallbackCtx {
    void (*fn)(CallbackCtx* self, int active_pixels /*<= kMaxStride_highp*/);

    // When called, fn() will have our active pixels available in rgba.
    // When fn() returns, the pipeline will read back those active pixels from read_from.
    float rgba[4 * kMaxStride_highp];
    float* read_from = rgba;
};

struct RewindCtx {
    float  r[kMaxStride_highp];
    float  g[kMaxStride_highp];
    float  b[kMaxStride_highp];
    float  a[kMaxStride_highp];
    float dr[kMaxStride_highp];
    float dg[kMaxStride_highp];
    float db[kMaxStride_highp];
    float da[kMaxStride_highp];
    std::byte* base;
    SkRasterPipelineStage* stage;
};

constexpr size_t kRGBAChannels = 4;

struct GradientCtx {
    size_t stopCount;
    float* factors[kRGBAChannels];
    float* biases[kRGBAChannels];
    float* ts;
};

struct EvenlySpaced2StopGradientCtx {
    float factor[kRGBAChannels];
    float bias[kRGBAChannels];
};

struct Conical2PtCtx {
    uint32_t fMask[kMaxStride_highp];
    float    fP0,
             fP1;
};

struct UniformColorCtx {
    float r,g,b,a;
    uint16_t rgba[4];  // [0,255] in a 16-bit lane.
};

struct EmbossCtx {
    MemoryCtx mul, add;
};

struct TablesCtx {
    const uint8_t *r, *g, *b, *a;
};

using SkRPOffset = uint32_t;

struct InitLaneMasksCtx {
    uint8_t* tail;
};

struct ConstantCtx {
    int32_t value;
    SkRPOffset dst;
};

struct UniformCtx {
    int32_t* dst;
    const int32_t* src;
};

struct BinaryOpCtx {
    SkRPOffset dst;
    SkRPOffset src;
};

struct TernaryOpCtx {
    SkRPOffset dst;
    SkRPOffset delta;
};

struct MatrixMultiplyCtx {
    SkRPOffset dst;
    uint8_t leftColumns, leftRows, rightColumns, rightRows;
};

struct SwizzleCtx {
    // If we are processing more than 16 pixels at a time, an 8-bit offset won't be sufficient and
    // `offsets` will need to use uint16_t (or dial down the premultiplication).
    static_assert(kMaxStride_highp <= 16);

    SkRPOffset dst;
    uint8_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct ShuffleCtx {
    int32_t* ptr;
    int count;
    uint16_t offsets[16];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SwizzleCopyCtx {
    int32_t* dst;
    const int32_t* src;   // src values must _not_ overlap dst values
    uint16_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct CopyIndirectCtx {
    int32_t* dst;
    const int32_t* src;
    const uint32_t *indirectOffset;  // this applies to `src` or `dst` based on the op
    uint32_t indirectLimit;          // the indirect offset is clamped to this upper bound
    uint32_t slots;                  // the number of slots to copy
};

struct SwizzleCopyIndirectCtx : public CopyIndirectCtx {
    uint16_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct BranchCtx {
    int offset;  // contains the label ID during compilation, and the program offset when compiled
};

struct BranchIfAllLanesActiveCtx : public BranchCtx {
    uint8_t* tail = nullptr;  // lanes past the tail are _never_ active, so we need to exclude them
};

struct BranchIfEqualCtx : public BranchCtx {
    int value;
    const int* ptr;
};

struct CaseOpCtx {
    int expectedValue;
    SkRPOffset offset;  // points to a pair of adjacent I32s: {I32 actualValue, I32 defaultMask}
};

struct TraceFuncCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int funcIdx;
};

struct TraceScopeCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int delta;
};

struct TraceLineCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int lineNumber;
};

struct TraceVarCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int slotIdx, numSlots;
    const int* data;
    const uint32_t *indirectOffset;  // can be null; if set, an offset applied to `data`
    uint32_t indirectLimit;          // the indirect offset is clamped to this upper bound
};

}  // namespace SkRasterPipelineContexts

#endif  // SkRasterPipelineOpContexts_DEFINED
