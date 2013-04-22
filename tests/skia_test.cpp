/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkRunnable.h"
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

static const char* result2string(Reporter::Result result) {
    return result == Reporter::kPassed ? "passed" : "FAILED";
}

class DebugfReporter : public Reporter {
public:
    DebugfReporter(bool allowExtendedTest, bool allowThreaded)
        : fNextIndex(0)
        , fPending(0)
        , fTotal(0)
        , fAllowExtendedTest(allowExtendedTest)
        , fAllowThreaded(allowThreaded) {
    }

    void setTotal(int total) {
        fTotal = total;
    }

    virtual bool allowExtendedTest() const {
        return fAllowExtendedTest;
    }

    virtual bool allowThreaded() const {
        return fAllowThreaded;
    }

protected:
    virtual void onStart(Test* test) {
        const int index = sk_atomic_inc(&fNextIndex);
        sk_atomic_inc(&fPending);
        SkDebugf("[%3d/%3d] (%d) %s\n", index+1, fTotal, fPending, test->getName());
    }
    virtual void onReport(const char desc[], Reporter::Result result) {
        SkDebugf("\t%s: %s\n", result2string(result), desc);
    }

    virtual void onEnd(Test* test) {
        if (!test->passed()) {
            SkDebugf("---- %s FAILED\n", test->getName());
        }

        sk_atomic_dec(&fPending);
        if (fNextIndex == fTotal) {
            // Just waiting on straggler tests.  Shame them by printing their name and runtime.
            SkDebugf("          (%d) %5.1fs %s\n",
                     fPending, test->elapsedMs() / 1e3, test->getName());
        }
    }

private:
    int32_t fNextIndex;
    int32_t fPending;
    int fTotal;
    bool fAllowExtendedTest;
    bool fAllowThreaded;
};

static const char* make_canonical_dir_path(const char* path, SkString* storage) {
    if (path) {
        // clean it up so it always has a trailing searator
        size_t len = strlen(path);
        if (0 == len) {
            path = NULL;
        } else if (SkPATH_SEPARATOR != path[len - 1]) {
            // resize to len + 1, to make room for searator
            storage->set(path, len + 1);
            storage->writable_str()[len] = SkPATH_SEPARATOR;
            path = storage->c_str();
        }
    }
    return path;
}

static SkString gTmpDir;

const SkString& Test::GetTmpDir() {
    return gTmpDir;
}

static SkString gResourcePath;

const SkString& Test::GetResourcePath() {
    return gResourcePath;
}

DEFINE_string2(match, m, NULL, "substring of test name to run.");
DEFINE_string2(tmpDir, t, NULL, "tmp directory for tests to use.");
DEFINE_string2(resourcePath, i, NULL, "directory for test resources.");
DEFINE_bool2(extendedTest, x, false, "run extended tests for pathOps.");
DEFINE_bool2(threaded, z, false, "allow tests to use multiple threads internally.");
DEFINE_bool2(verbose, v, false, "enable verbose output.");
DEFINE_int32(threads, SkThreadPool::kThreadPerCore,
             "Run threadsafe tests on a threadpool with this many threads.");

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

static bool shouldSkip(const char* testName) {
    return !FLAGS_match.isEmpty() && !strstr(testName, FLAGS_match[0]);
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("");
    SkCommandLineFlags::Parse(argc, argv);

    if (!FLAGS_tmpDir.isEmpty()) {
        make_canonical_dir_path(FLAGS_tmpDir[0], &gTmpDir);
    }
    if (!FLAGS_resourcePath.isEmpty()) {
        make_canonical_dir_path(FLAGS_resourcePath[0], &gResourcePath);
    }

#if SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif

    SkGraphics::Init();

    {
        SkString header("Skia UnitTests:");
        if (!FLAGS_match.isEmpty()) {
            header.appendf(" --match %s", FLAGS_match[0]);
        }
        if (!gTmpDir.isEmpty()) {
            header.appendf(" --tmpDir %s", gTmpDir.c_str());
        }
        if (!gResourcePath.isEmpty()) {
            header.appendf(" --resourcePath %s", gResourcePath.c_str());
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

    DebugfReporter reporter(FLAGS_extendedTest, FLAGS_threaded);
    Iter iter(&reporter);

    // Count tests first.
    int total = 0;
    int toRun = 0;
    Test* test;
    while ((test = iter.next()) != NULL) {
        SkAutoTDelete<Test> owned(test);
        if(!shouldSkip(test->getName())) {
            toRun++;
        }
        total++;
    }
    reporter.setTotal(toRun);

    // Now run them.
    iter.reset();
    int32_t failCount = 0;
    int skipCount = 0;

    SkAutoTDelete<SkThreadPool> threadpool(SkNEW_ARGS(SkThreadPool, (FLAGS_threads)));
    SkTArray<Test*> unsafeTests;  // Always passes ownership to an SkTestRunnable
    for (int i = 0; i < total; i++) {
        SkAutoTDelete<Test> test(iter.next());
        if (shouldSkip(test->getName())) {
            ++skipCount;
        } else if (!test->isThreadsafe()) {
            unsafeTests.push_back() = test.detach();
        } else {
            threadpool->add(SkNEW_ARGS(SkTestRunnable, (test.detach(), &failCount)));
        }
    }

    // Run the tests that aren't threadsafe.
    for (int i = 0; i < unsafeTests.count(); i++) {
        SkNEW_ARGS(SkTestRunnable, (unsafeTests[i], &failCount))->run();
    }

    // Blocks until threaded tests finish.
    threadpool.free();

    SkDebugf("Finished %d tests, %d failures, %d skipped.\n",
             toRun, failCount, skipCount);
    const int testCount = reporter.countTests();
    if (FLAGS_verbose && testCount > 0) {
        SkDebugf("Ran %d Internal tests.\n", testCount);
    }
#if SK_SUPPORT_GPU

#if GR_CACHE_STATS
    GrContext *gr = GpuTest::GetContext();

    gr->printCacheStats();
#endif

#endif

    SkGraphics::Term();
    GpuTest::DestroyContexts();

    return (failCount == 0) ? 0 : 1;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
