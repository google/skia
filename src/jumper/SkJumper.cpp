/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorData.h"
#include "SkCpu.h"
#include "SkJumper.h"
#include "SkOnce.h"
#include "SkRasterPipeline.h"
#include "SkTemplates.h"

#if !defined(SK_JUMPER_USE_ASSEMBLY)
    // We'll use __has_feature(memory_sanitizer) to detect MSAN.
    // SkJumper_generated.S is not compiled with MSAN, so MSAN would yell really loud.
    #if !defined(__has_feature)
        #define __has_feature(x) 0
    #endif

    #if 0 || __has_feature(memory_sanitizer)
        #define SK_JUMPER_USE_ASSEMBLY 0
    #else
        #define SK_JUMPER_USE_ASSEMBLY 1
    #endif
#endif

#define M(st) +1
static const int kNumStages = SK_RASTER_PIPELINE_STAGES(M);
#undef M

#ifndef SK_JUMPER_DISABLE_8BIT
    // Intentionally commented out; optional logging for local debugging.
    #if 0 && SK_JUMPER_USE_ASSEMBLY && (defined(__x86_64__) || defined(_M_X64))
        #include <atomic>

        #define M(st) #st,
        static const char* kStageNames[] = { SK_RASTER_PIPELINE_STAGES(M) };
        #undef M

        static std::atomic<int> gMissingStageCounters[kNumStages];

        static void log_missing(SkRasterPipeline::StockStage st) {
            static SkOnce once;
            once([] { atexit([] {
                int total = 0;
                for (int i = 0; i < kNumStages; i++) {
                    if (int count = gMissingStageCounters[i].load()) {
                        SkDebugf("%7d\t%s\n", count, kStageNames[i]);
                        total += count;
                    }
                }
                SkDebugf("%7d total\n", total);
            }); });

            gMissingStageCounters[st]++;
        }
    #else
        static void log_missing(SkRasterPipeline::StockStage) {}
    #endif
#endif

// We can't express the real types of most stage functions portably, so we use a stand-in.
// We'll only ever call start_pipeline(), which then chains into the rest.
using StageFn         = void(void);
using StartPipelineFn = void(size_t,size_t,size_t,size_t, void**);

// Some platforms expect C "name" maps to asm "_name", others to "name".
#if defined(__APPLE__)
    #define ASM(name, suffix)  sk_##name##_##suffix
#else
    #define ASM(name, suffix) _sk_##name##_##suffix
#endif

extern "C" {

#if !SK_JUMPER_USE_ASSEMBLY
    // We'll just run baseline code.

#elif defined(__x86_64__) || defined(_M_X64)
    StartPipelineFn ASM(start_pipeline,       skx),
                    ASM(start_pipeline,       hsw),
                    ASM(start_pipeline,       avx),
                    ASM(start_pipeline,     sse41),
                    ASM(start_pipeline,      sse2),
                    ASM(start_pipeline,  hsw_lowp),
                    ASM(start_pipeline,sse41_lowp),
                    ASM(start_pipeline, sse2_lowp);

    StageFn ASM(just_return,       skx),
            ASM(just_return,       hsw),
            ASM(just_return,       avx),
            ASM(just_return,     sse41),
            ASM(just_return,      sse2),
            ASM(just_return,  hsw_lowp),
            ASM(just_return,sse41_lowp),
            ASM(just_return, sse2_lowp);

    #define M(st) StageFn ASM(st,  skx),      \
                          ASM(st,  hsw),      \
                          ASM(st,  avx),      \
                          ASM(st,sse41),      \
                          ASM(st, sse2),      \
                          ASM(st,  hsw_lowp), \
                          ASM(st,sse41_lowp), \
                          ASM(st, sse2_lowp);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#elif defined(__i386__) || defined(_M_IX86)
    StartPipelineFn ASM(start_pipeline,sse2),
                    ASM(start_pipeline,sse2_lowp);
    StageFn ASM(just_return,sse2),
            ASM(just_return,sse2_lowp);
    #define M(st) StageFn ASM(st,sse2),      \
                          ASM(st,sse2_lowp);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#endif

    // Baseline code compiled as a normal part of Skia.
    StartPipelineFn sk_start_pipeline;
    StageFn sk_just_return;
    #define M(st) StageFn sk_##st;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#if defined(JUMPER_HAS_NEON_LOWP)
    StartPipelineFn sk_start_pipeline_lowp;
    StageFn sk_just_return_lowp;
    #define M(st) StageFn sk_##st##_lowp;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
#endif

}

