/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkCpu.h"
#include "SkJumper.h"
#include "SkOnce.h"
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
    {0,1,2,3,4,5,6,7},
    {0,1,2,3,4,5,6,7},
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

#define M(st) +1
static const int kNumStages = SK_RASTER_PIPELINE_STAGES(M);
#undef M

// Engines comprise everything we need to run SkRasterPipelines.
struct SkJumper_Engine {
    StageFn* stages[kNumStages];
    int      min_stride;
    size_t (*start_pipeline)(size_t, void**, K*, size_t);
    StageFn* just_return;
};

// We'll always have two engines.  A portable engine with guaranteed min_stride = 1...
static const SkJumper_Engine kPortable = {
#define M(stage) sk_##stage,
    { SK_RASTER_PIPELINE_STAGES(M) },
#undef M
    1,
    sk_start_pipeline,
    sk_just_return,
};
// ...and a platform-specific engine chosen on first use based on CPU features.
static SkJumper_Engine gPlatform = kPortable;
static SkOnce gChooseEngineOnce;

static SkJumper_Engine choose_engine() {
#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    return {
    #define M(stage) ASM(stage, aarch64),
        { SK_RASTER_PIPELINE_STAGES(M) },
        4, M(start_pipeline) M(just_return)
    #undef M
    };

#elif defined(__arm__)
    if (1 && SkCpu::Supports(SkCpu::NEON|SkCpu::NEON_FMA|SkCpu::VFP_FP16)) {
        return {
        #define M(stage) ASM(stage, vfp4),
            { SK_RASTER_PIPELINE_STAGES(M) },
            2, M(start_pipeline) M(just_return)
        #undef M
        };
    }

#elif defined(__x86_64__) || defined(_M_X64)
    if (1 && SkCpu::Supports(SkCpu::HSW)) {
        return {
        #define M(stage) ASM(stage, hsw),
            { SK_RASTER_PIPELINE_STAGES(M) },
            1, M(start_pipeline) M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::AVX)) {
        return {
        #define M(stage) ASM(stage, avx),
            { SK_RASTER_PIPELINE_STAGES(M) },
            1, M(start_pipeline) M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE41)) {
        return {
        #define M(stage) ASM(stage, sse41),
            { SK_RASTER_PIPELINE_STAGES(M) },
            4, M(start_pipeline) M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        return {
        #define M(stage) ASM(stage, sse2),
            { SK_RASTER_PIPELINE_STAGES(M) },
            4, M(start_pipeline) M(just_return)
        #undef M
        };
    }
#endif
    return kPortable;
}

static void build_pipeline(const SkRasterPipeline::Stage* stages, int nstages,
                           const SkJumper_Engine& engine, void** ip) {
    for (int i = 0; i < nstages; i++) {
        const auto& st = stages[i];
        StageFn* fn = engine.stages[st.stage];

        *ip++ = (void*)fn;
        if (st.ctx) {
            *ip++ = st.ctx;
        }
    }
    *ip = (void*)engine.just_return;
}

void SkRasterPipeline::run(size_t x, size_t n) const {
    if (fStages.empty()) {
        return;
    }
    gChooseEngineOnce([]{ gPlatform = choose_engine(); });

    SkAutoSTMalloc<64, void*> program(2*fStages.size() + 1);
    const size_t limit = x+n;

    if (x + gPlatform.min_stride <= limit) {
        build_pipeline(fStages.data(), SkToInt(fStages.size()), gPlatform, program.get());
        x = gPlatform.start_pipeline(x, program.get(), &kConstants, limit);
    }
    if (x < limit) {
        build_pipeline(fStages.data(), SkToInt(fStages.size()), kPortable, program.get());
        kPortable.start_pipeline(x, program.get(), &kConstants, limit);
    }
}

std::function<void(size_t, size_t)> SkRasterPipeline::compile(SkArenaAlloc* alloc) const {
    if (fStages.empty()) {
        return [](size_t, size_t) {};
    }
    gChooseEngineOnce([]{ gPlatform = choose_engine(); });

    void** platform = alloc->makeArray<void*>(2*fStages.size() + 1);
    build_pipeline(fStages.data(), SkToInt(fStages.size()), gPlatform, platform);

    if (gPlatform.min_stride == 1) {
        return [=](size_t x, size_t n) {
            const size_t limit = x+n;
            gPlatform.start_pipeline(x, platform, &kConstants, limit);
        };
    }

    void** portable = alloc->makeArray<void*>(2*fStages.size() + 1);
    build_pipeline(fStages.data(), SkToInt(fStages.size()), kPortable, portable);

    return [=](size_t x, size_t n) {
        const size_t limit = x+n;
        if (x + gPlatform.min_stride <= limit) {
            x = gPlatform.start_pipeline(x, platform, &kConstants, limit);
        }
        if (x < limit) {
            kPortable.start_pipeline(x, portable, &kConstants, limit);
        }
    };
}
