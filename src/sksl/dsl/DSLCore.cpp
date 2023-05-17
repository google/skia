/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLCore.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"

#include <memory>

using namespace skia_private;

namespace SkSL::dsl {

void Start(SkSL::Compiler* compiler, ProgramKind kind, const ProgramSettings& settings) {
    ThreadContext::SetInstance(std::make_unique<ThreadContext>(compiler, kind, settings,
                                                               compiler->moduleForProgramKind(kind),
                                                               /*isModule=*/false));
}

void StartModule(SkSL::Compiler* compiler,
                 ProgramKind kind,
                 const ProgramSettings& settings,
                 const SkSL::Module* parent) {
    ThreadContext::SetInstance(std::make_unique<ThreadContext>(compiler, kind, settings,
                                                               parent, /*isModule=*/true));
}

void End() {
    ThreadContext::SetInstance(nullptr);
}

ErrorReporter& GetErrorReporter() {
    return ThreadContext::GetErrorReporter();
}

void SetErrorReporter(ErrorReporter* errorReporter) {
    SkASSERT(errorReporter);
    ThreadContext::SetErrorReporter(errorReporter);
}

}  // namespace SkSL::dsl
