/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrFinishCallbacks.h"
#include "src/gpu/GrGpu.h"

GrFinishCallbacks::GrFinishCallbacks(GrGpu* gpu) : fGpu(gpu) {}

GrFinishCallbacks::~GrFinishCallbacks() {
    this->callAll(true);
}

void GrFinishCallbacks::add(GrGpuFinishedProc finishedProc,
                            GrGpuFinishedContext finishedContext) {
    SkASSERT(finishedProc);
    FinishCallback callback;
    callback.fCallback = finishedProc;
    callback.fContext = finishedContext;
    callback.fFence = fGpu->insertFence();
    fCallbacks.push_back(callback);
}

void GrFinishCallbacks::check() {
    // Bail after the first unfinished sync since we expect they signal in the order inserted.
    while (!fCallbacks.empty() && fGpu->waitFence(fCallbacks.front().fFence)) {
        fCallbacks.front().fCallback(fCallbacks.front().fContext);
        fGpu->deleteFence(fCallbacks.front().fFence);
        fCallbacks.pop_front();
    }
}

void GrFinishCallbacks::callAll(bool doDelete) {
    while (!fCallbacks.empty()) {
        fCallbacks.front().fCallback(fCallbacks.front().fContext);
        if (doDelete) {
            fGpu->deleteFence(fCallbacks.front().fFence);
        }
        fCallbacks.pop_front();
    }
}
