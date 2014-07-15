/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SysTimer_windows.h"

static ULONGLONG win_cpu_time() {
    FILETIME createTime;
    FILETIME exitTime;
    FILETIME usrTime;
    FILETIME sysTime;
    if (0 == GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &sysTime, &usrTime)) {
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

void SysTimer::startWall() {
    if (0 == ::QueryPerformanceCounter(&fStartWall)) {
        fStartWall.QuadPart = 0;
    }
}
void SysTimer::startCpu() {
    fStartCpu = win_cpu_time();
}

double SysTimer::endCpu() {
    ULONGLONG end_cpu = win_cpu_time();
    return static_cast<double>(end_cpu - fStartCpu) / 10000.0L;
}
double SysTimer::endWall() {
    LARGE_INTEGER end_wall;
    if (0 == ::QueryPerformanceCounter(&end_wall)) {
        end_wall.QuadPart = 0;
    }

    LARGE_INTEGER ticks_elapsed;
    ticks_elapsed.QuadPart = end_wall.QuadPart - fStartWall.QuadPart;

    LARGE_INTEGER frequency;
    if (0 == ::QueryPerformanceFrequency(&frequency)) {
        return 0.0L;
    } else {
        return static_cast<double>(ticks_elapsed.QuadPart)
             / static_cast<double>(frequency.QuadPart)
             * 1000.0L;
    }
}