#if SK_JUMPER_USE_ASSEMBLY
    #if defined(__x86_64__) || defined(_M_X64)
        template <SkRasterPipeline::StockStage st>
        static constexpr StageFn* hsw_lowp();

        template <SkRasterPipeline::StockStage st>
        static constexpr StageFn* sse41_lowp();

        template <SkRasterPipeline::StockStage st>
        static constexpr StageFn* sse2_lowp();

        #define LOWP(st) \
            template <> constexpr StageFn* hsw_lowp<SkRasterPipeline::st>() {   \
                return ASM(st,hsw_lowp);                                        \
            }                                                                   \
            template <> constexpr StageFn* sse41_lowp<SkRasterPipeline::st>() { \
                return ASM(st,sse41_lowp);                                      \
            }                                                                   \
            template <> constexpr StageFn* sse2_lowp<SkRasterPipeline::st>() {  \
                return ASM(st,sse2_lowp);                                       \
            }
        #define NOPE(st) \
            template <> constexpr StageFn* hsw_lowp<SkRasterPipeline::st>() {   \
                return nullptr;                                                 \
            }                                                                   \
            template <> constexpr StageFn* sse41_lowp<SkRasterPipeline::st>() { \
                return nullptr;                                                 \
            }                                                                   \
            template <> constexpr StageFn* sse2_lowp<SkRasterPipeline::st>() {  \
                return nullptr;                                                 \
            }

    #elif defined(__i386__) || defined(_M_IX86)
        template <SkRasterPipeline::StockStage st>
        static constexpr StageFn* sse2_lowp();

        #define LOWP(st) \
            template <> constexpr StageFn* sse2_lowp<SkRasterPipeline::st>() {  \
                return ASM(st,sse2_lowp);                                       \
            }
        #define NOPE(st) \
            template <> constexpr StageFn* sse2_lowp<SkRasterPipeline::st>() {  \
                return nullptr;                                                 \
            }

    #elif defined(JUMPER_HAS_NEON_LOWP)
        template <SkRasterPipeline::StockStage st>
        static constexpr StageFn* neon_lowp();

        #define LOWP(st)                                                         \
            template <> constexpr StageFn* neon_lowp<SkRasterPipeline::st>() {   \
                return sk_##st##_lowp;                                           \
            }
        #define NOPE(st)                                                         \
            template <> constexpr StageFn* neon_lowp<SkRasterPipeline::st>() {   \
                return nullptr;                                                  \
            }

    #else
        #define LOWP(st)
        #define NOPE(st)

    #endif

    #define TODO(st) NOPE(st)  // stages that should be implemented in lowp, but aren't.

    NOPE(callback)
    LOWP(move_src_dst) LOWP(move_dst_src)
    NOPE(clamp_0) NOPE(clamp_1) LOWP(clamp_a) LOWP(clamp_a_dst)
    NOPE(unpremul) LOWP(premul) LOWP(premul_dst)
    LOWP(force_opaque) LOWP(force_opaque_dst)
    LOWP(set_rgb) LOWP(swap_rb) LOWP(invert)
    NOPE(from_srgb) NOPE(from_srgb_dst) NOPE(to_srgb)
    LOWP(black_color) LOWP(white_color) LOWP(uniform_color)
    LOWP(seed_shader) NOPE(dither)
    LOWP(load_a8)   LOWP(load_a8_dst)   LOWP(store_a8)   LOWP(gather_a8)
    LOWP(load_g8)   LOWP(load_g8_dst)                    LOWP(gather_g8)
    LOWP(load_565)  LOWP(load_565_dst)  LOWP(store_565)  LOWP(gather_565)
    LOWP(load_4444) LOWP(load_4444_dst) LOWP(store_4444) LOWP(gather_4444)
    NOPE(load_f16)  NOPE(load_f16_dst)  NOPE(store_f16)  NOPE(gather_f16)
    NOPE(load_f32)  NOPE(load_f32_dst)  NOPE(store_f32)
    LOWP(load_8888) LOWP(load_8888_dst) LOWP(store_8888) LOWP(gather_8888)
    LOWP(load_bgra) LOWP(load_bgra_dst) LOWP(store_bgra) LOWP(gather_bgra)
    NOPE(load_1010102) NOPE(load_1010102_dst) NOPE(store_1010102) NOPE(gather_1010102)
    TODO(bilerp_clamp_8888)
    TODO(load_u16_be) TODO(load_rgb_u16_be) TODO(store_u16_be)
    NOPE(load_tables_u16_be) NOPE(load_tables_rgb_u16_be) NOPE(load_tables)
    NOPE(load_rgba) NOPE(store_rgba)
    LOWP(scale_u8) LOWP(scale_565) LOWP(scale_1_float)
    LOWP( lerp_u8) LOWP( lerp_565) LOWP( lerp_1_float)
    LOWP(dstatop) LOWP(dstin) LOWP(dstout) LOWP(dstover)
    LOWP(srcatop) LOWP(srcin) LOWP(srcout) LOWP(srcover)
    LOWP(clear) LOWP(modulate) LOWP(multiply) LOWP(plus_) LOWP(screen) LOWP(xor_)
    NOPE(colorburn) NOPE(colordodge) LOWP(darken) LOWP(difference)
    LOWP(exclusion) LOWP(hardlight) LOWP(lighten) LOWP(overlay) NOPE(softlight)
    NOPE(hue) NOPE(saturation) NOPE(color) NOPE(luminosity)
    LOWP(srcover_rgba_8888) LOWP(srcover_bgra_8888)
    LOWP(luminance_to_alpha)
    LOWP(matrix_translate) LOWP(matrix_scale_translate)
    LOWP(matrix_2x3) NOPE(matrix_3x4) TODO(matrix_4x5) TODO(matrix_4x3)
    LOWP(matrix_perspective)
    NOPE(parametric_r) NOPE(parametric_g) NOPE(parametric_b)
    NOPE(parametric_a) NOPE(gamma) NOPE(gamma_dst)
    NOPE(table_r) NOPE(table_g) NOPE(table_b) NOPE(table_a)
    NOPE(lab_to_xyz)
                    TODO(mirror_x)   TODO(repeat_x)
                    TODO(mirror_y)   TODO(repeat_y)
                    LOWP(decal_x)    LOWP(decal_y)   LOWP(decal_x_and_y)
    LOWP(check_decal_mask)
    TODO(bilinear_nx) TODO(bilinear_px) TODO(bilinear_ny) TODO(bilinear_py)
    TODO(bicubic_n3x) TODO(bicubic_n1x) TODO(bicubic_p1x) TODO(bicubic_p3x)
    TODO(bicubic_n3y) TODO(bicubic_n1y) TODO(bicubic_p1y) TODO(bicubic_p3y)
    TODO(save_xy) TODO(accumulate)
    LOWP(clamp_x_1) LOWP(mirror_x_1) LOWP(repeat_x_1)
    LOWP(evenly_spaced_gradient)
    LOWP(gradient)
    LOWP(evenly_spaced_2_stop_gradient)
    LOWP(xy_to_unit_angle)
    LOWP(xy_to_radius)
    TODO(negate_x)
    TODO(xy_to_2pt_conical_strip)
    TODO(xy_to_2pt_conical_focal_on_circle)
    TODO(xy_to_2pt_conical_well_behaved)
    TODO(xy_to_2pt_conical_greater)
    TODO(xy_to_2pt_conical_smaller)
    TODO(alter_2pt_conical_compensate_focal)
    TODO(alter_2pt_conical_unswap)
    TODO(mask_2pt_conical_nan) TODO(mask_2pt_conical_degenerates) TODO(apply_vector_mask)
    TODO(byte_tables) TODO(byte_tables_rgb)
    NOPE(rgb_to_hsl) NOPE(hsl_to_rgb)
    NOPE(clut_3D) NOPE(clut_4D)
    NOPE(gauss_a_to_rgba)

    #undef LOWP
    #undef TODO
    #undef NOPE
