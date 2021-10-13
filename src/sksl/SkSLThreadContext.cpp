/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLThreadContext.h"

#include "include/sksl/DSLSymbols.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLIntrinsicMap.h"

namespace SkSL {

ThreadContext::ThreadContext(SkSL::Compiler* compiler, SkSL::ProgramKind kind,
        const SkSL::ProgramSettings& settings, SkSL::ParsedModule module, bool isModule)
    : fCompiler(compiler)
    , fOldErrorReporter(*fCompiler->fContext->fErrors)
    , fSettings(settings) {
    fOldModifiersPool = fCompiler->fContext->fModifiersPool;

    fOldConfig = fCompiler->fContext->fConfig;

    if (!isModule) {
        if (compiler->context().fCaps.useNodePools() && settings.fDSLUseMemoryPool) {
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
    fCompiler->fContext->fIntrinsics = module.fIntrinsics.get();
    if (fCompiler->fContext->fIntrinsics) {
        fCompiler->fContext->fIntrinsics->resetAlreadyIncluded();
    }

    fCompiler->fIRGenerator->start(module, &fProgramElements, &fSharedElements);
}

ThreadContext::~ThreadContext() {
    if (SymbolTable()) {
        fCompiler->fIRGenerator->finish();
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


SkSL::IRGenerator& ThreadContext::IRGenerator() {
    return *Compiler().fIRGenerator;
}

SkSL::Context& ThreadContext::Context() {
    return Compiler().context();
}

SkSL::ProgramSettings& ThreadContext::Settings() {
    return Context().fConfig->fSettings;
}

SkSL::Program::Inputs& ThreadContext::Inputs() {
    return IRGenerator().fInputs;
}

const std::shared_ptr<SkSL::SymbolTable>& ThreadContext::SymbolTable() {
    return IRGenerator().fSymbolTable;
}

const SkSL::Modifiers* ThreadContext::Modifiers(const SkSL::Modifiers& modifiers) {
    return Context().fModifiersPool->add(modifiers);
}

ThreadContext::RTAdjustData& ThreadContext::RTAdjustState() {
    return Instance().fRTAdjust;
}

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
void ThreadContext::StartFragmentProcessor(GrFragmentProcessor::ProgramImpl* processor,
        GrFragmentProcessor::ProgramImpl::EmitArgs* emitArgs) {
    ThreadContext& instance = ThreadContext::Instance();
    instance.fStack.push({processor, emitArgs, StatementArray{}});
    CurrentEmitArgs()->fFragBuilder->fDeclarations.swap(instance.fStack.top().fSavedDeclarations);
    dsl::PushSymbolTable();
}

void ThreadContext::EndFragmentProcessor() {
    ThreadContext& instance = Instance();
    SkASSERT(!instance.fStack.empty());
    CurrentEmitArgs()->fFragBuilder->fDeclarations.swap(instance.fStack.top().fSavedDeclarations);
    instance.fStack.pop();
    dsl::PopSymbolTable();
}
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

void ThreadContext::SetErrorReporter(ErrorReporter* errorReporter) {
    SkASSERT(errorReporter);
    Context().fErrors = errorReporter;
}

void ThreadContext::ReportError(skstd::string_view msg, PositionInfo info) {
    GetErrorReporter().error(msg, info);
}

void ThreadContext::DefaultErrorReporter::handleError(skstd::string_view msg, PositionInfo pos) {
    if (pos.line() > -1) {
        SK_ABORT("error: %s: %d: %.*sNo SkSL error reporter configured, treating this as a fatal "
                 "error\n", pos.file_name(), pos.line(), (int)msg.length(), msg.data());
    } else {
        SK_ABORT("error: %.*s\nNo SkSL error reporter configured, treating this as a fatal error\n",
                 (int)msg.length(), msg.data());
    }

}

void ThreadContext::ReportErrors(PositionInfo pos) {
    GetErrorReporter().reportPendingErrors(pos);
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
