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

#define M(st) +1
static const int kNumStages = SK_RASTER_PIPELINE_STAGES(M);
#undef M

#ifndef SK_DISABLE_SSSE3_RUNTIME_CHECK_FOR_LOWP_STAGES
    #if 0 && !__has_feature(memory_sanitizer) && (defined(__x86_64__) || defined(_M_X64))
        #include <atomic>

        #define M(st) #st,
        static const char* kStageNames[] = { SK_RASTER_PIPELINE_STAGES(M) };
        #undef M

        static std::atomic<int> gMissingStageCounters[kNumStages];

        static void log_missing(SkRasterPipeline::StockStage st) {
            static SkOnce once;
            once([] { atexit([] {
                for (int i = 0; i < kNumStages; i++) {
                    if (int count = gMissingStageCounters[i].load()) {
                        SkDebugf("%7d\t%s\n", count, kStageNames[i]);
                    }
                }
            }); });

            gMissingStageCounters[st]++;
        }
    #else
        static void log_missing(SkRasterPipeline::StockStage) {}
    #endif
#endif

// We can't express the real types of most stage functions portably, so we use a stand-in.
// We'll only ever call start_pipeline() or start_pipeline_2d(), which then chain into the rest.
using StageFn           = void(void);
using StartPipelineFn   = void(size_t,size_t,size_t,        void**,K*);
using StartPipeline2dFn = void(size_t,size_t,size_t,size_t, void**,K*);

// Some platforms expect C "name" maps to asm "_name", others to "name".
#if defined(__APPLE__)
    #define ASM(name, suffix)  sk_##name##_##suffix
#else
    #define ASM(name, suffix) _sk_##name##_##suffix
#endif

// Some stages have low-precision (~15 bit) versions from SkJumper_stages_lowp.cpp.
#define LOWP_STAGES(M)   \
    M(black_color) M(white_color) M(uniform_color) \
    M(set_rgb)           \
    M(premul)            \
    M(load_8888) M(load_8888_dst) M(store_8888) \
    M(load_bgra) M(load_bgra_dst) M(store_bgra) \
    M(load_a8)   M(load_a8_dst)   M(store_a8)   \
    M(load_g8)   M(load_g8_dst)                 \
    M(swap_rb)           \
    M(srcover_rgba_8888) \
    M(lerp_1_float)      \
    M(lerp_u8)           \
    M(scale_1_float)     \
    M(scale_u8)          \
    M(move_src_dst)      \
    M(move_dst_src)      \
    M(clear)             \
    M(srcatop)           \
    M(dstatop)           \
    M(srcin)             \
    M(dstin)             \
    M(srcout)            \
    M(dstout)            \
    M(srcover)           \
    M(dstover)           \
    M(modulate)          \
    M(multiply)          \
    M(screen)            \
    M(xor_)

