/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline() {}

void SkRasterPipeline::append(SkRasterPipeline::Fn body, const void* body_ctx,
                              SkRasterPipeline::Fn tail, const void* tail_ctx) {
    // We can't add more stages after being rewired to run().
    SkASSERT(!fReadyToRun);

    // For now, just stash the stage's function in its own fNext slot.
    // We'll rewire our stages before running the pipeline so fNext makes sense.
    fBody.push_back({ body, const_cast<void*>(body_ctx) });
    fTail.push_back({ tail, const_cast<void*>(tail_ctx) });
}

void SkRasterPipeline::run(size_t n) {
    if (fBody.empty() || fTail.empty()) {
        return;
    }

    if (!fReadyToRun) {
        auto rewire = [](Stages* stages) {
            SkASSERT(!stages->empty());

            // Rotate the fNext pointers so they point to the next function to
            // call, not function we're currently calling as set by append().
            auto start = stages->front().fNext;
            for (int i = 0; i < stages->count() - 1; i++) {
                (*stages)[i].fNext = (*stages)[i+1].fNext;
            }
            stages->back().fNext = start;  // This is a pretty handy place to stash this.
        };
        rewire(&fBody);
        rewire(&fTail);
        fReadyToRun = true;
    }

    // It's fastest to start uninitialized if the compilers all let us.  If not, next fastest is 0.
    Sk4f v;

    auto start_body = fBody.back().fNext,  // See rewire().
         start_tail = fTail.back().fNext;

    auto body = fBody.begin(),
         tail = fTail.begin();

    size_t x = 0;
    while (n >= 4) {
        start_body(body, x, v,v,v,v, v,v,v,v);
        x += 4;
        n -= 4;
    }
    while (n > 0) {
        start_tail(tail, x, v,v,v,v, v,v,v,v);
        x += 1;
        n -= 1;
    }
}
