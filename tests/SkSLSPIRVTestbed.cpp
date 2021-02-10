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
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkSL::String(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        REPORTER_ASSERT(r, program);
    } else {
        REPORTER_ASSERT(r, compiler.toSPIRV(*program, &output));
    }
}

DEF_TEST(SkSLSPIRVTestbed, r) {
    // Add in your SkSL here.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
uniform half4 colorGreen, colorRed;

half2 tricky(half x, half y, inout half2 color, half z) {
    color.xy = color.yx;
    return half2(x + y, z);
}

void func(inout half4 color) {
    half2 t = tricky(1, 2, color.rb, 5);
    color.ga = t;
}

half4 main() {
    half4 result = half4(0, 1, 2, 3);
    func(result);
    return result == half4(2, 3, 0, 5) ? colorGreen : colorRed;
}
         )__SkSL__");
}
