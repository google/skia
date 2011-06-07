#include "BenchSysTimer_posix.h"

//Time
#include <time.h>

static double intervalInMSec(const timespec start_clock
                           , const timespec end_clock)
{
    double duration_clock;
    if ((end_clock.tv_nsec - start_clock.tv_nsec) < 0) {
        duration_clock = (end_clock.tv_sec - start_clock.tv_sec-1)*1000;
        duration_clock += (1000000000 + end_clock.tv_nsec - start_clock.tv_nsec)
                           / 1000000.0;
    } else {
        duration_clock = (end_clock.tv_sec - start_clock.tv_sec)*1000;
        duration_clock += (end_clock.tv_nsec - start_clock.tv_nsec) / 1000000.0;
    }
    return duration_clock;
}

void BenchSysTimer::startWall() {
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &this->fWall)) {
        timespec none = {0, 0};
        this->fWall = none;
    }
}
void BenchSysTimer::startCpu() {
    if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &this->fCpu)) {
        timespec none = {0, 0};
        this->fCpu = none;
    }
}

double BenchSysTimer::endCpu() {
    timespec end_cpu;
    if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu)) {
        timespec none = {0, 0};
        end_cpu = none;
    }
    return intervalInMSec(this->fCpu, end_cpu);
}

double BenchSysTimer::endWall() {
    timespec end_wall;
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &end_wall)) {
        timespec none = {0, 0};
        end_wall = none;
    }
    return intervalInMSec(this->fWall, end_wall);
}
