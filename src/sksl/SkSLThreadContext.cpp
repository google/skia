/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLThreadContext.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <type_traits>

namespace SkSL {

ThreadContext::ThreadContext(SkSL::Context& context,
                             SkSL::ProgramKind kind,
                             const SkSL::ProgramSettings& settings,
                             const SkSL::Module* module,
                             bool isModule)
        : fContext(context)
        , fOldConfig(context.fConfig)
        , fOldErrorReporter(*context.fErrors)
        , fSettings(settings) {
    if (!isModule) {
        if (settings.fUseMemoryPool) {
            fPool = Pool::Create();
            fPool->attachToThread();
        }
    }

    fConfig = std::make_unique<SkSL::ProgramConfig>();
    fConfig->fKind = kind;
    fConfig->fSettings = settings;
    fConfig->fIsBuiltinCode = isModule;
    fContext.fConfig = fConfig.get();
    fContext.fErrors = &fDefaultErrorReporter;
    fContext.fModule = module;
    fContext.fSymbolTable = module->fSymbols;
    this->setupSymbolTable();
}

ThreadContext::~ThreadContext() {
    if (fContext.fSymbolTable) {
        fContext.fSymbolTable = nullptr;
    }
    fContext.fErrors = &fOldErrorReporter;
    fContext.fConfig = fOldConfig;
    if (fPool) {
        fPool->detachFromThread();
    }
}

void ThreadContext::Start(SkSL::Compiler* compiler,
                          SkSL::ProgramKind kind,
                          const SkSL::ProgramSettings& settings) {
    ThreadContext::SetInstance(
            std::unique_ptr<ThreadContext>(new ThreadContext(compiler->context(),
                                                             kind,
                                                             settings,
                                                             compiler->moduleForProgramKind(kind),
                                                             /*isModule=*/false)));
}

void ThreadContext::StartModule(SkSL::Compiler* compiler,
                          SkSL::ProgramKind kind,
                          const SkSL::ProgramSettings& settings,
                          const SkSL::Module* parent) {
    ThreadContext::SetInstance(std::unique_ptr<ThreadContext>(
            new ThreadContext(compiler->context(), kind, settings, parent, /*isModule=*/true)));
}

void ThreadContext::End() {
    ThreadContext::SetInstance(nullptr);
}

void ThreadContext::setupSymbolTable() {
    SymbolTable::Push(&fContext.fSymbolTable, fContext.fConfig->fIsBuiltinCode);

    fContext.fSymbolTable->markModuleBoundary();
}

void ThreadContext::SetErrorReporter(ErrorReporter* errorReporter) {
    SkASSERT(errorReporter);
    Context().fErrors = errorReporter;
}

void ThreadContext::ReportError(std::string_view msg, Position pos) {
    GetErrorReporter().error(pos, msg);
}

void ThreadContext::DefaultErrorReporter::handleError(std::string_view msg, Position pos) {
    SK_ABORT("error: %.*s\nNo SkSL error reporter configured, treating this as a fatal error\n",
             (int)msg.length(), msg.data());
}

static thread_local ThreadContext* sInstance = nullptr;

void ThreadContext::SetInstance(std::unique_ptr<ThreadContext> newInstance) {
    SkASSERT((sInstance == nullptr) != (newInstance == nullptr));
    delete sInstance;
    sInstance = newInstance.release();
}

ThreadContext& ThreadContext::Instance() {
    SkASSERTF(sInstance, "ThreadContext::Start() has not been called");
    return *sInstance;
}

} // namespace SkSL
