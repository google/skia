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

// The largest number of pixels we handle at a time. We have a separate value for the largest number
// of pixels we handle in the highp pipeline. Many of the context structs in this file are only used
// by stages that have no lowp implementation. They can therefore use the (smaller) highp value to
// save memory in the arena.
inline static constexpr int SkRasterPipeline_kMaxStride = 16;
inline static constexpr int SkRasterPipeline_kMaxStride_highp = 16;

// How much space to allocate for each MemoryCtx scratch buffer, as part of tail-pixel handling.
inline static constexpr size_t SkRasterPipeline_MaxScratchPerPatch =
        std::max(SkRasterPipeline_kMaxStride_highp * 16,  // 16 == largest highp bpp (RGBA_F32)
                 SkRasterPipeline_kMaxStride * 4);        // 4 == largest lowp bpp (RGBA_8888)

// These structs hold the context data for many of the Raster Pipeline ops.
struct SkRasterPipeline_MemoryCtx {
    void* pixels;
    int   stride;
};

// Raster Pipeline typically processes N (4, 8, 16) pixels at a time, in SIMT fashion. If the
// number of pixels in a row isn't evenly divisible by N, there will be leftover pixels; this is
// called the "tail". To avoid reading or writing past the end of any source or destination buffers
// when we reach the tail:
//
//   1) Source buffers have their tail contents copied to a scratch buffer that is at least N wide.
//      In practice, each scratch buffer uses SkRasterPipeline_MaxScratchPerPatch bytes.
//   2) Each MemoryCtx in the pipeline is patched, such that access to them (at the current scanline
//      and x-offset) will land in the scratch buffer.
//   3) Pipeline is run as normal (with all memory access happening safely in the scratch buffers).
//   4) Destination buffers have their tail contents copied back from the scratch buffer.
//   5) Each MemoryCtx is "un-patched".
//
// To do all of this, the pipeline creates a MemoryCtxPatch for each unique MemoryCtx referenced by
// the pipeline.
struct SkRasterPipeline_MemoryCtxInfo {
    SkRasterPipeline_MemoryCtx* context;

    int bytesPerPixel;
    bool load;
    bool store;
};

struct SkRasterPipeline_MemoryCtxPatch {
    SkRasterPipeline_MemoryCtxInfo info;

    void* backup;  // Remembers context->pixels so we can restore it
    std::byte scratch[SkRasterPipeline_MaxScratchPerPatch];
};

struct SkRasterPipeline_GatherCtx {
    const void* pixels;
    int         stride;
    float       width;
    float       height;
    float       weights[16];  // for bicubic and bicubic_clamp_8888
    // Controls whether pixel i-1 or i is selected when floating point sample position is exactly i.
    bool        roundDownAtInteger = false;
};

// State shared by save_xy, accumulate, and bilinear_* / bicubic_*.
struct SkRasterPipeline_SamplerCtx {
    float      x[SkRasterPipeline_kMaxStride_highp];
    float      y[SkRasterPipeline_kMaxStride_highp];
    float     fx[SkRasterPipeline_kMaxStride_highp];
    float     fy[SkRasterPipeline_kMaxStride_highp];
    float scalex[SkRasterPipeline_kMaxStride_highp];
    float scaley[SkRasterPipeline_kMaxStride_highp];

    // for bicubic_[np][13][xy]
    float weights[16];
    float wx[4][SkRasterPipeline_kMaxStride_highp];
    float wy[4][SkRasterPipeline_kMaxStride_highp];
};

struct SkRasterPipeline_TileCtx {
    float scale;
    float invScale; // cache of 1/scale
    // When in the reflection portion of mirror tiling we need to snap the opposite direction
    // at integer sample points than when in the forward direction. This controls which way we bias
    // in the reflection. It should be 1 if SkRasterPipeline_GatherCtx::roundDownAtInteger is true
    // and otherwise -1.
    int   mirrorBiasDir = -1;
};

struct SkRasterPipeline_DecalTileCtx {
    uint32_t mask[SkRasterPipeline_kMaxStride];
    float    limit_x;
    float    limit_y;
    // These control which edge of the interval is included (i.e. closed interval at 0 or at limit).
    // They should be set to limit_x and limit_y if SkRasterPipeline_GatherCtx::roundDownAtInteger
    // is true and otherwise zero.
    float    inclusiveEdge_x = 0;
    float    inclusiveEdge_y = 0;
};

// State used by mipmap_linear_*
struct SkRasterPipeline_MipmapCtx {
    // Original coords, saved before the base level logic
    float x[SkRasterPipeline_kMaxStride_highp];
    float y[SkRasterPipeline_kMaxStride_highp];

    // Base level color
    float r[SkRasterPipeline_kMaxStride_highp];
    float g[SkRasterPipeline_kMaxStride_highp];
    float b[SkRasterPipeline_kMaxStride_highp];
    float a[SkRasterPipeline_kMaxStride_highp];

    // Scale factors to transform base level coords to lower level coords
    float scaleX;
    float scaleY;

    float lowerWeight;
};

struct SkRasterPipeline_CoordClampCtx {
    float min_x, min_y;
    float max_x, max_y;
};

