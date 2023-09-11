/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLProgram.h"  // IWYU pragma: keep
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace skia_private;

static std::vector<std::string> get_expected_errors(const char* shaderString) {
    // Error expectations are embedded in the source with a special *%%* marker, like so:
    //
    //     /*%%*
    //     expected 'foo', but found 'bar'
    //     'baz' is not a valid identifier
    //     *%%*/
    //
    // Extract them from the shader text.
    std::vector<std::string> expectedErrors;
    constexpr char kExpectedErrorsStart[] = "/*%%*";
    constexpr char kExpectedErrorsEnd[] = "*%%*/";
    if (const char* startPtr = strstr(shaderString, kExpectedErrorsStart)) {
        startPtr += strlen(kExpectedErrorsStart);
        if (const char* endPtr = strstr(startPtr, kExpectedErrorsEnd)) {
            // Store the text between these delimiters in an array of expected errors.
            std::stringstream stream{std::string{startPtr, endPtr}};
            while (stream.good()) {
                expectedErrors.push_back({});
                std::getline(stream, expectedErrors.back(), '\n');
                if (expectedErrors.back().empty()) {
                    expectedErrors.pop_back();
                }
            }
        }
    }

    return expectedErrors;
}

static void check_expected_errors(skiatest::Reporter* r,
                                  const char* testFile,
                                  const std::vector<std::string>& expectedErrors,
                                  std::string reportedErrors) {
    // Verify that the SkSL compiler actually emitted the expected error messages.
    // The list of expectations isn't necessarily exhaustive, though.
    std::string originalErrors = reportedErrors;
    bool reportOriginalErrors = false;
    for (const std::string& expectedError : expectedErrors) {
        // If this error wasn't reported, trigger an error.
        size_t pos = reportedErrors.find(expectedError.c_str());
        if (pos == std::string::npos) {
            ERRORF(r, "%s: Expected an error that wasn't reported:\n%s\n",
                   SkOSPath::Basename(testFile).c_str(), expectedError.c_str());
            reportOriginalErrors = true;
        } else {
            // We found the error that we expected to have. Remove that error from our report, and
            // everything preceding it as well. This ensures that we don't match the same error
            // twice, and that errors are reported in the order we expect.
            reportedErrors.erase(0, pos + expectedError.size());
        }
    }

    if (reportOriginalErrors) {
        ERRORF(r, "%s: The following errors were reported:\n%s\n",
               SkOSPath::Basename(testFile).c_str(), originalErrors.c_str());
    }
}

static void test_expect_fail(skiatest::Reporter* r, const char* testFile, SkSL::ProgramKind kind) {
    // In a size-optimized build, there are a handful of errors which report differently, or not at
    // all. Skip over those tests.
    static const auto* kTestsToSkip = new THashSet<std::string_view>{
        // These are tests that have been deleted, but which may still show up (and fail) on tasks,
        // because the resources directory isn't properly cleaned up. (skbug.com/12987)
        "sksl/errors/InvalidBackendBindingFlagsGL.sksl",
        "sksl/errors/InvalidThreadgroupRTS.rts",
        "sksl/errors/StaticIfTest.sksl",
        "sksl/errors/StaticSwitchConditionalBreak.sksl",
        "sksl/errors/StaticSwitchTest.sksl",
        "sksl/errors/StaticSwitchWithConditionalBreak.sksl",
        "sksl/errors/StaticSwitchWithConditionalContinue.sksl",
        "sksl/errors/StaticSwitchWithConditionalReturn.sksl",

        "sksl/errors/ComputeUniform.compute",
        "sksl/errors/DuplicateBinding.compute",
        "sksl/errors/InvalidThreadgroupCompute.compute",
        "sksl/errors/UnspecifiedBinding.compute",

        "sksl/runtime_errors/ReservedNameISampler2D.rts",

#ifdef SK_ENABLE_OPTIMIZE_SIZE
        "sksl/errors/ArrayInlinedIndexOutOfRange.sksl",
        "sksl/errors/MatrixInlinedIndexOutOfRange.sksl",
        "sksl/errors/OverflowInlinedLiteral.sksl",
        "sksl/errors/VectorInlinedIndexOutOfRange.sksl",
#endif
    };
    if (kTestsToSkip->contains(testFile)) {
        INFOF(r, "%s: skipped in SK_ENABLE_OPTIMIZE_SIZE mode", testFile);
        return;
    }

    sk_sp<SkData> shaderData = GetResourceAsData(testFile);
    if (!shaderData) {
        ERRORF(r, "%s: Unable to load file", SkOSPath::Basename(testFile).c_str());
        return;
    }

    std::string shaderString{reinterpret_cast<const char*>(shaderData->bytes()),
                             shaderData->size()};

    std::vector<std::string> expectedErrors = get_expected_errors(shaderString.c_str());

    // Compile the code.
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Standalone());
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, std::move(shaderString),
                                                                     settings);

    // If the code actually generated a working program, we've already failed.
    if (program) {
        ERRORF(r, "%s: Expected failure, but compiled successfully",
                  SkOSPath::Basename(testFile).c_str());
        return;
    }

    check_expected_errors(r, testFile, expectedErrors, compiler.errorText());
}

static void iterate_dir(const char* directory,
                        const char* extension,
                        const std::function<void(const char*)>& run) {
    SkString resourceDirectory = GetResourcePath(directory);
    SkOSFile::Iter iter(resourceDirectory.c_str(), extension);
    SkString name;

    while (iter.next(&name, /*getDir=*/false)) {
        SkString path(SkOSPath::Join(directory, name.c_str()));
        run(path.c_str());
    }
}

DEF_TEST(SkSLErrorTest, r) {
    iterate_dir("sksl/errors/", ".sksl", [&](const char* path) {
        test_expect_fail(r, path, SkSL::ProgramKind::kFragment);
    });
    iterate_dir("sksl/errors/", ".rts", [&](const char* path) {
        test_expect_fail(r, path, SkSL::ProgramKind::kRuntimeShader);
    });
    iterate_dir("sksl/errors/", ".compute", [&](const char* path) {
        test_expect_fail(r, path, SkSL::ProgramKind::kCompute);
    });
}

DEF_TEST(SkSLRuntimeShaderErrorTest, r) {
    iterate_dir("sksl/runtime_errors/", ".rts", [&](const char* path) {
        test_expect_fail(r, path, SkSL::ProgramKind::kRuntimeShader);
    });
}

DEF_TEST(SkSLRuntimeColorFilterErrorTest, r) {
    iterate_dir("sksl/runtime_errors/", ".rtcf", [&](const char* path) {
        test_expect_fail(r, path, SkSL::ProgramKind::kRuntimeColorFilter);
    });
}

DEF_TEST(SkSLRuntimeBlenderErrorTest, r) {
    iterate_dir("sksl/runtime_errors/", ".rtb", [&](const char* path) {
        test_expect_fail(r, path, SkSL::ProgramKind::kRuntimeBlender);
    });
}
