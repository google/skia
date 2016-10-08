/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline() {
    fBodyStart = SkOpts::body[just_return];
    fTailStart = SkOpts::tail[just_return];
}

void SkRasterPipeline::append(void (*body)(), void (*tail)(), void* ctx) {
    // Each stage holds its own context and the next function to call.
    // So the pipeline itself has to hold onto the first function that starts the pipeline.
    (fBody.empty() ? fBodyStart : fBody.back().fNext) = body;
    (fTail.empty() ? fTailStart : fTail.back().fNext) = tail;

    // Each last stage starts with its next function set to JustReturn as a safety net.
    // It'll be overwritten by the next call to append().
    fBody.push_back({ SkOpts::body[just_return], ctx });
    fTail.push_back({ SkOpts::tail[just_return], ctx });
}

void SkRasterPipeline::append(StockStage stage, void* ctx) {
    this->append(SkOpts::body[stage], SkOpts::tail[stage], ctx);
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    SkASSERT(src.fBody.count() == src.fTail.count());

    auto body = src.fBodyStart,
         tail = src.fTailStart;
    for (int i = 0; i < src.fBody.count(); i++) {
        SkASSERT(src.fBody[i].fCtx == src.fTail[i].fCtx);
        this->append(body, tail, src.fBody[i].fCtx);
        body = src.fBody[i].fNext;
        tail = src.fTail[i].fNext;
    }
}

void SkRasterPipeline::run(size_t x, size_t n) {
    SkOpts::run_pipeline(x,n, fBodyStart,fBody.begin(), fTailStart,fTail.begin());
}
