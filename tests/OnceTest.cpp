/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOnce.h"
#include "SkRunnable.h"
#include "SkTaskGroup.h"
#include "Test.h"

static void add_five(int* x) {
    *x += 5;
}

DEF_TEST(SkOnce_Singlethreaded, r) {
    int x = 0;

    SK_DECLARE_STATIC_ONCE(once);
    // No matter how many times we do this, x will be 5.
    SkOnce(&once, add_five, &x);
    SkOnce(&once, add_five, &x);
    SkOnce(&once, add_five, &x);
    SkOnce(&once, add_five, &x);
    SkOnce(&once, add_five, &x);

    REPORTER_ASSERT(r, 5 == x);
}

static void add_six(int* x) {
    *x += 6;
}

namespace {

class Racer : public SkRunnable {
public:
    SkOnceFlag* once;
    int* ptr;

    virtual void run() SK_OVERRIDE {
        SkOnce(once, add_six, ptr);
    }
};

}  // namespace

DEF_TEST(SkOnce_Multithreaded, r) {
    const int kTasks = 16;

    // Make a bunch of tasks that will race to be the first to add six to x.
    Racer racers[kTasks];
    SK_DECLARE_STATIC_ONCE(once);
    int x = 0;
    for (int i = 0; i < kTasks; i++) {
        racers[i].once = &once;
        racers[i].ptr = &x;
    }

    // Let them race.
    SkTaskGroup tg;
    for (int i = 0; i < kTasks; i++) {
        tg.add(&racers[i]);
    }
    tg.wait();

    // Only one should have done the +=.
    REPORTER_ASSERT(r, 6 == x);
}

static int gX = 0;
static void inc_gX() { gX++; }

DEF_TEST(SkOnce_NoArg, r) {
    SK_DECLARE_STATIC_ONCE(once);
    SkOnce(&once, inc_gX);
    SkOnce(&once, inc_gX);
    SkOnce(&once, inc_gX);
    REPORTER_ASSERT(r, 1 == gX);
}
