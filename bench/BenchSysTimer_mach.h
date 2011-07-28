
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBenchSysTimer_DEFINED
#define SkBenchSysTimer_DEFINED

//Time
#include <mach/mach.h>
#include <mach/mach_time.h>

class BenchSysTimer {
public:
    void startWall();
    void startCpu();
    double endCpu();
    double endWall();
private:
    time_value_t fStartCpu;
    uint64_t fStartWall;
};

#endif
