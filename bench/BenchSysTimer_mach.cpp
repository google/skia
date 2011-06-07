#include "BenchSysTimer_mach.h"

//Time
#include <mach/mach.h>
#include <mach/mach_time.h>

static time_value_t macCpuTime() {
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
                 &thread_info_count))
    {
        time_value_t none = {0, 0};
        return none;
    }
    
    time_value_add(&thread_info_data.user_time, &thread_info_data.system_time)
    return thread_info_data.user_time;
}

static double intervalInMSec(const time_value_t start_clock
                           , const time_value_t end_clock)
{
    double duration_clock;
    if ((end_clock.microseconds - start_clock.microseconds) < 0) {
        duration_clock = (end_clock.seconds - start_clock.seconds-1)*1000;
        duration_clock += (1000000
                           + end_clock.microseconds
                           - start_clock.microseconds) / 1000.0;
    } else {
        duration_clock = (end_clock.seconds - start_clock.seconds)*1000;
        duration_clock += (end_clock.microseconds - start_clock.microseconds)
                           / 1000.0;
    }
    return duration_clock;
}

void BenchSysTimer::startWall() {
    this->fStartWall = mach_absolute_time();
}
void BenchSysTimer::startCpu() {
    this->fStartCpu = macCpuTime();
}

double BenchSysTimer::endCpu() {
    time_value_t end_cpu = macCpuTime();
    return intervalInMSec(this->fStartCpu, end_cpu);
}
double BenchSysTimer::endWall() {
    uint64_t end_wall = mach_absolute_time();
    
    uint64_t elapsed = end_wall - this->fStartWall;
    mach_timebase_info_data_t sTimebaseInfo;
    if (KERN_SUCCESS != mach_timebase_info(&sTimebaseInfo)) {
        return 0;
    } else {
        uint64_t elapsedNano = elapsed * sTimebaseInfo.numer
                               / sTimebaseInfo.denom;
        return elapsedNano / 1000000;
    }
}
