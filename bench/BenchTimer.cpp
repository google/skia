
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "BenchTimer.h"
#if defined(SK_BUILD_FOR_WIN32)
    #include "BenchSysTimer_windows.h"
#elif defined(SK_BUILD_FOR_MAC)
    #include "BenchSysTimer_mach.h"
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)
    #include "BenchSysTimer_posix.h"
#else
    #include "BenchSysTimer_c.h"
#endif

#include "BenchGpuTimer_gl.h"

BenchTimer::BenchTimer(SkGLContext* gl)
        : fCpu(-1.0)
        , fWall(-1.0)
        , fGpu(-1.0)
{
    fSysTimer = new BenchSysTimer();
    if (gl) {
        fGpuTimer = new BenchGpuTimer(gl);
    } else {
        fGpuTimer = NULL;
    }
}

BenchTimer::~BenchTimer() {
    delete fSysTimer;
    delete fGpuTimer;
}

void BenchTimer::start() {
    fSysTimer->startWall();
    if (fGpuTimer) {
        fGpuTimer->startGpu();
    }
    fSysTimer->startCpu();
}

void BenchTimer::end() {
    fCpu = fSysTimer->endCpu();
    //It is important to stop the cpu clocks first,
    //as the following will cpu wait for the gpu to finish.
    if (fGpuTimer) {
        fGpu = fGpuTimer->endGpu();
    }
    fWall = fSysTimer->endWall();
}
