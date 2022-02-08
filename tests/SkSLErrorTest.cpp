/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <sstream>
#include <string_view>

static void test_expect_fail(skiatest::Reporter* r, const char* testFile) {
    sk_sp<SkData> shaderData = GetResourceAsData(testFile);
    if (!shaderData) {
        ERRORF(r, "%s: Unable to load file", SkOSPath::Basename(testFile).c_str());
        return;
    }

    std::string shaderString{reinterpret_cast<const char*>(shaderData->bytes()),
                             shaderData->size()};

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
    if (const char* startPtr = strstr(shaderString.c_str(), kExpectedErrorsStart)) {
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

    // Compile the code.
    std::unique_ptr<SkSL::ShaderCaps> caps = SkSL::ShaderCapsFactory::Standalone();
    SkSL::Compiler compiler(caps.get());
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::ProgramKind::kFragment,
                                                                     std::move(shaderString),
                                                                     settings);

    // If the code actually generated a working program, we've already failed.
    if (program) {
        ERRORF(r, "%s: Expected failure, but compiled successfully",
                  SkOSPath::Basename(testFile).c_str());
        return;
    }

    // Verify that the SkSL compiler actually emitted the expected error messages.
    // The list of expectations isn't necessarily exhaustive, though.
    std::string reportedErrors = compiler.errorText();
    for (const std::string& expectedError : expectedErrors) {
        // If this error wasn't reported, trigger an error.
        size_t pos = reportedErrors.find(expectedError.c_str());
        if (pos == std::string::npos) {
            ERRORF(r, "%s: Expected an error that wasn't reported.\n    \"%s\"",
                   SkOSPath::Basename(testFile).c_str(), expectedError.c_str());
        } else {
            // We found the error that we expected to have. Remove that error from our report, and
            // everything preceding it as well. This ensures that we don't match the same error
            // twice, and that errors are reported in the order we expect.
            reportedErrors.erase(0, pos + expectedError.size());
        }
    }
}

static void iterate_dir(const char* directory, const std::function<void(const char*)>& run) {
    SkString resourceDirectory = GetResourcePath(directory);
    SkOSFile::Iter iter(resourceDirectory.c_str(), ".sksl");
    SkString name;

    while (iter.next(&name, /*getDir=*/false)) {
        SkString path(SkOSPath::Join(directory, name.c_str()));
        run(path.c_str());
    }
}

DEF_TEST(SkSLErrorTest, r) {
    iterate_dir("sksl/errors/", [&](const char* path) {
        test_expect_fail(r, path);
    });
}
