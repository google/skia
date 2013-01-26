/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCountdown.h"
#include "SkThread.h"

SkCountdown::SkCountdown(int32_t count)
: fCount(count) {}

void SkCountdown::reset(int32_t count) {
    fCount = count;
}

void SkCountdown::run() {
    if (sk_atomic_dec(&fCount) == 1) {
        fReady.lock();
        fReady.signal();
        fReady.unlock();
    }
}

void SkCountdown::wait() {
    fReady.lock();
    while (fCount > 0) {
        fReady.wait();
    }
    fReady.unlock();
}
