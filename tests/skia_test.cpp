/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CrashHandler.h"
#include "OverwriteLine.h"
#include "Resources.h"
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
#include "GrContextFactory.h"
#endif

using namespace skiatest;

DEFINE_string2(match, m, NULL, "[~][^]substring[$] [...] of test name to run.\n" \
                               "Multiple matches may be separated by spaces.\n" \
                               "~ causes a matching test to always be skipped\n" \
                               "^ requires the start of the test to match\n" \
                               "$ requires the end of the test to match\n" \
                               "^ and $ requires an exact match\n" \
                               "If a test does not match any list entry,\n" \
                               "it is skipped unless some list entry starts with ~");
DEFINE_bool2(extendedTest, x, false, "run extended tests for pathOps.");
DEFINE_bool2(leaks, l, false, "show leaked ref cnt'd objects.");
DEFINE_bool2(single, z, false, "run tests on a single thread internally.");
DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");
DEFINE_bool2(veryVerbose, V, false, "tell individual tests to be verbose.");
DEFINE_bool(cpu, true, "whether or not to run CPU tests.");
DEFINE_bool(gpu, true, "whether or not to run GPU tests.");
DEFINE_int32(threads, SkThreadPool::kThreadPerCore,
             "Run threadsafe tests on a threadpool with this many threads.");

// need to explicitly declare this, or we get some weird infinite loop llist
template TestRegistry* TestRegistry::gHead;

class Iter {
public:
    Iter() { this->reset(); }
    void reset() { fReg = TestRegistry::Head(); }

    Test* next(Reporter* r) {
        if (fReg) {
            TestRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            Test* test = fact(NULL);
            test->setReporter(r);
            return test;
        }
        return NULL;
    }

private:
    const TestRegistry* fReg;
};

class DebugfReporter : public Reporter {
public:
    explicit DebugfReporter(int total) : fDone(0), fTotal(total) {}

    virtual bool allowExtendedTest() const SK_OVERRIDE { return FLAGS_extendedTest; }
    virtual bool allowThreaded()     const SK_OVERRIDE { return !FLAGS_single; }
    virtual bool verbose()           const SK_OVERRIDE { return FLAGS_veryVerbose; }

protected:
    virtual void onReportFailed(const SkString& desc) SK_OVERRIDE {
        SkDebugf("\nFAILED: %s", desc.c_str());
    }

    virtual void onEnd(Test* test) SK_OVERRIDE {
        const int done = 1 + sk_atomic_inc(&fDone);

        if (!test->passed()) {
            SkDebugf("\n---- %s FAILED", test->getName());
        }

        SkString prefix(kSkOverwriteLine);
        SkString time;
        if (FLAGS_verbose) {
            prefix.printf("\n");
            time.printf("%5dms ", test->elapsedMs());
        }
        SkDebugf("%s[%3d/%3d] %s%s", prefix.c_str(), done, fTotal, time.c_str(), test->getName());
    }

private:
    int32_t fDone;  // atomic
    const int fTotal;
};

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

static bool should_run(const char* testName, bool isGPUTest) {
    if (SkCommandLineFlags::ShouldSkip(FLAGS_match, testName)) {
        return false;
    }
    if (!FLAGS_cpu && !isGPUTest) {
        return false;
    }
    if (!FLAGS_gpu && isGPUTest) {
        return false;
    }
    return true;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SetupCrashHandler();
    SkCommandLineFlags::SetUsage("");
    SkCommandLineFlags::Parse(argc, argv);

#if SK_ENABLE_INST_COUNT
    if (FLAGS_leaks) {
        gPrintInstCount = true;
    }
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
        SkString resourcePath = GetResourcePath();
        if (!resourcePath.isEmpty()) {
            header.appendf(" --resourcePath %s", resourcePath.c_str());
        }
#ifdef SK_DEBUG
        header.append(" SK_DEBUG");
#else
        header.append(" SK_RELEASE");
#endif
        header.appendf(" skia_arch_width=%d", (int)sizeof(void*) * 8);
        if (FLAGS_veryVerbose) {
            header.appendf("\n");
        }
        SkDebugf(header.c_str());
    }


    // Count tests first.
    int total = 0;
    int toRun = 0;
    Test* test;

    Iter iter;
    while ((test = iter.next(NULL/*reporter not needed*/)) != NULL) {
        SkAutoTDelete<Test> owned(test);
        if (should_run(test->getName(), test->isGPUTest())) {
            toRun++;
        }
        total++;
    }

    // Now run them.
    iter.reset();
    int32_t failCount = 0;
    int skipCount = 0;

    SkThreadPool threadpool(FLAGS_threads);
    SkTArray<Test*> gpuTests;  // Always passes ownership to an SkTestRunnable

    DebugfReporter reporter(toRun);
    for (int i = 0; i < total; i++) {
        SkAutoTDelete<Test> test(iter.next(&reporter));
        if (!should_run(test->getName(), test->isGPUTest())) {
            ++skipCount;
        } else if (test->isGPUTest()) {
            gpuTests.push_back() = test.detach();
        } else {
            threadpool.add(SkNEW_ARGS(SkTestRunnable, (test.detach(), &failCount)));
        }
    }

#if SK_SUPPORT_GPU
    // Give GPU tests a context factory if that makes sense on this machine.
    GrContextFactory grContextFactory;
    for (int i = 0; i < gpuTests.count(); i++) {
        gpuTests[i]->setGrContextFactory(&grContextFactory);
    }
#endif

    // Run GPU tests on this thread.
    for (int i = 0; i < gpuTests.count(); i++) {
        SkNEW_ARGS(SkTestRunnable, (gpuTests[i], &failCount))->run();
    }

    // Block until threaded tests finish.
    threadpool.wait();

    if (FLAGS_verbose) {
        SkDebugf("\nFinished %d tests, %d failures, %d skipped. (%d internal tests)",
                 toRun, failCount, skipCount, reporter.countTests());
    }
    SkGraphics::Term();

    SkDebugf("\n");
    return (failCount == 0) ? 0 : 1;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
