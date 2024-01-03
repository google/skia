/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"

#include "fuzz/Fuzz.h"

bool FuzzSKSL2GLSL(const uint8_t *data, size_t size) {
    SkSL::Compiler compiler;
    SkSL::ProgramSettings settings;
    std::unique_ptr<SkSL::Program> program =
            compiler.convertProgram(SkSL::ProgramKind::kFragment,
                                    std::string(reinterpret_cast<const char*>(data), size),
                                    settings);
    std::string output;
    if (!program || !SkSL::ToGLSL(*program, SkSL::ShaderCapsFactory::Default(), &output)) {
        return false;
    }
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    FuzzSKSL2GLSL(data, size);
    return 0;
}
#endif
