
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBenchTimer_DEFINED
#define SkBenchTimer_DEFINED

#include <SkTypes.h>


class BenchSysTimer;
class BenchGpuTimer;

class SkGLContext;

/**
 * SysTimers and GpuTimers are implemented orthogonally.
 * This class combines 2 SysTimers and a GpuTimer into one single,
 * platform specific Timer with a simple interface. The truncated
 * timer doesn't include the time required for the GPU to finish
 * its rendering. It should always be <= the un-truncated system
 * times and (for GPU configurations) can be used to roughly (very
 * roughly) gauge the GPU load/backlog.
 */
class BenchTimer {
public:
    BenchTimer(SkGLContext* gl = NULL);
    ~BenchTimer();
    void start();
    void end();
    void truncatedEnd();
    double fCpu;
    double fWall;
    double fTruncatedCpu;
    double fTruncatedWall;
    double fGpu;

private:
    BenchSysTimer *fSysTimer;
    BenchSysTimer *fTruncatedSysTimer;
#if SK_SUPPORT_GPU
    BenchGpuTimer *fGpuTimer;
#endif
};

#endif
