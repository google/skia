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
        REPORTER_ASSERT(r, compiler.toSPIRV(*program, &output));
    }
}

DEF_TEST(SkSLSPIRVTestbed, r) {
    // Add in your SkSL here.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(

struct A {
    int x;
    half y;
} a1, a2;
A a3, a4 = A(1, 2);

struct B {
    half x;
    float y[2];
    layout(binding=1) A z;
};
B b1, b2, b3;
B b4 = B(1, float[2](2, 3), A(4, 5));

void main() {
    a1.x = 0;
    b1.x = 0;
    sk_FragColor.r = half(a1.x) + half(b1.x) + a4.y + b4.x + A(1, 2).y;
}


         )__SkSL__");
}
