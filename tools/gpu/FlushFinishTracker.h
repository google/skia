/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FlushFinishTracker_DEFINED
#define FlushFinishTracker_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"

#include <functional>

class GrDirectContext;

#if defined(SK_GRAPHITE)
namespace skgpu::graphite { class Context; }
#endif

namespace sk_gpu_test {

class FlushFinishTracker : public SkRefCnt {
public:
    static void FlushFinished(void* finishedContext) {
        auto tracker = static_cast<FlushFinishTracker*>(finishedContext);
        tracker->setFinished();
        tracker->unref();
    }

    static void FlushFinishedResult(void* finishedContext, skgpu::CallbackResult) {
        FlushFinished(finishedContext);
    }

    FlushFinishTracker(GrDirectContext* context) : fContext(context) {}
#if defined(SK_GRAPHITE)
    FlushFinishTracker(skgpu::graphite::Context* context) : fGraphiteContext(context) {}
#endif

    void setFinished() { fIsFinished = true; }

    void waitTillFinished(std::function<void()> tick = {});

private:
    GrDirectContext* fContext = nullptr;
#if defined(SK_GRAPHITE)
    skgpu::graphite::Context*  fGraphiteContext = nullptr;
#endif

    // Currently we don't have the this bool be atomic cause all current uses of this class happen
    // on a single thread. In other words we call flush, checkAsyncWorkCompletion, and
    // waitTillFinished all on the same thread. If we ever want to support the flushing and waiting
    // to happen on different threads then we should make this atomic.
    bool fIsFinished = false;
};

} //namespace sk_gpu_test

#endif
