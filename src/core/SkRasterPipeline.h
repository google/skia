/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_DEFINED
#define SkRasterPipeline_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkArenaAlloc.h"

#include <functional>

struct skcms_TransferFunction;

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

// There are two macros here: The first defines stages that have lowp (and highp) implementations
// The second defines stages that are only present in the highp pipeline.
#define SK_RASTER_PIPELINE_STAGES_LOWP(M)                          \
    M(move_src_dst) M(move_dst_src) M(swap_src_dst)                \
    M(clamp_01) M(clamp_gamut)                                     \
    M(premul) M(premul_dst)                                        \
    M(force_opaque) M(force_opaque_dst)                            \
    M(set_rgb) M(swap_rb) M(swap_rb_dst)                           \
    M(black_color) M(white_color)                                  \
    M(uniform_color) M(uniform_color_dst)                          \
    M(seed_shader)                                                 \
    M(load_a8)     M(load_a8_dst)   M(store_a8)    M(gather_a8)    \
    M(load_565)    M(load_565_dst)  M(store_565)   M(gather_565)   \
    M(load_4444)   M(load_4444_dst) M(store_4444)  M(gather_4444)  \
    M(load_8888)   M(load_8888_dst) M(store_8888)  M(gather_8888)  \
    M(load_rg88)   M(load_rg88_dst) M(store_rg88)  M(gather_rg88)  \
    M(store_r8)                                                    \
    M(alpha_to_gray) M(alpha_to_gray_dst)                          \
    M(alpha_to_red) M(alpha_to_red_dst)                            \
    M(bt709_luminance_or_luma_to_alpha) M(bt709_luminance_or_luma_to_rgb) \
    M(bilerp_clamp_8888)                                           \
    M(load_src) M(store_src) M(store_src_a) M(load_dst) M(store_dst) \
    M(scale_u8) M(scale_565) M(scale_1_float) M(scale_native)      \
    M( lerp_u8) M( lerp_565) M( lerp_1_float) M(lerp_native)       \
    M(dstatop) M(dstin) M(dstout) M(dstover)                       \
    M(srcatop) M(srcin) M(srcout) M(srcover)                       \
    M(clear) M(modulate) M(multiply) M(plus_) M(screen) M(xor_)    \
    M(darken) M(difference)                                        \
    M(exclusion) M(hardlight) M(lighten) M(overlay)                \
    M(srcover_rgba_8888)                                           \
    M(matrix_translate) M(matrix_scale_translate)                  \
    M(matrix_2x3)                                                  \
    M(matrix_perspective)                                          \
    M(decal_x)    M(decal_y)   M(decal_x_and_y)                    \
    M(check_decal_mask)                                            \
    M(clamp_x_1) M(mirror_x_1) M(repeat_x_1)                       \
    M(evenly_spaced_gradient)                                      \
    M(gradient)                                                    \
    M(evenly_spaced_2_stop_gradient)                               \
    M(xy_to_unit_angle)                                            \
    M(xy_to_radius)                                                \
    M(emboss)                                                      \
    M(swizzle)

