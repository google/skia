/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FlushFinishTracker_DEFINED
#define FlushFinishTracker_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkTraceEvent.h"

#include <chrono>

namespace sk_gpu_test {

class FlushFinishTracker : public SkRefCnt {
public:
    static void FlushFinished(void* finishedContext) {
        auto tracker = static_cast<FlushFinishTracker*>(finishedContext);
        tracker->setFinished();
        tracker->unref();
    }

    FlushFinishTracker(GrContext* context) : fContext(context) {}

    void setFinished() { fIsFinished = true; }

    void waitTillFinished() {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);
        auto begin = std::chrono::steady_clock::now();
        auto end = begin;
        while (!fIsFinished && (end - begin) < std::chrono::seconds(2)) {
            fContext->checkAsyncWorkCompletion();
            end = std::chrono::steady_clock::now();
        }
        if (!fIsFinished) {
            SkDebugf("WARNING: Wait failed for flush sync. Timings might not be accurate.\n");
        }
    }

private:
    GrContext* fContext;

    // Currently we don't have the this bool be atomic cause all current uses of this class happen
    // on a single thread. In other words we call flush, checkAsyncWorkCompletion, and
    // waitTillFinished all on the same thread. If we ever want to support the flushing and waiting
    // to happen on different threads then we should make this atomic.
    bool fIsFinished = false;
};

} //namespace sk_gpu_test

#endif
