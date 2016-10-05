/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline() {}

void SkRasterPipeline::append(SkRasterPipeline::Fn body,
                              SkRasterPipeline::Fn tail,
                              void* ctx) {
    // Each stage holds its own context and the next function to call.
    // So the pipeline itself has to hold onto the first function that starts the pipeline.
    (fBody.empty() ? fBodyStart : fBody.back().fNext) = body;
    (fTail.empty() ? fTailStart : fTail.back().fNext) = tail;

    // Each last stage starts with its next function set to JustReturn as a safety net.
    // It'll be overwritten by the next call to append().
    fBody.push_back({ &JustReturn, ctx });
    fTail.push_back({ &JustReturn, ctx });
}

void SkRasterPipeline::append(StockStage stage, void* ctx) {
    this->append(SkOpts::stages_4[stage], SkOpts::stages_1_3[stage], ctx);
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    SkASSERT(src.fBody.count() == src.fTail.count());

    Fn body = src.fBodyStart,
       tail = src.fTailStart;
    for (int i = 0; i < src.fBody.count(); i++) {
        SkASSERT(src.fBody[i].fCtx == src.fTail[i].fCtx);
        this->append(body, tail, src.fBody[i].fCtx);
        body = src.fBody[i].fNext;
        tail = src.fTail[i].fNext;
    }
}

void SkRasterPipeline::run(size_t x, size_t n) {
    // It's fastest to start uninitialized if the compilers all let us.  If not, next fastest is 0.
    Sk4f v;

    while (n >= 4) {
        fBodyStart(fBody.begin(), x,0, v,v,v,v, v,v,v,v);
        x += 4;
        n -= 4;
    }
    if (n > 0) {
        fTailStart(fTail.begin(), x,n, v,v,v,v, v,v,v,v);
    }
}

void SK_VECTORCALL SkRasterPipeline::JustReturn(Stage*, size_t, size_t, Sk4f,Sk4f,Sk4f,Sk4f,
                                                                        Sk4f,Sk4f,Sk4f,Sk4f) {}
