/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/gl/GrGLFinishCallbacks.h"

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"

#include <utility>

GrGLFinishCallbacks::GrGLFinishCallbacks(GrGLGpu* gpu) : fGpu(gpu) {}

GrGLFinishCallbacks::~GrGLFinishCallbacks() {
    this->callAll(true);
}

void GrGLFinishCallbacks::add(skgpu::AutoCallback callback, GrGLint timerQuery) {
    SkASSERT(callback);
    FinishCallback finishCallback;
    finishCallback.fCallback   = std::move(callback);
    finishCallback.fSync       = fGpu->insertSync();
    finishCallback.fTimerQuery = timerQuery;
    fCallbacks.push_back(std::move(finishCallback));
}

void GrGLFinishCallbacks::check() {
    // Bail after the first unfinished sync since we expect they signal in the order inserted.
    while (!fCallbacks.empty() && fGpu->testSync(fCallbacks.front().fSync)) {
        // While we are processing a proc we need to make sure to remove it from the callback list
        // before calling it. This is because the client could trigger a call (e.g. calling
        // flushAndSubmit(/*sync=*/true)) that has us process the finished callbacks. We also must
        // process deleting the sync before a client may abandon the context.
        auto& finishCallback = fCallbacks.front();
        if (finishCallback.fSync) {
            fGpu->deleteSync(finishCallback.fSync);
        }
        skgpu::GpuStats stats;
        if (auto timerQuery = finishCallback.fTimerQuery) {
            stats.elapsedTime = fGpu->getTimerQueryResult(timerQuery);
            if (finishCallback.fCallback.receivesGpuStats()) {
                finishCallback.fCallback.setStats(stats);
            }
        }
        fCallbacks.pop_front();
    }
}

void GrGLFinishCallbacks::callAll(bool doDelete) {
    while (!fCallbacks.empty()) {
        // While we are processing a proc we need to make sure to remove it from the callback list
        // before calling it. This is because the client could trigger a call (e.g. calling
        // flushAndSubmit(/*sync=*/true)) that has us process the finished callbacks. We also must
        // process deleting the sync before a client may abandon the context.
        auto& finishCallback = fCallbacks.front();
        skgpu::GpuStats stats;
        if (doDelete && finishCallback.fSync) {
            fGpu->deleteSync(finishCallback.fSync);
            if (finishCallback.fTimerQuery) {
                stats.elapsedTime = fGpu->getTimerQueryResult(finishCallback.fTimerQuery);
                if (finishCallback.fCallback.receivesGpuStats()) {
                    finishCallback.fCallback.setStats(stats);
                }
            }
        }
        fCallbacks.pop_front();
    }
}
