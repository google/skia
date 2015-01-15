/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Timer.h"

Timer::Timer(SkGLContext* gl)
        : fCpu(-1.0)
        , fWall(-1.0)
        , fTruncatedCpu(-1.0)
        , fTruncatedWall(-1.0)
        , fGpu(-1.0)
#if SK_SUPPORT_GPU
        , fGpuTimer(gl)
#endif
        {}

void Timer::start() {
    fSysTimer.startWall();
    fTruncatedSysTimer.startWall();
#if SK_SUPPORT_GPU
    fGpuTimer.start();
#endif
    fSysTimer.startCpu();
    fTruncatedSysTimer.startCpu();
}

void Timer::end() {
    fCpu = fSysTimer.endCpu();
#if SK_SUPPORT_GPU
    //It is important to stop the cpu clocks first,
    //as the following will cpu wait for the gpu to finish.
    fGpu = fGpuTimer.end();
#endif
    fWall = fSysTimer.endWall();
}

void Timer::truncatedEnd() {
    fTruncatedCpu = fTruncatedSysTimer.endCpu();
    fTruncatedWall = fTruncatedSysTimer.endWall();
}

WallTimer::WallTimer() : fWall(-1.0) {}

void WallTimer::start() {
    fSysTimer.startWall();
}

void WallTimer::end() {
    fWall = fSysTimer.endWall();
}
