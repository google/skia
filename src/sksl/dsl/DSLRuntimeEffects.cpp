/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLRuntimeEffects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/sksl/DSLCore.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

namespace dsl {

#ifndef SKSL_STANDALONE

void StartRuntimeEffect(SkSL::Compiler* compiler) {
    Start(compiler, SkSL::ProgramKind::kFragmentProcessor);
}

sk_sp<SkRuntimeEffect> EndRuntimeEffect() {
    std::unique_ptr<SkSL::Program> program = DSLWriter::ReleaseProgram();
    auto result = SkRuntimeEffect::Make(std::move(program));
    SkASSERTF(result.effect, "%s\n", result.errorText.c_str());
    End();
    return result.effect;
}

#endif // SKSL_STANDALONE

} // namespace dsl

} // namespace SkSL
