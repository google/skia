/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_DEFINED
#define SkRasterPipeline_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMacros.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <cstddef>
#include <cstdint>
#include <functional>

class SkMatrix;
enum SkColorType : int;
struct SkImageInfo;
struct skcms_TransferFunction;

#if __has_cpp_attribute(clang::musttail) && !defined(__EMSCRIPTEN__) && !defined(SK_CPU_ARM32)
    #define SK_HAS_MUSTTAIL 1
#else
    #define SK_HAS_MUSTTAIL 0
#endif

/**
 * SkRasterPipeline provides a cheap way to chain together a pixel processing pipeline.
 *
 * It's particularly designed for situations where the potential pipeline is extremely
 * combinatoric: {N dst formats} x {M source formats} x {K mask formats} x {C transfer modes} ...
 * No one wants to write specialized routines for all those combinations, and if we did, we'd
 * end up bloating our code size dramatically.  SkRasterPipeline stages can be chained together
 * at runtime, so we can scale this problem linearly rather than combinatorically.
 *
 * Each stage is represented by a function conforming to a common interface and by an
 * arbitrary context pointer.  The stage function arguments and calling convention are
 * designed to maximize the amount of data we can pass along the pipeline cheaply, and
 * vary depending on CPU feature detection.
 */

// The largest number of pixels we handle at a time. We have a separate value for the largest number
// of pixels we handle in the highp pipeline. Many of the context structs in this file are only used
// by stages that have no lowp implementation. They can therefore use the (smaller) highp value to
// save memory in the arena.
inline static constexpr int SkRasterPipeline_kMaxStride = 16;
inline static constexpr int SkRasterPipeline_kMaxStride_highp = 8;

// Raster pipeline programs are stored as a contiguous array of SkRasterPipelineStages.
SK_BEGIN_REQUIRE_DENSE
struct SkRasterPipelineStage {
    // A function pointer from `stages_lowp` or `stages_highp`. The exact function pointer type
    // varies depending on architecture (specifically, see `Stage` in SkRasterPipeline_opts.h).
    void (*fn)();

    // Data used by the stage function. Most context structures are declared at the top of
    // SkRasterPipeline.h, and have names ending in Ctx (e.g. "SkRasterPipeline_SamplerCtx").
    void* ctx;
};
SK_END_REQUIRE_DENSE

// These structs hold the context data for many of the above Raster Pipeline ops.

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

struct SkRasterPipeline_CallbackCtx {
    void (*fn)(SkRasterPipeline_CallbackCtx* self,
               int active_pixels /*<= SkRasterPipeline_kMaxStride_highp*/);

    // When called, fn() will have our active pixels available in rgba.
    // When fn() returns, the pipeline will read back those active pixels from read_from.
    float rgba[4*SkRasterPipeline_kMaxStride_highp];
    float* read_from = rgba;
};

// state shared by stack_checkpoint and stack_rewind
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

class SkRasterPipeline {
public:
    explicit SkRasterPipeline(SkArenaAlloc*);

    SkRasterPipeline(const SkRasterPipeline&) = delete;
    SkRasterPipeline(SkRasterPipeline&&)      = default;

    SkRasterPipeline& operator=(const SkRasterPipeline&) = delete;
    SkRasterPipeline& operator=(SkRasterPipeline&&)      = default;

    void reset();

#define M(st) +1
    static constexpr int kNumLowpOps  = SK_RASTER_PIPELINE_OPS_LOWP(M);
    static constexpr int kNumHighpOps = SK_RASTER_PIPELINE_OPS_ALL(M);
#undef M

    void append(SkRasterPipelineOp, void* = nullptr);
    void append(SkRasterPipelineOp op, const void* ctx) { this->append(op,const_cast<void*>(ctx)); }
    void append(SkRasterPipelineOp, uintptr_t ctx);

    // Append all stages to this pipeline.
    void extend(const SkRasterPipeline&);

    // Runs the pipeline in 2d from (x,y) inclusive to (x+w,y+h) exclusive.
    void run(size_t x, size_t y, size_t w, size_t h) const;

    // Allocates a thunk which amortizes run() setup cost in alloc.
    std::function<void(size_t, size_t, size_t, size_t)> compile() const;

    // Callers can inspect the stage list for debugging purposes.
    struct StageList {
        StageList*          prev;
        SkRasterPipelineOp  stage;
        void*               ctx;
    };

    static const char* GetOpName(SkRasterPipelineOp op);
    const StageList* getStageList() const { return fStages; }
    int getNumStages() const { return fNumStages; }

    // Prints the entire StageList using SkDebugf.
    void dump() const;

    // Appends a stage for the specified matrix.
    // Tries to optimize the stage by analyzing the type of matrix.
    void append_matrix(SkArenaAlloc*, const SkMatrix&);

    // Appends a stage for a constant uniform color.
    // Tries to optimize the stage based on the color.
    void append_constant_color(SkArenaAlloc*, const float rgba[4]);

    void append_constant_color(SkArenaAlloc* alloc, const SkColor4f& color) {
        this->append_constant_color(alloc, color.vec());
    }

    // Like append_constant_color() but only affecting r,g,b, ignoring the alpha channel.
    void append_set_rgb(SkArenaAlloc*, const float rgb[3]);

    void append_set_rgb(SkArenaAlloc* alloc, const SkColor4f& color) {
        this->append_set_rgb(alloc, color.vec());
    }

    void append_load    (SkColorType, const SkRasterPipeline_MemoryCtx*);
    void append_load_dst(SkColorType, const SkRasterPipeline_MemoryCtx*);
    void append_store   (SkColorType, const SkRasterPipeline_MemoryCtx*);

    void append_clamp_if_normalized(const SkImageInfo&);

    void append_transfer_function(const skcms_TransferFunction&);

    void append_stack_rewind();

    bool empty() const { return fStages == nullptr; }

private:
    bool build_lowp_pipeline(SkRasterPipelineStage* ip) const;
    void build_highp_pipeline(SkRasterPipelineStage* ip) const;

    using StartPipelineFn = void(*)(size_t,size_t,size_t,size_t, SkRasterPipelineStage* program);
    StartPipelineFn build_pipeline(SkRasterPipelineStage*) const;

    void unchecked_append(SkRasterPipelineOp, void*);
    int stages_needed() const;

    SkArenaAlloc*               fAlloc;
    SkRasterPipeline_RewindCtx* fRewindCtx;
    StageList*                  fStages;
    int                         fNumStages;
};

template <size_t bytes>
class SkRasterPipeline_ : public SkRasterPipeline {
public:
    SkRasterPipeline_()
        : SkRasterPipeline(&fBuiltinAlloc) {}

private:
    SkSTArenaAlloc<bytes> fBuiltinAlloc;
};


#endif//SkRasterPipeline_DEFINED
