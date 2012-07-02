

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

    int a;
    int b;
    int c;
    int d;
    pthread_t threadID;
    SkCanvas* canvas;
    SkBitmap bitmap;
    bool abcIsATriangle;
};

void createThread(State4* statePtr, void* (*test)(void* ));
void waitForCompletion(State4 threadState[], int& threadIndex);
