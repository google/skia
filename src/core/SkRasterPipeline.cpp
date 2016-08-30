/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline() {}

void SkRasterPipeline::append(SkRasterPipeline::Fn body_fn, const void* body_ctx,
                              SkRasterPipeline::Fn tail_fn, const void* tail_ctx) {
    // Each stage holds its own context and the next function to call.
    // So the pipeline itself has to hold onto the first function that starts the pipeline.
    (fBody.empty() ? fBodyStart : fBody.back().fNext) = body_fn;
    (fTail.empty() ? fTailStart : fTail.back().fNext) = tail_fn;

    // Each last stage starts with its next function set to JustReturn as a safety net.
    // It'll be overwritten by the next call to append().
    fBody.push_back({ &JustReturn, const_cast<void*>(body_ctx) });
    fTail.push_back({ &JustReturn, const_cast<void*>(tail_ctx) });
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    SkASSERT(src.fBody.count() == src.fTail.count());

    Fn body_fn = src.fBodyStart,
       tail_fn = src.fTailStart;
    for (int i = 0; i < src.fBody.count(); i++) {
        this->append(body_fn, src.fBody[i].fCtx,
                     tail_fn, src.fTail[i].fCtx);
        body_fn = src.fBody[i].fNext;
        tail_fn = src.fTail[i].fNext;
    }
}

void SkRasterPipeline::run(size_t x, size_t n) {
    // It's fastest to start uninitialized if the compilers all let us.  If not, next fastest is 0.
    Sk4f v;

    while (n >= 4) {
        fBodyStart(fBody.begin(), x, v,v,v,v, v,v,v,v);
        x += 4;
        n -= 4;
    }
    while (n > 0) {
        fTailStart(fTail.begin(), x, v,v,v,v, v,v,v,v);
        x += 1;
        n -= 1;
    }
}

void SK_VECTORCALL SkRasterPipeline::JustReturn(Stage*, size_t, Sk4f,Sk4f,Sk4f,Sk4f,
                                                                Sk4f,Sk4f,Sk4f,Sk4f) {}
