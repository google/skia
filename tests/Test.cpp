/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkCommandLineFlags.h"
#include "SkError.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTime.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "gl/SkGLContext.h"
#else
class GrContext;
#endif

DEFINE_string2(tmpDir, t, NULL, "tmp directory for tests to use.");

using namespace skiatest;

Reporter::Reporter() : fTestCount(0) {
}

void Reporter::startTest(Test* test) {
    this->onStart(test);
}

void Reporter::reportFailed(const skiatest::Failure& failure) {
    this->onReportFailed(failure);
}

void Reporter::endTest(Test* test) {
    this->onEnd(test);
}

///////////////////////////////////////////////////////////////////////////////

Test::Test() : fReporter(NULL), fPassed(true) {}

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

class LocalReporter : public Reporter {
public:
    explicit LocalReporter(Reporter* reporterToMimic) : fReporter(reporterToMimic) {}

    int numFailures() const { return fFailures.count(); }
    const skiatest::Failure& failure(int i) const { return fFailures[i]; }

protected:
    virtual void onReportFailed(const Failure& failure) SK_OVERRIDE {
        fFailures.push_back(failure);
    }

    // Proxy down to fReporter.  We assume these calls are threadsafe.
    virtual bool allowExtendedTest() const SK_OVERRIDE {
        return fReporter->allowExtendedTest();
    }

    virtual void bumpTestCount() SK_OVERRIDE {
        fReporter->bumpTestCount();
    }

    virtual bool verbose() const SK_OVERRIDE {
        return fReporter->verbose();
    }

private:
    Reporter* fReporter;  // Unowned.
    SkTArray<skiatest::Failure> fFailures;
};

void Test::run() {
    // Clear the Skia error callback before running any test, to ensure that tests
    // don't have unintended side effects when running more than one.
    SkSetErrorCallback( NULL, NULL );

    // Tell (likely shared) fReporter that this test has started.
    fReporter->startTest(this);

    const SkMSec start = SkTime::GetMSecs();
    // Run the test into a LocalReporter so we know if it's passed or failed without interference
    // from other tests that might share fReporter.
    LocalReporter local(fReporter);
    this->onRun(&local);
    fPassed = local.numFailures() == 0;
    fElapsed = SkTime::GetMSecs() - start;

    // Now tell fReporter about any failures and wrap up.
    for (int i = 0; i < local.numFailures(); i++) {
      fReporter->reportFailed(local.failure(i));
    }
    fReporter->endTest(this);

}

SkString Test::GetTmpDir() {
    const char* tmpDir = FLAGS_tmpDir.isEmpty() ? NULL : FLAGS_tmpDir[0];
    return SkString(tmpDir);
}
