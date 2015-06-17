/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkLazyPtr.h"
#include "SkRunnable.h"
#include "SkTaskGroup.h"

namespace {

struct CreateIntFromFloat {
    CreateIntFromFloat(float val) : fVal(val) {}
    int* operator()() const { return SkNEW_ARGS(int, ((int)fVal)); }
    float fVal;
};

// As a template argument this must have external linkage.
void custom_destroy(int* ptr) { *ptr = 99; }

} // namespace

DEF_TEST(LazyPtr, r) {
    // Basic usage: calls SkNEW(int).
    SkLazyPtr<int> lazy;
    int* ptr = lazy.get();
    REPORTER_ASSERT(r, ptr);
    REPORTER_ASSERT(r, lazy.get() == ptr);

    // Advanced usage: calls a functor.
    SkLazyPtr<int> lazyFunctor;
    int* six = lazyFunctor.get(CreateIntFromFloat(6.4f));
    REPORTER_ASSERT(r, six);
    REPORTER_ASSERT(r, 6 == *six);

    // Just makes sure this is safe.
    SkLazyPtr<double> neverRead;

    // SkLazyPtr supports custom destroy methods.
    {
        SkLazyPtr<int, custom_destroy> customDestroy;
        ptr = customDestroy.get();
        // custom_destroy called here.
    }
    REPORTER_ASSERT(r, ptr);
    REPORTER_ASSERT(r, 99 == *ptr);
    // Since custom_destroy didn't actually delete ptr, we do now.
    SkDELETE(ptr);
}

DEF_TEST(LazyPtr_Threaded, r) {
    static const int kRacers = 321;

    // Race to intialize the pointer by calling .get().
    SkLazyPtr<int> lazy;
    int* seen[kRacers];

    sk_parallel_for(kRacers, [&](int i) {
        seen[i] = lazy.get();
    });

    // lazy.get() should return the same pointer to all threads.
    for (int i = 1; i < kRacers; i++) {
        REPORTER_ASSERT(r, seen[i] != nullptr);
        REPORTER_ASSERT(r, seen[i] == seen[0]);
    }
}
