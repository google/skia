#ifndef SkBenchGpuTimer_DEFINED
#define SkBenchGpuTimer_DEFINED

#if defined(SK_MESA)
    #include <GL/osmesa.h>

#elif defined(SK_BUILD_FOR_WIN32)
    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
    #include <GL/GL.h>
    
#elif defined(SK_BUILD_FOR_MAC)
    #include <OpenGL/gl.h>
    
#elif defined(SK_BUILD_FOR_UNIX)
    #include <GL/gl.h>

#else
    #error unsupported platform
#endif

class BenchGpuTimer {
public:
    BenchGpuTimer();
    ~BenchGpuTimer();
    void startGpu();
    double endGpu();
private:
    GLuint fQuery;
    int fStarted;
};

#endif
