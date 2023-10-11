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
bool IsMockContextType(skgpu::ContextType type) {
    return type == skgpu::ContextType::kMock;
}

skgpu::ContextType compiledInContextTypes[] = {
#if defined(SK_GL)
    // Use "native" instead of explicitly trying both OpenGL and OpenGL ES. Do not use GLES on
    // desktop since tests do not account for not fixing http://skbug.com/2809
#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    skgpu::ContextType::kGL,
#else
    skgpu::ContextType::kGLES,
#endif
#endif // defined(SK_GL)
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
void RunWithGaneshTestContexts(GrContextTestFn* testFn, ContextTypeFilterFn* filter,
                               Reporter* reporter, const GrContextOptions& options) {
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
            SkDebugf("Unable to make direct context for Ganesh test.\n");
            SkASSERT(false);
            return;
        }
    }
}

} // namespace skiatest
#endif // #if defined(SK_GANESH)

TestHarness CurrentTestHarness() {
    return TestHarness::kBazelUnitTestRunner;
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

    if (argc < 2) {
        SkDebugf("Test runner invoked with no arguments.\n");
    } else {
        std::ostringstream oss;
        oss << "Test runner invoked with arguments:";
        for (int i = 1; i < argc; i++) {
            oss << " " << argv[i];
        }
        SkDebugf("%s\n", oss.str().c_str());
    }

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
