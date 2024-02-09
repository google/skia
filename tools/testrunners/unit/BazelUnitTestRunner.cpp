/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This runs *all* the tests registered via the Test registry in series. If one or more fail, those
 * error messages will be printed out and a non-zero exit code will be returned. Otherwise, the
 * exit code will be 0.
 */

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/testrunners/common/TestRunner.h"

#if defined(SK_GANESH)
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/TestContext.h"
#endif

#include <ctime>
#include <cwchar>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>

struct tm;

static DEFINE_string(skip, "", "Space-separated list of test cases (regexps) to skip.");
static DEFINE_string(
        match,
        "",
        "Space-separated list of test cases (regexps) to run. Will run all tests if omitted.");

// Set in //bazel/devicesrc but consumed by other C++ test runners.
static DEFINE_string(key, "", "Ignored by this test runner.");
static DEFINE_string(cpuName, "", "Ignored by this test runner.");
static DEFINE_string(gpuName, "", "Ignored by this test runner.");

// Set in //bazel/devicesrc but only consumed by adb_test_runner.go. We cannot use the
// DEFINE_string macro because the flag name includes dashes.
[[maybe_unused]] static bool unused =
        SkFlagInfo::CreateStringFlag("device-specific-bazel-config",
                                     nullptr,
                                     new CommandLineFlags::StringArray(),
                                     nullptr,
                                     "Ignored by this test runner.",
                                     nullptr);

class BazelReporter : public skiatest::Reporter {
public:
    void reportFailed(const skiatest::Failure& failure) override {
        TestRunner::Log("FAIL: %s", failure.toString().c_str());
        fFailed = true;
    }
    bool allowExtendedTest() const override { return false; }
    bool verbose() const override { return false; }
    bool ok() { return !fFailed; }

private:
    bool fFailed = false;
};

#if defined(SK_GANESH)
namespace skiatest {
bool IsGLContextType(skgpu::ContextType type) {
    return skgpu::ganesh::ContextTypeBackend(type) == GrBackendApi::kOpenGL;
}
bool IsVulkanContextType(skgpu::ContextType type) {
    return skgpu::ganesh::ContextTypeBackend(type) == GrBackendApi::kVulkan;
}
bool IsMetalContextType(skgpu::ContextType type) {
    return skgpu::ganesh::ContextTypeBackend(type) == GrBackendApi::kMetal;
}
bool IsDirect3DContextType(skgpu::ContextType type) {
    return skgpu::ganesh::ContextTypeBackend(type) == GrBackendApi::kDirect3D;
}
bool IsMockContextType(skgpu::ContextType type) { return type == skgpu::ContextType::kMock; }

skgpu::ContextType compiledInContextTypes[] = {
#if defined(SK_GL)
// Use "native" instead of explicitly trying both OpenGL and OpenGL ES. Do not use GLES on
// desktop since tests do not account for not fixing http://skbug.com/2809
#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
        skgpu::ContextType::kGL,
#else
        skgpu::ContextType::kGLES,
#endif
#endif  // defined(SK_GL)
#if defined(SK_VULKAN)
        skgpu::ContextType::kVulkan,
#endif
#if defined(SK_DAWN)
        skgpu::ContextType::kDawn,
#endif
        // TODO(kjlubick) Other Ganesh backends
        skgpu::ContextType::kMock,
};

// The macros defined in Test.h eventually call into this function. For each GPU backend that is
// compiled in, we run the testFn with a freshly created
void RunWithGaneshTestContexts(GrContextTestFn* testFn,
                               ContextTypeFilterFn* filter,
                               Reporter* reporter,
                               const GrContextOptions& options) {
    sk_gpu_test::GrContextFactory factory(options);

    for (skgpu::ContextType ctxType : compiledInContextTypes) {
        if (filter && !(*filter)(ctxType)) {
            continue;
        }

        sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(ctxType);
        if (ctxInfo.directContext()) {
            (*testFn)(reporter, ctxInfo);
            // In case the test changed the current context make sure we move it back before
            // calling flush.
            ctxInfo.testContext()->makeCurrent();
            // Sync so any release/finished procs get called.
            ctxInfo.directContext()->flushAndSubmit(GrSyncCpu::kYes);
        } else {
            TestRunner::Log("Unable to make direct context for Ganesh test.");
            SkASSERT(false);
            return;
        }
    }
}

}  // namespace skiatest
#endif  // #if defined(SK_GANESH)

TestHarness CurrentTestHarness() { return TestHarness::kBazelUnitTestRunner; }

void maybeRunTest(const char* name, std::function<void()> testFn) {
    if (!TestRunner::ShouldRunTestCase(name, FLAGS_match, FLAGS_skip)) {
        TestRunner::Log("Skipping %s", name);
        return;
    }

    TestRunner::Log("Running %s", name);
    testFn();
    TestRunner::Log("\tDone");
}

int main(int argc, char** argv) {
    TestRunner::InitAndLogCmdlineArgs(argc, argv);

    CommandLineFlags::Parse(argc, argv);

    BazelReporter reporter;
    for (skiatest::Test test : skiatest::TestRegistry::Range()) {
        if (test.fTestType == skiatest::TestType::kCPU) {
            maybeRunTest(test.fName, [&]() { test.cpu(&reporter); });
        }
    }

#if defined(SK_GANESH)
    GrContextOptions grCtxOptions;
    // TODO(kjlubick) DM has grContextOptions set via flags. Should this runner have that too?
    grCtxOptions.fExecutor = nullptr;
    grCtxOptions.fAllowPathMaskCaching = true;
    grCtxOptions.fFailFlushTimeCallbacks = false;
    grCtxOptions.fAllPathsVolatile = false;
    grCtxOptions.fGpuPathRenderers = GpuPathRenderers::kDefault;
    grCtxOptions.fDisableDriverCorrectnessWorkarounds = false;
    grCtxOptions.fResourceCacheLimitOverride = -1;
    grCtxOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
    for (skiatest::Test test : skiatest::TestRegistry::Range()) {
        if (test.fTestType == skiatest::TestType::kGanesh) {
            maybeRunTest(test.fName, [&]() { test.ganesh(&reporter, grCtxOptions); });
        }
    }
#endif

    // TODO(kjlubick) Graphite support

    if (reporter.ok()) {
        TestRunner::Log("PASS");
        return 0;
    }
    TestRunner::Log("FAIL");
    return 1;
}
