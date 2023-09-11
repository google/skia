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

// Returns a list of every unit test to be run.
static std::vector<SkQP::UnitTest> get_unit_tests(int enforcedAndroidAPILevel) {
    std::vector<SkQP::UnitTest> unitTests;
    for (const skiatest::Test& test : skiatest::TestRegistry::Range()) {
        const auto ctsMode = test.fCTSEnforcement.eval(enforcedAndroidAPILevel);
        if (ctsMode != CtsEnforcement::RunMode::kSkip) {
            SkASSERTF(test.fTestType != skiatest::TestType::kCPU,
                      "Non-GPU test was included in SkQP: %s\n", test.fName);
            unitTests.push_back(&test);
        }
    }
    auto lt = [](SkQP::UnitTest u, SkQP::UnitTest v) { return strcmp(u->fName, v->fName) < 0; };
    std::sort(unitTests.begin(), unitTests.end(), lt);
    return unitTests;
}

/**
 * SkSL error tests are used by CTS to verify that Android's RuntimeShader API fails when certain
 * shader programs are compiled.  Unlike unit tests these error tests are defined in resource files
 * not source code.  As such, we are unable to mark each test with a CtsEnforcement value.  This
 * list of exclusion rules excludes tests based on their file name so that we can have some form of
 * control for which Android version an SkSL error test is expected to run.
 */
static const std::pair<std::regex, CtsEnforcement> sExclusionRulesForSkSLTests[] = {
        // disable all ES3 tests until AGSL supports it.
        {std::regex(".*ES3.*"), CtsEnforcement::kNever}};

// Returns a list of every SkSL error test to be run.
static std::vector<SkQP::SkSLErrorTest> get_sksl_error_tests(SkQPAssetManager* assetManager,
                                                             int enforcedAndroidAPILevel) {
    std::vector<SkQP::SkSLErrorTest> skslErrorTests;
    auto iterateFn = [&](const char* directory, const char* extension) {
        std::vector<std::string> paths = assetManager->iterateDir(directory, extension);
        for (const std::string& path : paths) {
            SkString name = SkOSPath::Basename(path.c_str());
            for (auto& exclusionEntry : sExclusionRulesForSkSLTests) {
                if (std::regex_match(name.c_str(), exclusionEntry.first) &&
                    exclusionEntry.second.eval(enforcedAndroidAPILevel) ==
                            CtsEnforcement::RunMode::kSkip) {
                    continue;
                }
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

#ifdef SK_BUILD_FOR_ANDROID
    // ro.vendor.api_level contains the minAPI level based on the order defined in
    // docs.partner.android.com/gms/building/integrating/extending-os-upgrade-support-windows
    //  1. board's current api level (for boards that have been upgraded by the SoC vendor)
    //  2. board's first api level (for devices that initially shipped with an older version)
    //  3. product's first api level
    //  4. product's current api level
    char minAPIVersionStr[PROP_VALUE_MAX];
    int strLength = __system_property_get("ro.vendor.api_level", minAPIVersionStr);
    if (strLength != 0) {
        fEnforcedAndroidAPILevel = atoi(minAPIVersionStr);
    }
#endif

    fUnitTests = get_unit_tests(fEnforcedAndroidAPILevel);
    fSkSLErrorTests = get_sksl_error_tests(assetManager, fEnforcedAndroidAPILevel);

    printBackendInfo((fReportDirectory + "/grdump.txt").c_str());
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
    if (test->fCTSEnforcement.eval(fEnforcedAndroidAPILevel) ==
        CtsEnforcement::RunMode::kRunStrict) {
        options.fDisableDriverCorrectnessWorkarounds = true;
    }
    if (test->fGaneshContextOptionsProc) {
        test->fGaneshContextOptionsProc(&options);
    }
    test->ganesh(&r, options);
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
