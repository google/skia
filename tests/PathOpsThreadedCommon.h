/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PathOpsThreadedCommon_DEFINED
#define PathOpsThreadedCommon_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkTDArray.h"

#include <string>

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
    std::string fPathStr;
    const char* fKey;
    char fSerialNo[256];
    skiatest::Reporter* fReporter;
    SkBitmap* fBitmap;

    void outputProgress(const char* pathStr, SkPath::FillType);
    void outputProgress(const char* pathStr, SkPathOp);
};

class PathOpsThreadedTestRunner {
public:
    PathOpsThreadedTestRunner(skiatest::Reporter* reporter) : fReporter(reporter) {}

    ~PathOpsThreadedTestRunner();

    void render();

public:
    SkTDArray<PathOpsThreadedRunnable*> fRunnables;
    skiatest::Reporter* fReporter;
};

class PathOpsThreadedRunnable {
public:
    PathOpsThreadedRunnable(void (*testFun)(PathOpsThreadState*), int a, int b, int c, int d,
            PathOpsThreadedTestRunner* runner) {
        fState.fA = (a & 0xFF);
        fState.fB = (b & 0xFF);
        fState.fC = (c & 0xFF);
        fState.fD = (d & 0xFF);
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

    void operator()() {
        SkBitmap bitmap;
        fState.fBitmap = &bitmap;
        (*fTestFun)(&fState);
    }

private:
    PathOpsThreadState fState;
    void (*fTestFun)(PathOpsThreadState*);
};

#endif
