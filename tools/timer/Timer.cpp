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

SkString HumanizeMs(double ms) {
    if (ms > 60e+3)  return SkStringPrintf("%.3gm", ms/60e+3);
    if (ms >  1e+3)  return SkStringPrintf("%.3gs",  ms/1e+3);
    if (ms <  1e-3)  return SkStringPrintf("%.3gns", ms*1e+6);
#ifdef SK_BUILD_FOR_WIN
    if (ms < 1)      return SkStringPrintf("%.3gus", ms*1e+3);
#else
    if (ms < 1)      return SkStringPrintf("%.3gµs", ms*1e+3);
#endif
    return SkStringPrintf("%.3gms", ms);
}
