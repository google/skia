/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SysTimer_windows.h"

#include <intrin.h>

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

void SysTimer::startCpu() {
    fStartCpu = win_cpu_time();
}

double SysTimer::endCpu() {
    ULONGLONG end_cpu = win_cpu_time();
    return static_cast<double>(end_cpu - fStartCpu) / 10000.0L;
}

// On recent Intel chips (roughly, "has Core or Atom in its name") __rdtsc will always tick
// at the CPU's maximum rate, even while power management clocks the CPU up and down.
// That's great, because it makes measuring wall time super simple.

void SysTimer::startWall() {
    fStartWall = __rdtsc();
}

double SysTimer::endWall() {
    unsigned __int64 end = __rdtsc();

    // This seems to, weirdly, give the CPU frequency in kHz.  That's exactly what we want!
    LARGE_INTEGER freq_khz;
    QueryPerformanceFrequency(&freq_khz);

    return static_cast<double>(end - fStartWall) / static_cast<double>(freq_khz.QuadPart);
}