extern "C" {

#if __has_feature(memory_sanitizer)
    // We'll just run portable code.

#elif defined(__aarch64__)
    StartPipelineFn   ASM(start_pipeline   ,aarch64);
    StartPipeline2dFn ASM(start_pipeline_2d,aarch64);
    StageFn ASM(just_return,aarch64);
    #define M(st) StageFn ASM(st,aarch64);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#elif defined(__arm__)
    StartPipelineFn   ASM(start_pipeline   ,vfp4);
    StartPipeline2dFn ASM(start_pipeline_2d,vfp4);
    StageFn ASM(just_return,vfp4);
    #define M(st) StageFn ASM(st,vfp4);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#elif defined(__x86_64__) || defined(_M_X64)
    StartPipelineFn ASM(start_pipeline,hsw       ),
                    ASM(start_pipeline,avx       ),
                    ASM(start_pipeline,sse41     ),
                    ASM(start_pipeline,sse2      ),
                    ASM(start_pipeline,hsw_lowp  ),
                    ASM(start_pipeline,ssse3_lowp);

    StartPipeline2dFn ASM(start_pipeline_2d,hsw       ),
                      ASM(start_pipeline_2d,avx       ),
                      ASM(start_pipeline_2d,sse41     ),
                      ASM(start_pipeline_2d,sse2      ),
                      ASM(start_pipeline_2d,hsw_lowp  ),
                      ASM(start_pipeline_2d,ssse3_lowp);

    StageFn ASM(just_return,hsw),
            ASM(just_return,avx),
            ASM(just_return,sse41),
            ASM(just_return,sse2),
            ASM(just_return,hsw_lowp  ),
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

    #define M(st) StageFn ASM(st,hsw_lowp);
        LOWP_STAGES(M)
    #undef M
    #define M(st) StageFn ASM(st,ssse3_lowp);
        LOWP_STAGES(M)
    #undef M

#elif (defined(__i386__) || defined(_M_IX86)) && \
        !(defined(_MSC_VER) && defined(SK_SUPPORT_LEGACY_WIN32_JUMPER))
    StartPipelineFn   ASM(start_pipeline   ,sse2);
    StartPipeline2dFn ASM(start_pipeline_2d,sse2);
    StageFn ASM(just_return,sse2);
    #define M(st) StageFn ASM(st,sse2);
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M

#endif

    // Portable, single-pixel stages.
    StartPipelineFn   sk_start_pipeline;
    StartPipeline2dFn sk_start_pipeline_2d;
    StageFn sk_just_return;
    #define M(st) StageFn sk_##st;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
}

#if !__has_feature(memory_sanitizer) && (defined(__x86_64__) || defined(_M_X64))
    template <SkRasterPipeline::StockStage st>
    static constexpr StageFn* hsw_lowp() { return nullptr; }

    template <SkRasterPipeline::StockStage st>
    static constexpr StageFn* ssse3_lowp() { return nullptr; }

    #define M(st) \
        template <> constexpr StageFn* hsw_lowp<SkRasterPipeline::st>() {   \
            return ASM(st,hsw_lowp);                                        \
        }                                                                   \
        template <> constexpr StageFn* ssse3_lowp<SkRasterPipeline::st>() { \
            return ASM(st,ssse3_lowp);                                      \
        }
        LOWP_STAGES(M)
    #undef M
#endif

// Engines comprise everything we need to run SkRasterPipelines.
struct SkJumper_Engine {
    StageFn*           stages[kNumStages];
    StartPipelineFn*   start_pipeline;
    StartPipeline2dFn* start_pipeline_2d;
    StageFn*           just_return;
};

// We'll default to this portable engine, but try to choose a better one at runtime.
static const SkJumper_Engine kPortable = {
#define M(stage) sk_##stage,
    { SK_RASTER_PIPELINE_STAGES(M) },
#undef M
    sk_start_pipeline,
    sk_start_pipeline_2d,
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
        M(start_pipeline)
        M(start_pipeline_2d)
        M(just_return)
    #undef M
    };

#elif defined(__arm__)
    if (1 && SkCpu::Supports(SkCpu::NEON|SkCpu::NEON_FMA|SkCpu::VFP_FP16)) {
        return {
        #define M(stage) ASM(stage, vfp4),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(start_pipeline_2d)
            M(just_return)
        #undef M
        };
    }

#elif defined(__x86_64__) || defined(_M_X64)
    if (1 && SkCpu::Supports(SkCpu::HSW)) {
        return {
        #define M(stage) ASM(stage, hsw),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(start_pipeline_2d)
            M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::AVX)) {
        return {
        #define M(stage) ASM(stage, avx),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(start_pipeline_2d)
            M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE41)) {
        return {
        #define M(stage) ASM(stage, sse41),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(start_pipeline_2d)
            M(just_return)
        #undef M
        };
    }
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        return {
        #define M(stage) ASM(stage, sse2),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(start_pipeline_2d)
            M(just_return)
        #undef M
        };
    }

