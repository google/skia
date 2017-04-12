/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkCpu.h"
#include "SkJumper.h"
#include "SkRasterPipeline.h"
#include "SkTemplates.h"

// A debugging mode that helps prioritize porting stages to SkJumper.
#if 0
    #include "SkOnce.h"
    #include <atomic>

    #define M(st) {0},
    static std::atomic<int> gMissing[] = { SK_RASTER_PIPELINE_STAGES(M) };
    #undef M

    #define M(st) #st,
    static const char* gNames[] = { SK_RASTER_PIPELINE_STAGES(M) };
    #undef M

    #define WHATS_NEXT
#endif

// We'll use __has_feature(memory_sanitizer) to detect MSAN.
// SkJumper_generated.S is not compiled with MSAN, so MSAN would yell really loud.
#if !defined(__has_feature)
    #define __has_feature(x) 0
#endif

// Stages expect these constants to be set to these values.
// It's fine to rearrange and add new ones if you update SkJumper_constants.
using K = const SkJumper_constants;
static K kConstants = {
    {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f},
};

#define STAGES(M)         \
    M(seed_shader)        \
    M(constant_color)     \
    M(clear)              \
    M(srcatop)            \
    M(dstatop)            \
    M(srcin)              \
    M(dstin)              \
    M(srcout)             \
    M(dstout)             \
    M(srcover)            \
    M(dstover)            \
    M(modulate)           \
    M(multiply)           \
    M(plus_)              \
    M(screen)             \
    M(xor_)               \
    M(darken)             \
    M(lighten)            \
    M(difference)         \
    M(exclusion)          \
    M(colorburn)          \
    M(colordodge)         \
    M(hardlight)          \
    M(overlay)            \
    M(softlight)          \
    M(clamp_0)            \
    M(clamp_1)            \
    M(clamp_a)            \
    M(set_rgb)            \
    M(swap_rb)            \
    M(swap)               \
    M(move_src_dst)       \
    M(move_dst_src)       \
    M(premul)             \
    M(unpremul)           \
    M(from_srgb)          \
    M(to_srgb)            \
    M(from_2dot2)         \
    M(to_2dot2)           \
    M(rgb_to_hsl)         \
    M(hsl_to_rgb)         \
    M(scale_1_float)      \
    M(scale_u8)           \
    M(lerp_1_float)       \
    M(lerp_u8)            \
    M(lerp_565)           \
    M(load_tables)        \
    M(byte_tables)        \
    M(byte_tables_rgb)    \
    M(load_a8)            \
    M(gather_a8)          \
    M(store_a8)           \
    M(load_g8)            \
    M(gather_g8)          \
    M(gather_i8)          \
    M(load_565)           \
    M(gather_565)         \
    M(store_565)          \
    M(load_4444)          \
    M(gather_4444)        \
    M(store_4444)         \
    M(load_8888)          \
    M(gather_8888)        \
    M(store_8888)         \
    M(load_f16)           \
    M(gather_f16)         \
    M(store_f16)          \
    M(load_u16_be)        \
    M(store_u16_be)       \
    M(load_f32)           \
    M(store_f32)          \
    M(luminance_to_alpha) \
    M(matrix_2x3)         \
    M(matrix_3x4)         \
    M(matrix_4x5)         \
    M(matrix_perspective) \
    M(clamp_x)            \
    M(clamp_y)            \
    M(repeat_x)           \
    M(repeat_y)           \
    M(mirror_x)           \
    M(mirror_y)           \
    M(save_xy)            \
    M(accumulate)         \
    M(bilinear_nx) M(bilinear_px) M(bilinear_ny) M(bilinear_py)  \
    M(bicubic_n3x) M(bicubic_n1x) M(bicubic_p1x) M(bicubic_p3x)  \
    M(bicubic_n3y) M(bicubic_n1y) M(bicubic_p1y) M(bicubic_p3y)  \
    M(linear_gradient)    \
    M(linear_gradient_2stops)

// We can't express the real types of most stage functions portably, so we use a stand-in.
// We'll only ever call start_pipeline(), which then chains into the rest for us.
using StageFn = void(void);

// Some platforms expect C "name" maps to asm "_name", others to "name".
#if defined(__APPLE__)
    #define ASM(name, suffix)  sk_##name##_##suffix
#else
    #define ASM(name, suffix) _sk_##name##_##suffix
#endif

extern "C" {

#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    size_t ASM(start_pipeline,aarch64)(size_t, void**, K*, size_t);
    StageFn ASM(just_return,aarch64);
    #define M(st) StageFn ASM(st,aarch64);
        STAGES(M)
    #undef M

#elif defined(__arm__)
    size_t ASM(start_pipeline,vfp4)(size_t, void**, K*, size_t);
    StageFn ASM(just_return,vfp4);
    #define M(st) StageFn ASM(st,vfp4);
        STAGES(M)
    #undef M

#elif defined(__x86_64__) || defined(_M_X64)
    size_t ASM(start_pipeline,hsw  )(size_t, void**, K*, size_t);
    size_t ASM(start_pipeline,avx  )(size_t, void**, K*, size_t);
    size_t ASM(start_pipeline,sse41)(size_t, void**, K*, size_t);
    size_t ASM(start_pipeline,sse2 )(size_t, void**, K*, size_t);

    StageFn ASM(just_return,hsw),
            ASM(just_return,avx),
            ASM(just_return,sse41),
            ASM(just_return,sse2);

    #define M(st) StageFn ASM(st,hsw);
        STAGES(M)
    #undef M
    #define M(st) StageFn ASM(st,avx);
        STAGES(M)
    #undef M
    #define M(st) StageFn ASM(st,sse41);
        STAGES(M)
    #undef M
    #define M(st) StageFn ASM(st,sse2);
        STAGES(M)
    #undef M
#endif

    // Portable, single-pixel stages.
    size_t sk_start_pipeline(size_t, void**, K*, size_t);
    StageFn sk_just_return;
    #define M(st) StageFn sk_##st;
        STAGES(M)
    #undef M
}

