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
#include "src/sksl/SkSLModifiersPool.h"  // IWYU pragma: keep
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <type_traits>
#include <utility>
#include <vector>

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

class DSLCore {
public:
    static std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<std::string> source) {
        ThreadContext& instance = ThreadContext::Instance();
        SkSL::Compiler& compiler = *instance.fCompiler;
        Pool* pool = instance.fPool.get();
        auto result = std::make_unique<SkSL::Program>(std::move(source),
                                                      std::move(instance.fConfig),
                                                      compiler.fContext,
                                                      std::move(instance.fProgramElements),
                                                      std::move(instance.fSharedElements),
                                                      std::move(instance.fModifiersPool),
                                                      std::move(compiler.fContext->fSymbolTable),
                                                      std::move(instance.fPool),
                                                      instance.fInterface);
        bool success = false;
        if (!compiler.finalize(*result)) {
            // Do not return programs that failed to compile.
        } else if (!compiler.optimize(*result)) {
            // Do not return programs that failed to optimize.
        } else {
            // We have a successful program!
            success = true;
        }
        if (pool) {
            pool->detachFromThread();
        }
        SkASSERT(instance.fProgramElements.empty());
        SkASSERT(!compiler.fContext->fSymbolTable);
        return success ? std::move(result) : nullptr;
    }
};

std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<std::string> source) {
    return DSLCore::ReleaseProgram(std::move(source));
}

}  // namespace SkSL::dsl
