/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PathOpsExtendedTest_DEFINED
#define PathOpsExtendedTest_DEFINED

#include "Test.h"
#include "SkPathOpsTypes.h"
#include "SkBitmap.h"
#include "SkPath.h"
#include "SkStream.h"

#ifdef SK_BUILD_FOR_WIN
#define HARD_CODE_PTHREAD 0
#else
#define HARD_CODE_PTHREAD 1
#endif

#if HARD_CODE_PTHREAD
#include <pthread.h>
#else
#include "SkThread.h"
#include "../../src/utils/SkThreadUtils.h"
#endif

#if HARD_CODE_PTHREAD
typedef void* (*ThreadFunction)(void*);
#define THREAD_TYPE void*
#define THREAD_RETURN return NULL;
#else
typedef void (*ThreadFunction)(void*);
#define THREAD_TYPE void
#define THREAD_RETURN
#endif

struct State4;

struct TestDesc {
    void (*fun)(skiatest::Reporter*);
    const char* str;
};

extern bool gShowTestProgress;
extern bool gAllowExtendedTest;

//extern int comparePaths(const SkPath& one, const SkPath& two);
extern int comparePaths(const SkPath& one, const SkPath& two, SkBitmap& bitmap);
extern bool drawAsciiPaths(const SkPath& one, const SkPath& two, bool drawPaths);
extern void showOp(const SkPathOp op);
extern void showPath(const SkPath& path, const char* str);
extern void showPath(const SkPath& path);
extern void showPathData(const SkPath& path);
extern bool testPathOp(skiatest::Reporter* reporter, const SkPath& a, const SkPath& b,
                        const SkPathOp );
extern bool testSimplify(SkPath& path, bool useXor, SkPath& out,
                         State4& state, const char* pathStr);
extern bool testSimplify(skiatest::Reporter* reporter, const SkPath& path);

struct State4 {
    State4();
#if HARD_CODE_PTHREAD
    static pthread_mutex_t addQueue;
    static pthread_cond_t checkQueue;
    pthread_cond_t initialized;
    pthread_t threadID;
#else
    SkThread* thread;
#endif
    static State4* queue;
    int index;
    bool done;
    bool last;
    int a;
    int b;
    int c;
    int d;  // sometimes 1 if abc_is_a_triangle
    int testsRun;
    char filename[256];
    skiatest::Reporter* reporter;
    SkBitmap bitmap;
    mutable SkDynamicMemoryWStream ramStream;
};

void createThread(State4* statePtr, void* (*test)(void* ));
int dispatchTest4(ThreadFunction testFun, int a, int b, int c, int d);
void initializeTests(skiatest::Reporter* reporter, const char* testName, size_t testNameSize);
void outputProgress(const State4& state, const char* pathStr, SkPath::FillType );
void outputProgress(const State4& state, const char* pathStr, SkPathOp op);
void outputToStream(const State4& state, const char* pathStr, const char* pathPrefix,
                    const char* nameSuffix,
                    const char* testFunction, SkWStream& outFile);
bool runNextTestSet(State4& state);
int waitForCompletion();

void RunTestSet(skiatest::Reporter* reporter, TestDesc tests[], size_t count,
                void (*firstTest)(skiatest::Reporter* ),
                void (*stopTest)(skiatest::Reporter* ), bool reverse);

#endif
