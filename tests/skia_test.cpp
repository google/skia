
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkGraphics.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

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
    DebugfReporter() : fIndex(0), fTotal(0) {}

    void setIndexOfTotal(int index, int total) {
        fIndex = index;
        fTotal = total;
    }
protected:
    virtual void onStart(Test* test) {
        SkDebugf("[%d/%d] %s...\n", fIndex+1, fTotal, test->getName());
    }
    virtual void onReport(const char desc[], Reporter::Result result) {
        SkDebugf("\t%s: %s\n", result2string(result), desc);
    }
    virtual void onEnd(Test* test) {
        if (!this->getCurrSuccess()) {
            SkDebugf("---- FAILED\n");
        }
    }
private:
    int fIndex, fTotal;
};

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
#if SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif
    SkGraphics::Init();

    const char* matchStr = NULL;

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "--match") == 0) {
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
        SkDebugf("%s\n", header.c_str());
    }

    DebugfReporter reporter;
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

    SkDebugf("Finished %d tests, %d failures, %d skipped.\n",
             count, failCount, skipCount);

#if SK_SUPPORT_GPU

#if GR_CACHE_STATS
    GrContext *gr = GpuTest::GetContext();

    gr->printCacheStats();
#endif

#endif

    SkGraphics::Term();
    GpuTest::DestroyContext();

    return (failCount == 0) ? 0 : 1;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
