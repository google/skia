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
    success = compiler.toDSLCPP(*program, "Test", output);
    if (!success) {
        SkDebugf("Unexpected error generating .cpp file for %s\n%s",
                 src, compiler.errorText().c_str());
    }
        SkDebugf("%s",output.str().c_str());
    REPORTER_ASSERT(r, success);
}

DEF_TEST(SkSLFPTestbed, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
layout(key) in half one;                       // always equals 1.0
layout(key, when one != 1.0f) in half unused;  // never true

half4 main() {
    half4 color = half4(0);

    // Basic if statement. (00 == 00: true --> color=0001)
    if (color.rg == color.ba) color.a = one;

    // Basic if statement with Block. (00 == 01: false)
    if (color.rg == color.ba) {
        color.r = color.a;
    }

    // TODO(skia:11872): Add test for If statement with comma-expression statement instead of Block.

    // Basic if-else statement. (0 == 0: true --> color=1011)
    if (color.r == color.g) color = color.araa; else color = color.rrra;

    // Chained if-else statements.
    if (color.r + color.g + color.b + color.a == one) {  // (3 == 1: false)
        color = half4(-1);
    } else if (color.r + color.g + color.b + color.a == 2) {  // (3 == 2: false)
        color = half4(-2);
    } else {
        color = color.ggaa; // (color=0011)
    }

    // Nested if-else statements.
    if (color.r == one) {  // (0 == 1: false)
        if (color.r == 2) {
            color = color.rrrr;
        } else {
            color = color.gggg;
        }
    } else {
        if (color.b * color.a == one) { // (1*1 == 1: true)
            color = color.rbga; // (color = 0101)
        } else {
            color = color.aaaa;
        }
    }

    return color;
}
         )__SkSL__");
}
