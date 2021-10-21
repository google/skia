/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"

#include "fuzz/Fuzz.h"

bool FuzzSKSL2Metal(sk_sp<SkData> bytes) {
    std::unique_ptr<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    SkSL::Compiler compiler(caps.get());
    SkSL::String output;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                    SkSL::ProgramKind::kFragment,
                                                    SkSL::String((const char*) bytes->data(),
                                                                 bytes->size()),
                                                    settings);
    if (!program || !compiler.toMetal(*program, &output)) {
        return false;
    }
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSKSL2Metal(bytes);
    return 0;
}
#endif
