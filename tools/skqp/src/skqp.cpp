/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/skqp/src/skqp.h"

#include "gm/gm.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkFontMgrPriv.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkStreamPriv.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/fonts/TestFontMgr.h"
#ifdef SK_GL
#include "tools/gpu/gl/GLTestContext.h"
#endif
#ifdef SK_VULKAN
#include "tools/gpu/vk/VkTestContext.h"
#endif

#include <limits.h>
#include <algorithm>
#include <cinttypes>
#include <regex>

static constexpr char kUnitTestReportPath[] = "unit_tests.txt";
static constexpr char kUnitTestsPath[]   = "skqp/unittests.txt";

// Kind of like Python's readlines(), but without any allocation.
// Calls f() on each line.
// F is [](const char*, size_t) -> void
template <typename F>
static void readlines(const void* data, size_t size, F f) {
    const char* start = (const char*)data;
    const char* end = start + size;
    const char* ptr = start;
    while (ptr < end) {
        while (*ptr++ != '\n' && ptr < end) {}
        size_t len = ptr - start;
        f(start, len);
        start = ptr;
    }
}

// Parses the unittests.txt file.
// when exclude is true, all tests are run except those matching lines from the file
// when exclude is false, only tests matching lines from the file are run.
// Each line is a regular expression matching test names.
// Lines may start with # to indicate a comment
static void get_unit_tests(SkQPAssetManager* mgr,
                           std::vector<SkQP::UnitTest>* unitTests,
                           bool exclude) {
    std::vector<std::regex> patterns;
    auto insert = [&patterns](const char* s, size_t l) {
        SkASSERT(l > 1) ;
        if (l > 0 && s[l - 1] == '\n') {  // strip line endings.
            --l;
        }
        if (l > 0 && s[0] != '#') {  // only add non-empty strings, and ignore comments.
            patterns.emplace_back(std::string(s, l));
        }
    };
    if (sk_sp<SkData> dat = mgr->open(kUnitTestsPath)) {
        readlines(dat->data(), dat->size(), insert);
    }
    for (const skiatest::Test& test : skiatest::TestRegistry::Range()) {
        bool matches_one = false;
        for (const auto& pat : patterns) {
            if (std::regex_match(std::string(test.fName), pat)) {
                matches_one = true;
                continue;
            }
        }
        if (exclude != matches_one && test.fNeedsGpu) {
            unitTests->push_back(&test);
        }
    }
    auto lt = [](SkQP::UnitTest u, SkQP::UnitTest v) { return strcmp(u->fName, v->fName) < 0; };
    std::sort(unitTests->begin(), unitTests->end(), lt);
}

static std::unique_ptr<sk_gpu_test::TestContext> make_test_context(SkQP::SkiaBackend backend) {
    using U = std::unique_ptr<sk_gpu_test::TestContext>;
    switch (backend) {
// TODO(halcanary): Fuchsia will have SK_SUPPORT_GPU and SK_VULKAN, but *not* SK_GL.
#ifdef SK_GL
        case SkQP::SkiaBackend::kGL:
            return U(sk_gpu_test::CreatePlatformGLTestContext(kGL_GrGLStandard, nullptr));
        case SkQP::SkiaBackend::kGLES:
            return U(sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard, nullptr));
#endif
#ifdef SK_VULKAN
        case SkQP::SkiaBackend::kVulkan:
            return U(sk_gpu_test::CreatePlatformVkTestContext(nullptr));
#endif
        default:
            return nullptr;
    }
}

static GrContextOptions context_options(skiagm::GM* gm = nullptr) {
    GrContextOptions grContextOptions;
    grContextOptions.fAllowPathMaskCaching = true;
    grContextOptions.fDisableDriverCorrectnessWorkarounds = true;
    if (gm) {
        gm->modifyGrContextOptions(&grContextOptions);
    }
    return grContextOptions;
}

static std::vector<SkQP::SkiaBackend> get_backends() {
    std::vector<SkQP::SkiaBackend> result;
    SkQP::SkiaBackend backends[] = {
        #ifdef SK_GL
        #ifndef SK_BUILD_FOR_ANDROID
        SkQP::SkiaBackend::kGL,  // Used for testing on desktop machines.
        #endif
        SkQP::SkiaBackend::kGLES,
        #endif  // SK_GL
        #ifdef SK_VULKAN
        SkQP::SkiaBackend::kVulkan,
        #endif
    };
    for (SkQP::SkiaBackend backend : backends) {
        std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
        if (testCtx) {
            testCtx->makeCurrent();
            if (nullptr != testCtx->makeContext(context_options())) {
                result.push_back(backend);
            }
        }
    }
    SkASSERT_RELEASE(result.size() > 0);
    return result;
}

