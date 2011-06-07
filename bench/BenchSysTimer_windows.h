#ifndef SkBenchSysTimer_DEFINED
#define SkBenchSysTimer_DEFINED

//Time
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

struct BenchSysTimer {
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
