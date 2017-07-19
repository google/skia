/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_DEFINED
#define SkRasterPipeline_DEFINED

#include "SkArenaAlloc.h"
#include "SkImageInfo.h"
#include "SkNx.h"
#include "SkTArray.h"
#include "SkTypes.h"
#include <functional>
#include <vector>

struct SkJumper_constants;
struct SkJumper_Engine;
struct SkPM4f;

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
 * arbitrary context pointer.  The stage funciton arguments and calling convention are
 * designed to maximize the amount of data we can pass along the pipeline cheaply, and
 * vary depending on CPU feature detection.
 *
 * If you'd like to see how this works internally, you want to start digging around src/jumper.
 */

#define SK_RASTER_PIPELINE_STAGES(M)                             \
    M(callback)                                                  \
    M(move_src_dst) M(move_dst_src)                              \
    M(clamp_0) M(clamp_1) M(clamp_a) M(clamp_a_dst)              \
    M(unpremul) M(premul) M(premul_dst)                          \
    M(set_rgb) M(swap_rb)                                        \
    M(from_srgb) M(from_srgb_dst) M(to_srgb)                     \
    M(black_color) M(white_color) M(uniform_color)               \
    M(seed_shader) M(dither)                                     \
    M(load_a8)   M(load_a8_dst)   M(store_a8)   M(gather_a8)     \
    M(load_g8)   M(load_g8_dst)                 M(gather_g8)     \
    M(load_565)  M(load_565_dst)  M(store_565)  M(gather_565)    \
    M(load_4444) M(load_4444_dst) M(store_4444) M(gather_4444)   \
    M(load_f16)  M(load_f16_dst)  M(store_f16)  M(gather_f16)    \
    M(load_f32)  M(load_f32_dst)  M(store_f32)                   \
    M(load_8888) M(load_8888_dst) M(store_8888) M(gather_8888)   \
    M(load_bgra) M(load_bgra_dst) M(store_bgra) M(gather_bgra)   \
    M(load_u16_be) M(load_rgb_u16_be) M(store_u16_be)            \
    M(load_tables_u16_be) M(load_tables_rgb_u16_be)              \
    M(load_tables) M(load_rgba) M(store_rgba)                    \
    M(scale_u8) M(scale_1_float)                                 \
    M(lerp_u8) M(lerp_565) M(lerp_1_float)                       \
    M(dstatop) M(dstin) M(dstout) M(dstover)                     \
    M(srcatop) M(srcin) M(srcout) M(srcover)                     \
    M(clear) M(modulate) M(multiply) M(plus_) M(screen) M(xor_)  \
    M(colorburn) M(colordodge) M(darken) M(difference)           \
    M(exclusion) M(hardlight) M(lighten) M(overlay) M(softlight) \
    M(hue) M(saturation) M(color) M(luminosity)                  \
    M(srcover_rgba_8888)                                         \
    M(luminance_to_alpha)                                        \
    M(matrix_translate) M(matrix_scale_translate)                \
    M(matrix_2x3) M(matrix_3x4) M(matrix_4x5) M(matrix_4x3)      \
    M(matrix_perspective)                                        \
    M(parametric_r) M(parametric_g) M(parametric_b)              \
    M(parametric_a)                                              \
    M(table_r) M(table_g) M(table_b) M(table_a)                  \
    M(lab_to_xyz)                                                \
    M(clamp_x)   M(mirror_x)   M(repeat_x)                       \
    M(clamp_y)   M(mirror_y)   M(repeat_y)                       \
    M(clamp_x_1) M(mirror_x_1) M(repeat_x_1)                     \
    M(bilinear_nx) M(bilinear_px) M(bilinear_ny) M(bilinear_py)  \
    M(bicubic_n3x) M(bicubic_n1x) M(bicubic_p1x) M(bicubic_p3x)  \
    M(bicubic_n3y) M(bicubic_n1y) M(bicubic_p1y) M(bicubic_p3y)  \
    M(save_xy) M(accumulate)                                     \
    M(evenly_spaced_gradient)                                    \
    M(gauss_a_to_rgba) M(gradient)                               \
    M(evenly_spaced_2_stop_gradient)                             \
    M(xy_to_unit_angle)                                          \
    M(xy_to_radius)                                              \
    M(xy_to_2pt_conical_quadratic_min)                           \
    M(xy_to_2pt_conical_quadratic_max)                           \
    M(xy_to_2pt_conical_linear)                                  \
    M(mask_2pt_conical_degenerates) M(apply_vector_mask)         \
    M(byte_tables) M(byte_tables_rgb)                            \
    M(rgb_to_hsl) M(hsl_to_rgb)                                  \
    M(store_8888_2d)

class SkRasterPipeline {
public:
    explicit SkRasterPipeline(SkArenaAlloc*);

    SkRasterPipeline(const SkRasterPipeline&) = delete;
    SkRasterPipeline(SkRasterPipeline&&)      = default;

    SkRasterPipeline& operator=(const SkRasterPipeline&) = delete;
    SkRasterPipeline& operator=(SkRasterPipeline&&)      = default;

    void reset();

    enum StockStage {
    #define M(stage) stage,
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
    };
    void append(StockStage, void* = nullptr);
    void append(StockStage stage, const void* ctx) { this->append(stage, const_cast<void*>(ctx)); }

    // Append all stages to this pipeline.
    void extend(const SkRasterPipeline&);

    // Runs the pipeline walking x through [x,x+n).
    void run(size_t x, size_t y, size_t n) const;

    // Runs the pipeline in 2d from (x,y) inclusive to (x+w,y+h) exclusive.
    void run_2d(size_t x, size_t y, size_t w, size_t h) const;

    // Allocates a thunk which amortizes run() setup cost in alloc.
    std::function<void(size_t, size_t, size_t)> compile() const;

    void dump() const;

    // Conversion from sRGB can be subtly tricky when premultiplication is involved.
    // Use these helpers to keep things sane.
    void append_from_srgb(SkAlphaType);
    void append_from_srgb_dst(SkAlphaType);

    // Appends a stage for the specified matrix. Tries to optimize the stage by analyzing
    // the type of matrix.
    void append_matrix(SkArenaAlloc*, const SkMatrix&);

    // Appends a stage for the uniform color. Tries to optimize the stage based on the color.
    void append_uniform_color(SkArenaAlloc*, const SkPM4f& color);

    bool empty() const { return fStages == nullptr; }

private:
    struct StageList {
        StageList* prev;
        StockStage stage;
        void*      ctx;
    };

    const SkJumper_Engine& build_pipeline(void**) const;
    void unchecked_append(StockStage, void*);

    SkArenaAlloc* fAlloc;
    StageList*    fStages;
    int           fNumStages;
    int           fSlotsNeeded;
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
