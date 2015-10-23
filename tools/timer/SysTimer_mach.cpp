/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SysTimer_mach.h"

static time_value_t mac_cpu_time() {
    mach_port_t task = mach_task_self();
    if (task == MACH_PORT_NULL) {
        time_value_t none = {0, 0};
        return none;
    }

    task_thread_times_info thread_info_data;
    mach_msg_type_number_t thread_info_count = TASK_THREAD_TIMES_INFO_COUNT;
    if (KERN_SUCCESS != task_info(task,
                                  TASK_THREAD_TIMES_INFO,
                                  reinterpret_cast<task_info_t>(&thread_info_data),
                                  &thread_info_count)) {
        time_value_t none = {0, 0};
        return none;
    }

    time_value_add(&thread_info_data.user_time, &thread_info_data.system_time)
    return thread_info_data.user_time;
}

static double interval_in_ms(time_value_t start_clock, time_value_t end_clock) {
    double duration_clock;
    if ((end_clock.microseconds - start_clock.microseconds) < 0) {
        duration_clock = (end_clock.seconds - start_clock.seconds-1) * 1000;
        duration_clock += (1000000 + end_clock.microseconds - start_clock.microseconds) / 1000.0;
    } else {
        duration_clock = (end_clock.seconds - start_clock.seconds) * 1000;
        duration_clock += (end_clock.microseconds - start_clock.microseconds) / 1000.0;
    }
    return duration_clock;
}

void SysTimer::startWall() {
    fStartWall = mach_absolute_time();
}

void SysTimer::startCpu() {
    fStartCpu = mac_cpu_time();
}

double SysTimer::endCpu() {
    time_value_t end_cpu = mac_cpu_time();
    return interval_in_ms(fStartCpu, end_cpu);
}

double SysTimer::endWall() {
    uint64_t end_wall = mach_absolute_time();

    uint64_t elapsed = end_wall - fStartWall;
    mach_timebase_info_data_t sTimebaseInfo;
    if (KERN_SUCCESS != mach_timebase_info(&sTimebaseInfo)) {
        return 0;
    } else {
        uint64_t elapsedNano = elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
        return elapsedNano / 1000000.0;
    }
}
