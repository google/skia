/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCountdown_DEFINED
#define SkCountdown_DEFINED

#include "SkCondVar.h"
#include "SkRunnable.h"
#include "SkTypes.h"

class SkCountdown : public SkRunnable {
public:
    explicit SkCountdown(int32_t count);

    /**
     * Resets the countdown to the count provided.
     */
    void reset(int32_t count);

    virtual void run() SK_OVERRIDE;

    /**
     * Blocks until run() has been called count times.
     */
    void wait();

private:
    SkCondVar fReady;
    int32_t   fCount;
};

#endif
