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

#if defined(SK_GANESH)
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "tools/gpu/TestContext.h"
#endif

#include <ctime>
#include <cwchar>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>

struct tm;

static DEFINE_string(skip, "", "Space-separated list of test cases to skip.");

class BazelReporter : public skiatest::Reporter {
public:
    void reportFailed(const skiatest::Failure& failure) override {
            SkDebugf("FAIL: %s\n", failure.toString().c_str());
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
bool IsGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kOpenGL == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
}
bool IsVulkanContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kVulkan == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
}
bool IsMetalContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kMetal == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
}
bool IsDirect3DContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kDirect3D == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
}
bool IsDawnContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kDawn == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
}
bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return IsGLContextType(type) && sk_gpu_test::GrContextFactory::IsRenderingContext(type);
}
bool IsMockContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == sk_gpu_test::GrContextFactory::kMock_ContextType;
}

sk_gpu_test::GrContextFactory::ContextType compiledInContextTypes[] = {
#if defined(SK_GL)
    // Use "native" instead of explicitly trying both OpenGL and OpenGL ES. Do not use GLES on
    // desktop since tests do not account for not fixing http://skbug.com/2809
#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    sk_gpu_test::GrContextFactory::kGL_ContextType,
#else
    sk_gpu_test::GrContextFactory::kGLES_ContextType,
#endif
#endif // defined(SK_GL)
#if defined(SK_VULKAN)
    sk_gpu_test::GrContextFactory::kVulkan_ContextType,
#endif
#if defined(SK_DAWN)
    sk_gpu_test::GrContextFactory::kDawn_ContextType,
#endif
// TODO(kjlubick) Other Ganesh backends
    sk_gpu_test::GrContextFactory::kMock_ContextType,
};

// The macros defined in Test.h eventually call into this function. For each GPU backend that is
// compiled in, we run the testFn with a freshly created
void RunWithGaneshTestContexts(GrContextTestFn* testFn, GrContextTypeFilterFn* filter,
                               Reporter* reporter, const GrContextOptions& options) {
    sk_gpu_test::GrContextFactory factory(options);

    for (sk_gpu_test::GrContextFactory::ContextType ctxType : compiledInContextTypes) {
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
            ctxInfo.directContext()->flushAndSubmit(/*syncCPU=*/true);
        } else {
            SkDebugf("Unable to make direct context for Ganesh test.\n");
            SkASSERT(false);
            return;
        }
    }
}

} // namespace skiatest
#endif // #if defined(SK_GANESH)

TestHarness CurrentTestHarness() {
    return TestHarness::kBazelTestRunner;
}

std::string now() {
    std::time_t t = std::time(nullptr);
    std::tm *now = std::gmtime(&t);

    std::ostringstream oss;
    oss << std::put_time(now, "%Y-%m-%d %H:%M:%S UTC");
    return oss.str();
}

void maybeRunTest(const char* name, std::function<void()> testFn) {
    if (FLAGS_skip.contains(name)) {
        SkDebugf("[%s] Skipping %s\n", now().c_str(), name);
        return;
    }

    SkDebugf("[%s] Running %s\n", now().c_str(), name);
    testFn();
    SkDebugf("[%s]\tDone\n", now().c_str());
}

int main(int argc, char** argv) {
#ifdef SK_BUILD_FOR_ANDROID
    extern bool gSkDebugToStdOut; // If true, sends SkDebugf to stdout as well.
    gSkDebugToStdOut = true;
#endif

    CommandLineFlags::Parse(argc, argv);

    BazelReporter reporter;
    for (skiatest::Test test : skiatest::TestRegistry::Range()) {
        if (test.fTestType == skiatest::TestType::kCPU) {
            maybeRunTest(test.fName, [&]() {
                test.cpu(&reporter);
            });
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
            maybeRunTest(test.fName, [&]() {
                test.ganesh(&reporter, grCtxOptions);
            });
        }
    }
#endif

    // TODO(kjlubick) Graphite support

    if (reporter.ok()) {
        SkDebugf("PASS\n");
        return 0;
    }
    SkDebugf("FAIL\n");
    return 1;
}