#define SK_RASTER_PIPELINE_STAGES_HIGHP_ONLY(M)                    \
    M(callback)                                                    \
    M(stack_checkpoint) M(stack_rewind)                            \
    M(unbounded_set_rgb) M(unbounded_uniform_color)                \
    M(unpremul) M(unpremul_polar) M(dither)                        \
    M(load_16161616) M(load_16161616_dst) M(store_16161616) M(gather_16161616) \
    M(load_a16)    M(load_a16_dst)  M(store_a16)   M(gather_a16)   \
    M(load_rg1616) M(load_rg1616_dst) M(store_rg1616) M(gather_rg1616) \
    M(load_f16)    M(load_f16_dst)  M(store_f16)   M(gather_f16)   \
    M(load_af16)   M(load_af16_dst) M(store_af16)  M(gather_af16)  \
    M(load_rgf16)  M(load_rgf16_dst) M(store_rgf16) M(gather_rgf16) \
    M(load_f32)    M(load_f32_dst)  M(store_f32)   M(gather_f32)   \
    M(load_rgf32)                   M(store_rgf32)                 \
    M(load_1010102) M(load_1010102_dst) M(store_1010102) M(gather_1010102) \
    M(store_u16_be)                                                \
    M(byte_tables)                                                 \
    M(colorburn) M(colordodge) M(softlight)                        \
    M(hue) M(saturation) M(color) M(luminosity)                    \
    M(matrix_3x3) M(matrix_3x4) M(matrix_4x5) M(matrix_4x3)        \
    M(parametric) M(gamma_) M(PQish) M(HLGish) M(HLGinvish)        \
    M(rgb_to_hsl) M(hsl_to_rgb)                                    \
    M(css_lab_to_xyz) M(css_oklab_to_linear_srgb)                  \
    M(css_hcl_to_lab)                                              \
    M(css_hsl_to_srgb) M(css_hwb_to_srgb)                          \
    M(gauss_a_to_rgba)                                             \
    M(mirror_x)   M(repeat_x)                                      \
    M(mirror_y)   M(repeat_y)                                      \
    M(negate_x)                                                    \
    M(bicubic_clamp_8888)                                          \
    M(bilinear_nx) M(bilinear_px) M(bilinear_ny) M(bilinear_py)    \
    M(bicubic_setup)                                               \
    M(bicubic_n3x) M(bicubic_n1x) M(bicubic_p1x) M(bicubic_p3x)    \
    M(bicubic_n3y) M(bicubic_n1y) M(bicubic_p1y) M(bicubic_p3y)    \
    M(save_xy) M(accumulate)                                       \
    M(xy_to_2pt_conical_strip)                                     \
    M(xy_to_2pt_conical_focal_on_circle)                           \
    M(xy_to_2pt_conical_well_behaved)                              \
    M(xy_to_2pt_conical_smaller)                                   \
    M(xy_to_2pt_conical_greater)                                   \
    M(alter_2pt_conical_compensate_focal)                          \
    M(alter_2pt_conical_unswap)                                    \
    M(mask_2pt_conical_nan)                                        \
    M(mask_2pt_conical_degenerates) M(apply_vector_mask)           \
    /* Dedicated SkSL stages begin here: */                        \
    M(store_src_rg) M(immediate_f) M(store_unmasked)

// The combined list of all stages:
#define SK_RASTER_PIPELINE_STAGES_ALL(M) \
    SK_RASTER_PIPELINE_STAGES_LOWP(M)    \
    SK_RASTER_PIPELINE_STAGES_HIGHP_ONLY(M)

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

// Structs representing the arguments to some common stages.

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
    int         coordBiasInULPs = 0;
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
};

struct SkRasterPipeline_DecalTileCtx {
    uint32_t mask[SkRasterPipeline_kMaxStride];
    float    limit_x;
    float    limit_y;
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

class SkRasterPipeline {
public:
    explicit SkRasterPipeline(SkArenaAlloc*);

    SkRasterPipeline(const SkRasterPipeline&) = delete;
    SkRasterPipeline(SkRasterPipeline&&)      = default;

    SkRasterPipeline& operator=(const SkRasterPipeline&) = delete;
    SkRasterPipeline& operator=(SkRasterPipeline&&)      = default;

    void reset();

    enum Stage {
    #define M(stage) stage,
        SK_RASTER_PIPELINE_STAGES_ALL(M)
    #undef M
    };

#define M(st) +1
    static constexpr int kNumLowpStages  = SK_RASTER_PIPELINE_STAGES_LOWP(M);
    static constexpr int kNumHighpStages = SK_RASTER_PIPELINE_STAGES_ALL(M);
#undef M

    void append(Stage, void* = nullptr);
    void append(Stage stage, const void* ctx) { this->append(stage, const_cast<void*>(ctx)); }
    void append(Stage, uintptr_t ctx);

    // Append all stages to this pipeline.
    void extend(const SkRasterPipeline&);

    // Runs the pipeline in 2d from (x,y) inclusive to (x+w,y+h) exclusive.
    void run(size_t x, size_t y, size_t w, size_t h) const;

    // Allocates a thunk which amortizes run() setup cost in alloc.
    std::function<void(size_t, size_t, size_t, size_t)> compile() const;

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
    struct StageList {
        StageList* prev;
        Stage      stage;
        void*      ctx;
    };

    bool build_lowp_pipeline(SkRasterPipelineStage* ip) const;
    void build_highp_pipeline(SkRasterPipelineStage* ip) const;

    using StartPipelineFn = void(*)(size_t,size_t,size_t,size_t, SkRasterPipelineStage* program);
    StartPipelineFn build_pipeline(SkRasterPipelineStage*) const;

    void unchecked_append(Stage, void*);
    int stages_needed() const;

    SkArenaAlloc*               fAlloc;
    SkRasterPipeline_RewindCtx* fRewindCtx;
    StageList*                  fStages;
    int                         fNumStages;

    friend struct TestingOnly_SkRasterPipelineInspector;
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
