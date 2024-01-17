/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/gl/GrGLFinishCallbacks.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"

GrGLFinishCallbacks::GrGLFinishCallbacks(GrGLGpu* gpu) : fGpu(gpu) {}

GrGLFinishCallbacks::~GrGLFinishCallbacks() {
    this->callAll(true);
}

void GrGLFinishCallbacks::add(GrGpuFinishedProc finishedProc,
                              GrGpuFinishedContext finishedContext) {
    SkASSERT(finishedProc);
    FinishCallback callback;
    callback.fCallback = finishedProc;
    callback.fContext = finishedContext;
    callback.fFence = fGpu->insertFence();
    fCallbacks.push_back(callback);
}

void GrGLFinishCallbacks::check() {
    // Bail after the first unfinished sync since we expect they signal in the order inserted.
    while (!fCallbacks.empty() && fGpu->waitFence(fCallbacks.front().fFence)) {
        // While we are processing a proc we need to make sure to remove it from the callback list
        // before calling it. This is because the client could trigger a call (e.g. calling
        // flushAndSubmit(/*sync=*/true)) that has us process the finished callbacks. We also must
        // process deleting the fence before a client may abandon the context.
        auto finishCallback = fCallbacks.front();
        fGpu->deleteFence(finishCallback.fFence);
        fCallbacks.pop_front();
        finishCallback.fCallback(finishCallback.fContext);
    }
}

void GrGLFinishCallbacks::callAll(bool doDelete) {
    while (!fCallbacks.empty()) {
        // While we are processing a proc we need to make sure to remove it from the callback list
        // before calling it. This is because the client could trigger a call (e.g. calling
        // flushAndSubmit(/*sync=*/true)) that has us process the finished callbacks. We also must
        // process deleting the fence before a client may abandon the context.
        auto finishCallback = fCallbacks.front();
        if (doDelete) {
            fGpu->deleteFence(finishCallback.fFence);
        }
        fCallbacks.pop_front();
        finishCallback.fCallback(finishCallback.fContext);
    }
}