struct SkRasterPipeline_CallbackCtx {
    void (*fn)(SkRasterPipeline_CallbackCtx* self,
               int active_pixels /*<= SkRasterPipeline_kMaxStride_highp*/);

    // When called, fn() will have our active pixels available in rgba.
    // When fn() returns, the pipeline will read back those active pixels from read_from.
    float rgba[4*SkRasterPipeline_kMaxStride_highp];
    float* read_from = rgba;
};

// state shared by stack_checkpoint and stack_rewind
struct SkRasterPipelineStage;

struct SkRasterPipeline_RewindCtx {
    float  r[SkRasterPipeline_kMaxStride_highp];
    float  g[SkRasterPipeline_kMaxStride_highp];
    float  b[SkRasterPipeline_kMaxStride_highp];
    float  a[SkRasterPipeline_kMaxStride_highp];
    float dr[SkRasterPipeline_kMaxStride_highp];
    float dg[SkRasterPipeline_kMaxStride_highp];
    float db[SkRasterPipeline_kMaxStride_highp];
    float da[SkRasterPipeline_kMaxStride_highp];
    std::byte* base;
    SkRasterPipelineStage* stage;
};

struct SkRasterPipeline_GradientCtx {
    size_t stopCount;
    float* fs[4];
    float* bs[4];
    float* ts;
};

struct SkRasterPipeline_EvenlySpaced2StopGradientCtx {
    float f[4];
    float b[4];
};

struct SkRasterPipeline_2PtConicalCtx {
    uint32_t fMask[SkRasterPipeline_kMaxStride_highp];
    float    fP0,
             fP1;
};

struct SkRasterPipeline_UniformColorCtx {
    float r,g,b,a;
    uint16_t rgba[4];  // [0,255] in a 16-bit lane.
};

struct SkRasterPipeline_EmbossCtx {
    SkRasterPipeline_MemoryCtx mul,
                               add;
};

struct SkRasterPipeline_TablesCtx {
    const uint8_t *r, *g, *b, *a;
};

using SkRPOffset = uint32_t;

struct SkRasterPipeline_InitLaneMasksCtx {
    uint8_t* tail;
};

struct SkRasterPipeline_ConstantCtx {
    int32_t value;
    SkRPOffset dst;
};

struct SkRasterPipeline_UniformCtx {
    int32_t* dst;
    const int32_t* src;
};

struct SkRasterPipeline_BinaryOpCtx {
    SkRPOffset dst;
    SkRPOffset src;
};

struct SkRasterPipeline_TernaryOpCtx {
    SkRPOffset dst;
    SkRPOffset delta;
};

struct SkRasterPipeline_MatrixMultiplyCtx {
    SkRPOffset dst;
    uint8_t leftColumns, leftRows, rightColumns, rightRows;
};

struct SkRasterPipeline_SwizzleCtx {
    // If we are processing more than 16 pixels at a time, an 8-bit offset won't be sufficient and
    // `offsets` will need to use uint16_t (or dial down the premultiplication).
    static_assert(SkRasterPipeline_kMaxStride_highp <= 16);

    SkRPOffset dst;
    uint8_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SkRasterPipeline_ShuffleCtx {
    int32_t* ptr;
    int count;
    uint16_t offsets[16];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SkRasterPipeline_SwizzleCopyCtx {
    int32_t* dst;
    const int32_t* src;   // src values must _not_ overlap dst values
    uint16_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SkRasterPipeline_CopyIndirectCtx {
    int32_t* dst;
    const int32_t* src;
    const uint32_t *indirectOffset;  // this applies to `src` or `dst` based on the op
    uint32_t indirectLimit;          // the indirect offset is clamped to this upper bound
    uint32_t slots;                  // the number of slots to copy
};

struct SkRasterPipeline_SwizzleCopyIndirectCtx : public SkRasterPipeline_CopyIndirectCtx {
    uint16_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SkRasterPipeline_BranchCtx {
    int offset;  // contains the label ID during compilation, and the program offset when compiled
};

struct SkRasterPipeline_BranchIfAllLanesActiveCtx : public SkRasterPipeline_BranchCtx {
    uint8_t* tail = nullptr;  // lanes past the tail are _never_ active, so we need to exclude them
};

struct SkRasterPipeline_BranchIfEqualCtx : public SkRasterPipeline_BranchCtx {
    int value;
    const int* ptr;
};

struct SkRasterPipeline_CaseOpCtx {
    int expectedValue;
    SkRPOffset offset;  // points to a pair of adjacent I32s: {I32 actualValue, I32 defaultMask}
};

struct SkRasterPipeline_TraceFuncCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int funcIdx;
};

struct SkRasterPipeline_TraceScopeCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int delta;
};

struct SkRasterPipeline_TraceLineCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int lineNumber;
};

struct SkRasterPipeline_TraceVarCtx {
    const int* traceMask;
    SkSL::TraceHook* traceHook;
    int slotIdx, numSlots;
    const int* data;
    const uint32_t *indirectOffset;  // can be null; if set, an offset applied to `data`
    uint32_t indirectLimit;          // the indirect offset is clamped to this upper bound
};

#endif  // SkRasterPipelineOpContexts_DEFINED
