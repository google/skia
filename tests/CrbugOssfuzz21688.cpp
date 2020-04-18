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

DEF_TEST(crbug_ossfuzz_21688, r) {
    // Tests that an array of zero-sized interface blocks can be compiled without crashing.
    SkSL::Compiler compiler;
    SkSL::String output;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program =
        compiler.convertProgram(SkSL::Program::kFragment_Kind, "testBlock {} x[2];", settings);
    compiler.toSPIRV(*program, &output);
}
