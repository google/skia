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
    (fBody4.empty() ? fBody4Start : fBody4.back().fNext) = (Fn)body;
    (fTail4.empty() ? fTail4Start : fTail4.back().fNext) = (Fn)tail;

    // Each last stage starts with its next function set to JustReturn as a safety net.
    // It'll be overwritten by the next call to append().
    fBody4.push_back({ (Fn)&JustReturn4, const_cast<void*>(ctx) });
    fTail4.push_back({ (Fn)&JustReturn4, const_cast<void*>(ctx) });
}

void SkRasterPipeline::append(StockStage stage, const void* ctx) {
    this->append(SkOpts::stages_4[stage], SkOpts::stages_1_3[stage], ctx);

    (fBody8.empty() ? fBody8Start : fBody8.back().fNext) = (Fn)SkOpts::stages_8  [stage];
    (fTail8.empty() ? fTail8Start : fTail8.back().fNext) = (Fn)SkOpts::stages_1_7[stage];

    fBody8.push_back({ (Fn)&JustReturn8, const_cast<void*>(ctx) });
    fTail8.push_back({ (Fn)&JustReturn8, const_cast<void*>(ctx) });
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    SkASSERT(src.fBody4.count() == src.fTail4.count());

    Fn4 body = (Fn4)src.fBody4Start,
        tail = (Fn4)src.fTail4Start;
    for (int i = 0; i < src.fBody4.count(); i++) {
        SkASSERT(src.fBody4[i].fCtx == src.fTail4[i].fCtx);
        this->append(body, tail, src.fBody4[i].fCtx);
        body = (Fn4)src.fBody4[i].fNext;
        tail = (Fn4)src.fTail4[i].fNext;
    }

    /* TODO: 8-pixel pipelines */
}

void SkRasterPipeline::run(size_t x, size_t n) {
    if (this->run8(x,n)) {
        return;
    }

    // It's fastest to start uninitialized if the compilers all let us.  If not, next fastest is 0.
    Sk4f v;

    while (n >= 4) {
        ((Fn4)fBody4Start)(fBody4.begin(), x,0, v,v,v,v, v,v,v,v);
        x += 4;
        n -= 4;
    }
    if (n > 0) {
        ((Fn4)fTail4Start)(fTail4.begin(), x,n, v,v,v,v, v,v,v,v);
    }
}

void SK_VECTORCALL SkRasterPipeline::JustReturn4(Stage*, size_t, size_t, Sk4f,Sk4f,Sk4f,Sk4f,
                                                                         Sk4f,Sk4f,Sk4f,Sk4f) {}
