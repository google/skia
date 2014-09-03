/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PathOpsThreadedCommon_DEFINED
#define PathOpsThreadedCommon_DEFINED

#include "SkGraphics.h"
#include "SkRunnable.h"
#include "SkTDArray.h"

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
    char fSerialNo[256];
    skiatest::Reporter* fReporter;
    SkBitmap* fBitmap;
};

class PathOpsThreadedTestRunner {
public:
    PathOpsThreadedTestRunner(skiatest::Reporter* reporter, int threadCount)
        : fNumThreads(threadCount)
        , fReporter(reporter) {
    }

    ~PathOpsThreadedTestRunner();

    void render();

public:
    int fNumThreads;
    SkTDArray<PathOpsThreadedRunnable*> fRunnables;
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
    }

    PathOpsThreadedRunnable(void (*testFun)(PathOpsThreadState*), const char* str,
            PathOpsThreadedTestRunner* runner) {
        SkASSERT(strlen(str) < sizeof(fState.fSerialNo) - 1);
        strcpy(fState.fSerialNo, str);
        fState.fReporter = runner->fReporter;
        fTestFun = testFun;
    }

    PathOpsThreadedRunnable(void (*testFun)(PathOpsThreadState*), int dirNo, const char* str,
            PathOpsThreadedTestRunner* runner) {
        SkASSERT(strlen(str) < sizeof(fState.fSerialNo) - 1);
        fState.fA = dirNo;
        strcpy(fState.fSerialNo, str);
        fState.fReporter = runner->fReporter;
        fTestFun = testFun;
    }

    virtual void run() SK_OVERRIDE {
        SkBitmap bitmap;
        fState.fBitmap = &bitmap;
        char pathStr[PATH_STR_SIZE];
        fState.fPathStr = pathStr;
        SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);
        (*fTestFun)(&fState);
    }

private:
    PathOpsThreadState fState;
    void (*fTestFun)(PathOpsThreadState*);
};

#endif
