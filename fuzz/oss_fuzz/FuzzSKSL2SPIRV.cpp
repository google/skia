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
#include "src/sksl/ir/SkSLProgram.h"

#include "fuzz/Fuzz.h"

bool FuzzSKSL2SPIRV(const uint8_t *data, size_t size) {
    SkSL::Compiler compiler(SkSL::ShaderCapsFactory::Default());
    SkSL::ProgramSettings settings;

    // This tells the compiler where the rt-flip uniform will live should it be required. For
    // fuzzing purposes we don't care where that is, but the compiler will report an error if we
    // leave them at their default invalid values, or if the offset overlaps another uniform.
    settings.fRTFlipOffset  = 16384;
    settings.fRTFlipSet     = 0;
    settings.fRTFlipBinding = 0;

    std::unique_ptr<SkSL::Program> program =
            compiler.convertProgram(SkSL::ProgramKind::kFragment,
                                    std::string(reinterpret_cast<const char*>(data), size),
                                    settings);
    std::string output;
    if (!program || !compiler.toSPIRV(*program, &output)) {
        return false;
    }
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    FuzzSKSL2SPIRV(data, size);
    return 0;
}
#endif
