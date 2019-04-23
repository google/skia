/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <atomic>
#include "include/core/SkGraphics.h"
#include "include/core/SkTime.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/pathops/SkPathOpsDebug.h"
#include "tests/PathOpsDebug.h"
#include "tests/Test.h"
#include "tools/CrashHandler.h"
#include "tools/OverwriteLine.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"

using namespace skiatest;
using namespace sk_gpu_test;

static DEFINE_bool2(dumpOp, d, false, "dump the pathOps to a file to recover mid-crash.");
static DEFINE_bool2(extendedTest, x, false, "run extended tests for pathOps.");
static DEFINE_bool2(runFail, f, false, "check for success on tests known to fail.");
static DEFINE_bool2(verifyOp, y, false, "compare the pathOps result against a region.");
static DEFINE_string2(json, J, "", "write json version of tests.");
static DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");
static DEFINE_bool2(veryVerbose, V, false, "tell individual tests to be verbose.");
static DEFINE_bool(cpu, true, "master switch for running CPU-bound work.");
static DEFINE_bool(gpu, true, "master switch for running GPU-bound work.");

static DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching name to always be skipped\n"
               "^ requires the start of the name to match\n"
               "$ requires the end of the name to match\n"
               "^ and $ requires an exact match\n"
               "If a name does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

static DEFINE_int_2(threads, j, -1,
               "Run threadsafe tests on a threadpool with this many extra threads, "
               "defaulting to one extra thread per core.");

#if DEBUG_COIN
static DEFINE_bool2(coinTest, c, false, "detect unused coincidence algorithms.");
#endif

// need to explicitly declare this, or we get some weird infinite loop llist
template TestRegistry* TestRegistry::gHead;
void (*gVerboseFinalize)() = nullptr;

// The threads report back to this object when they are done.
class Status {
public:
    explicit Status(int total)
        : fDone(0), fTestCount(0), fFailCount(0), fTotal(total) {}
    // Threadsafe.
    void endTest(const char* testName,
                 bool success,
                 SkMSec elapsed,
                 int testCount) {
        const int done = ++fDone;
        fTestCount += testCount;
        if (!success) {
            SkDebugf("\n---- %s FAILED", testName);
        }

        SkString prefix(kSkOverwriteLine);
        SkString time;
        if (FLAGS_verbose) {
            prefix.printf("\n");
            time.printf("%5dms ", elapsed);
        }
        SkDebugf("%s[%3d/%3d] %s%s", prefix.c_str(), done, fTotal, time.c_str(),
                 testName);
    }

    void reportFailure() { fFailCount++; }

    int32_t testCount() { return fTestCount; }
    int32_t failCount() { return fFailCount; }

private:
    std::atomic<int32_t> fDone;
    std::atomic<int32_t> fTestCount;
    std::atomic<int32_t> fFailCount;
    const int fTotal;
};

class SkTestRunnable {
public:
    SkTestRunnable(const Test& test, Status* status) : fTest(test), fStatus(status) {}

    void operator()() {
        struct TestReporter : public skiatest::Reporter {
        public:
            TestReporter() : fStats(nullptr), fError(false), fTestCount(0) {}
            void bumpTestCount() override { ++fTestCount; }
            bool allowExtendedTest() const override { return FLAGS_extendedTest; }
            bool verbose() const override { return FLAGS_veryVerbose; }
            void reportFailed(const skiatest::Failure& failure) override {
                SkDebugf("\nFAILED: %s", failure.toString().c_str());
                fError = true;
            }
            void* stats() const override { return fStats; }
            void* fStats;
            bool fError;
            int fTestCount;
        } reporter;

        const Timer timer;
        fTest.proc(&reporter, GrContextOptions());
        SkMSec elapsed = timer.elapsedMsInt();
        if (reporter.fError) {
            fStatus->reportFailure();
        }
        fStatus->endTest(fTest.name, !reporter.fError, elapsed, reporter.fTestCount);
  }

private:
    Test fTest;
    Status* fStatus;
};

