
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestContext.h"

#include <chrono>
#include "include/gpu/GrContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrContextPriv.h"
#include "tools/gpu/GpuTimer.h"

namespace sk_gpu_test {
TestContext::TestContext() : fGpuTimer(nullptr) {}

TestContext::~TestContext() {
    // Subclass should call teardown.
    SkASSERT(!fGpuTimer);
}

sk_sp<GrContext> TestContext::makeGrContext(const GrContextOptions&) {
    return nullptr;
}

void TestContext::makeNotCurrent() const { this->onPlatformMakeNotCurrent(); }
void TestContext::makeCurrent() const { this->onPlatformMakeCurrent(); }

SkScopeExit TestContext::makeCurrentAndAutoRestore() const {
    auto asr = SkScopeExit(this->onPlatformGetAutoContextRestore());
    this->makeCurrent();
    return asr;
}

class SubmitFinishTracker : public SkRefCnt {
public:
    static void SubmitFinished(void* context) {
        auto tracker = static_cast<SubmitFinishTracker*>(context);
        tracker->setFinished();
        tracker->unref();
    }

    SubmitFinishTracker(GrContext* context) : fContext(context) {}

    void setFinished() { fIsFinished = true; }

    void waitTillFinished() {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        auto begin = std::chrono::steady_clock::now();
        auto end = begin;
        while (!fIsFinished && (end - begin) < std::chrono::seconds(2)) {
            fContext->checkAsyncWorkCompletion();
        }
        if (!fIsFinished) {
            SkDebugf("WARNING: Wait failed for flush sync. Timings might not be accurate.\n");
        }
    }

private:
    GrContext* fContext;
    bool fIsFinished = false;
};

void TestContext::flushAndWaitOnSync(GrContext* context) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    SkASSERT(context);

    if (fFinishTrackers[fCurrentSubmitIdx]) {
        fFinishTrackers[fCurrentSubmitIdx]->waitTillFinished();
    }

    fFinishTrackers[fCurrentSubmitIdx].reset(new SubmitFinishTracker(context));
    fFinishTrackers[fCurrentSubmitIdx]->ref();

    GrFlushInfo flushInfo;
    flushInfo.fFinishedProc = SubmitFinishTracker::SubmitFinished;
    flushInfo.fFinishedContext = fFinishTrackers[fCurrentSubmitIdx].get();

    context->flush(flushInfo);

    fCurrentSubmitIdx = (fCurrentSubmitIdx + 1) % SK_ARRAY_COUNT(fFinishTrackers);
}

void TestContext::testAbandon() {
}

void TestContext::teardown() {
    fGpuTimer.reset();
}

}
