
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBenchSysTimer_DEFINED
#define SkBenchSysTimer_DEFINED

//Time
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

class BenchSysTimer {
public:
    void startWall();
    void startCpu();
    double endCpu();
    double endWall();
private:
    ULONGLONG fStartCpu;
    LARGE_INTEGER fStartWall;
};

#endif
