#include "BenchGpuTimer_gl.h"
#include <string.h>

//GL
#define BENCH_GL_FUNCTION_TYPE
#if defined(SK_MESA)
    #include <GL/osmesa.h>
    #define SK_BENCH_CONTEXT_CHECK (NULL != OSMesaGetCurrentContext())
    
    #define SK_GL_GET_PROC(F) gBenchGL.f ## F = (BenchGL ## F ## Proc) \
            OSMesaGetProcAddress("gl" #F);
    #define SK_GL_GET_PROC_SUFFIX(F, S) gBenchGL.f ## F = (BenchGL##F##Proc)\
            OSMesaGetProcAddress("gl" #F #S);

#elif defined(SK_BUILD_FOR_WIN32)
    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
    #include <GL/GL.h>
    #define SK_BENCH_CONTEXT_CHECK (NULL != wglGetCurrentContext())
    
    #undef BENCH_GL_FUNCTION_TYPE
    #define BENCH_GL_FUNCTION_TYPE __stdcall

    #define SK_GL_GET_PROC(F) gBenchGL.f ## F = (BenchGL ## F ## Proc) \
            wglGetProcAddress("gl" #F);
    #define SK_GL_GET_PROC_SUFFIX(F, S) gBenchGL.f ## F = (BenchGL##F##Proc)\
            wglGetProcAddress("gl" #F #S);
    
#elif defined(SK_BUILD_FOR_MAC)
    #include <OpenGL/gl.h>
    #include <OpenGL/CGLCurrent.h>
    #define SK_BENCH_CONTEXT_CHECK (NULL != CGLGetCurrentContext())
    
#elif defined(SK_BUILD_FOR_UNIX)
    #include <GL/gl.h>
    #include <GL/glx.h>
    #define SK_BENCH_CONTEXT_CHECK (NULL != glXGetCurrentContext())
    
    #define SK_GL_GET_PROC(F) gBenchGL.f ## F = (BenchGL ## F ## Proc) \
            glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("gl" #F));
    #define SK_GL_GET_PROC_SUFFIX(F, S) gBenchGL.f ## F = (BenchGL##F##Proc)\
            glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("gl" #F #S));
#else
    #error unsupported platform
#endif

#define BenchGL_TIME_ELAPSED 0x88BF
#define BenchGL_QUERY_RESULT 0x8866
#define BenchGL_QUERY_RESULT_AVAILABLE 0x8867

#if defined(SK_BUILD_FOR_WIN32)
typedef UINT64 BenchGLuint64;
#else
#include <stdint.h>
typedef uint64_t BenchGLuint64;
#endif

typedef void (BENCH_GL_FUNCTION_TYPE *BenchGLGenQueriesProc) (GLsizei n, GLuint *ids);
typedef void (BENCH_GL_FUNCTION_TYPE *BenchGLBeginQueryProc) (GLenum target, GLuint id);
typedef void (BENCH_GL_FUNCTION_TYPE *BenchGLEndQueryProc) (GLenum target);
typedef void (BENCH_GL_FUNCTION_TYPE *BenchGLDeleteQueriesProc) (GLsizei n, const GLuint *ids);
typedef void (BENCH_GL_FUNCTION_TYPE *BenchGLGetQueryObjectivProc) (GLuint id, GLenum pname, GLint *params);
typedef void (BENCH_GL_FUNCTION_TYPE *BenchGLGetQueryObjectui64vProc) (GLuint id, GLenum pname, BenchGLuint64 *params);

struct BenchGLInterface {
    bool fHasTimer;
    BenchGLGenQueriesProc fGenQueries;
    BenchGLBeginQueryProc fBeginQuery;
    BenchGLEndQueryProc fEndQuery;
    BenchGLDeleteQueriesProc fDeleteQueries;
    BenchGLGetQueryObjectivProc fGetQueryObjectiv;
    BenchGLGetQueryObjectui64vProc fGetQueryObjectui64v;
};

static bool BenchGLCheckExtension(const char* ext,
                                  const char* extensionString) {
    int extLength = strlen(ext);

    while (true) {
        int n = strcspn(extensionString, " ");
        if (n == extLength && 0 == strncmp(ext, extensionString, n)) {
            return true;
        }
        if (0 == extensionString[n]) {
            return false;
        }
        extensionString += n+1;
    }

    return false;
}

static BenchGLInterface gBenchGL;
static bool gBenchGLInterfaceInit = false;

static void BenchGLSetDefaultGLInterface() {
    gBenchGL.fHasTimer = false;
    if (gBenchGLInterfaceInit || !SK_BENCH_CONTEXT_CHECK) return;

    const char* glExts =
        reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    const GLboolean ext =
        BenchGLCheckExtension("GL_EXT_timer_query", glExts);
    const GLboolean arb =
        BenchGLCheckExtension("GL_ARB_timer_query", glExts);
    if (ext || arb) {
#if defined(SK_BUILD_FOR_MAC)
        #if GL_EXT_timer_query || GL_ARB_timer_query
        gBenchGL.fHasTimer = true;
        gBenchGL.fGenQueries = glGenQueries;
        gBenchGL.fBeginQuery = glBeginQuery;
        gBenchGL.fEndQuery = glEndQuery;
        gBenchGL.fDeleteQueries = glDeleteQueries;
        gBenchGL.fGetQueryObjectiv = glGetQueryObjectiv;
        #endif
        #if GL_ARB_timer_query
        gBenchGL.fGetQueryObjectui64v = glGetQueryObjectui64v;
        #elif GL_EXT_timer_query
        gBenchGL.fGetQueryObjectui64v = glGetQueryObjectui64vEXT;
        #endif
#else
        gBenchGL.fHasTimer = true;
        SK_GL_GET_PROC(GenQueries)
        SK_GL_GET_PROC(BeginQuery)
        SK_GL_GET_PROC(EndQuery)
        SK_GL_GET_PROC(DeleteQueries)
        
        SK_GL_GET_PROC(GetQueryObjectiv)
        if (arb) {
            SK_GL_GET_PROC(GetQueryObjectui64v)
        } else {
            SK_GL_GET_PROC_SUFFIX(GetQueryObjectui64v, EXT)
        }
#endif
    }
    gBenchGLInterfaceInit = true;
}

BenchGpuTimer::BenchGpuTimer() {
    BenchGLSetDefaultGLInterface();
    if (gBenchGL.fHasTimer) {
        gBenchGL.fGenQueries(1, &this->fQuery);
    }
}

BenchGpuTimer::~BenchGpuTimer() {
    if (gBenchGL.fHasTimer) {
        gBenchGL.fDeleteQueries(1, &this->fQuery);
    }
}

void BenchGpuTimer::startGpu() {
    if (!gBenchGL.fHasTimer) return;
    
    this->fStarted = true;
    gBenchGL.fBeginQuery(BenchGL_TIME_ELAPSED, this->fQuery);
}

/**
 * It is important to stop the cpu clocks first,
 * as this will cpu wait for the gpu to finish.
 */
double BenchGpuTimer::endGpu() {
    if (!gBenchGL.fHasTimer) return 0;
    
    this->fStarted = false;
    gBenchGL.fEndQuery(BenchGL_TIME_ELAPSED);
    
    GLint available = 0;
    while (!available) {
        gBenchGL.fGetQueryObjectiv(this->fQuery
                                 , BenchGL_QUERY_RESULT_AVAILABLE
                                 , &available);
    }
    BenchGLuint64 totalGPUTimeElapsed = 0;
    gBenchGL.fGetQueryObjectui64v(this->fQuery
                                , BenchGL_QUERY_RESULT
                                , &totalGPUTimeElapsed);
    
    return totalGPUTimeElapsed / 1000000.0;
}
