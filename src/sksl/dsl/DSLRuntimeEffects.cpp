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
#include "src/sksl/SkSLThreadContext.h"

namespace SkSL {

namespace dsl {

#ifndef SKSL_STANDALONE

void StartRuntimeShader(SkSL::Compiler* compiler) {
    Start(compiler, SkSL::ProgramKind::kRuntimeShader);
    SkSL::ProgramSettings& settings = ThreadContext::Settings();
    SkASSERT(settings.fInlineThreshold == SkSL::kDefaultInlineThreshold);
    settings.fInlineThreshold = 0;
    SkASSERT(!settings.fAllowNarrowingConversions);
    settings.fAllowNarrowingConversions = true;
}

sk_sp<SkRuntimeEffect> EndRuntimeShader(SkRuntimeEffect::Options options) {
    std::unique_ptr<SkSL::Program> program = ReleaseProgram();
    ThreadContext::ReportErrors(PositionInfo{});
    sk_sp<SkRuntimeEffect> result;
    if (program) {
        result = SkRuntimeEffect::MakeForShader(std::move(program), options, &GetErrorReporter());
    }
    End();
    return result;
}

#endif // SKSL_STANDALONE

} // namespace dsl

} // namespace SkSL
