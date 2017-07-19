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
#include "../jumper/SkJumper.h"

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

#define SK_RASTER_PIPELINE_STAGES(M)                               \
    M(callback)                                                    \
    M(move_src_dst) M(move_dst_src)                                \
    M(clamp_0) M(clamp_1) M(clamp_a) M(clamp_a_dst)                \
    M(unpremul) M(premul)                                          \
    M(set_rgb) M(swap_rb)                                          \
    M(from_srgb) M(from_srgb_dst) M(to_srgb)                       \
    M(black_color) M(white_color) M(uniform_color)                 \
    M(seed_shader) M(dither)                                       \
    M(load_a8)   M(load_a8_dst)   M(store_a8)   M(gather_a8)       \
    M(load_g8)   M(load_g8_dst)                 M(gather_g8)       \
    M(load_565)  M(load_565_dst)  M(store_565)  M(gather_565)      \
    M(load_4444) M(load_4444_dst) M(store_4444) M(gather_4444)     \
    M(load_f16)  M(load_f16_dst)  M(store_f16)  M(gather_f16)      \
    M(load_f32)  M(load_f32_dst)  M(store_f32)                     \
    M(load_8888) M(load_8888_dst) M(store_8888) M(gather_8888)     \
    M(load_bgra) M(load_bgra_dst) M(store_bgra) M(gather_bgra)     \
    M(load_u16_be) M(load_rgb_u16_be) M(store_u16_be)              \
    M(load_tables) M(load_tables_u16_be) M(load_tables_rgb_u16_be) \
    M(load_rgba) M(store_rgba)                                     \
    M(scale_u8) M(scale_1_float)                                   \
    M(lerp_u8) M(lerp_565) M(lerp_1_float)                         \
    M(dstatop) M(dstin) M(dstout) M(dstover)                       \
    M(srcatop) M(srcin) M(srcout) M(srcover)                       \
    M(clear) M(modulate) M(multiply) M(plus_) M(screen) M(xor_)    \
    M(colorburn) M(colordodge) M(darken) M(difference)             \
    M(exclusion) M(hardlight) M(lighten) M(overlay) M(softlight)   \
    M(hue) M(saturation) M(color) M(luminosity)                    \
    M(srcover_rgba_8888)                                           \
    M(luminance_to_alpha)                                          \
    M(matrix_translate) M(matrix_scale_translate)                  \
    M(matrix_2x3) M(matrix_3x4) M(matrix_4x5) M(matrix_4x3)        \
    M(matrix_perspective)                                          \
    M(parametric_r) M(parametric_g) M(parametric_b)                \
    M(parametric_a)                                                \
    M(table_r) M(table_g) M(table_b) M(table_a)                    \
    M(lab_to_xyz)                                                  \
    M(clamp_x)   M(mirror_x)   M(repeat_x)                         \
    M(clamp_y)   M(mirror_y)   M(repeat_y)                         \
    M(clamp_x_1) M(mirror_x_1) M(repeat_x_1)                       \
    M(bilinear_nx) M(bilinear_px) M(bilinear_ny) M(bilinear_py)    \
    M(bicubic_n3x) M(bicubic_n1x) M(bicubic_p1x) M(bicubic_p3x)    \
    M(bicubic_n3y) M(bicubic_n1y) M(bicubic_p1y) M(bicubic_p3y)    \
    M(save_xy) M(accumulate)                                       \
    M(evenly_spaced_gradient)                                      \
    M(gauss_a_to_rgba) M(gradient)                                 \
    M(evenly_spaced_2_stop_gradient)                               \
    M(xy_to_unit_angle)                                            \
    M(xy_to_radius)                                                \
    M(xy_to_2pt_conical_quadratic_min)                             \
    M(xy_to_2pt_conical_quadratic_max)                             \
    M(xy_to_2pt_conical_linear)                                    \
    M(mask_2pt_conical_degenerates) M(apply_vector_mask)           \
    M(byte_tables) M(byte_tables_rgb)                              \
    M(rgb_to_hsl) M(hsl_to_rgb)                                    \
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

    // Typesafe append() methods.
    void append_callback(SkJumper_CallbackCtx* ctx) { this->append(callback, ctx); }

    void append_move_src_dst() { this->append(move_src_dst); }
    void append_move_dst_src() { this->append(move_dst_src); }

    void append_clamp_0()     { this->append(clamp_0); }
    void append_clamp_1()     { this->append(clamp_1); }
    void append_clamp_a()     { this->append(clamp_a); }
    void append_clamp_a_dst() { this->append(clamp_a_dst); }

    void append_set_rgb(const float rgb[3]) { this->append(set_rgb, rgb); }
    void append_swap_rb() { this->append(swap_rb); }

    void append_unpremul() { this->append(unpremul); }
    void append_premul()   { this->append(premul); }

    void append_from_srgb()     { this->append(from_srgb); }
    void append_from_srgb_dst() { this->append(from_srgb_dst); }
    void append_to_srgb()       { this->append(to_srgb); }

    void append_black_color()                      { this->append(black_color); }
    void append_white_color()                      { this->append(white_color); }
    void append_uniform_color(const float rgba[4]) { this->append(uniform_color, rgba); }

    void append_seed_shader()             { this->append(seed_shader); }
    void append_dither(const float* rate) { this->append(dither, rate); }

    void append_load_a8    (    const uint8_t** ctx) { this->append(load_a8    , ctx); }
    void append_load_a8_dst(    const uint8_t** ctx) { this->append(load_a8_dst, ctx); }
    void append_store_a8   (          uint8_t** ctx) { this->append(store_a8   , ctx); }
    void append_gather_a8  (SkJumper_MemoryCtx* ctx) { this->append(gather_a8  , ctx); }

    void append_load_g8    (    const uint8_t** ctx) { this->append(load_g8    , ctx); }
    void append_load_g8_dst(    const uint8_t** ctx) { this->append(load_g8_dst, ctx); }
    void append_gather_g8  (SkJumper_MemoryCtx* ctx) { this->append(gather_g8  , ctx); }

    void append_load_565    (   const uint16_t** ctx) { this->append(load_565    , ctx); }
    void append_load_565_dst(   const uint16_t** ctx) { this->append(load_565_dst, ctx); }
    void append_store_565   (         uint16_t** ctx) { this->append(store_565   , ctx); }
    void append_gather_565  (SkJumper_MemoryCtx* ctx) { this->append(gather_565  , ctx); }

    void append_load_4444    (   const uint16_t** ctx) { this->append(load_4444    , ctx); }
    void append_load_4444_dst(   const uint16_t** ctx) { this->append(load_4444_dst, ctx); }
    void append_store_4444   (         uint16_t** ctx) { this->append(store_4444   , ctx); }
    void append_gather_4444  (SkJumper_MemoryCtx* ctx) { this->append(gather_4444  , ctx); }

    void append_load_f16    (   const uint64_t** ctx) { this->append(load_f16    , ctx); }
    void append_load_f16_dst(   const uint64_t** ctx) { this->append(load_f16_dst, ctx); }
    void append_store_f16   (         uint64_t** ctx) { this->append(store_f16   , ctx); }
    void append_gather_f16  (SkJumper_MemoryCtx* ctx) { this->append(gather_f16  , ctx); }

    void append_load_f32    (      const float** ctx) { this->append(load_f32    , ctx); }
    void append_load_f32_dst(      const float** ctx) { this->append(load_f32_dst, ctx); }
    void append_store_f32   (            float** ctx) { this->append(store_f32   , ctx); }

    void append_load_8888    (   const uint32_t** ctx) { this->append(load_8888    , ctx); }
    void append_load_8888_dst(   const uint32_t** ctx) { this->append(load_8888_dst, ctx); }
    void append_store_8888   (         uint32_t** ctx) { this->append(store_8888   , ctx); }
    void append_gather_8888  (SkJumper_MemoryCtx* ctx) { this->append(gather_8888  , ctx); }

    void append_load_bgra    (   const uint32_t** ctx) { this->append(load_bgra    , ctx); }
    void append_load_bgra_dst(   const uint32_t** ctx) { this->append(load_bgra_dst, ctx); }
    void append_store_bgra   (         uint32_t** ctx) { this->append(store_bgra   , ctx); }
    void append_gather_bgra  (SkJumper_MemoryCtx* ctx) { this->append(gather_bgra  , ctx); }

    void append_load_u16_be    (const uint16_t** ctx) { this->append(load_u16_be    , ctx); }
    void append_load_rgb_u16_be(const uint16_t** ctx) { this->append(load_rgb_u16_be, ctx); }
    void append_store_u16_be   (      uint16_t** ctx) { this->append(store_u16_be   , ctx); }

    void append_load_tables(const SkJumper_LoadTablesCtx* ctx) { this->append(load_tables, ctx); }
    void append_load_tables_u16_be(const SkJumper_LoadTablesCtx* ctx) {
        this->append(load_tables_u16_be, ctx);
    }
    void append_load_tables_rgb_u16_be(const SkJumper_LoadTablesCtx* ctx) {
        this->append(load_tables_rgb_u16_be, ctx);
    }

    void append_load_rgba (const float ctx[SkJumper_kMaxStride*4]) {
        this->append(load_rgba , ctx);
    }
    void append_store_rgba(      float ctx[SkJumper_kMaxStride*4]) {
        this->append(store_rgba, ctx);
    }

    void append_scale_u8     (const uint8_t** ctx) { this->append(scale_u8,      ctx); }
    void append_scale_1_float(const float*    ctx) { this->append(scale_1_float, ctx); }

    void append_lerp_u8     (const uint8_t**  ctx) { this->append(lerp_u8,      ctx); }
    void append_lerp_565    (const uint16_t** ctx) { this->append(lerp_565,     ctx); }
    void append_lerp_1_float(const float*     ctx) { this->append(lerp_1_float, ctx); }

    void append_dstatop() { this->append(dstatop); }
    void append_dstin()   { this->append(dstin); }
    void append_dstout()  { this->append(dstout); }
    void append_dstover() { this->append(dstover); }

    void append_srcatop() { this->append(srcatop); }
    void append_srcin()   { this->append(srcin); }
    void append_srcout()  { this->append(srcout); }
    void append_srcover() { this->append(srcover); }

    void append_clear()    { this->append(clear); }
    void append_modulate() { this->append(modulate); }
    void append_multiply() { this->append(multiply); }
    void append_plus_()    { this->append(plus_); }
    void append_screen()   { this->append(screen); }
    void append_xor_()     { this->append(xor_); }

    void append_colorburn()  { this->append(colorburn); }
    void append_colordodge() { this->append(colordodge); }
    void append_darken()     { this->append(darken); }
    void append_difference() { this->append(difference); }
    void append_exclusion()  { this->append(exclusion); }
    void append_hardlight()  { this->append(hardlight); }
    void append_lighten()    { this->append(lighten); }
    void append_overlay()    { this->append(overlay); }
    void append_softlight()  { this->append(softlight); }

    void append_hue()        { this->append(hue); }
    void append_saturation() { this->append(saturation); }
    void append_color()      { this->append(color); }
    void append_luminosity() { this->append(luminosity); }

    void append_srcover_rgba_8888(uint32_t** ctx) { this->append(srcover_rgba_8888, ctx); }

    void append_luminance_to_alpha() { this->append(luminance_to_alpha); }

    /*
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
    */

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
