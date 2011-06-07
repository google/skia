#ifndef SkBenchGpuTimer_DEFINED
#define SkBenchGpuTimer_DEFINED

class BenchGpuTimer {
public:
    BenchGpuTimer();
    ~BenchGpuTimer();
    void startGpu();
    double endGpu();
};

#endif
