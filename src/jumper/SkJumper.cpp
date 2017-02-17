/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCpu.h"
#include "SkJumper.h"
#include "SkRasterPipeline.h"
#include "SkTemplates.h"


// Stages expect these constants to be set to these values.
// It's fine to rearrange and add new ones if you update SkJumper_constants.
using K = const SkJumper_constants;
static K kConstants = {
    1.0f, 0.5f, 255.0f, 1/255.0f, 0x000000ff,
    {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f},
    0.0025f, 0.6975f, 0.3000f, 1/12.92f, 0.055f,       // from_srgb
    12.46f, 0.411192f, 0.689206f, -0.0988f, 0.0043f,   //   to_srgb
    0x77800000, 0x07800000, 0x04000400,                // fp16 <-> fp32
};

#define STAGES(M)     \
    M(seed_shader)    \
    M(constant_color) \
    M(clear)          \
    M(plus_)          \
    M(srcover)        \
    M(dstover)        \
    M(clamp_0)        \
    M(clamp_1)        \
    M(clamp_a)        \
    M(swap)           \
    M(move_src_dst)   \
    M(move_dst_src)   \
    M(premul)         \
    M(unpremul)       \
    M(from_srgb)      \
    M(to_srgb)        \
    M(scale_u8)       \
    M(load_tables)    \
    M(load_8888)      \
    M(store_8888)     \
    M(load_f16)       \
    M(store_f16)      \
    M(matrix_2x3)     \
    M(matrix_3x4)     \
    M(clamp_x)        \
    M(clamp_y)        \
    M(linear_gradient_2stops)

// We can't express the real types of most stage functions portably, so we use a stand-in.
// We'll only ever call start_pipeline(), which then chains into the rest for us.
using StageFn = void(void);

extern "C" {

#if defined(__x86_64__) || defined(_M_X64)
    void sk_start_pipeline_hsw  (size_t, void**, K*);
    void sk_start_pipeline_sse41(size_t, void**, K*);
    void sk_start_pipeline_sse2 (size_t, void**, K*);

    StageFn sk_just_return_hsw,
            sk_just_return_sse41,
            sk_just_return_sse2;

    #define M(st) StageFn sk_##st##_hsw;
        STAGES(M)
    #undef M
    #define M(st) StageFn sk_##st##_sse41;
        STAGES(M)
    #undef M
    #define M(st) StageFn sk_##st##_sse2;
        STAGES(M)
    #undef M
#endif

    // Portable, single-pixel stages.
    void sk_start_pipeline(size_t, void**, K*);
    StageFn sk_just_return;
    #define M(st) StageFn sk_##st;
        STAGES(M)
    #undef M
}

// Translate SkRasterPipeline's StockStage enum to StageFn function pointers.

#if defined(__x86_64__) || defined(_M_X64)
    static StageFn* lookup_hsw(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return sk_##st##_hsw;
            STAGES(M)
        #undef M
        }
    }
    static StageFn* lookup_sse41(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return sk_##st##_sse41;
            STAGES(M)
        #undef M
        }
    }
    static StageFn* lookup_sse2(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return sk_##st##_sse2;
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
    SkAutoSTMalloc<64, void*> program(2*fStages.size() + 1);
    const size_t limit = x+n;

    auto build_and_run = [&](size_t   stride,
                             StageFn* (*lookup)(SkRasterPipeline::StockStage),
                             StageFn* just_return,
                             void     (*start_pipeline)(size_t, void**, K*)) {
        if (x + stride <= limit) {
            void** ip = program.get();
            for (auto&& st : fStages) {
                auto fn = lookup(st.stage);
                if (!fn) {
                    return false;
                }
                *ip++ = (void*)fn;
                *ip++ = st.ctx;
            }
            *ip = (void*)just_return;

            while (x + stride <= limit) {
                start_pipeline(x, program.get(), &kConstants);
                x += stride;
            }
        }
        return true;
    };

    // While possible, build and run at full vector stride.
#if defined(__x86_64__) || defined(_M_X64)
    if (1 && SkCpu::Supports(SkCpu::HSW)) {
        if (!build_and_run(8, lookup_hsw, sk_just_return_hsw, sk_start_pipeline_hsw)) {
            return false;
        }
    }
    if (1 && SkCpu::Supports(SkCpu::SSE41)) {
        if (!build_and_run(4, lookup_sse41, sk_just_return_sse41, sk_start_pipeline_sse41)) {
            return false;
        }
    }
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        if (!build_and_run(4, lookup_sse2, sk_just_return_sse2, sk_start_pipeline_sse2)) {
            return false;
        }
    }
#endif

    // Finish up any leftover with portable code one pixel at a time.
    return build_and_run(1, lookup_portable, sk_just_return, sk_start_pipeline);
}
