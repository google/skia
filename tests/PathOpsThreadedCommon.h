/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PathOpsThreadedCommon_DEFINED
#define PathOpsThreadedCommon_DEFINED

#include "SkCountdown.h"
#include "SkRunnable.h"
#include "SkTDArray.h"
#include "SkThreadPool.h"

#define PATH_STR_SIZE 512

class PathOpsThreadedRunnable;

namespace skiatest {
class Reporter;
}

struct PathOpsThreadState {
    unsigned char fA;
    unsigned char fB;
    unsigned char fC;
    unsigned char fD;
    char* fPathStr;
    const char* fKey;
    char fSerialNo[9];
    skiatest::Reporter* fReporter;
    SkBitmap* fBitmap;
};

class PathOpsThreadedTestRunner {
public:
    PathOpsThreadedTestRunner(skiatest::Reporter* reporter, int threadCount)
        : fNumThreads(threadCount)
        , fThreadPool(threadCount)
        , fCountdown(threadCount)
        , fReporter(reporter) {
    }

    ~PathOpsThreadedTestRunner();

    void render();

public:
    int fNumThreads;
    SkTDArray<PathOpsThreadedRunnable*> fRunnables;
    SkThreadPool fThreadPool;
    SkCountdown fCountdown;
    skiatest::Reporter* fReporter;
};

class PathOpsThreadedRunnable : public SkRunnable {
public:
    PathOpsThreadedRunnable(void (*testFun)(PathOpsThreadState*), int a, int b, int c, int d,
            PathOpsThreadedTestRunner* runner) {
        fState.fA = a;
        fState.fB = b;
        fState.fC = c;
        fState.fD = d;
        fState.fReporter = runner->fReporter;
        fTestFun = testFun;
        fDone = &runner->fCountdown;
    }

    virtual void run() SK_OVERRIDE {
        SkBitmap bitmap;
        fState.fBitmap = &bitmap;
        char pathStr[PATH_STR_SIZE];
        fState.fPathStr = pathStr;
        (*fTestFun)(&fState);
        fDone->run();
    }

private:
    PathOpsThreadState fState;
    void (*fTestFun)(PathOpsThreadState*);
    SkRunnable* fDone;
};

#endif
