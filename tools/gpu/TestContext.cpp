
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestContext.h"

#include "tools/gpu/GpuTimer.h"

#include "include/gpu/GrContext.h"

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

sk_sp<GrContext> TestContext::makeGrContext(const GrContextOptions&) {
    return nullptr;
}

void TestContext::makeCurrent() const { this->onPlatformMakeCurrent(); }

SkScopeExit TestContext::makeCurrentAndAutoRestore() const {
    auto asr = SkScopeExit(this->onPlatformGetAutoContextRestore());
    this->makeCurrent();
    return asr;
}

void TestContext::swapBuffers() { this->onPlatformSwapBuffers(); }


void TestContext::waitOnSyncOrSwap() {
    if (!fFenceSync) {
        // Fallback on the platform SwapBuffers method for synchronization. This may have no effect.
        this->swapBuffers();
        return;
    }

    this->submit();
    if (fFrameFences[fCurrentFenceIdx]) {
        if (!fFenceSync->waitFence(fFrameFences[fCurrentFenceIdx])) {
            SkDebugf("WARNING: Wait failed for fence sync. Timings might not be accurate.\n");
        }
        fFenceSync->deleteFence(fFrameFences[fCurrentFenceIdx]);
    }

    fFrameFences[fCurrentFenceIdx] = fFenceSync->insertFence();
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
