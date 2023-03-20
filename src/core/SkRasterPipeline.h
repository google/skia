/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_DEFINED
#define SkRasterPipeline_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMacros.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <cstddef>
#include <cstdint>
#include <functional>

class SkMatrix;
enum SkColorType : int;
struct SkImageInfo;
struct skcms_TransferFunction;

#if __has_cpp_attribute(clang::musttail) && !defined(__EMSCRIPTEN__) && !defined(SK_CPU_ARM32)
    #define SK_HAS_MUSTTAIL 1
#else
    #define SK_HAS_MUSTTAIL 0
#endif

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
 * arbitrary context pointer.  The stage function arguments and calling convention are
 * designed to maximize the amount of data we can pass along the pipeline cheaply, and
 * vary depending on CPU feature detection.
 */

// Raster pipeline programs are stored as a contiguous array of SkRasterPipelineStages.
SK_BEGIN_REQUIRE_DENSE
struct SkRasterPipelineStage {
    // A function pointer from `stages_lowp` or `stages_highp`. The exact function pointer type
    // varies depending on architecture (specifically, see `Stage` in SkRasterPipeline_opts.h).
    void (*fn)();

    // Data used by the stage function. Most context structures are declared at the top of
    // SkRasterPipeline.h, and have names ending in Ctx (e.g. "SkRasterPipeline_SamplerCtx").
    void* ctx;
};
SK_END_REQUIRE_DENSE

class SkRasterPipeline {
public:
    explicit SkRasterPipeline(SkArenaAlloc*);

    SkRasterPipeline(const SkRasterPipeline&) = delete;
    SkRasterPipeline(SkRasterPipeline&&)      = default;

    SkRasterPipeline& operator=(const SkRasterPipeline&) = delete;
    SkRasterPipeline& operator=(SkRasterPipeline&&)      = default;

    void reset();

    void append(SkRasterPipelineOp, void* = nullptr);
    void append(SkRasterPipelineOp op, const void* ctx) { this->append(op,const_cast<void*>(ctx)); }
    void append(SkRasterPipelineOp, uintptr_t ctx);

    // Append all stages to this pipeline.
    void extend(const SkRasterPipeline&);

    // Runs the pipeline in 2d from (x,y) inclusive to (x+w,y+h) exclusive.
    void run(size_t x, size_t y, size_t w, size_t h) const;

    // Allocates a thunk which amortizes run() setup cost in alloc.
    std::function<void(size_t, size_t, size_t, size_t)> compile() const;

    // Callers can inspect the stage list for debugging purposes.
    struct StageList {
        StageList*          prev;
        SkRasterPipelineOp  stage;
        void*               ctx;
    };

    static const char* GetOpName(SkRasterPipelineOp op);
    const StageList* getStageList() const { return fStages; }
    int getNumStages() const { return fNumStages; }

    // Prints the entire StageList using SkDebugf.
    void dump() const;

    // Appends a stage for the specified matrix.
    // Tries to optimize the stage by analyzing the type of matrix.
    void append_matrix(SkArenaAlloc*, const SkMatrix&);

    // Appends a stage for a constant uniform color.
    // Tries to optimize the stage based on the color.
    void append_constant_color(SkArenaAlloc*, const float rgba[4]);

    void append_constant_color(SkArenaAlloc* alloc, const SkColor4f& color) {
        this->append_constant_color(alloc, color.vec());
    }

    // Like append_constant_color() but only affecting r,g,b, ignoring the alpha channel.
    void append_set_rgb(SkArenaAlloc*, const float rgb[3]);

    void append_set_rgb(SkArenaAlloc* alloc, const SkColor4f& color) {
        this->append_set_rgb(alloc, color.vec());
    }

    void append_load    (SkColorType, const SkRasterPipeline_MemoryCtx*);
    void append_load_dst(SkColorType, const SkRasterPipeline_MemoryCtx*);
    void append_store   (SkColorType, const SkRasterPipeline_MemoryCtx*);

    void append_clamp_if_normalized(const SkImageInfo&);

    void append_transfer_function(const skcms_TransferFunction&);

    void append_stack_rewind();

    bool empty() const { return fStages == nullptr; }

private:
    bool build_lowp_pipeline(SkRasterPipelineStage* ip) const;
    void build_highp_pipeline(SkRasterPipelineStage* ip) const;

    using StartPipelineFn = void(*)(size_t,size_t,size_t,size_t, SkRasterPipelineStage* program);
    StartPipelineFn build_pipeline(SkRasterPipelineStage*) const;

    void unchecked_append(SkRasterPipelineOp, void*);
    int stages_needed() const;

    SkArenaAlloc*               fAlloc;
    SkRasterPipeline_RewindCtx* fRewindCtx;
    StageList*                  fStages;
    int                         fNumStages;
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
