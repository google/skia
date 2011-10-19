
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
 * This class combines a SysTimer and a GpuTimer into one single,
 * platform specific, Timer with a simple interface.
 */
class BenchTimer {
public:
    BenchTimer(SkGLContext* gl = NULL);
    ~BenchTimer();
    void start();
    void end();
    double fCpu;
    double fWall;
    double fGpu;
    
private:
    BenchSysTimer *fSysTimer;
    BenchGpuTimer *fGpuTimer;
};

#endif
