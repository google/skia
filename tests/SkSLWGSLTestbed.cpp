/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "tests/Test.h"

#include <memory>
#include <string>

static void test(skiatest::Reporter* r,
                 const char* src,
                 SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
    SkSL::Compiler compiler;
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, std::string(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        std::string output;
        REPORTER_ASSERT(r, SkSL::ToWGSL(*program, SkSL::ShaderCapsFactory::Default(), &output));
        REPORTER_ASSERT(r, output != "");
        //SkDebugf("WGSL output:\n\n%s", output.c_str());
    }
}

DEF_TEST(SkSLWGSLTestbed, r) {
    // Add in your SkSL here.
    test(r,
         R"__SkSL__(
             void main() {
                 sk_FragColor = half4(0);
             }
         )__SkSL__");
}
