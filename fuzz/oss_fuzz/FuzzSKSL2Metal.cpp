/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShaderCaps.h"
#include "SkSLCompiler.h"

#include "../Fuzz.h"

bool FuzzSKSL2Metal(sk_sp<SkData> bytes) {
    SkSL::Compiler compiler;
    SkSL::String output;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                    SkSL::Program::kFragment_Kind,
                                                    SkSL::String((const char*) bytes->data(),
                                                                 bytes->size()),
                                                    settings);
    if (!program || !compiler.toMetal(*program, &output)) {
        return false;
    }
    return true;
}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKSL2Metal(bytes);
    return 0;
}
#endif
