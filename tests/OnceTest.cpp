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

SK_DECLARE_STATIC_ONCE(st_once);
DEF_TEST(SkOnce_Singlethreaded, r) {
    int x = 0;

    // No matter how many times we do this, x will be 5.
    SkOnce(&st_once, add_five, &x);
    SkOnce(&st_once, add_five, &x);
    SkOnce(&st_once, add_five, &x);
    SkOnce(&st_once, add_five, &x);
    SkOnce(&st_once, add_five, &x);

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

    void run() SK_OVERRIDE {
        SkOnce(once, add_six, ptr);
    }
};

}  // namespace

SK_DECLARE_STATIC_ONCE(mt_once);
DEF_TEST(SkOnce_Multithreaded, r) {
    const int kTasks = 16;

    // Make a bunch of tasks that will race to be the first to add six to x.
    Racer racers[kTasks];
    int x = 0;
    for (int i = 0; i < kTasks; i++) {
        racers[i].once = &mt_once;
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

SK_DECLARE_STATIC_ONCE(noarg_once);
DEF_TEST(SkOnce_NoArg, r) {
    SkOnce(&noarg_once, inc_gX);
    SkOnce(&noarg_once, inc_gX);
    SkOnce(&noarg_once, inc_gX);
    REPORTER_ASSERT(r, 1 == gX);
}
