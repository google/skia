#ifndef SkBenchTimer_DEFINED
#define SkBenchTimer_DEFINED

class BenchSysTimer;
class BenchGpuTimer;

/**
 * SysTimers and GpuTimers are implemented orthogonally.
 * This class combines a SysTimer and a GpuTimer into one single,
 * platform specific, Timer with a simple interface.
 */
class BenchTimer {
public:
    BenchTimer();
    ~BenchTimer();
    void start();
    void end();
    double fCpu;
    double fWall;
    double fGpu;
    
private:
    BenchSysTimer *fSysTimer;
    BenchGpuTimer *fGpuTimer;
};

#endif
