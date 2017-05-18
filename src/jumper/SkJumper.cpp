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
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#elif defined(__arm__)
    size_t ASM(start_pipeline,vfp4)(size_t, void**, K*, size_t);
    StageFn ASM(just_return,vfp4);
    #define M(st) StageFn ASM(st,vfp4);
        SK_RASTER_PIPELINE_STAGES(M)
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
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
    #define M(st) StageFn ASM(st,avx);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
    #define M(st) StageFn ASM(st,sse41);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
    #define M(st) StageFn ASM(st,sse2);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
#endif

    // Portable, single-pixel stages.
    size_t sk_start_pipeline(size_t, void**, K*, size_t);
    StageFn sk_just_return;
    #define M(st) StageFn sk_##st;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
}

// Translate SkRasterPipeline's StockStage enum to StageFn function pointers.

#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    static StageFn* lookup_aarch64(SkRasterPipeline::StockStage st) {
        switch (st) {
        #define M(st) case SkRasterPipeline::st: return ASM(st,aarch64);
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        return nullptr;
    }

#elif defined(__arm__)
    static StageFn* lookup_vfp4(SkRasterPipeline::StockStage st) {
        switch (st) {
        #define M(st) case SkRasterPipeline::st: return ASM(st,vfp4);
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        return nullptr;
    }

#elif defined(__x86_64__) || defined(_M_X64)
    static StageFn* lookup_hsw(SkRasterPipeline::StockStage st) {
        switch (st) {
        #define M(st) case SkRasterPipeline::st: return ASM(st,hsw);
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        return nullptr;
    }
    static StageFn* lookup_avx(SkRasterPipeline::StockStage st) {
        switch (st) {
        #define M(st) case SkRasterPipeline::st: return ASM(st,avx);
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        return nullptr;
    }
    static StageFn* lookup_sse41(SkRasterPipeline::StockStage st) {
        switch (st) {
        #define M(st) case SkRasterPipeline::st: return ASM(st,sse41);
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        return nullptr;
    }
    static StageFn* lookup_sse2(SkRasterPipeline::StockStage st) {
        switch (st) {
        #define M(st) case SkRasterPipeline::st: return ASM(st,sse2);
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        return nullptr;
    }
#endif

static StageFn* lookup_portable(SkRasterPipeline::StockStage st) {
    switch (st) {
    #define M(st) case SkRasterPipeline::st: return sk_##st;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
    }
    return nullptr;
}

void SkRasterPipeline::run(size_t x, size_t n) const {
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
                SkASSERT(fn);
                *ip++ = (void*)fn;
                if (st.ctx) {
                    *ip++ = st.ctx;
                }
            }
            *ip = (void*)just_return;

            x = start_pipeline(x, program.get(), &kConstants, limit);
        }
    };

    // While possible, build and run at full vector stride.
#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    build_and_run(4, lookup_aarch64, ASM(just_return,aarch64), ASM(start_pipeline,aarch64));

#elif defined(__arm__)
    if (1 && SkCpu::Supports(SkCpu::NEON|SkCpu::NEON_FMA|SkCpu::VFP_FP16)) {
        build_and_run(2, lookup_vfp4, ASM(just_return,vfp4), ASM(start_pipeline,vfp4));
    }

#elif defined(__x86_64__) || defined(_M_X64)
    if (1 && SkCpu::Supports(SkCpu::HSW)) {
        build_and_run(1, lookup_hsw, ASM(just_return,hsw), ASM(start_pipeline,hsw));
    }
    if (1 && SkCpu::Supports(SkCpu::AVX)) {
        build_and_run(1, lookup_avx, ASM(just_return,avx), ASM(start_pipeline,avx));
    }
    if (1 && SkCpu::Supports(SkCpu::SSE41)) {
        build_and_run(4, lookup_sse41, ASM(just_return,sse41), ASM(start_pipeline,sse41));
    }
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        build_and_run(4, lookup_sse2, ASM(just_return,sse2), ASM(start_pipeline,sse2));
    }
#endif

    // Finish up any leftover with portable code one pixel at a time.
    build_and_run(1, lookup_portable, sk_just_return, sk_start_pipeline);
}
