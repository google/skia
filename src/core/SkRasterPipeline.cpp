/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline() {}

void SkRasterPipeline::append(SkRasterPipeline::Fn4 body,
                              SkRasterPipeline::Fn4 tail,
                              const void* ctx) {
    // Each stage holds its own context and the next function to call.
    // So the pipeline itself has to hold onto the first function that starts the pipeline.
    (fBody.empty() ? fBodyStart : fBody.back().fNext) = (Fn)body;
    (fTail.empty() ? fTailStart : fTail.back().fNext) = (Fn)tail;

    // Each last stage starts with its next function set to JustReturn as a safety net.
    // It'll be overwritten by the next call to append().
    fBody.push_back({ (Fn)&JustReturn, const_cast<void*>(ctx) });
    fTail.push_back({ (Fn)&JustReturn, const_cast<void*>(ctx) });
}

void SkRasterPipeline::append(StockStage stage, const void* ctx) {
    this->append(SkOpts::stages_4[stage], SkOpts::stages_1_3[stage], ctx);
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    SkASSERT(src.fBody.count() == src.fTail.count());

    Fn4 body = (Fn4)src.fBodyStart,
        tail = (Fn4)src.fTailStart;
    for (int i = 0; i < src.fBody.count(); i++) {
        SkASSERT(src.fBody[i].fCtx == src.fTail[i].fCtx);
        this->append(body, tail, src.fBody[i].fCtx);
        body = (Fn4)src.fBody[i].fNext;
        tail = (Fn4)src.fTail[i].fNext;
    }
}

void SkRasterPipeline::run(size_t x, size_t n) {
    // It's fastest to start uninitialized if the compilers all let us.  If not, next fastest is 0.
    Sk4f v;

    while (n >= 4) {
        ((Fn4)fBodyStart)(fBody.begin(), x,0, v,v,v,v, v,v,v,v);
        x += 4;
        n -= 4;
    }
    if (n > 0) {
        ((Fn4)fTailStart)(fTail.begin(), x,n, v,v,v,v, v,v,v,v);
    }
}

void SK_VECTORCALL SkRasterPipeline::JustReturn(Stage*, size_t, size_t, Sk4f,Sk4f,Sk4f,Sk4f,
                                                                        Sk4f,Sk4f,Sk4f,Sk4f) {}
