#include "BenchSysTimer_c.h"

//Time
#include <time.h>

void BenchSysTimer::startWall() {
    this->fStartWall = time();
}
void BenchSysTimer::startCpu() {
    this->fStartCpu = clock();
}

double BenchSysTimer::endCpu() {
    clock_t end_cpu = clock();
    this->fCpu = (end_cpu - this->fStartCpu) * CLOCKS_PER_SEC / 1000.0;
}
double BenchSysTimer::endWall() {
    time_t end_wall = time();
    this->fWall = difftime(end_wall, this->fstartWall) / 1000.0;
}
