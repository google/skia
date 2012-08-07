

#include "ShapeOps.h"
#include "SkBitmap.h"
#include <pthread.h>

class SkCanvas;

//extern int comparePaths(const SkPath& one, const SkPath& two);
extern int comparePaths(const SkPath& one, const SkPath& two, SkBitmap& bitmap,
        SkCanvas* canvas);
extern void comparePathsTiny(const SkPath& one, const SkPath& two);
extern bool drawAsciiPaths(const SkPath& one, const SkPath& two,
        bool drawPaths);
extern void showPath(const SkPath& path, const char* str = NULL);
extern bool testSimplify(const SkPath& path, bool fill, SkPath& out,
        SkBitmap& bitmap, SkCanvas* canvas = 0);
extern bool testSimplifyx(const SkPath& path, SkPath& out,
        SkBitmap& bitmap, SkCanvas* canvas = 0);
extern bool testSimplifyx(const SkPath& path);

struct State4 {
    State4();
    static pthread_mutex_t addQueue;
    static pthread_cond_t checkQueue;
    pthread_cond_t initialized;
    static State4* queue;
    State4* next;
    pthread_t threadID;
    int index;
    bool done;
    bool last;
    int a;
    int b;
    int c;
    int d;
    int testsRun;
    char filename[256];
    
    SkCanvas* canvas;
    SkBitmap bitmap;
    bool abcIsATriangle;
};

void createThread(State4* statePtr, void* (*test)(void* ));
void waitForCompletion(State4 threadState[], int& threadIndex);
