/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOnce.h"
#include "SkRunnable.h"
#include "SkThreadPool.h"
#include "Test.h"
#include "TestClassDef.h"

DEF_SK_ONCE(add_five, int* x) {
    *x += 5;
}

DEF_TEST(SkOnce_Singlethreaded, r) {
    int x = 0;

    // No matter how many times we do this, x will be 5.
    SK_ONCE(add_five, &x);
    SK_ONCE(add_five, &x);
    SK_ONCE(add_five, &x);
    SK_ONCE(add_five, &x);
    SK_ONCE(add_five, &x);

    REPORTER_ASSERT(r, 5 == x);
}


DEF_SK_ONCE(add_six, int* x) {
    *x += 6;
}

namespace {

class Racer : public SkRunnable {
public:
    int* ptr;
    virtual void run() SK_OVERRIDE {
        SK_ONCE(add_six, ptr);
    }
};

}  // namespace

DEF_TEST(SkOnce_Multithreaded, r) {
    const int kTasks = 16, kThreads = 4;

    // Make a bunch of tasks that will race to be the first to add six to x.
    Racer racers[kTasks];
    int x = 0;
    for (int i = 0; i < kTasks; i++) {
        racers[i].ptr = &x;
    }

    // Let them race.
    SkAutoTDelete<SkThreadPool> pool(new SkThreadPool(kThreads));
    for (int i = 0; i < kTasks; i++) {
        pool->add(&racers[i]);
    }
    pool.free();  // Blocks until all threads are done.

    // Only one should have done the +=.
    REPORTER_ASSERT(r, 6 == x);
}
