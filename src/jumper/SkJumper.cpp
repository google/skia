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
static const SkJumper_constants kConstants = {
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

extern "C" {
    // Declare the portable, single pixel stages that are linked into Skia from SkJumper_stages.o.
    void sk_start_pipeline(size_t, void**, const SkJumper_constants*);

    // We use void() as a convenient stand-in for the real stage function type.
    // We never call these directly, so we don't really need to know their real types.
    void sk_just_return(void);
    #define M(st) void sk_##st(void);
        STAGES(M)
    #undef M

    // Same deal for platform-specific stages from SkJumper_generated_*.o.
#if defined(__x86_64__) || defined(_M_X64)
    void sk_start_pipeline_sse2 (size_t, void**, const SkJumper_constants*);
    void sk_start_pipeline_sse41(size_t, void**, const SkJumper_constants*);
    void sk_start_pipeline_hsw  (size_t, void**, const SkJumper_constants*);

    void sk_start_pipeline_ms_sse2 (size_t, void**, const SkJumper_constants*);
    void sk_start_pipeline_ms_sse41(size_t, void**, const SkJumper_constants*);
    void sk_start_pipeline_ms_hsw  (size_t, void**, const SkJumper_constants*);

    void sk_just_return_sse2 (void);
    void sk_just_return_sse41(void);
    void sk_just_return_hsw  (void);

    #define M(st) void sk_##st##_sse2(void);
        STAGES(M)
    #undef M
    #define M(st) void sk_##st##_sse41(void);
        STAGES(M)
    #undef M
    #define M(st) void sk_##st##_hsw(void);
        STAGES(M)
    #undef M
#endif
}

// Translate SkRasterPipeline's enum to pointers to stages.
static void* lookup_portable(SkRasterPipeline::StockStage st) {
    switch (st) {
        default: return nullptr;
    #define M(st) case SkRasterPipeline::st: return (void*)sk_##st;
        STAGES(M)
    #undef M
    }
}

#if defined(__x86_64__) || defined(_M_X64)
    static void* lookup_sse2(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return (void*)sk_##st##_sse2;
            STAGES(M)
        #undef M
        }
    }
    static void* lookup_sse41(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return (void*)sk_##st##_sse41;
            STAGES(M)
        #undef M
        }
    }
    static void* lookup_hsw(SkRasterPipeline::StockStage st) {
        switch (st) {
            default: return nullptr;
        #define M(st) case SkRasterPipeline::st: return (void*)sk_##st##_hsw;
            STAGES(M)
        #undef M
        }
    }
#endif

bool SkRasterPipeline::run_with_jumper(size_t x, size_t n) const {
    // We'll look for the best vector instruction set and stride we can use.
    size_t stride                                                     = 0;
    void* (*lookup)(SkRasterPipeline::StockStage)                     = nullptr;
    void (*start_pipeline)(size_t, void**, const SkJumper_constants*) = nullptr;
    void (*just_return)(void)                                         = nullptr;

#if defined(__x86_64__) || defined(_M_X64)
    stride         = 4;
    lookup         = lookup_sse2;
    start_pipeline = sk_start_pipeline_sse2;
    just_return    = sk_just_return_sse2;
    if (SkCpu::Supports(SkCpu::SSE41)) {
        stride         = 4;
        lookup         = lookup_sse41;
        start_pipeline = sk_start_pipeline_sse41;
        just_return    = sk_just_return_sse41;
    }
    if (SkCpu::Supports(SkCpu::HSW)) {
        stride         = 8;
        lookup         = lookup_hsw;
        start_pipeline = sk_start_pipeline_hsw;
        just_return    = sk_just_return_hsw;
    }
#endif

#if defined(_MSC_VER)
    if (start_pipeline == sk_start_pipeline_sse2)  { start_pipeline = sk_start_pipeline_ms_sse2; }
    if (start_pipeline == sk_start_pipeline_sse41) { start_pipeline = sk_start_pipeline_ms_sse41; }
    if (start_pipeline == sk_start_pipeline_hsw)   { start_pipeline = sk_start_pipeline_ms_hsw; }
#endif

    SkAutoSTMalloc<64, void*> program(2*fStages.size() + 1);

    // If possible, build and run a program to run at full vector stride.
    const size_t limit = x+n;

    if (stride) {
        void** ip = program.get();
        for (auto&& st : fStages) {
            auto fn = lookup(st.stage);
            if (!fn) {
                return false;
            }
            *ip++ = fn;
            *ip++ = st.ctx;
        }
        *ip = (void*)just_return;

        while (x + stride <= limit) {
            start_pipeline(x, program.get(), &kConstants);
            x += stride;
        }
    }

    // If there's any leftover, build and run stride=1 portable code.
    if (x < limit) {
        stride = 1;

        void** ip = program.get();
        for (auto&& st : fStages) {
            auto fn = lookup_portable(st.stage);
            if (!fn) {
                return false;
            }
            *ip++ = fn;
            *ip++ = st.ctx;
        }
        *ip = (void*)sk_just_return;

        while (x + stride <= limit) {
            sk_start_pipeline(x, program.get(), &kConstants);
            x += stride;
        }
    }

    return true;
}
