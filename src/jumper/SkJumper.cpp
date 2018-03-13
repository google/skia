/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"
#include "SkTemplates.h"

SkRasterPipeline::StartPipelineFn SkRasterPipeline::build_pipeline(void** ip) const {
#ifndef SK_JUMPER_DISABLE_8BIT
    // We'll try to build a lowp pipeline, but if that fails fallback to a highp float pipeline.
    void** reset_point = ip;

    // Stages are stored backwards in fStages, so we reverse here, back to front.
    *--ip = (void*)SkOpts::just_return_lowp;
    for (const StageList* st = fStages; st; st = st->prev) {
        if (st->stage == SkRasterPipeline::clamp_0 ||
            st->stage == SkRasterPipeline::clamp_1) {
            continue;  // No-ops in lowp.
        }
        SkOpts::StageFn fn;
        if (!st->rawFunction && (fn = SkOpts::stages_lowp[st->stage])) {
            if (st->ctx) {
                *--ip = st->ctx;
            }
            *--ip = (void*)fn;
        } else {
            ip = reset_point;
            break;
        }
    }
    if (ip != reset_point) {
        return SkOpts::start_pipeline_lowp;
    }
#endif

    *--ip = (void*)SkOpts::just_return_highp;
    for (const StageList* st = fStages; st; st = st->prev) {
        if (st->ctx) {
            *--ip = st->ctx;
        }
        if (st->rawFunction) {
            *--ip = (void*)st->stage;
        } else {
            *--ip = (void*)SkOpts::stages_highp[st->stage];
        }
    }
    return SkOpts::start_pipeline_highp;
}

void SkRasterPipeline::run(size_t x, size_t y, size_t w, size_t h) const {
    if (this->empty()) {
        return;
    }

    // Best to not use fAlloc here... we can't bound how often run() will be called.
    SkAutoSTMalloc<64, void*> program(fSlotsNeeded);

    auto start_pipeline = this->build_pipeline(program.get() + fSlotsNeeded);
    start_pipeline(x,y,x+w,y+h, program.get());
}

std::function<void(size_t, size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    if (this->empty()) {
        return [](size_t, size_t, size_t, size_t) {};
    }

    void** program = fAlloc->makeArray<void*>(fSlotsNeeded);

    auto start_pipeline = this->build_pipeline(program + fSlotsNeeded);
    return [=](size_t x, size_t y, size_t w, size_t h) {
        start_pipeline(x,y,x+w,y+h, program);
    };
}
