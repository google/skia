/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkString.h"
#include "include/private/base/SkMutex.h"
#include "src/base/SkSpinlock.h"

#include <shared_mutex>

template <typename Mutex>
class MutexBench : public Benchmark {
public:
    MutexBench(SkString benchPrefix) : fBenchName(benchPrefix += "UncontendedBenchmark") { }
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return fBenchName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            fMu.acquire();
            fMu.release();
        }
    }

private:
    SkString fBenchName;
    Mutex fMu;
};


// Wrappers around a possibly null shared mutex that acquire the lock for
// as long as the object is in scope.
class SK_SCOPED_CAPABILITY Exclusive {
public:
    explicit Exclusive(std::shared_mutex* maybe_lock)
        : fLock(maybe_lock) {
        if (fLock) {
            fLock->lock();
        }
    }
    ~Exclusive() {
        if (fLock) {
            fLock->unlock();
        }
    }

private:
    std::shared_mutex* fLock;
};
class SK_SCOPED_CAPABILITY Shared {
public:
    explicit Shared(std::shared_mutex* maybe_lock)
        : fLock(maybe_lock)  {
        if (fLock) {
            fLock->lock_shared();
        }
    }

    ~Shared() {
        if (fLock) {
            fLock->unlock_shared();
        }
    }

private:
    std::shared_mutex* fLock;
};

static std::shared_mutex* maybe_dw_mutex(size_t typeface) {
    static std::shared_mutex mutex;
    return typeface > 60 ? nullptr : &mutex;
}

bool is_hinted(size_t typeface) {
    Exclusive l(maybe_dw_mutex(typeface));
    Exclusive l2(maybe_dw_mutex(typeface));
    if (typeface > 20) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MutexBench<SkMutex>(SkString("SkMutex")); )
DEF_BENCH( return new MutexBench<SkSpinlock>(SkString("SkSpinlock")); )
