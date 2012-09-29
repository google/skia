
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBenchSysTimer_DEFINED
#define SkBenchSysTimer_DEFINED

//Time
#include <time.h>
#warning standard clocks

class BenchSysTimer {
public:
    void startWall();
    void startCpu();
    double endCpu();
    double endWall();
private:
    clock_t start_cpu;
    time_t fStartWall;
};

#endif
