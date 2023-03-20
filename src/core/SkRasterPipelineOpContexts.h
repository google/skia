/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipelineOpContexts_DEFINED
#define SkRasterPipelineOpContexts_DEFINED

// The largest number of pixels we handle at a time. We have a separate value for the largest number
// of pixels we handle in the highp pipeline. Many of the context structs in this file are only used
// by stages that have no lowp implementation. They can therefore use the (smaller) highp value to
// save memory in the arena.
inline static constexpr int SkRasterPipeline_kMaxStride = 16;
inline static constexpr int SkRasterPipeline_kMaxStride_highp = 8;

// These structs hold the context data for many of the Raster Pipeline ops.
struct SkRasterPipeline_MemoryCtx {
    void* pixels;
    int   stride;
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

struct SkRasterPipeline_BinaryOpCtx {
    float *dst;
    const float *src;
};

struct SkRasterPipeline_TernaryOpCtx {
    float *dst;
    const float *src0;
    const float *src1;
};

struct SkRasterPipeline_SwizzleCtx {
    float *ptr;
    uint16_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SkRasterPipeline_ShuffleCtx {
    float *ptr;
    int count;
    uint16_t offsets[16];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SkRasterPipeline_SwizzleCopyCtx {
    float *dst;
    float *src;           // src values must _not_ overlap dst values
    uint16_t offsets[4];  // values must be byte offsets (4 * highp-stride * component-index)
};

struct SkRasterPipeline_CopyIndirectCtx {
    float *dst;
    const float *src;
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

struct SkRasterPipeline_BranchIfEqualCtx : public SkRasterPipeline_BranchCtx {
    int value;
    const int *ptr;
};

struct SkRasterPipeline_CaseOpCtx {
    int expectedValue;
    int* ptr;  // points to a pair of adjacent I32s: {I32 actualValue, I32 defaultMask}
};

#endif  // SkRasterPipelineOpContexts_DEFINED
