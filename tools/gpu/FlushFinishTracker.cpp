/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/FlushFinishTracker.h"

#include "src/core/SkTraceEvent.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#endif

#include <chrono>

namespace sk_gpu_test {

void FlushFinishTracker::waitTillFinished(std::function<void()> tick) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    auto begin = std::chrono::steady_clock::now();
    auto end = begin;
    while (!fIsFinished && (end - begin) < std::chrono::seconds(2)) {
        if (tick) {
            tick();
        }
        bool foundContext = false;
#if defined(SK_GANESH)
        if (fContext) {
            fContext->checkAsyncWorkCompletion();
            foundContext = true;
        }
#endif
#if defined(SK_GRAPHITE)
        if (fGraphiteContext) {
            SkASSERT(fGraphiteContext);
            fGraphiteContext->checkAsyncWorkCompletion();
            foundContext = true;
        }
#endif
        if (!foundContext) {
            SkDEBUGFAIL("No valid Context");
        }
        end = std::chrono::steady_clock::now();
    }
    if (!fIsFinished) {
        SkDebugf("WARNING: Wait failed for flush sync. Timings might not be accurate.\n");
    }
}

} //namespace sk_gpu_test
