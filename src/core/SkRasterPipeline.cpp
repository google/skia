/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline() {}

void SkRasterPipeline::append(SkRasterPipeline::Fn fn, const void* ctx) {
    // Each stage holds its own context and the next function to call.
    // So the pipeline itself has to hold onto the first function that starts the pipeline.
    (fStages.empty() ? fStart : fStages.back().fNext) = fn;

    // Each last stage starts with its next function set to JustReturn as a safety net.
    // It'll be overwritten by the next call to append().
    fStages.push_back({ &JustReturn, const_cast<void*>(ctx) });
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    Fn fn = src.fStart;
    for (int i = 0; i < src.fStages.count(); i++) {
        this->append(fn, src.fStages[i].fCtx);
        fn = src.fStages[i].fNext;
    }
}

void SkRasterPipeline::run(size_t x, size_t n) {
    // It's fastest to start uninitialized if the compilers all let us.  If not, next fastest is 0.
    Sk4f v;

    while (n >= 4) {
        fStart(fStages.begin(), x,0, v,v,v,v, v,v,v,v);
        x += 4;
        n -= 4;
    }
    if (n > 0) {
        fStart(fStages.begin(), x,n, v,v,v,v, v,v,v,v);
    }
}

void SK_VECTORCALL SkRasterPipeline::JustReturn(Stage*, size_t, size_t, Sk4f,Sk4f,Sk4f,Sk4f,
                                                                        Sk4f,Sk4f,Sk4f,Sk4f) {}