// Translate SkRasterPipeline's StockStage enum to StageFn function pointers.

#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    static StageFn* lookup_aarch64(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return ASM(st,aarch64);
            STAGES(M)
        #undef M
        }
    }

#elif defined(__arm__)
    static StageFn* lookup_vfp4(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return ASM(st,vfp4);
            STAGES(M)
        #undef M
        }
    }

#elif defined(__x86_64__) || defined(_M_X64)
    static StageFn* lookup_hsw(SkRasterPipeline::StockStage st) {
        switch (st) {
            default:
        #ifdef WHATS_NEXT
                gMissing[st]++;
        #endif
                return nullptr;
        #define M(st) case SkRasterPipeline::st: return ASM(st,hsw);
            STAGES(M)
        #undef M
        }
    }
    static StageFn* lookup_avx(SkRasterPipeline::StockStage st) {
        switch (st) {
            default:
        #ifdef WHATS_NEXT
                gMissing[st]++;
        #endif
                return nullptr;
        #define M(st) case SkRasterPipeline::st: return ASM(st,avx);
            STAGES(M)
        #undef M
        }
    }
    static StageFn* lookup_sse41(SkRasterPipeline::StockStage st) {
        switch (st) {
            default:
        #ifdef WHATS_NEXT
                gMissing[st]++;
        #endif
                return nullptr;
        #define M(st) case SkRasterPipeline::st: return ASM(st,sse41);
            STAGES(M)
        #undef M
        }
    }
    static StageFn* lookup_sse2(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return ASM(st,sse2);
            STAGES(M)
        #undef M
        }
    }
#endif

static StageFn* lookup_portable(SkRasterPipeline::StockStage st) {
    switch (st) {
        default: return nullptr;
    #define M(st) case SkRasterPipeline::st: return sk_##st;
        STAGES(M)
    #undef M
    }
}

bool SkRasterPipeline::run_with_jumper(size_t x, size_t n) const {
#ifdef WHATS_NEXT
    static SkOnce once;
    once([] {
        atexit([] {
            for (int i = 0; i < (int)SK_ARRAY_COUNT(gMissing); i++) {
                if (int n = gMissing[i].load()) {
                    SkDebugf("%10d %s\n", n, gNames[i]);
                }
            }
        });
    });
#endif

    SkAutoSTMalloc<64, void*> program(2*fStages.size() + 1);
    const size_t limit = x+n;

    auto build_and_run = [&](size_t   min_stride,
                             StageFn* (*lookup)(SkRasterPipeline::StockStage),
                             StageFn* just_return,
                             size_t   (*start_pipeline)(size_t, void**, K*, size_t)) {
        if (x + min_stride <= limit) {
            void** ip = program.get();
            for (auto&& st : fStages) {
                auto fn = lookup(st.stage);
                if (!fn) {
                    return false;
                }
                *ip++ = (void*)fn;
                if (st.ctx) {
                    *ip++ = st.ctx;
                }
            }
            *ip = (void*)just_return;

            x = start_pipeline(x, program.get(), &kConstants, limit);
        }
        return true;
    };

    // While possible, build and run at full vector stride.
#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    if (!build_and_run(4, lookup_aarch64, ASM(just_return,aarch64), ASM(start_pipeline,aarch64))) {
        return false;
    }

#elif defined(__arm__)
    if (1 && SkCpu::Supports(SkCpu::NEON|SkCpu::NEON_FMA|SkCpu::VFP_FP16)) {
        if (!build_and_run(2, lookup_vfp4, ASM(just_return,vfp4), ASM(start_pipeline,vfp4))) {
            return false;
        }
    }

#elif defined(__x86_64__) || defined(_M_X64)
    if (1 && SkCpu::Supports(SkCpu::HSW)) {
        if (!build_and_run(1, lookup_hsw, ASM(just_return,hsw), ASM(start_pipeline,hsw))) {
            return false;
        }
    }
    if (1 && SkCpu::Supports(SkCpu::AVX)) {
        if (!build_and_run(1, lookup_avx, ASM(just_return,avx), ASM(start_pipeline,avx))) {
            return false;
        }
    }
    if (1 && SkCpu::Supports(SkCpu::SSE41)) {
        if (!build_and_run(4, lookup_sse41, ASM(just_return,sse41), ASM(start_pipeline,sse41))) {
            return false;
        }
    }
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        if (!build_and_run(4, lookup_sse2, ASM(just_return,sse2), ASM(start_pipeline,sse2))) {
            return false;
        }
    }
#endif

    // Finish up any leftover with portable code one pixel at a time.
    return build_and_run(1, lookup_portable, sk_just_return, sk_start_pipeline);
}
