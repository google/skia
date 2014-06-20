/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SysTimer_posix.h"

static double interval_in_ms(timespec start_clock, timespec end_clock)
{
    double duration_clock;
    if ((end_clock.tv_nsec - start_clock.tv_nsec) < 0) {
        duration_clock = (end_clock.tv_sec - start_clock.tv_sec - 1) * 1000;
        duration_clock += (1000000000 + end_clock.tv_nsec - start_clock.tv_nsec) / 1000000.0;
    } else {
        duration_clock = (end_clock.tv_sec - start_clock.tv_sec) * 1000;
        duration_clock += (end_clock.tv_nsec - start_clock.tv_nsec) / 1000000.0;
    }
    return duration_clock;
}

void SysTimer::startWall() {
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &fWall)) {
        timespec none = {0, 0};
        fWall = none;
    }
}
void SysTimer::startCpu() {
    if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &fCpu)) {
        timespec none = {0, 0};
        fCpu = none;
    }
}

double SysTimer::endCpu() {
    timespec end_cpu;
    if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu)) {
        timespec none = {0, 0};
        end_cpu = none;
    }
    return interval_in_ms(fCpu, end_cpu);
}

double SysTimer::endWall() {
    timespec end_wall;
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &end_wall)) {
        timespec none = {0, 0};
        end_wall = none;
    }
    return interval_in_ms(fWall, end_wall);
}
