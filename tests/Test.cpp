
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

SK_DEFINE_INST_COUNT(skiatest::Reporter)

using namespace skiatest;

Reporter::Reporter()
    : fTestCount(0) {
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

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
GrContextFactory gGrContextFactory;
#endif

GrContextFactory* GpuTest::GetGrContextFactory() {
#if SK_SUPPORT_GPU
    return &gGrContextFactory;
#else
    return NULL;
#endif
}

void GpuTest::DestroyContexts() {
#if SK_SUPPORT_GPU
    gGrContextFactory.destroyContexts();
#endif
}
