/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"

#include "fuzz/Fuzz.h"

bool FuzzSKSL2Pipeline(sk_sp<SkData> bytes) {
    SkSL::Compiler compiler;
    SkSL::String output;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                    SkSL::Program::kPipelineStage_Kind,
                                                    SkSL::String((const char*) bytes->data(),
                                                                 bytes->size()),
                                                    settings);
    std::vector<SkSL::Compiler::FormatArg> formatArgs;
    if (!program || !compiler.toPipelineStage(*program, &output, &formatArgs)) {
        return false;
    }
    return true;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKSL2Pipeline(bytes);
    return 0;
}
#endif
