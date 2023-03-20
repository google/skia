/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLThreadContext.h"

#include "include/private/SkSLProgramElement.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLPool.h"
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
        , fOldModifiersPool(fCompiler->fContext->fModifiersPool)
        , fOldErrorReporter(*fCompiler->fContext->fErrors)
        , fSettings(settings) {
    if (!isModule) {
        if (settings.fUseMemoryPool) {
            fPool = Pool::Create();
            fPool->attachToThread();
        }
        fModifiersPool = std::make_unique<SkSL::ModifiersPool>();
        fCompiler->fContext->fModifiersPool = fModifiersPool.get();
    }

    fConfig = std::make_unique<SkSL::ProgramConfig>();
    fConfig->fKind = kind;
    fConfig->fSettings = settings;
    fConfig->fIsBuiltinCode = isModule;
    fCompiler->fContext->fConfig = fConfig.get();
    fCompiler->fContext->fErrors = &fDefaultErrorReporter;
    fCompiler->fContext->fModule = module;
    fCompiler->fSymbolTable = module->fSymbols;
    this->setupSymbolTable();
}

ThreadContext::~ThreadContext() {
    if (SymbolTable()) {
        fCompiler->fSymbolTable = nullptr;
        fProgramElements.clear();
    } else {
        // We should only be here with a null symbol table if ReleaseProgram was called
        SkASSERT(fProgramElements.empty());
    }
    fCompiler->fContext->fErrors = &fOldErrorReporter;
    fCompiler->fContext->fConfig = fOldConfig;
    fCompiler->fContext->fModifiersPool = fOldModifiersPool;
    if (fPool) {
        fPool->detachFromThread();
    }
}

void ThreadContext::setupSymbolTable() {
    SkSL::Context& context = *fCompiler->fContext;
    SymbolTable::Push(&fCompiler->fSymbolTable, context.fConfig->fIsBuiltinCode);

    SkSL::SymbolTable& symbolTable = *fCompiler->fSymbolTable;
    symbolTable.markModuleBoundary();
}

SkSL::Context& ThreadContext::Context() {
    return Compiler().context();
}

const SkSL::ProgramSettings& ThreadContext::Settings() {
    return Context().fConfig->fSettings;
}

std::shared_ptr<SkSL::SymbolTable>& ThreadContext::SymbolTable() {
    return Compiler().fSymbolTable;
}

const SkSL::Modifiers* ThreadContext::Modifiers(const SkSL::Modifiers& modifiers) {
    return Context().fModifiersPool->add(modifiers);
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

thread_local ThreadContext* instance = nullptr;

bool ThreadContext::IsActive() {
    return instance != nullptr;
}

ThreadContext& ThreadContext::Instance() {
    SkASSERTF(instance, "dsl::Start() has not been called");
    return *instance;
}

void ThreadContext::SetInstance(std::unique_ptr<ThreadContext> newInstance) {
    SkASSERT((instance == nullptr) != (newInstance == nullptr));
    delete instance;
    instance = newInstance.release();
}

} // namespace SkSL
