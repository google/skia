/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/skqp/src/skqp.h"

#include "gm/gm.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkFontMgrPriv.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/Resources.h"
#include "tools/fonts/TestFontMgr.h"
#ifdef SK_GL
#include "tools/gpu/gl/GLTestContext.h"
#endif
#ifdef SK_VULKAN
#include "tools/gpu/vk/VkTestContext.h"
#endif

#ifdef SK_BUILD_FOR_ANDROID
#include <sys/system_properties.h>
#endif

#include <algorithm>
#include <regex>

static constexpr char kUnitTestReportPath[] = "unit_tests.txt";
static constexpr char kUnitTestsPath[]   = "skqp/unittests.txt";

// Kind of like Python's readlines(), but without any allocation.
// Calls `lineFn` on each line.
static void read_lines(const void* data,
                       size_t size,
                       const std::function<void(std::string_view)>& lineFn) {
    const char* start = (const char*)data;
    const char* end = start + size;
    const char* ptr = start;
    while (ptr < end) {
        while (*ptr++ != '\n' && ptr < end) {}
        size_t len = ptr - start;
        lineFn(std::string_view(start, len));
        start = ptr;
    }
}

namespace {

// Parses the contents of the `skqp/unittests.txt` file.
// Each line in the exclusion list is a regular expression that matches against test names.
// Matches indicate tests that should be excluded. Lines may start with # to indicate a comment.
class ExclusionList {
public:
    ExclusionList() {}

    void initialize(SkQPAssetManager* assetManager, sk_sp<SkData> dat, int enforcedAndroidAPILevel);

    bool isExcluded(const std::string& name) const {
        for (const auto& entry : fEntries) {
            if (std::regex_match(name, entry.regexPattern)) {
                return fEnforcedAndroidAPILevel < entry.excludeUntilAndroidAPILevel;
            }
        }
        return false;
    }

private:
    int fEnforcedAndroidAPILevel;

    struct ExclusionEntry {
        std::regex regexPattern;
        int excludeUntilAndroidAPILevel;
    };

    std::vector<ExclusionEntry> fEntries;
};
}

// Returns a list of every unit test to be run.
static std::vector<SkQP::UnitTest> get_unit_tests(const ExclusionList& exclusionList) {
    std::vector<SkQP::UnitTest> unitTests;
    for (const skiatest::Test& test : skiatest::TestRegistry::Range()) {
        if (!test.fNeedsGpu) {
            continue;
        }
        if (exclusionList.isExcluded(test.fName)) {
            continue;
        }
        unitTests.push_back(&test);
    }
    auto lt = [](SkQP::UnitTest u, SkQP::UnitTest v) { return strcmp(u->fName, v->fName) < 0; };
    std::sort(unitTests.begin(), unitTests.end(), lt);
    return unitTests;
}

// Returns a list of every SkSL error test to be run.
static std::vector<SkQP::SkSLErrorTest> get_sksl_error_tests(SkQPAssetManager* assetManager,
                                                             const ExclusionList& exclusionList) {
    std::vector<SkQP::SkSLErrorTest> skslErrorTests;

    auto iterateFn = [&](const char* directory, const char* extension) {
        std::vector<std::string> paths = assetManager->iterateDir(directory, extension);
        for (const std::string& path : paths) {
            SkString name = SkOSPath::Basename(path.c_str());
            if (exclusionList.isExcluded(name.c_str())) {
                continue;
            }
            sk_sp<SkData> shaderText = GetResourceAsData(path.c_str());
            if (!shaderText) {
                continue;
            }
            skslErrorTests.push_back({
                name.c_str(),
                std::string(static_cast<const char*>(shaderText->data()), shaderText->size())
            });
        }
    };

    // Android only supports runtime shaders, not fragment shaders, color filters or blenders.
    iterateFn("sksl/errors/", ".rts");
    iterateFn("sksl/runtime_errors/", ".rts");

    auto lt = [](const SkQP::SkSLErrorTest& a, const SkQP::SkSLErrorTest& b) {
        return a.name < b.name;
    };
    std::sort(skslErrorTests.begin(), skslErrorTests.end(), lt);
    return skslErrorTests;
}

