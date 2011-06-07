#include "BenchTimer.h"
#if defined(SK_BUILD_FOR_WIN32)
    #include "BenchSysTimer_windows.h"
#elif defined(SK_BUILD_FOR_MAC)
    #include "BenchSysTimer_mach.h"
#elif defined(SK_BUILD_FOR_UNIX)
    #include "BenchSysTimer_posix.h"
#else
    #include "BenchSysTimer_c.h"
#endif

#if defined(SK_MESA) || \
    defined(SK_BUILD_FOR_WIN32) || \
    defined(SK_BUILD_FOR_MAC) || \
    defined(SK_BUILD_FOR_UNIX)
    #include "BenchGpuTimer_gl.h"

#else
    #include "BenchGpuTimer_none.h"
#endif

BenchTimer::BenchTimer()
        : fCpu(-1.0)
        , fWall(-1.0)
        , fGpu(-1.0)
{
    this->fSysTimer = new BenchSysTimer();
    this->fGpuTimer = new BenchGpuTimer();
}

BenchTimer::~BenchTimer() {
    delete this->fSysTimer;
    delete this->fGpuTimer;
}

void BenchTimer::start() {
    this->fSysTimer->startWall();
    this->fGpuTimer->startGpu();
    this->fSysTimer->startCpu();
}

void BenchTimer::end() {
    this->fCpu = this->fSysTimer->endCpu();
    //It is important to stop the cpu clocks first,
    //as the following will cpu wait for the gpu to finish.
    this->fGpu = this->fGpuTimer->endGpu();
    this->fWall = this->fSysTimer->endWall();
}
