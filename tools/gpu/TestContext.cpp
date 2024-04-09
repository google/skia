
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestContext.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTraceEvent.h"
#include "tools/gpu/FlushFinishTracker.h"
#include "tools/gpu/GpuTimer.h"

namespace sk_gpu_test {
TestContext::TestContext() : fGpuTimer(nullptr) {}

TestContext::~TestContext() {
    // Subclass should call teardown.
    SkASSERT(!fGpuTimer);
}

sk_sp<GrDirectContext> TestContext::makeContext(const GrContextOptions&) {
    return nullptr;
}

void TestContext::makeNotCurrent() const { this->onPlatformMakeNotCurrent(); }
void TestContext::makeCurrent() const { this->onPlatformMakeCurrent(); }

SkScopeExit TestContext::makeCurrentAndAutoRestore() const {
    auto asr = SkScopeExit(this->onPlatformGetAutoContextRestore());
    this->makeCurrent();
    return asr;
}

void TestContext::flushAndWaitOnSync(GrDirectContext* context) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(context);

    if (fFinishTrackers[fCurrentFlushIdx]) {
        fFinishTrackers[fCurrentFlushIdx]->waitTillFinished();
    }

    fFinishTrackers[fCurrentFlushIdx].reset(new FlushFinishTracker(context));

    // We add an additional ref to the current flush tracker here. This ref is owned by the finish
    // callback on the flush call. The finish callback will unref the tracker when called.
    fFinishTrackers[fCurrentFlushIdx]->ref();

    GrFlushInfo flushInfo;
    flushInfo.fFinishedProc = FlushFinishTracker::FlushFinished;
    flushInfo.fFinishedContext = fFinishTrackers[fCurrentFlushIdx].get();

    context->flush(flushInfo);
    context->submit();

    fCurrentFlushIdx = (fCurrentFlushIdx + 1) % std::size(fFinishTrackers);
}

void TestContext::flushAndSyncCpu(GrDirectContext* context) {
    SkASSERT(context);
    context->flush();
    context->submit(GrSyncCpu::kYes);
}

void TestContext::testAbandon() {
}

void TestContext::teardown() {
    fGpuTimer.reset();
}

}  // namespace sk_gpu_test
