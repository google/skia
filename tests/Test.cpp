#include "Test.h"

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

