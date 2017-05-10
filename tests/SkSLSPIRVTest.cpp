/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

#if SK_SUPPORT_GPU

static void test_failure(skiatest::Reporter* r, const char* src, const char* error) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kFragment_Kind,
                                                                     SkString(src), settings);
    if (program) {
        SkSL::String ignored;
        compiler.toSPIRV(*program, &ignored);
    }
    SkSL::String skError(error);
    if (compiler.errorText() != skError) {
        SkDebugf("SKSL ERROR:\n    source: %s\n    expected: %s    received: %s", src, error,
                 compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, compiler.errorText() == skError);
}

DEF_TEST(SkSLBadOffset, r) {
    test_failure(r,
                 "struct Bad { layout (offset = 5) int x; } bad; void main() { bad.x = 5; }",
                 "error: 1: offset of field 'x' must be a multiple of 4\n1 error\n");
    test_failure(r,
                 "struct Bad { int x; layout (offset = 0) int y; } bad; void main() { bad.x = 5; }",
                 "error: 1: offset of field 'y' must be at least 4\n1 error\n");
}

#endif
