
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "BenchSysTimer_windows.h"

//Time
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

static ULONGLONG winCpuTime() {
    FILETIME createTime;
    FILETIME exitTime;
    FILETIME usrTime;
    FILETIME sysTime;
    if (0 == GetProcessTimes(GetCurrentProcess()
                           , &createTime, &exitTime
                           , &sysTime, &usrTime))
    {
        return 0;
    }
    ULARGE_INTEGER start_cpu_sys;
    ULARGE_INTEGER start_cpu_usr;
    start_cpu_sys.LowPart  = sysTime.dwLowDateTime;
    start_cpu_sys.HighPart = sysTime.dwHighDateTime;
    start_cpu_usr.LowPart  = usrTime.dwLowDateTime;
    start_cpu_usr.HighPart = usrTime.dwHighDateTime;
    return start_cpu_sys.QuadPart + start_cpu_usr.QuadPart;
}

void BenchSysTimer::startWall() {
    if (0 == ::QueryPerformanceCounter(&this->fStartWall)) {
        this->fStartWall.QuadPart = 0;
    }
}
void BenchSysTimer::startCpu() {
    this->fStartCpu = winCpuTime();
}

double BenchSysTimer::endCpu() {
    ULONGLONG end_cpu = winCpuTime();
    return static_cast<double>((end_cpu - this->fStartCpu)) / 10000.0L;
}
double BenchSysTimer::endWall() {
    LARGE_INTEGER end_wall;
    if (0 == ::QueryPerformanceCounter(&end_wall)) {
        end_wall.QuadPart = 0;
    }

    LARGE_INTEGER ticks_elapsed;
    ticks_elapsed.QuadPart = end_wall.QuadPart - this->fStartWall.QuadPart;

    LARGE_INTEGER frequency;
    if (0 == ::QueryPerformanceFrequency(&frequency)) {
        return 0.0L;
    } else {
        return static_cast<double>(ticks_elapsed.QuadPart)
             / static_cast<double>(frequency.QuadPart)
             * 1000.0L;
    }
}
