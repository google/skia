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
using StartPipelineFn = void(size_t,size_t,size_t,void**,K*);

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
    StartPipelineFn ASM(start_pipeline,aarch64);
    StageFn ASM(just_return,aarch64);
    #define M(st) StageFn ASM(st,aarch64);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#elif defined(__arm__)
    StartPipelineFn ASM(start_pipeline,vfp4);
    StageFn ASM(just_return,vfp4);
    #define M(st) StageFn ASM(st,vfp4);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#elif defined(__x86_64__) || defined(_M_X64)
    StartPipelineFn ASM(start_pipeline,hsw       ),
                    ASM(start_pipeline,avx       ),
                    ASM(start_pipeline,sse41     ),
                    ASM(start_pipeline,sse2      ),
                    ASM(start_pipeline,ssse3_lowp);

    StageFn ASM(just_return,hsw),
            ASM(just_return,avx),
            ASM(just_return,sse41),
            ASM(just_return,sse2),
            ASM(just_return,ssse3_lowp);

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

    StageFn ASM(load_8888,  ssse3_lowp),
            ASM(store_8888, ssse3_lowp),
            ASM(swap_rb,    ssse3_lowp);
#endif

    // Portable, single-pixel stages.
    StartPipelineFn sk_start_pipeline;
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
    void (*start_pipeline)(size_t,size_t,size_t, void**, K*);
    StageFn* just_return;
};

// We'll default to this portable engine, but try to choose a better one at runtime.
static const SkJumper_Engine kPortable = {
#define M(stage) sk_##stage,
    { SK_RASTER_PIPELINE_STAGES(M) },
#undef M
    sk_start_pipeline,
    sk_just_return,
};
static SkJumper_Engine gEngine = kPortable;
static SkOnce gChooseEngineOnce;

static SkJumper_Engine choose_engine() {
#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    return {
    #define M(stage) ASM(stage, aarch64),
        { SK_RASTER_PIPELINE_STAGES(M) },
        M(start_pipeline) M(just_return)
    #undef M
    };

#elif defined(__arm__)
    if (1 && SkCpu::Supports(SkCpu::NEON|SkCpu::NEON_FMA|SkCpu::VFP_FP16)) {
        return {
        #define M(stage) ASM(stage, vfp4),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline) M(just_return)
        #undef M
        };
    }

#elif defined(__x86_64__) || defined(_M_X64)
    if (1 && SkCpu::Supports(SkCpu::HSW)) {
        return {
        #define M(stage) ASM(stage, hsw),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline) M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::AVX)) {
        return {
        #define M(stage) ASM(stage, avx),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline) M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE41)) {
        return {
        #define M(stage) ASM(stage, sse41),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline) M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        return {
        #define M(stage) ASM(stage, sse2),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline) M(just_return)
        #undef M
        };
    }
#endif
    return kPortable;
}

StartPipelineFn* SkRasterPipeline::build_pipeline(void** ip) const {
#if defined(__x86_64__) || defined(_M_X64)
    if (SkCpu::Supports(SkCpu::SSSE3)) {
        void** reset_point = ip;

        *--ip = (void*)ASM(just_return,ssse3_lowp);
        for (const StageList* st = fStages; st; st = st->prev) {
            StageFn* fn = nullptr;
            switch (st->stage) {
                case SkRasterPipeline::load_8888:  fn = ASM(load_8888, ssse3_lowp); break;
                case SkRasterPipeline::store_8888: fn = ASM(store_8888,ssse3_lowp); break;
                case SkRasterPipeline::swap_rb:    fn = ASM(swap_rb,   ssse3_lowp); break;
                default:
                    //SkDebugf("can't %d\n", st->stage);
                    ip = reset_point;
            }
            if (ip == reset_point) {
                break;
            }
            if (st->ctx) {
                *--ip = st->ctx;
            }
            *--ip = (void*)fn;
        }

        if (ip != reset_point) {
            return ASM(start_pipeline,ssse3_lowp);
        }
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
    return gEngine.start_pipeline;
}

void SkRasterPipeline::run(size_t x, size_t y, size_t n) const {
    if (this->empty()) {
        return;
    }

    // Best to not use fAlloc here... we can't bound how often run() will be called.
    SkAutoSTMalloc<64, void*> program(fSlotsNeeded);

    auto start_pipeline = this->build_pipeline(program.get() + fSlotsNeeded);
    start_pipeline(x,y,x+n, program.get(), &kConstants);
}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    if (this->empty()) {
        return [](size_t, size_t, size_t) {};
    }

    void** program = fAlloc->makeArray<void*>(fSlotsNeeded);
    auto start_pipeline = this->build_pipeline(program + fSlotsNeeded);

    return [=](size_t x, size_t y, size_t n) {
        start_pipeline(x,y,x+n, program, &kConstants);
    };
}
