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
    M(unpremul) M(premul) M(premul_dst)                            \
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
    M(luminance_to_alpha) M(gauss_a_to_rgba)                       \
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
    M(gradient)                                                    \
    M(evenly_spaced_gradient)                                      \
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

    void append_unpremul()   { this->append(unpremul); }
    void append_premul()     { this->append(premul); }
    void append_premul_dst() { this->append(premul_dst); }

    void append_from_srgb    (SkAlphaType);
    void append_from_srgb_dst(SkAlphaType);
    void append_to_srgb() { this->append(to_srgb); }

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
    void append_gauss_a_to_rgba()    { this->append(gauss_a_to_rgba); }

    void append_matrix_translate(const float ctx[2]) { this->append(matrix_translate, ctx); }
    void append_matrix_scale_translate(const float ctx[4]) {
        this->append(matrix_scale_translate, ctx);
    }
    void append_matrix_2x3        (const float ctx[ 6]) { this->append(matrix_2x3        , ctx); }
    void append_matrix_3x4        (const float ctx[12]) { this->append(matrix_3x4        , ctx); }
    void append_matrix_4x5        (const float ctx[20]) { this->append(matrix_4x5        , ctx); }
    void append_matrix_4x3        (const float ctx[12]) { this->append(matrix_4x3        , ctx); }
    void append_matrix_perspective(const float ctx[ 9]) { this->append(matrix_perspective, ctx); }

    void append_parametric_r(const SkJumper_ParametricTransferFunction* ctx) {
        this->append(parametric_r, ctx);
    }
    void append_parametric_g(const SkJumper_ParametricTransferFunction* ctx) {
        this->append(parametric_g, ctx);
    }
    void append_parametric_b(const SkJumper_ParametricTransferFunction* ctx) {
        this->append(parametric_b, ctx);
    }
    void append_parametric_a(const SkJumper_ParametricTransferFunction* ctx) {
        this->append(parametric_a, ctx);
    }

    void append_table_r(const SkJumper_TableCtx* ctx) { this->append(table_r, ctx); }
    void append_table_g(const SkJumper_TableCtx* ctx) { this->append(table_g, ctx); }
    void append_table_b(const SkJumper_TableCtx* ctx) { this->append(table_b, ctx); }
    void append_table_a(const SkJumper_TableCtx* ctx) { this->append(table_a, ctx); }

    void append_lab_to_xyz() { this->append(lab_to_xyz); }

    void append_clamp_x (const SkJumper_TileCtx* ctx) { this->append(clamp_x , ctx); }
    void append_mirror_x(const SkJumper_TileCtx* ctx) { this->append(mirror_x, ctx); }
    void append_repeat_x(const SkJumper_TileCtx* ctx) { this->append(repeat_x, ctx); }

    void append_clamp_y (const SkJumper_TileCtx* ctx) { this->append(clamp_y , ctx); }
    void append_mirror_y(const SkJumper_TileCtx* ctx) { this->append(mirror_y, ctx); }
    void append_repeat_y(const SkJumper_TileCtx* ctx) { this->append(repeat_y, ctx); }

    void append_clamp_x_1 () { this->append(clamp_x_1 ); }
    void append_mirror_x_1() { this->append(mirror_x_1); }
    void append_repeat_x_1() { this->append(repeat_x_1); }

    void append_bilinear_nx(SkJumper_SamplerCtx* ctx) { this->append(bilinear_nx, ctx); }
    void append_bilinear_px(SkJumper_SamplerCtx* ctx) { this->append(bilinear_px, ctx); }
    void append_bilinear_ny(SkJumper_SamplerCtx* ctx) { this->append(bilinear_ny, ctx); }
    void append_bilinear_py(SkJumper_SamplerCtx* ctx) { this->append(bilinear_py, ctx); }

    void append_bicubic_n1x(SkJumper_SamplerCtx* ctx) { this->append(bicubic_n1x, ctx); }
    void append_bicubic_p1x(SkJumper_SamplerCtx* ctx) { this->append(bicubic_p1x, ctx); }
    void append_bicubic_n1y(SkJumper_SamplerCtx* ctx) { this->append(bicubic_n1y, ctx); }
    void append_bicubic_p1y(SkJumper_SamplerCtx* ctx) { this->append(bicubic_p1y, ctx); }

    void append_bicubic_n3x(SkJumper_SamplerCtx* ctx) { this->append(bicubic_n3x, ctx); }
    void append_bicubic_p3x(SkJumper_SamplerCtx* ctx) { this->append(bicubic_p3x, ctx); }
    void append_bicubic_n3y(SkJumper_SamplerCtx* ctx) { this->append(bicubic_n3y, ctx); }
    void append_bicubic_p3y(SkJumper_SamplerCtx* ctx) { this->append(bicubic_p3y, ctx); }

    void append_save_xy   (      SkJumper_SamplerCtx* ctx) { this->append(save_xy   , ctx); }
    void append_accumulate(const SkJumper_SamplerCtx* ctx) { this->append(accumulate, ctx); }

    void append_gradient(const SkJumper_GradientCtx* ctx) { this->append(gradient, ctx); }
    void append_evenly_spaced_gradient(const SkJumper_GradientCtx* ctx) {
        this->append(evenly_spaced_gradient, ctx);
    }
    void append_evenly_spaced_2_stop_gradient(const SkJumper_2StopGradientCtx* ctx) {
        this->append(evenly_spaced_2_stop_gradient, ctx);
    }

    void append_xy_to_unit_angle() { this->append(xy_to_unit_angle); }
    void append_xy_to_radius()     { this->append(xy_to_radius); }

    void append_xy_to_2pt_conical_quadratic_min(const SkJumper_2PtConicalCtx* ctx) {
        this->append(xy_to_2pt_conical_quadratic_min, ctx);
    }
    void append_xy_to_2pt_conical_quadratic_max(const SkJumper_2PtConicalCtx* ctx) {
        this->append(xy_to_2pt_conical_quadratic_max, ctx);
    }
    void append_xy_to_2pt_conical_linear(const SkJumper_2PtConicalCtx* ctx) {
        this->append(xy_to_2pt_conical_linear, ctx);
    }
    void append_mask_2pt_conical_degenerates(SkJumper_2PtConicalCtx* ctx) {
        this->append(mask_2pt_conical_degenerates, ctx);
    }
    void append_apply_vector_mask(const uint32_t ctx[SkJumper_kMaxStride]) {
        this->append(apply_vector_mask, ctx);
    }

    void append_byte_tables(const SkJumper_ByteTablesCtx* ctx) {
        this->append(byte_tables, ctx);
    }
    void append_byte_tables_rgb(const SkJumper_ByteTablesRGBCtx* ctx) {
        this->append(byte_tables_rgb, ctx);
    }

    void append_rgb_to_hsl() { this->append(rgb_to_hsl); }
    void append_hsl_to_rgb() { this->append(hsl_to_rgb); }

    void append_store_8888_2d(SkJumper_MemoryCtx* ctx) { this->append(store_8888_2d, ctx); }


    // Appends a stage for the specified matrix. Tries to optimize the stage by analyzing
    // the type of matrix.
    void append_matrix(SkArenaAlloc*, const SkMatrix&);

    // Appends a stage for a constant color. Tries to optimize the stage based on the color.
    void append_constant_color(SkArenaAlloc*, const SkPM4f& color);

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
