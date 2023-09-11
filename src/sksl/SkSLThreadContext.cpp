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
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <type_traits>

namespace SkSL {

ThreadContext::ThreadContext(SkSL::Compiler* compiler,
                             SkSL::ProgramKind kind,
                             const SkSL::ProgramSettings& settings,
                             const SkSL::Module* module,
                             bool isModule)
        : fCompiler(compiler)
        , fOldConfig(fCompiler->fContext->fConfig)
        , fOldErrorReporter(*fCompiler->fContext->fErrors)
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
    fCompiler->fContext->fConfig = fConfig.get();
    fCompiler->fContext->fErrors = &fDefaultErrorReporter;
    fCompiler->fContext->fModule = module;
    fCompiler->fContext->fSymbolTable = module->fSymbols;
    this->setupSymbolTable();
}

ThreadContext::~ThreadContext() {
    if (fCompiler->fContext->fSymbolTable) {
        fCompiler->fContext->fSymbolTable = nullptr;
        fProgramElements.clear();
    } else {
        // We should only be here with a null symbol table if ReleaseProgram was called
        SkASSERT(fProgramElements.empty());
    }
    fCompiler->fContext->fErrors = &fOldErrorReporter;
    fCompiler->fContext->fConfig = fOldConfig;
    if (fPool) {
        fPool->detachFromThread();
    }
}

void ThreadContext::Start(SkSL::Compiler* compiler,
                          SkSL::ProgramKind kind,
                          const SkSL::ProgramSettings& settings) {
    ThreadContext::SetInstance(
            std::unique_ptr<ThreadContext>(new ThreadContext(compiler,
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
            new ThreadContext(compiler, kind, settings, parent, /*isModule=*/true)));
}

void ThreadContext::End() {
    ThreadContext::SetInstance(nullptr);
}

void ThreadContext::setupSymbolTable() {
    SkSL::Context& context = *fCompiler->fContext;
    SymbolTable::Push(&context.fSymbolTable, context.fConfig->fIsBuiltinCode);

    context.fSymbolTable->markModuleBoundary();
}

SkSL::Context& ThreadContext::Context() {
    return Compiler().context();
}

ThreadContext::RTAdjustData& ThreadContext::RTAdjustState() {
    return Instance().fRTAdjust;
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
