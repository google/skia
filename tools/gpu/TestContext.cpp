
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "TestContext.h"

#include "GpuTimer.h"

namespace sk_gpu_test {
TestContext::TestContext()
    : fFenceSync(nullptr)
    , fGpuTimer(nullptr)
    , fCurrentFenceIdx(0) {
    memset(fFrameFences, 0, sizeof(fFrameFences));
}

TestContext::~TestContext() {
    // Subclass should call teardown.
#ifdef SK_DEBUG
    for (size_t i = 0; i < SK_ARRAY_COUNT(fFrameFences); i++) {
        SkASSERT(0 == fFrameFences[i]);
    }
#endif
    SkASSERT(!fFenceSync);
    SkASSERT(!fGpuTimer);
}

void TestContext::makeCurrent() const { this->onPlatformMakeCurrent(); }

void TestContext::swapBuffers() { this->onPlatformSwapBuffers(); }

void TestContext::waitOnSyncOrSwap() {
    SkDebugf("In wait on sync or swap\n");
    if (!fFenceSync) {
        SkDebugf("fallback\n");
        // Fallback on the platform SwapBuffers method for synchronization. This may have no effect.
        this->swapBuffers();
        return;
    }
    SkDebugf("not fallback\n");

    this->submit();
    if (fFrameFences[fCurrentFenceIdx]) {
        if (!fFenceSync->waitFence(fFrameFences[fCurrentFenceIdx])) {
            SkDebugf("WARNING: Wait failed for fence sync. Timings might not be accurate.\n");
        }
        fFenceSync->deleteFence(fFrameFences[fCurrentFenceIdx]);
    }
    SkDebugf("going to insert fence\n");

    fFrameFences[fCurrentFenceIdx] = fFenceSync->insertFence();
    SkDebugf("after insert fence\n");
    fCurrentFenceIdx = (fCurrentFenceIdx + 1) % SK_ARRAY_COUNT(fFrameFences);
}

void TestContext::testAbandon() {
    if (fFenceSync) {
        memset(fFrameFences, 0, sizeof(fFrameFences));
    }
}

void TestContext::teardown() {
    if (fFenceSync) {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fFrameFences); i++) {
            if (fFrameFences[i]) {
                fFenceSync->deleteFence(fFrameFences[i]);
                fFrameFences[i] = 0;
            }
        }
        fFenceSync.reset();
    }
    fGpuTimer.reset();
}

}
