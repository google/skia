/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkTArray.h"
#include "SkTemplates.h"
#include "SkThreadPool.h"
#include "SkTime.h"
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
        this->reset();
    }

    void reset() {
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

private:
    Reporter* fReporter;
    const TestRegistry* fReg;
};

class DebugfReporter : public Reporter {
public:
    DebugfReporter(bool allowExtendedTest, bool allowThreaded, bool verbose)
        : fNextIndex(0)
        , fPending(0)
        , fTotal(0)
        , fAllowExtendedTest(allowExtendedTest)
        , fAllowThreaded(allowThreaded)
        , fVerbose(verbose) {
    }

    void setTotal(int total) {
        fTotal = total;
    }

    virtual bool allowExtendedTest() const SK_OVERRIDE {
        return fAllowExtendedTest;
    }

    virtual bool allowThreaded() const SK_OVERRIDE {
        return fAllowThreaded;
    }

    virtual bool verbose() const SK_OVERRIDE {
        return fVerbose;
    }

protected:
    virtual void onStart(Test* test) {
        SkAutoMutexAcquire lock(fStartEndMutex);
        fNextIndex++;
        fPending++;
        SkDebugf("[%3d/%3d] (%d) %s\n", fNextIndex, fTotal, fPending, test->getName());
    }

    virtual void onReportFailed(const SkString& desc) {
        SkDebugf("\tFAILED: %s\n", desc.c_str());
    }

    virtual void onEnd(Test* test) {
        SkAutoMutexAcquire lock(fStartEndMutex);
        if (!test->passed()) {
            SkDebugf("---- %s FAILED\n", test->getName());
        }

        fPending--;
        if (fNextIndex == fTotal) {
            // Just waiting on straggler tests.  Shame them by printing their name and runtime.
            SkDebugf("          (%d) %5.1fs %s\n",
                     fPending, test->elapsedMs() / 1e3, test->getName());
        }
    }

private:
    SkMutex fStartEndMutex;  // Guards fNextIndex and fPending.
    int32_t fNextIndex;
    int32_t fPending;

    // Once the tests get going, these are logically const.
    int fTotal;
    bool fAllowExtendedTest;
    bool fAllowThreaded;
    bool fVerbose;
};

DEFINE_string2(match, m, NULL, "[~][^]substring[$] [...] of test name to run.\n" \
                               "Multiple matches may be separated by spaces.\n" \
                               "~ causes a matching test to always be skipped\n" \
                               "^ requires the start of the test to match\n" \
                               "$ requires the end of the test to match\n" \
                               "^ and $ requires an exact match\n" \
                               "If a test does not match any list entry,\n" \
                               "it is skipped unless some list entry starts with ~");
DEFINE_string2(tmpDir, t, NULL, "tmp directory for tests to use.");
DEFINE_string2(resourcePath, i, "resources", "directory for test resources.");
DEFINE_bool2(extendedTest, x, false, "run extended tests for pathOps.");
DEFINE_bool2(single, z, false, "run tests on a single thread internally.");
DEFINE_bool2(verbose, v, false, "enable verbose output.");
DEFINE_int32(threads, SkThreadPool::kThreadPerCore,
             "Run threadsafe tests on a threadpool with this many threads.");

SkString Test::GetTmpDir() {
    const char* tmpDir = FLAGS_tmpDir.isEmpty() ? NULL : FLAGS_tmpDir[0];
    return SkString(tmpDir);
}

SkString Test::GetResourcePath() {
    const char* resourcePath = FLAGS_resourcePath.isEmpty() ? NULL : FLAGS_resourcePath[0];
    return SkString(resourcePath);
}

// Deletes self when run.
class SkTestRunnable : public SkRunnable {
public:
  // Takes ownership of test.
  SkTestRunnable(Test* test, int32_t* failCount) : fTest(test), fFailCount(failCount) {}

  virtual void run() {
      fTest->run();
      if(!fTest->passed()) {
          sk_atomic_inc(fFailCount);
      }
      SkDELETE(this);
  }

private:
    SkAutoTDelete<Test> fTest;
    int32_t* fFailCount;
};

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("");
    SkCommandLineFlags::Parse(argc, argv);

#if SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif

    SkGraphics::Init();

    {
        SkString header("Skia UnitTests:");
        if (!FLAGS_match.isEmpty()) {
            header.appendf(" --match");
            for (int index = 0; index < FLAGS_match.count(); ++index) {
                header.appendf(" %s", FLAGS_match[index]);
            }
        }
        SkString tmpDir = Test::GetTmpDir();
        if (!tmpDir.isEmpty()) {
            header.appendf(" --tmpDir %s", tmpDir.c_str());
        }
        SkString resourcePath = Test::GetResourcePath();
        if (!resourcePath.isEmpty()) {
            header.appendf(" --resourcePath %s", resourcePath.c_str());
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
        header.appendf(" skia_arch_width=%d", (int)sizeof(void*) * 8);
        SkDebugf("%s\n", header.c_str());
    }

    DebugfReporter reporter(FLAGS_extendedTest, !FLAGS_single, FLAGS_verbose);
    Iter iter(&reporter);

    // Count tests first.
    int total = 0;
    int toRun = 0;
    Test* test;

    while ((test = iter.next()) != NULL) {
        SkAutoTDelete<Test> owned(test);

        if(!SkCommandLineFlags::ShouldSkip(FLAGS_match, test->getName())) {
            toRun++;
        }
        total++;
    }
    reporter.setTotal(toRun);

    // Now run them.
    iter.reset();
    int32_t failCount = 0;
    int skipCount = 0;

    SkThreadPool threadpool(FLAGS_threads);
    SkTArray<Test*> unsafeTests;  // Always passes ownership to an SkTestRunnable
    for (int i = 0; i < total; i++) {
        SkAutoTDelete<Test> test(iter.next());
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, test->getName())) {
            ++skipCount;
        } else if (!test->isThreadsafe()) {
            unsafeTests.push_back() = test.detach();
        } else {
            threadpool.add(SkNEW_ARGS(SkTestRunnable, (test.detach(), &failCount)));
        }
    }

    // Run the tests that aren't threadsafe.
    for (int i = 0; i < unsafeTests.count(); i++) {
        SkNEW_ARGS(SkTestRunnable, (unsafeTests[i], &failCount))->run();
    }

    // Block until threaded tests finish.
    threadpool.wait();

    SkDebugf("Finished %d tests, %d failures, %d skipped.\n",
             toRun, failCount, skipCount);
    const int testCount = reporter.countTests();
    if (FLAGS_verbose && testCount > 0) {
        SkDebugf("Ran %d Internal tests.\n", testCount);
    }
    SkGraphics::Term();
    GpuTest::DestroyContexts();

    return (failCount == 0) ? 0 : 1;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