void ExclusionList::initialize(SkQPAssetManager* assetManager,
                               sk_sp<SkData> dat,
                               int enforcedAndroidAPILevel) {
    fEnforcedAndroidAPILevel = enforcedAndroidAPILevel;
    fEntries = {};

    //TODO: explore refactoring this code to collect the test lists only once in SkQP::init
    ExclusionList noExclusions;
    const std::vector<SkQP::UnitTest> unitTestList = get_unit_tests(noExclusions);
    const std::vector<SkQP::SkSLErrorTest> skslTestList = get_sksl_error_tests(assetManager,
                                                                               noExclusions);

    // function to check whether or not the provided regex matches an existing test
    auto testExists = [&unitTestList, &skslTestList](const std::regex& exclusionRegex) {
        for (const auto& test : unitTestList) {
            if (std::regex_match(std::string(test->fName), exclusionRegex)) {
                return true;
            }
        }
        for (const auto& test : skslTestList) {
            if (std::regex_match(test.name, exclusionRegex)) {
                return true;
            }
        }
        return false;
    };

    read_lines(dat->data(), dat->size(), [this, &testExists](std::string_view line) {
        if (!line.empty() && line.back() == '\n') {
            // Strip line endings.
            line.remove_suffix(1);
        }
        // Only add non-empty strings, and ignore comments.
        if (line.empty() || line.front() == '#') {
            return;
        }

        std::string_view testName = line;
        int excludeUntilAndroidAPILevel = fEnforcedAndroidAPILevel + 1;

        // Check to see if the test has a min Android API level defined
        auto commaLocation = line.find_first_of(',');
        if (commaLocation != std::string::npos) {
            testName = line.substr(0, commaLocation);
            std::string apiString(line.substr(commaLocation + 1));
            excludeUntilAndroidAPILevel = std::stoi(apiString);
        }

        const std::string exclusionString(testName);
        const std::regex exclusionRegex(exclusionString);

        // Throw an error if there are no unit or sksl tests that match the exclusion
        if (!testExists(exclusionRegex)) {
            SK_ABORT("Exclusion list contains tests not found in the test registry: %s",
                     exclusionString.c_str());
        }

      fEntries.push_back({exclusionRegex, excludeUntilAndroidAPILevel});
    });
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

const char* SkQP::GetUnitTestName(SkQP::UnitTest t) { return t->fName; }

SkQP::SkQP() {}

SkQP::~SkQP() {}

void SkQP::init(SkQPAssetManager* assetManager, const char* reportDirectory) {
    SkASSERT_RELEASE(assetManager);
    fReportDirectory = reportDirectory;

    SkGraphics::Init();
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;

    int minAndroidAPILevel = 0;
#ifdef SK_BUILD_FOR_ANDROID
    char firstAPIVersionStr[PROP_VALUE_MAX];
    int strLength = __system_property_get("ro.product.first_api_level", firstAPIVersionStr);
    // Defaults to zero since most checks care if it is greater than a specific value. So this will
    // just default to it being less.
    minAndroidAPILevel = (strLength == 0) ? 0 : atoi(firstAPIVersionStr);
#endif

    // Load the exclusion list `skqp/unittests.txt`, if it exists.
    // The list is checked in at platform_tools/android/apps/skqp/src/main/assets/skqp/unittests.txt
    ExclusionList exclusionList;
    if (sk_sp<SkData> dat = assetManager->open(kUnitTestsPath)) {
        exclusionList.initialize(assetManager, dat, minAndroidAPILevel);
    }

    fUnitTests = get_unit_tests(exclusionList);
    fSkSLErrorTests = get_sksl_error_tests(assetManager, exclusionList);
    fSupportedBackends = get_backends();

    print_backend_info((fReportDirectory + "/grdump.txt").c_str(), fSupportedBackends);
}

std::vector<std::string> SkQP::executeTest(SkQP::UnitTest test) {
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
    fTestResults.push_back(TestResult{test->fName, r.fErrors});
    return r.fErrors;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void write(SkWStream* wStream, const T& text) {
    wStream->write(text.c_str(), text.size());
}

void SkQP::makeReport() {
    if (!sk_isdir(fReportDirectory.c_str())) {
        SkDebugf("Report destination does not exist: '%s'\n", fReportDirectory.c_str());
        return;
    }
    SkFILEWStream report(SkOSPath::Join(fReportDirectory.c_str(), kUnitTestReportPath).c_str());
    SkASSERT_RELEASE(report.isValid());
    for (const SkQP::TestResult& result : fTestResults) {
        report.writeText(result.name.c_str());
        if (result.errors.empty()) {
            report.writeText(" PASSED\n* * *\n");
        } else {
            write(&report, SkStringPrintf(" FAILED (%zu errors)\n", result.errors.size()));
            for (const std::string& err : result.errors) {
                write(&report, err);
                report.newline();
            }
            report.writeText("* * *\n");
        }
    }
}
