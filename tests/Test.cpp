
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"

#include "SkString.h"
#include "SkTArray.h"
#include "SkTime.h"
#include "SkError.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "gl/SkNativeGLContext.h"
#else
class GrContext;
#endif

using namespace skiatest;

Reporter::Reporter() : fTestCount(0) {
}

void Reporter::startTest(Test* test) {
    this->bumpTestCount();
    this->onStart(test);
}

void Reporter::reportFailed(const SkString& desc) {
    this->onReportFailed(desc);
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

namespace {
    class LocalReporter : public Reporter {
    public:
        explicit LocalReporter(Reporter* reporterToMimic) : fReporter(reporterToMimic) {}

        int failure_size() const { return fFailures.count(); }
        const SkString& failure(int i) const { return fFailures[i]; }

    protected:
        void onReportFailed(const SkString& desc) SK_OVERRIDE {
            fFailures.push_back(desc);
        }

        // Proxy down to fReporter.  We assume these calls are threadsafe.
        virtual bool allowExtendedTest() const SK_OVERRIDE {
            return fReporter->allowExtendedTest();
        }

        virtual bool allowThreaded() const SK_OVERRIDE {
            return fReporter->allowThreaded();
        }

        virtual void bumpTestCount() SK_OVERRIDE {
            fReporter->bumpTestCount();
        }

        virtual bool verbose() const SK_OVERRIDE {
            return fReporter->verbose();
        }

    private:
        Reporter* fReporter;  // Unowned.
        SkTArray<SkString> fFailures;
    };
}  // namespace

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
    fPassed = local.failure_size() == 0;
    fElapsed = SkTime::GetMSecs() - start;

    // Now tell fReporter about any failures and wrap up.
    for (int i = 0; i < local.failure_size(); i++) {
      fReporter->reportFailed(local.failure(i));
    }
    fReporter->endTest(this);

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
