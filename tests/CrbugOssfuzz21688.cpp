/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include <memory>

#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLProgram.h"

DEF_TEST(crbug_ossfuzz_21688_interfaceblock, reporter) {
    // Tests that arrays of zero-sized interface blocks are disallowed.
    SkSL::Compiler compiler;
    SkSL::String output;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    const char* const kProgramText = "testBlock {} x[2];";
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kFragment_Kind,
                                                                     kProgramText, settings);
    REPORTER_ASSERT(reporter, program == nullptr);
    REPORTER_ASSERT(reporter, compiler.errorText().find(
                                    "interface block 'testBlock' must "
                                    "contain at least one member") != SkSL::String::npos);
}