#endif

// Engines comprise everything we need to run SkRasterPipelines.
struct SkJumper_Engine {
    StageFn*         stages[kNumStages];
    StartPipelineFn* start_pipeline;
    StageFn*         just_return;
};

// We'll default to this baseline engine, but try to choose a better one at runtime.
static const SkJumper_Engine kBaseline = {
#define M(stage) sk_##stage,
    { SK_RASTER_PIPELINE_STAGES(M) },
#undef M
    sk_start_pipeline,
    sk_just_return,
};
static SkJumper_Engine gEngine = kBaseline;
static SkOnce gChooseEngineOnce;

static SkJumper_Engine choose_engine() {
#if !SK_JUMPER_USE_ASSEMBLY
    // We'll just run baseline code.

#elif defined(__x86_64__) || defined(_M_X64)
    #if !defined(_MSC_VER)  // No _skx stages for Windows yet.
        if (1 && SkCpu::Supports(SkCpu::SKX)) {
            return {
            #define M(stage) ASM(stage, skx),
                { SK_RASTER_PIPELINE_STAGES(M) },
                M(start_pipeline)
                M(just_return)
            #undef M
            };
        }
    #endif
    if (1 && SkCpu::Supports(SkCpu::HSW)) {
        return {
        #define M(stage) ASM(stage, hsw),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::AVX)) {
        return {
        #define M(stage) ASM(stage, avx),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE41)) {
        return {
        #define M(stage) ASM(stage, sse41),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        return {
        #define M(stage) ASM(stage, sse2),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(just_return)
        #undef M
        };
    }

#elif defined(__i386__) || defined(_M_IX86)
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        return {
        #define M(stage) ASM(stage, sse2),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(just_return)
        #undef M
        };
    }

#endif
    return kBaseline;
}

#ifndef SK_JUMPER_DISABLE_8BIT
    static const SkJumper_Engine kNone = {
    #define M(stage) nullptr,
        { SK_RASTER_PIPELINE_STAGES(M) },
    #undef M
        nullptr,
        nullptr,
    };
    static SkJumper_Engine gLowp = kNone;
    static SkOnce gChooseLowpOnce;

    static SkJumper_Engine choose_lowp() {
    #if SK_JUMPER_USE_ASSEMBLY
        #if defined(__x86_64__) || defined(_M_X64)
            if (1 && SkCpu::Supports(SkCpu::HSW)) {
                return {
                #define M(st) hsw_lowp<SkRasterPipeline::st>(),
                    { SK_RASTER_PIPELINE_STAGES(M) },
                    ASM(start_pipeline,hsw_lowp),
                    ASM(just_return   ,hsw_lowp),
                #undef M
                };
            }
            if (1 && SkCpu::Supports(SkCpu::SSE41)) {
                return {
                #define M(st) sse41_lowp<SkRasterPipeline::st>(),
                    { SK_RASTER_PIPELINE_STAGES(M) },
                    ASM(start_pipeline,sse41_lowp),
                    ASM(just_return   ,sse41_lowp),
                #undef M
                };
            }
            if (1 && SkCpu::Supports(SkCpu::SSE2)) {
                return {
                #define M(st) sse2_lowp<SkRasterPipeline::st>(),
                    { SK_RASTER_PIPELINE_STAGES(M) },
                    ASM(start_pipeline,sse2_lowp),
                    ASM(just_return   ,sse2_lowp),
                #undef M
                };
            }
        #elif defined(__i386__) || defined(_M_IX86)
            if (1 && SkCpu::Supports(SkCpu::SSE2)) {
                return {
                #define M(st) sse2_lowp<SkRasterPipeline::st>(),
                    { SK_RASTER_PIPELINE_STAGES(M) },
                    ASM(start_pipeline,sse2_lowp),
                    ASM(just_return   ,sse2_lowp),
                #undef M
                };
            }

        #elif defined(JUMPER_HAS_NEON_LOWP)
            return {
            #define M(st) neon_lowp<SkRasterPipeline::st>(),
                { SK_RASTER_PIPELINE_STAGES(M) },
                sk_start_pipeline_lowp,
                sk_just_return_lowp,
            #undef M
            };
        #endif
    #endif
        return kNone;
    }
#endif

const SkJumper_Engine& SkRasterPipeline::build_pipeline(void** ip) const {
#ifndef SK_JUMPER_DISABLE_8BIT
    gChooseLowpOnce([]{ gLowp = choose_lowp(); });

    // First try to build a lowp pipeline.  If that fails, fall back to normal float gEngine.
    void** reset_point = ip;
    *--ip = (void*)gLowp.just_return;
    for (const StageList* st = fStages; st; st = st->prev) {
        if (st->stage == SkRasterPipeline::clamp_0 ||
            st->stage == SkRasterPipeline::clamp_1) {
            continue;  // No-ops in lowp.
        }
        if (StageFn* fn = gLowp.stages[st->stage]) {
            if (st->ctx) {
                *--ip = st->ctx;
            }
            *--ip = (void*)fn;
        } else {
            log_missing(st->stage);
            ip = reset_point;
            break;
        }
    }
    if (ip != reset_point) {
        return gLowp;
    }
#endif

    gChooseEngineOnce([]{ gEngine = choose_engine(); });
    // We're building the pipeline backwards, so we start with the final stage just_return.
    *--ip = (void*)gEngine.just_return;

    // Still going backwards, each stage's context pointer then its StageFn.
    for (const StageList* st = fStages; st; st = st->prev) {
        if (st->ctx) {
            *--ip = st->ctx;
        }
        *--ip = (void*)gEngine.stages[st->stage];
    }
    return gEngine;
}

void SkRasterPipeline::run(size_t x, size_t y, size_t w, size_t h) const {
    if (this->empty()) {
        return;
    }

    // Best to not use fAlloc here... we can't bound how often run() will be called.
    SkAutoSTMalloc<64, void*> program(fSlotsNeeded);

    const SkJumper_Engine& engine = this->build_pipeline(program.get() + fSlotsNeeded);
    engine.start_pipeline(x,y,x+w,y+h, program.get());
}

std::function<void(size_t, size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    if (this->empty()) {
        return [](size_t, size_t, size_t, size_t) {};
    }

    void** program = fAlloc->makeArray<void*>(fSlotsNeeded);
    const SkJumper_Engine& engine = this->build_pipeline(program + fSlotsNeeded);

    auto start_pipeline = engine.start_pipeline;
    return [=](size_t x, size_t y, size_t w, size_t h) {
        start_pipeline(x,y,x+w,y+h, program);
    };
}
