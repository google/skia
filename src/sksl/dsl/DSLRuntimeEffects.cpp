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

void StartRuntimeShader(SkSL::Compiler* compiler) {
    Start(compiler, SkSL::ProgramKind::kRuntimeShader);
    SkSL::ProgramSettings& settings = DSLWriter::IRGenerator().fContext.fConfig->fSettings;
    SkASSERT(settings.fInlineThreshold == SkSL::kDefaultInlineThreshold);
    settings.fInlineThreshold = 0;
    SkASSERT(!settings.fAllowNarrowingConversions);
    settings.fAllowNarrowingConversions = true;
}

sk_sp<SkRuntimeEffect> EndRuntimeShader() {
    std::unique_ptr<SkSL::Program> program = DSLWriter::ReleaseProgram();
    auto result = SkRuntimeEffect::MakeForShader(std::move(program));
    // TODO(skbug.com/11862): propagate errors properly
    SkASSERTF(result.effect, "%s\n", result.errorText.c_str());
    SkSL::ProgramSettings& settings = DSLWriter::IRGenerator().fContext.fConfig->fSettings;
    settings.fInlineThreshold = SkSL::kDefaultInlineThreshold;
    settings.fAllowNarrowingConversions = false;
    End();
    return result.effect;
}

#endif // SKSL_STANDALONE

} // namespace dsl

} // namespace SkSL
