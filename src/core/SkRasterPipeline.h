/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_DEFINED
#define SkRasterPipeline_DEFINED

#include "SkNx.h"
#include "SkTArray.h"
#include "SkTypes.h"

/**
 * SkRasterPipeline provides a cheap way to chain together a pixel processing pipeline.
 *
 * It's particularly designed for situations where the potential pipeline is extremely
 * combinatoric: {N dst formats} x {M source formats} x {K mask formats} x {C transfer modes} ...
 * No one wants to write specialized routines for all those combinations, and if we did, we'd
 * end up bloating our code size dramatically.  SkRasterPipeline stages can be chained together
 * at runtime, so we can scale this problem linearly rather than combinatorically.
 *
 * Each stage is represented by a function conforming to a common interface, SkRasterPipeline::Fn,
 * and by an arbitrary context pointer.  Fn's arguments, and sometimes custom calling convention,
 * are designed to maximize the amount of data we can pass along the pipeline cheaply.
 * On many machines all arguments stay in registers the entire time.
 *
 * The meaning of the arguments to Fn are sometimes fixed:
 *    - The Stage* always represents the current stage, mainly providing access to ctx().
 *    - The first size_t is always the destination x coordinate.
 *      (If you need y, put it in your context.)
 *    - The second size_t is always tail: 0 when working on a full 4-pixel slab,
 *      or 1..3 when using only the bottom 1..3 lanes of each register.
 *    - By the time the shader's done, the first four vectors should hold source red,
 *      green, blue, and alpha, up to 4 pixels' worth each.
 *
 * Sometimes arguments are flexible:
 *    - In the shader, the first four vectors can be used for anything, e.g. sample coordinates.
 *    - The last four vectors are scratch registers that can be used to communicate between
 *      stages; transfer modes use these to hold the original destination pixel components.
 *
 * On some platforms the last four vectors are slower to work with than the other arguments.
 *
 * When done mutating its arguments and/or context, a stage can either:
 *   1) call st->next() with its mutated arguments, chaining to the next stage of the pipeline; or
 *   2) return, indicating the pipeline is complete for these pixels.
 *
 * Some stages that typically return are those that write a color to a destination pointer,
 * but any stage can short-circuit the rest of the pipeline by returning instead of calling next().
 *
 * Most simple pipeline stages can use the SK_RASTER_STAGE macro to define a static EasyFn,
 * which simplifies the user interface a bit:
 *    - the context pointer is available directly as the first parameter;
 *    - instead of manually calling a next() function, just modify registers in place.
 *
 * To add an EasyFn stage to the pipeline, call append<fn>() instead of append(&fn).
 * It's a slight performance benefit to call last<fn>() for the last stage of a pipeline.
 */

// TODO: There may be a better place to stuff tail, e.g. in the bottom alignment bits of
// the Stage*.  This mostly matters on 64-bit Windows where every register is precious.

class SkRasterPipeline {
public:
    struct Stage;
    using Fn = void(SK_VECTORCALL *)(Stage*, size_t, size_t, Sk4f,Sk4f,Sk4f,Sk4f,
                                                             Sk4f,Sk4f,Sk4f,Sk4f);
    using EasyFn = void(void*, size_t, size_t, Sk4f&, Sk4f&, Sk4f&, Sk4f&,
                                               Sk4f&, Sk4f&, Sk4f&, Sk4f&);

    struct Stage {
        template <typename T>
        T ctx() { return static_cast<T>(fCtx); }

        void SK_VECTORCALL next(size_t x, size_t tail, Sk4f v0, Sk4f v1, Sk4f v2, Sk4f v3,
                                                       Sk4f v4, Sk4f v5, Sk4f v6, Sk4f v7) {
            // Stages are logically a pipeline, and physically are contiguous in an array.
            // To get to the next stage, we just increment our pointer to the next array element.
            fNext(this+1, x,tail, v0,v1,v2,v3, v4,v5,v6,v7);
        }

        // It makes next() a good bit cheaper if we hold the next function to call here,
        // rather than logically simpler choice of the function implementing this stage.
        Fn fNext;
        void* fCtx;
    };


    SkRasterPipeline();

    // Run the pipeline constructed with append(), walking x through [x,x+n),
    // generally in 4-pixel steps, with perhaps one jagged tail step.
    void run(size_t x, size_t n);
    void run(size_t n) { this->run(0, n); }

    // body() will only be called with tail=0, indicating it always works on a full 4 pixels.
    // tail() will only be called with tail=1..3 to handle the jagged end of n%4 pixels.
    void append(Fn body, Fn tail, const void* ctx = nullptr);
    void append(Fn fn, const void* ctx = nullptr) { this->append(fn, fn, ctx); }

    // Version of append that can be used with static EasyFn (see SK_RASTER_STAGE).
    template <EasyFn fn>
    void append(const void* ctx = nullptr) {
        this->append(Body<fn,true>, Tail<fn,true>, ctx);
    }

    // If this is the last stage of the pipeline, last() is a bit faster than append().
    template <EasyFn fn>
    void last(const void* ctx = nullptr) {
        this->append(Body<fn,false>, Tail<fn,false>, ctx);
    }

    // Append all stages to this pipeline.
    void extend(const SkRasterPipeline&);

private:
    using Stages = SkSTArray<10, Stage, /*MEM_COPY=*/true>;

    // This no-op default makes fBodyStart and fTailStart unconditionally safe to call,
    // and is always the last stage's fNext as a sort of safety net to make sure even a
    // buggy pipeline can't walk off its own end.
    static void SK_VECTORCALL JustReturn(Stage*, size_t, size_t, Sk4f,Sk4f,Sk4f,Sk4f,
                                                                 Sk4f,Sk4f,Sk4f,Sk4f);

    template <EasyFn kernel, bool kCallNext>
    static void SK_VECTORCALL Body(SkRasterPipeline::Stage* st, size_t x, size_t tail,
                                   Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                   Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
        // Passing 0 lets the optimizer completely drop any "if (tail) {...}" code in kernel.
        kernel(st->ctx<void*>(), x,0, r,g,b,a, dr,dg,db,da);
        if (kCallNext) {
            st->next(x,tail, r,g,b,a, dr,dg,db,da);  // It's faster to pass tail here than 0.
        }
    }

    template <EasyFn kernel, bool kCallNext>
    static void SK_VECTORCALL Tail(SkRasterPipeline::Stage* st, size_t x, size_t tail,
                                   Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                   Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    #if defined(__clang__)
        __builtin_assume(tail > 0);  // This flourish lets Clang compile away any tail==0 code.
    #endif
        kernel(st->ctx<void*>(), x,tail, r,g,b,a, dr,dg,db,da);
        if (kCallNext) {
            st->next(x,tail, r,g,b,a, dr,dg,db,da);
        }
    }

    Stages fBody,
           fTail;
    Fn fBodyStart = &JustReturn,
       fTailStart = &JustReturn;
};

// These are always static, and we _really_ want them to inline.
// If you find yourself wanting a non-inline stage, write a SkRasterPipeline::Fn directly.
#define SK_RASTER_STAGE(name)                                           \
    static SK_ALWAYS_INLINE void name(void* ctx, size_t x, size_t tail, \
                            Sk4f&  r, Sk4f&  g, Sk4f&  b, Sk4f&  a,     \
                            Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da)

#endif//SkRasterPipeline_DEFINED
