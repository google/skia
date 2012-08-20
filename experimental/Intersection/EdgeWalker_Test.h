

#include "ShapeOps.h"
#include "SkBitmap.h"
#include <pthread.h>

class SkCanvas;
struct State4;

//extern int comparePaths(const SkPath& one, const SkPath& two);
extern int comparePaths(const SkPath& one, const SkPath& two, SkBitmap& bitmap,
        SkCanvas* canvas);
extern void comparePathsTiny(const SkPath& one, const SkPath& two);
extern bool drawAsciiPaths(const SkPath& one, const SkPath& two,
        bool drawPaths);
extern void showPath(const SkPath& path, const char* str = NULL);
extern bool testSimplify(const SkPath& path, bool fill, SkPath& out,
        SkBitmap& bitmap, SkCanvas* canvas = 0);
extern bool testSimplifyx(SkPath& path, bool useXor, SkPath& out,
        State4& state, const char* pathStr);
extern bool testSimplifyx(const SkPath& path);

struct State4 {
    State4();
    static pthread_mutex_t addQueue;
    static pthread_cond_t checkQueue;
    pthread_cond_t initialized;
    static State4* queue;
    pthread_t threadID;
    int index;
    bool done;
    bool last;
    int a;
    int b;
    int c;
    int d; // sometimes 1 if abc_is_a_triangle
    int testsRun;
    char filename[256];
    
    SkCanvas* canvas;
    SkBitmap bitmap;
};

void createThread(State4* statePtr, void* (*test)(void* ));
int dispatchTest4(void* (*testFun)(void* ), int a, int b, int c, int d);
void initializeTests(const char* testName, size_t testNameSize);
void outputProgress(const State4& state, const char* pathStr, SkPath::FillType );
void outputToStream(const State4& state, const char* pathStr, SkPath::FillType, SkWStream& outFile);
bool runNextTestSet(State4& state);
int waitForCompletion();
