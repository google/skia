
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"

#include "SkTLazy.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "gl/SkNativeGLContext.h"
#else
class GrContext;
#endif

using namespace skiatest;

Reporter::Reporter() {
    this->resetReporting();
}

void Reporter::resetReporting() {
    fCurrTest = NULL;
    fTestCount = 0;
    sk_bzero(fResultCount, sizeof(fResultCount));
}

void Reporter::startTest(Test* test) {
    SkASSERT(NULL == fCurrTest);
    fCurrTest = test;
    this->onStart(test);
    fTestCount += 1;
    fCurrTestSuccess = true;    // we're optimistic
}

void Reporter::report(const char desc[], Result result) {
    if (NULL == desc) {
        desc = "<no description>";
    }
    this->onReport(desc, result);
    fResultCount[result] += 1;
    if (kFailed == result) {
        fCurrTestSuccess = false;
    }
}

void Reporter::endTest(Test* test) {
    SkASSERT(test == fCurrTest);
    this->onEnd(test);
    fCurrTest = NULL;
}

///////////////////////////////////////////////////////////////////////////////

Test::Test() : fReporter(NULL) {}

Test::~Test() {
    SkSafeUnref(fReporter);
}

void Test::setReporter(Reporter* r) {
    SkRefCnt_SafeAssign(fReporter, r);
}

const char* Test::getName() {
    if (fName.size() == 0) {
        this->onGetName(&fName);
    }
    return fName.c_str();
}

bool Test::run() {
    fReporter->startTest(this);
    this->onRun(fReporter);
    fReporter->endTest(this);
    return fReporter->getCurrSuccess();
}

///////////////////////////////////////////////////////////////////////////////


GrContext* GpuTest::GetContext() {
#if SK_SUPPORT_GPU
    // preserve this order, we want gGrContext destroyed after gEGLContext
    static SkTLazy<SkNativeGLContext> gGLContext;
    static SkAutoTUnref<GrContext> gGrContext;

    if (NULL == gGrContext.get()) {
        gGLContext.init();
        if (gGLContext.get()->init(800, 600)) {
            GrPlatform3DContext ctx = reinterpret_cast<GrPlatform3DContext>(gGLContext.get()->gl());
            gGrContext.reset(GrContext::Create(kOpenGL_Shaders_GrEngine, ctx));
        }
    }
    if (gGLContext.get()) {
        gGLContext.get()->makeCurrent();
    }
    return gGrContext.get();
#else
    return NULL;
#endif
}

