
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

#if SK_SUPPORT_GPU
#include "BenchGpuTimer_gl.h"
#endif

BenchTimer::BenchTimer(SkGLContext* gl)
        : fCpu(-1.0)
        , fWall(-1.0)
        , fGpu(-1.0)
{
    fSysTimer = new BenchSysTimer();
#if SK_SUPPORT_GPU
    if (gl) {
        fGpuTimer = new BenchGpuTimer(gl);
    } else {
        fGpuTimer = NULL;
    }
#endif
}

BenchTimer::~BenchTimer() {
    delete fSysTimer;
#if SK_SUPPORT_GPU
    delete fGpuTimer;
#endif
}

void BenchTimer::start() {
    fSysTimer->startWall();
#if SK_SUPPORT_GPU
    if (fGpuTimer) {
        fGpuTimer->startGpu();
    }
#endif
    fSysTimer->startCpu();
}

void BenchTimer::end() {
    fCpu = fSysTimer->endCpu();
#if SK_SUPPORT_GPU
    //It is important to stop the cpu clocks first,
    //as the following will cpu wait for the gpu to finish.
    if (fGpuTimer) {
        fGpu = fGpuTimer->endGpu();
    }
#endif
    fWall = fSysTimer->endWall();
}