static void print_backend_info(const char* dstPath,
                               const std::vector<SkQP::SkiaBackend>& backends) {
#ifdef SK_ENABLE_DUMP_GPU
    SkFILEWStream out(dstPath);
    out.writeText("[\n");
    for (SkQP::SkiaBackend backend : backends) {
        if (std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend)) {
            testCtx->makeCurrent();
            if (sk_sp<GrDirectContext> ctx = testCtx->makeContext(context_options())) {
                SkString info = ctx->dump();
                // remove null
                out.write(info.c_str(), info.size());
                out.writeText(",\n");
            }
        }
    }
    out.writeText("]\n");
#endif
}

////////////////////////////////////////////////////////////////////////////////

TestHarness CurrentTestHarness() {
    return TestHarness::kSkQP;
}

////////////////////////////////////////////////////////////////////////////////

const char* SkQP::GetBackendName(SkQP::SkiaBackend b) {
    switch (b) {
        case SkQP::SkiaBackend::kGL:     return "gl";
        case SkQP::SkiaBackend::kGLES:   return "gles";
        case SkQP::SkiaBackend::kVulkan: return "vk";
    }
    return "";
}

const char* SkQP::GetUnitTestName(SkQP::UnitTest t) { return t->fName; }

SkQP::SkQP() {}

SkQP::~SkQP() {}

void SkQP::init(SkQPAssetManager* am, const char* reportDirectory) {
    SkASSERT_RELEASE(!fAssetManager);
    SkASSERT_RELEASE(am);
    fAssetManager = am;
    fReportDirectory = reportDirectory;

    SkGraphics::Init();
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;

    /* If the file "skqp/unittests.txt" does not exist or is empty, run all gpu
       unit tests.  Otherwise run only tests that do not match a line in that file.
       The list is checked in at platform_tools/android/apps/skqp/src/main/assets/skqp/unittests.txt
    */
    get_unit_tests(fAssetManager, &fUnitTests, true);
    fSupportedBackends = get_backends();

    print_backend_info((fReportDirectory + "/grdump.txt").c_str(), fSupportedBackends);
}

std::vector<std::string> SkQP::executeTest(SkQP::UnitTest test) {
    SkASSERT_RELEASE(fAssetManager);
    struct : public skiatest::Reporter {
        std::vector<std::string> fErrors;
        void reportFailed(const skiatest::Failure& failure) override {
            SkString desc = failure.toString();
            fErrors.push_back(std::string(desc.c_str(), desc.size()));
        }
    } r;
    GrContextOptions options;
    options.fDisableDriverCorrectnessWorkarounds = true;
    if (test->fContextOptionsProc) {
        test->fContextOptionsProc(&options);
    }
    test->fProc(&r, options);
    fUnitTestResults.push_back(UnitTestResult{test, r.fErrors});
    return r.fErrors;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void write(SkWStream* wStream, const T& text) {
    wStream->write(text.c_str(), text.size());
}

void SkQP::makeReport() {
    SkASSERT_RELEASE(fAssetManager);
    if (!sk_isdir(fReportDirectory.c_str())) {
        SkDebugf("Report destination does not exist: '%s'\n", fReportDirectory.c_str());
        return;
    }
    SkFILEWStream unitOut(SkOSPath::Join(fReportDirectory.c_str(), kUnitTestReportPath).c_str());
    SkASSERT_RELEASE(unitOut.isValid());
    for (const SkQP::UnitTestResult& result : fUnitTestResults) {
        unitOut.writeText(GetUnitTestName(result.fUnitTest));
        if (result.fErrors.empty()) {
            unitOut.writeText(" PASSED\n* * *\n");
        } else {
            write(&unitOut, SkStringPrintf(" FAILED (%zu errors)\n", result.fErrors.size()));
            for (const std::string& err : result.fErrors) {
                write(&unitOut, err);
                unitOut.newline();
            }
            unitOut.writeText("* * *\n");
        }
    }
}