static bool should_run(const char* testName, bool isGPUTest) {
    if (CommandLineFlags::ShouldSkip(FLAGS_match, testName)) {
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

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
#if DEBUG_DUMP_VERIFY
    SkPathOpsDebug::gDumpOp = FLAGS_dumpOp;
    SkPathOpsDebug::gVerifyOp = FLAGS_verifyOp;
#endif
    SkPathOpsDebug::gRunFail = FLAGS_runFail;
    SkPathOpsDebug::gVeryVerbose = FLAGS_veryVerbose;
    PathOpsDebug::gOutFirst = true;
    PathOpsDebug::gCheckForDuplicateNames = false;
    PathOpsDebug::gOutputSVG = false;
    if ((PathOpsDebug::gJson = !FLAGS_json.isEmpty())) {
        PathOpsDebug::gOut = fopen(FLAGS_json[0], "wb");
        fprintf(PathOpsDebug::gOut, "{\n");
        FLAGS_threads = 0;
        PathOpsDebug::gMarkJsonFlaky = false;
    }
    SetupCrashHandler();

    SkAutoGraphics ag;

    {
        SkString header("Skia UnitTests:");
        if (!FLAGS_match.isEmpty()) {
            header.appendf(" --match");
            for (int index = 0; index < FLAGS_match.count(); ++index) {
                header.appendf(" %s", FLAGS_match[index]);
            }
        }
        SkString tmpDir = skiatest::GetTmpDir();
        if (!tmpDir.isEmpty()) {
            header.appendf(" --tmpDir %s", tmpDir.c_str());
        }
        SkString resourcePath = GetResourcePath();
        if (!resourcePath.isEmpty()) {
            header.appendf(" --resourcePath %s", resourcePath.c_str());
        }
#if DEBUG_COIN
        if (FLAGS_coinTest) {
            header.appendf(" -c");
        }
#endif
        if (FLAGS_dumpOp) {
            header.appendf(" -d");
        }
#ifdef SK_DEBUG
        if (FLAGS_runFail) {
            header.appendf(" -f");
        }
#endif
        if (FLAGS_verbose) {
            header.appendf(" -v");
        }
        if (FLAGS_veryVerbose) {
            header.appendf(" -V");
        }
        if (FLAGS_extendedTest) {
            header.appendf(" -x");
        }
        if (FLAGS_verifyOp) {
            header.appendf(" -y");
        }
#ifdef SK_DEBUG
        header.append(" SK_DEBUG");
#else
        header.append(" SK_RELEASE");
#endif
        if (FLAGS_veryVerbose) {
            header.appendf("\n");
        }
        SkDebugf("%s", header.c_str());
    }


    // Count tests first.
    int total = 0;
    int toRun = 0;

    for (const Test& test : TestRegistry::Range()) {
        if (should_run(test.name, test.needsGpu)) {
            toRun++;
        }
        total++;
    }

    // Now run them.
    int skipCount = 0;

    SkTaskGroup::Enabler enabled(FLAGS_threads);
    SkTaskGroup cpuTests;
    SkTArray<const Test*> gpuTests;

    Status status(toRun);

    for (const Test& test : TestRegistry::Range()) {
        if (!should_run(test.name, test.needsGpu)) {
            ++skipCount;
        } else if (test.needsGpu) {
            gpuTests.push_back(&test);
        } else {
            cpuTests.add(SkTestRunnable(test, &status));
        }
    }

    // Run GPU tests on this thread.
    for (int i = 0; i < gpuTests.count(); i++) {
        SkTestRunnable(*gpuTests[i], &status)();
    }

    // Block until threaded tests finish.
    cpuTests.wait();

    if (FLAGS_verbose) {
        SkDebugf(
                "\nFinished %d tests, %d failures, %d skipped. "
                "(%d internal tests)",
                toRun, status.failCount(), skipCount, status.testCount());
        if (gVerboseFinalize) {
            (*gVerboseFinalize)();
        }
    }

    SkDebugf("\n");
#if DEBUG_COIN
    if (FLAGS_coinTest) {
        SkPathOpsDebug::DumpCoinDict();
    }
#endif
    if (PathOpsDebug::gJson) {
        fprintf(PathOpsDebug::gOut, "\n}\n");
        fclose(PathOpsDebug::gOut);
    }
    return (status.failCount() == 0) ? 0 : 1;
}
