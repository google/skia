/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "tests/Test.h"

static void test(skiatest::Reporter* r,
                 const GrShaderCaps& caps,
                 const char* src,
                 SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkSL::String(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
        REPORTER_ASSERT(r, output != "");
        //SkDebugf("GLSL output:\n\n%s", output.c_str());
    }
}

DEF_TEST(SkSLGLSLTestbed, r) {
    // Add in your SkSL here.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             half4 main() {
                int x = 2;
                switch (x) {
                    case 1:  return half4(0, 0, 0, 1);
                    case 2:  return half4(0.5, 0.5, 0.5, 1);
                    case 3:  return half4(1);
                    default: return half4(1, 0, 0, 1);
                }
             }
         )__SkSL__");
}
