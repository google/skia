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
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
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
    // `fn` holds a function pointer from `ops_lowp` or `ops_highp` in SkOpts.cpp. These functions
    // correspond to operations from the SkRasterPipelineOp enum in SkRasterPipelineOpList.h. The
    // exact function pointer type varies depending on architecture (specifically, look for `using
    // Stage =` in SkRasterPipeline_opts.h).
    void (*fn)();

    // `ctx` holds data used by the stage function.
    // Most context structures are declared in SkRasterPipelineOpContexts.h, and have names ending
    // in Ctx (e.g. "SkRasterPipeline_SamplerCtx"). Some Raster Pipeline stages pack non-pointer
    // data into this field using `SkRPCtxUtils::Pack`.
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
    void appendMatrix(SkArenaAlloc*, const SkMatrix&);

    // Appends a stage for a constant uniform color.
    // Tries to optimize the stage based on the color.
    void appendConstantColor(SkArenaAlloc*, const float rgba[4]);

    void appendConstantColor(SkArenaAlloc* alloc, const SkColor4f& color) {
        this->appendConstantColor(alloc, color.vec());
    }

    // Like appendConstantColor() but only affecting r,g,b, ignoring the alpha channel.
    void appendSetRGB(SkArenaAlloc*, const float rgb[3]);

    void appendSetRGB(SkArenaAlloc* alloc, const SkColor4f& color) {
        this->appendSetRGB(alloc, color.vec());
    }

    void appendLoad   (SkColorType, const SkRasterPipeline_MemoryCtx*);
    void appendLoadDst(SkColorType, const SkRasterPipeline_MemoryCtx*);
    void appendStore  (SkColorType, const SkRasterPipeline_MemoryCtx*);

    void appendClampIfNormalized(const SkImageInfo&);

    void appendTransferFunction(const skcms_TransferFunction&);

    void appendStackRewind();

    bool empty() const { return fStages == nullptr; }

private:
    bool buildLowpPipeline(SkRasterPipelineStage* ip) const;
    void buildHighpPipeline(SkRasterPipelineStage* ip) const;

    using StartPipelineFn = void (*)(size_t, size_t, size_t, size_t,
                                     SkRasterPipelineStage* program,
                                     SkSpan<SkRasterPipeline_MemoryCtxPatch>,
                                     uint8_t*);
    StartPipelineFn buildPipeline(SkRasterPipelineStage*) const;

    void uncheckedAppend(SkRasterPipelineOp, void*);
    int stagesNeeded() const;

    void addMemoryContext(SkRasterPipeline_MemoryCtx*, int bytesPerPixel, bool load, bool store);
    uint8_t* tailPointer();

    SkArenaAlloc*               fAlloc;
    SkRasterPipeline_RewindCtx* fRewindCtx;
    StageList*                  fStages;
    uint8_t*                    fTailPointer;
    int                         fNumStages;

    // Only 1 in 2 million CPU-backend pipelines used more than two MemoryCtxs.
    // (See the comment in SkRasterPipelineOpContexts.h for how MemoryCtx patching works)
    skia_private::STArray<2, SkRasterPipeline_MemoryCtxInfo> fMemoryCtxInfos;
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
