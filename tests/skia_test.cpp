
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkGraphics.h"
#include "Test.h"

using namespace skiatest;

// need to explicitly declare this, or we get some weird infinite loop llist
template TestRegistry* TestRegistry::gHead;

class Iter {
public:
    Iter(Reporter* r) : fReporter(r) {
        r->ref();
        fReg = TestRegistry::Head();
    }

    ~Iter() {
        fReporter->unref();
    }

    Test* next() {
        if (fReg) {
            TestRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            Test* test = fact(NULL);
            test->setReporter(fReporter);
            return test;
        }
        return NULL;
    }

    static int Count() {
        const TestRegistry* reg = TestRegistry::Head();
        int count = 0;
        while (reg) {
            count += 1;
            reg = reg->next();
        }
        return count;
    }

private:
    Reporter* fReporter;
    const TestRegistry* fReg;
};

static const char* result2string(Reporter::Result result) {
    return result == Reporter::kPassed ? "passed" : "FAILED";
}

class DebugfReporter : public Reporter {
public:
    DebugfReporter(bool androidMode) : fAndroidMode(androidMode) {}

    void setIndexOfTotal(int index, int total) {
        fIndex = index;
        fTotal = total;
    }
protected:
    virtual void onStart(Test* test) {
        this->dumpState(test, kStarting_State);
    }
    virtual void onReport(const char desc[], Reporter::Result result) {
        if (!fAndroidMode) {
            SkDebugf("\t%s: %s\n", result2string(result), desc);
        }
    }
    virtual void onEnd(Test* test) {
        this->dumpState(test, this->getCurrSuccess() ?
                        kSucceeded_State : kFailed_State);
    }
private:
    enum State {
        kStarting_State = 1,
        kSucceeded_State = 0,
        kFailed_State = -2
    };

    void dumpState(Test* test, State state) {
        if (fAndroidMode) {
            SkDebugf("INSTRUMENTATION_STATUS: test=%s\n", test->getName());
            SkDebugf("INSTRUMENTATION_STATUS: class=com.skia\n");
            SkDebugf("INSTRUMENTATION_STATUS: current=%d\n", fIndex+1);
            SkDebugf("INSTRUMENTATION_STATUS: numtests=%d\n", fTotal);
            SkDebugf("INSTRUMENTATION_STATUS_CODE: %d\n", state);
        } else {
            if (kStarting_State == state) {
                SkDebugf("[%d/%d] %s...\n", fIndex+1, fTotal, test->getName());
            } else if (kFailed_State == state) {
                SkDebugf("---- FAILED\n");
            }
        }
    }

    int fIndex, fTotal;
    bool fAndroidMode;
};

int main (int argc, char * const argv[]) {
    SkAutoGraphics ag;

    bool androidMode = false;
    const char* matchStr = NULL;

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-android") == 0) {
            androidMode = true;
        
        } else if (strcmp(*argv, "--match") == 0) {
            ++argv;
            if (argv < stop && **argv) {
                matchStr = *argv;
            }
        }
    }

    {
        SkString header("Skia UnitTests:");
        if (matchStr) {
            header.appendf(" --match %s", matchStr);
        }
#ifdef SK_DEBUG
        header.append(" SK_DEBUG");
#else
        header.append(" SK_RELEASE");
#endif
#ifdef SK_SCALAR_IS_FIXED
        header.append(" SK_SCALAR_IS_FIXED");
#else
        header.append(" SK_SCALAR_IS_FLOAT");
#endif
        if (!androidMode) {
            SkDebugf("%s\n", header.c_str());
        }
    }

    DebugfReporter reporter(androidMode);
    Iter iter(&reporter);
    Test* test;

    const int count = Iter::Count();
    int index = 0;
    int failCount = 0;
    int skipCount = 0;
    while ((test = iter.next()) != NULL) {
        reporter.setIndexOfTotal(index, count);
        if (NULL != matchStr && !strstr(test->getName(), matchStr)) {
            ++skipCount;
        } else {
            if (!test->run()) {
                ++failCount;
            }
        }
        SkDELETE(test);
        index += 1;
    }

    if (!androidMode) {
        SkDebugf("Finished %d tests, %d failures, %d skipped.\n",
                 count, failCount, skipCount);
    }
    return (failCount == 0) ? 0 : 1;
}
