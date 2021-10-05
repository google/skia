/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLSymbols.h"
#include "include/sksl/SkSLErrorReporter.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/mock/GrMockCaps.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLIntrinsicMap.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"

namespace SkSL {

namespace dsl {

DSLWriter::DSLWriter(SkSL::Compiler* compiler, SkSL::ProgramKind kind,
                     const SkSL::ProgramSettings& settings, SkSL::ParsedModule module,
                     bool isModule)
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

DSLWriter::~DSLWriter() {
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


SkSL::IRGenerator& DSLWriter::IRGenerator() {
    return *Compiler().fIRGenerator;
}

SkSL::Context& DSLWriter::Context() {
    return Compiler().context();
}

SkSL::ProgramSettings& DSLWriter::Settings() {
    return Context().fConfig->fSettings;
}

SkSL::Program::Inputs& DSLWriter::Inputs() {
    return IRGenerator().fInputs;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::SymbolTable() {
    return IRGenerator().fSymbolTable;
}

void DSLWriter::Reset() {
    dsl::PopSymbolTable();
    dsl::PushSymbolTable();
    ProgramElements().clear();
    GetModifiersPool()->clear();
}

const SkSL::Modifiers* DSLWriter::Modifiers(const SkSL::Modifiers& modifiers) {
    return Context().fModifiersPool->add(modifiers);
}

skstd::string_view DSLWriter::Name(skstd::string_view name) {
    if (ManglingEnabled()) {
        const String* s = SymbolTable()->takeOwnershipOfString(
                Instance().fMangler.uniqueName(name, SymbolTable().get()));
        return s->c_str();
    }
    return name;
}

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
void DSLWriter::StartFragmentProcessor(GrFragmentProcessor::ProgramImpl* processor,
                                       GrFragmentProcessor::ProgramImpl::EmitArgs* emitArgs) {
    DSLWriter& instance = Instance();
    instance.fStack.push({processor, emitArgs, StatementArray{}});
    CurrentEmitArgs()->fFragBuilder->fDeclarations.swap(instance.fStack.top().fSavedDeclarations);
    dsl::PushSymbolTable();
}

void DSLWriter::EndFragmentProcessor() {
    DSLWriter& instance = Instance();
    SkASSERT(!instance.fStack.empty());
    CurrentEmitArgs()->fFragBuilder->fDeclarations.swap(instance.fStack.top().fSavedDeclarations);
    instance.fStack.pop();
    dsl::PopSymbolTable();
}

GrGLSLUniformHandler::UniformHandle DSLWriter::VarUniformHandle(const DSLGlobalVar& var) {
    return GrGLSLUniformHandler::UniformHandle(var.fUniformHandle);
}
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

void DSLWriter::AddVarDeclaration(DSLStatement& existing, DSLVar& additional) {
    if (existing.fStatement->is<Block>()) {
        SkSL::Block& block = existing.fStatement->as<Block>();
        SkASSERT(!block.isScope());
        block.children().push_back(Declare(additional).release());
    } else if (existing.fStatement->is<VarDeclaration>()) {
        StatementArray stmts;
        stmts.reserve_back(2);
        stmts.push_back(std::move(existing.fStatement));
        stmts.push_back(Declare(additional).release());
        existing.fStatement = SkSL::Block::MakeUnscoped(/*line=*/-1, std::move(stmts));
    } else if (existing.fStatement->isEmpty()) {
        // If the variable declaration generated an error, we can end up with a Nop statement here.
        existing.fStatement = Declare(additional).release();
    }
}

void DSLWriter::SetErrorReporter(ErrorReporter* errorReporter) {
    SkASSERT(errorReporter);
    Context().fErrors = errorReporter;
}

void DSLWriter::ReportError(skstd::string_view msg, PositionInfo info) {
    GetErrorReporter().error(msg, info);
}

void DSLWriter::DefaultErrorReporter::handleError(skstd::string_view msg, PositionInfo pos) {
    if (pos.line() > -1) {
        SK_ABORT("error: %s: %d: %.*sNo SkSL DSL error reporter configured, treating this as a "
                 "fatal error\n", pos.file_name(), pos.line(), (int)msg.length(), msg.data());
    } else {
        SK_ABORT("error: %.*s\nNo SkSL DSL error reporter configured, treating this as a fatal "
                 "error\n", (int)msg.length(), msg.data());
    }

}

const SkSL::Variable* DSLWriter::Var(DSLVarBase& var) {
    // fInitialized is true if we have attempted to create a var, whether or not we actually
    // succeeded. If it's true, we don't want to try again, to avoid reporting the same error
    // multiple times.
    if (!var.fInitialized) {
        // We haven't even attempted to create a var yet, so fVar ought to be null
        SkASSERT(!var.fVar);
        var.fInitialized = true;
        if (var.storage() != SkSL::VariableStorage::kParameter) {
            const SkSL::Type* baseType = &var.fType.skslType();
            if (baseType->isArray()) {
                baseType = &baseType->componentType();
            }
            DSLWriter::IRGenerator().checkVarDeclaration(var.fPosition.line(),
                                                         var.fModifiers.fModifiers,
                                                         baseType,
                                                         var.storage());
        }
        std::unique_ptr<SkSL::Variable> skslvar = DSLWriter::IRGenerator().convertVar(
                                                                          var.fPosition.line(),
                                                                          var.fModifiers.fModifiers,
                                                                          &var.fType.skslType(),
                                                                          var.fName,
                                                                          /*isArray=*/false,
                                                                          /*arraySize=*/nullptr,
                                                                          var.storage());
        SkSL::Variable* varPtr = skslvar.get();
        if (var.storage() != SkSL::VariableStorage::kParameter) {
            // We can't call VarDeclaration::Convert directly here, because the IRGenerator has
            // special treatment for sk_FragColor that we want to preserve in DSL. We also do not
            // want the variable added to the symbol table for several reasons - DSLParser handles
            // the symbol table itself, parameters don't go into the symbol table until after the
            // FunctionDeclaration is created which makes this the wrong spot for them, and outside
            // of DSLParser we don't even need DSL variables to show up in the symbol table in the
            // first place.
            var.fDeclaration = DSLWriter::IRGenerator().convertVarDeclaration(
                    std::move(skslvar), var.fInitialValue.releaseIfPossible(),
                    /*addToSymbolTable=*/false);
            if (var.fDeclaration) {
                var.fVar = varPtr;
                var.fInitialized = true;
            }
        }
        ReportErrors(var.fPosition);
    }
    return var.fVar;
}

std::unique_ptr<SkSL::Variable> DSLWriter::CreateParameterVar(DSLParameter& var) {
    // This should only be called on undeclared parameter variables, but we allow the creation to go
    // ahead regardless so we don't have to worry about null pointers potentially sneaking in and
    // breaking things. DSLFunction is responsible for reporting errors for invalid parameters.
    return DSLWriter::IRGenerator().convertVar(var.fPosition.line(), var.fModifiers.fModifiers,
                                               &var.fType.skslType(), var.fName, /*isArray=*/false,
                                               /*arraySize=*/nullptr, var.storage());
}

std::unique_ptr<SkSL::Statement> DSLWriter::Declaration(DSLVarBase& var) {
    Var(var);
    if (!var.fDeclaration) {
        // We should have already reported an error before ending up here, just clean up the
        // initial value so it doesn't assert and return a nop.
        var.fInitialValue.releaseIfPossible();
        return SkSL::Nop::Make();
    }
    return std::move(var.fDeclaration);
}

void DSLWriter::MarkDeclared(DSLVarBase& var) {
    SkASSERT(!var.fDeclared);
    var.fDeclared = true;
}

void DSLWriter::ReportErrors(PositionInfo pos) {
    GetErrorReporter().reportPendingErrors(pos);
}

thread_local DSLWriter* instance = nullptr;

bool DSLWriter::IsActive() {
    return instance != nullptr;
}

DSLWriter& DSLWriter::Instance() {
    SkASSERTF(instance, "dsl::Start() has not been called");
    return *instance;
}

void DSLWriter::SetInstance(std::unique_ptr<DSLWriter> newInstance) {
    SkASSERT((instance == nullptr) != (newInstance == nullptr));
    delete instance;
    instance = newInstance.release();
}

} // namespace dsl

} // namespace SkSL