#elif (defined(__i386__) || defined(_M_IX86)) && \
        !(defined(_MSC_VER) && defined(SK_SUPPORT_LEGACY_WIN32_JUMPER))
    if (1 && SkCpu::Supports(SkCpu::SSE2)) {
        return {
        #define M(stage) ASM(stage, sse2),
            { SK_RASTER_PIPELINE_STAGES(M) },
            M(start_pipeline)
            M(start_pipeline_2d)
            M(just_return)
        #undef M
        };
    }

#endif
    return kPortable;
}

#ifndef SK_DISABLE_SSSE3_RUNTIME_CHECK_FOR_LOWP_STAGES
    static const SkJumper_Engine kNone = {
    #define M(stage) nullptr,
        { SK_RASTER_PIPELINE_STAGES(M) },
    #undef M
        nullptr,
        nullptr,
        nullptr,
    };
    static SkJumper_Engine gLowp = kNone;
    static SkOnce gChooseLowpOnce;

    static SkJumper_Engine choose_lowp() {
    #if !__has_feature(memory_sanitizer) && (defined(__x86_64__) || defined(_M_X64))
        if (1 && SkCpu::Supports(SkCpu::HSW)) {
            return {
            #define M(st) hsw_lowp<SkRasterPipeline::st>(),
                { SK_RASTER_PIPELINE_STAGES(M) },
                ASM(start_pipeline   ,hsw_lowp),
                ASM(start_pipeline_2d,hsw_lowp),
                ASM(just_return      ,hsw_lowp)
            #undef M
            };
        }
        if (1 && SkCpu::Supports(SkCpu::SSSE3)) {
            return {
            #define M(st) ssse3_lowp<SkRasterPipeline::st>(),
                { SK_RASTER_PIPELINE_STAGES(M) },
                ASM(start_pipeline   ,ssse3_lowp),
                ASM(start_pipeline_2d,ssse3_lowp),
                ASM(just_return      ,ssse3_lowp)
            #undef M
            };
        }
    #endif
        return kNone;
    }
#endif

const SkJumper_Engine& SkRasterPipeline::build_pipeline(void** ip) const {
#ifndef SK_DISABLE_SSSE3_RUNTIME_CHECK_FOR_LOWP_STAGES
    gChooseLowpOnce([]{ gLowp = choose_lowp(); });

    // First try to build a lowp pipeline.  If that fails, fall back to normal float gEngine.
    void** reset_point = ip;
    *--ip = (void*)gLowp.just_return;
    for (const StageList* st = fStages; st; st = st->prev) {
        if (st->stage == SkRasterPipeline::clamp_0) {
            continue;  // No-op in lowp.
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

void SkRasterPipeline::run(size_t x, size_t y, size_t n) const {
    if (this->empty()) {
        return;
    }

    // Best to not use fAlloc here... we can't bound how often run() will be called.
    SkAutoSTMalloc<64, void*> program(fSlotsNeeded);

    const SkJumper_Engine& engine = this->build_pipeline(program.get() + fSlotsNeeded);
    engine.start_pipeline(x,y,x+n, program.get(), &kConstants);
}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    if (this->empty()) {
        return [](size_t, size_t, size_t) {};
    }

    void** program = fAlloc->makeArray<void*>(fSlotsNeeded);
    const SkJumper_Engine& engine = this->build_pipeline(program + fSlotsNeeded);

    auto start_pipeline = engine.start_pipeline;
    return [=](size_t x, size_t y, size_t n) {
        start_pipeline(x,y,x+n, program, &kConstants);
    };
}

void SkRasterPipeline::run_2d(size_t x, size_t y, size_t w, size_t h) const {
    if (this->empty()) {
        return;
    }

    // Like in run(), it's best to not use fAlloc here... we can't bound how often we'll be called.
    SkAutoSTMalloc<64, void*> program(fSlotsNeeded);

    const SkJumper_Engine& engine = this->build_pipeline(program.get() + fSlotsNeeded);
    engine.start_pipeline_2d(x,y,x+w,y+h, program.get(), &kConstants);
}
