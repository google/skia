/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLStringStream.h"

#include "tests/Test.h"

static void test(skiatest::Reporter* r, const GrShaderCaps& caps, const char* src) {
    SkSL::Program::Settings settings;
    settings.fRemoveDeadFunctions = false;
    SkSL::Compiler compiler(&caps);
    SkSL::StringStream output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::ProgramKind::kFragmentProcessor,
                                                             SkSL::String(src),
                                                             settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        return;
    }
    REPORTER_ASSERT(r, program);
    bool success = compiler.toH(*program, "Test", output);
    if (!success) {
        SkDebugf("Unexpected error generating .h file for %s\n%s",
                 src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, success);
    output.reset();
    success = compiler.toCPP(*program, "Test", output);
    if (!success) {
        SkDebugf("Unexpected error generating .cpp file for %s\n%s",
                 src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, success);
}

DEF_TEST(SkSLFPTestbed, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             half4 main(float2 coord) {
                 return half4(0);
             }
         )__SkSL__");
}
