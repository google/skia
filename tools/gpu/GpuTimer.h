/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GpuTimer_DEFINED
#define GpuTimer_DEFINED

#include "SkTypes.h"
#include "SkExchange.h"
#include <chrono>

namespace sk_gpu_test {

using PlatformTimerQuery = uint64_t;
static constexpr PlatformTimerQuery kInvalidTimerQuery = 0;

/**
 * Platform-independent interface for timing operations on the GPU.
 */
class GpuTimer {
public:
    GpuTimer(bool disjointSupport)
        : fDisjointSupport(disjointSupport)
        , fActiveTimer(kInvalidTimerQuery) {
    }
    virtual ~GpuTimer() { SkASSERT(!fActiveTimer); }

    /**
     * Returns whether this timer can detect disjoint GPU operations while timing. If false, a query
     * has less confidence when it completes with QueryStatus::kAccurate.
     */
    bool disjointSupport() const { return fDisjointSupport; }

    /**
     * Inserts a "start timing" command in the GPU command stream.
     */
    void queueStart() {
        SkASSERT(!fActiveTimer);
        fActiveTimer = this->onQueueTimerStart();
    }

    /**
     * Inserts a "stop timing" command in the GPU command stream.
     *
     * @return a query object that can retrieve the time elapsed once the timer has completed.
     */
    PlatformTimerQuery SK_WARN_UNUSED_RESULT queueStop() {
        SkASSERT(fActiveTimer);
        this->onQueueTimerStop(fActiveTimer);
        return skstd::exchange(fActiveTimer, kInvalidTimerQuery);
    }

    enum class QueryStatus {
        kInvalid,  //<! the timer query is invalid.
        kPending,  //<! the timer is still running on the GPU.
        kDisjoint, //<! the query is complete, but dubious due to disjoint GPU operations.
        kAccurate  //<! the query is complete and reliable.
    };

    virtual QueryStatus checkQueryStatus(PlatformTimerQuery) = 0;
    virtual std::chrono::nanoseconds getTimeElapsed(PlatformTimerQuery) = 0;
    virtual void deleteQuery(PlatformTimerQuery) = 0;

private:
    virtual PlatformTimerQuery onQueueTimerStart() const = 0;
    virtual void onQueueTimerStop(PlatformTimerQuery) const = 0;

    bool const           fDisjointSupport;
    PlatformTimerQuery   fActiveTimer;
};

}  // namespace sk_gpu_test

#endif
