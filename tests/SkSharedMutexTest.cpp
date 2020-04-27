/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSharedMutex.h"
#include "src/core/SkTaskGroup.h"

#include "tests/Test.h"

DEF_TEST(SkSharedMutexBasic, r) {
    SkSharedMutex sm;
    sm.acquire();
    sm.assertHeld();
    sm.release();
    sm.acquireShared();
    sm.assertHeldShared();
    sm.releaseShared();
}

DEF_TEST(SkSharedMutexMultiThreaded, r) {
    SkSharedMutex sm;
    int shared[] = {0,0,0,0, 0,0,0,0, 0,0};

    SkTaskGroup().batch(8, [&](int i) {
        for (int n = 3; n --> 0; ) {
            const bool writer = i%4 == 0;
            if (writer) {
                sm.acquire();
                sm.assertHeld();
                for (int& x : shared) {
                    x++;
                }
                sm.release();
            } else {
                sm.acquireShared();
                sm.assertHeldShared();
                for (int x : shared) {
                    REPORTER_ASSERT(r, x == shared[0]);
                }
                sm.releaseShared();
            }
        }
    });
}
