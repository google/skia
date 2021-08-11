/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLErrorHandling.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/mock/GrMockCaps.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"

#if !SKSL_USE_THREAD_LOCAL
#include <pthread.h>
#endif // !SKSL_USE_THREAD_LOCAL

namespace SkSL {

namespace dsl {

DSLWriter::DSLWriter(SkSL::Compiler* compiler, SkSL::ProgramKind kind,
                     const SkSL::ProgramSettings& settings, SkSL::ParsedModule module,
                     bool isModule)
    : fCompiler(compiler)
    , fSettings(settings)
    , fIsModule(isModule) {
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
    fCompiler->fContext->fConfig = fConfig.get();

    fCompiler->fIRGenerator->start(module, isModule, &fProgramElements, &fSharedElements);
}

DSLWriter::~DSLWriter() {
    if (SymbolTable()) {
        fCompiler->fIRGenerator->finish();
        fProgramElements.clear();
    } else {
        // We should only be here with a null symbol table if ReleaseProgram was called
        SkASSERT(fProgramElements.empty());
    }
    fCompiler->fContext->fConfig = fOldConfig;
    fCompiler->fContext->fModifiersPool = fOldModifiersPool;
    if (fPool) {
        fPool->detachFromThread();
    }
}

SkSL::IRGenerator& DSLWriter::IRGenerator() {
    return *Compiler().fIRGenerator;
}

const SkSL::Context& DSLWriter::Context() {
    return Compiler().context();
}

SkSL::ProgramSettings& DSLWriter::Settings() {
    return Context().fConfig->fSettings;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::SymbolTable() {
    return IRGenerator().fSymbolTable;
}

void DSLWriter::Reset() {
    IRGenerator().popSymbolTable();
    IRGenerator().pushSymbolTable();
    ProgramElements().clear();
    Instance().fModifiersPool->clear();
}

const SkSL::Modifiers* DSLWriter::Modifiers(const SkSL::Modifiers& modifiers) {
    return Context().fModifiersPool->add(modifiers);
}

skstd::string_view DSLWriter::Name(skstd::string_view name) {
    if (ManglingEnabled()) {
        const String* s = SymbolTable()->takeOwnershipOfString(
                Instance().fMangler.uniqueName(SkString(name).c_str(), SymbolTable().get()));
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
    IRGenerator().pushSymbolTable();
}

void DSLWriter::EndFragmentProcessor() {
    DSLWriter& instance = Instance();
    SkASSERT(!instance.fStack.empty());
    CurrentEmitArgs()->fFragBuilder->fDeclarations.swap(instance.fStack.top().fSavedDeclarations);
    instance.fStack.pop();
    IRGenerator().popSymbolTable();
}

GrGLSLUniformHandler::UniformHandle DSLWriter::VarUniformHandle(const DSLGlobalVar& var) {
    return GrGLSLUniformHandler::UniformHandle(var.fUniformHandle);
}
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

std::unique_ptr<SkSL::Expression> DSLWriter::Call(const FunctionDeclaration& function,
                                                  ExpressionArray arguments) {
    // We can't call FunctionCall::Convert directly here, because intrinsic management is handled in
    // IRGenerator::call.
    return IRGenerator().call(/*offset=*/-1, function, std::move(arguments));
}

std::unique_ptr<SkSL::Expression> DSLWriter::Call(std::unique_ptr<SkSL::Expression> expr,
                                                  ExpressionArray arguments) {
    // We can't call FunctionCall::Convert directly here, because intrinsic management is handled in
    // IRGenerator::call.
    return IRGenerator().call(/*offset=*/-1, std::move(expr), std::move(arguments));
}

DSLPossibleExpression DSLWriter::Coerce(std::unique_ptr<Expression> expr, const SkSL::Type& type) {
    return IRGenerator().coerce(std::move(expr), type);
}

DSLPossibleExpression DSLWriter::Construct(const SkSL::Type& type, SkSpan<DSLExpression> rawArgs) {
    SkSL::ExpressionArray args;
    args.reserve_back(rawArgs.size());

    for (DSLExpression& arg : rawArgs) {
        if (!arg.valid()) {
            return DSLPossibleExpression(nullptr);
        }
        args.push_back(arg.release());
    }
    return SkSL::Constructor::Convert(Context(), /*offset=*/-1, type, std::move(args));
}

std::unique_ptr<SkSL::Expression> DSLWriter::ConvertBinary(std::unique_ptr<Expression> left,
                                                           Operator op,
                                                           std::unique_ptr<Expression> right) {
    return BinaryExpression::Convert(Context(), std::move(left), op, std::move(right));
}

std::unique_ptr<SkSL::Expression> DSLWriter::ConvertField(std::unique_ptr<Expression> base,
                                                          skstd::string_view name) {
    return FieldAccess::Convert(Context(), std::move(base), name);
}

std::unique_ptr<SkSL::Expression> DSLWriter::ConvertIndex(std::unique_ptr<Expression> base,
                                                          std::unique_ptr<Expression> index) {
    return IndexExpression::Convert(Context(), *SymbolTable(), std::move(base), std::move(index));
}

std::unique_ptr<SkSL::Expression> DSLWriter::ConvertPostfix(std::unique_ptr<Expression> expr,
                                                            Operator op) {
    return PostfixExpression::Convert(Context(), std::move(expr), op);
}

std::unique_ptr<SkSL::Expression> DSLWriter::ConvertPrefix(Operator op,
                                                           std::unique_ptr<Expression> expr) {
    return PrefixExpression::Convert(Context(), op, std::move(expr));
}

DSLPossibleStatement DSLWriter::ConvertSwitch(std::unique_ptr<Expression> value,
                                              ExpressionArray caseValues,
                                              SkTArray<SkSL::StatementArray> caseStatements,
                                              bool isStatic) {
    StatementArray caseBlocks;
    caseBlocks.resize(caseStatements.count());
    for (int index = 0; index < caseStatements.count(); ++index) {
        caseBlocks[index] = std::make_unique<SkSL::Block>(/*offset=*/-1,
                                                          std::move(caseStatements[index]),
                                                          /*symbols=*/nullptr,
                                                          /*isScope=*/false);
    }

    return SwitchStatement::Convert(Context(), /*offset=*/-1, isStatic, std::move(value),
                                    std::move(caseValues), std::move(caseBlocks),
                                    IRGenerator().fSymbolTable);
}

void DSLWriter::ReportError(const char* msg, PositionInfo info) {
    Instance().fEncounteredErrors = true;
    if (Instance().fErrorHandler) {
        Instance().fErrorHandler->handleError(msg, info);
    } else if (info.file_name()) {
        SK_ABORT("%s: %d: %sNo SkSL DSL error handler configured, treating this as a fatal error\n",
                 info.file_name(), info.line(), msg);
    } else {
        SK_ABORT("%sNo SkSL DSL error handler configured, treating this as a fatal error\n", msg);
    }
}

const SkSL::Variable* DSLWriter::Var(DSLVarBase& var) {
    if (!var.fVar) {
        if (var.storage() != SkSL::VariableStorage::kParameter) {
            DSLWriter::IRGenerator().checkVarDeclaration(/*offset=*/-1, var.fModifiers.fModifiers,
                                                         &var.fType.skslType(), var.storage());
        }
        std::unique_ptr<SkSL::Variable> skslvar = DSLWriter::IRGenerator().convertVar(
                                                                          /*offset=*/-1,
                                                                          var.fModifiers.fModifiers,
                                                                          &var.fType.skslType(),
                                                                          var.fName,
                                                                          /*isArray=*/false,
                                                                          /*arraySize=*/nullptr,
                                                                          var.storage());
        SkSL::Variable* varPtr = skslvar.get();
        // We can't call VarDeclaration::Convert directly here, because the IRGenerator has special
        // treatment for sk_FragColor that we want to preserve in DSL. We also do not want the
        // variable added to the symbol table for several reasons - DSLParser handles the symbol
        // table itself, parameters don't go into the symbol table until after the
        // FunctionDeclaration is created which makes this the wrong spot for them, and outside of
        // DSLParser we don't even need DSL variables to show up in the symbol table in the first
        // place.
        var.fDeclaration = DSLWriter::IRGenerator().convertVarDeclaration(
                                                                 std::move(skslvar),
                                                                 var.fInitialValue.releaseIfValid(),
                                                                 /*addToSymbolTable=*/false);
        if (var.fDeclaration) {
            var.fVar = varPtr;
        }
        ReportErrors();
    }
    return var.fVar;
}

std::unique_ptr<SkSL::Variable> DSLWriter::CreateParameterVar(DSLParameter& var) {
    // This should only be called on undeclared parameter variables, but we allow the creation to go
    // ahead regardless so we don't have to worry about null pointers potentially sneaking in and
    // breaking things. DSLFunction is responsible for reporting errors for invalid parameters.
    return DSLWriter::IRGenerator().convertVar(/*offset=*/-1, var.fModifiers.fModifiers,
                                               &var.fType.skslType(), var.fName, /*isArray=*/false,
                                               /*arraySize=*/nullptr, var.storage());
}

std::unique_ptr<SkSL::Statement> DSLWriter::Declaration(DSLVarBase& var) {
    Var(var);
    if (!var.fDeclaration) {
        // We should have already reported an error before ending up here, just clean up the
        // initial value so it doesn't assert and return a nop.
        var.fInitialValue.releaseIfValid();
        return SkSL::Nop::Make();
    }
    return std::move(var.fDeclaration);
}

void DSLWriter::MarkDeclared(DSLVarBase& var) {
    SkASSERT(!var.fDeclared);
    var.fDeclared = true;
}

void DSLWriter::ReportErrors(PositionInfo pos) {
    if (Compiler().errorCount()) {
        ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str(), pos);
        Compiler().resetErrors();
    }
}

#if SKSL_USE_THREAD_LOCAL

thread_local DSLWriter* instance = nullptr;

DSLWriter& DSLWriter::Instance() {
    SkASSERTF(instance, "dsl::Start() has not been called");
    return *instance;
}

void DSLWriter::SetInstance(std::unique_ptr<DSLWriter> newInstance) {
    SkASSERT((instance == nullptr) != (newInstance == nullptr));
    delete instance;
    instance = newInstance.release();
}

#else

static void destroy_dslwriter(void* dslWriter) {
    delete static_cast<DSLWriter*>(dslWriter);
}

static pthread_key_t get_pthread_key() {
    static pthread_key_t sKey = []{
        pthread_key_t key;
        int result = pthread_key_create(&key, destroy_dslwriter);
        if (result != 0) {
            SK_ABORT("pthread_key_create failure: %d", result);
        }
        return key;
    }();
    return sKey;
}

DSLWriter& DSLWriter::Instance() {
    DSLWriter* instance = static_cast<DSLWriter*>(pthread_getspecific(get_pthread_key()));
    SkASSERTF(instance, "dsl::Start() has not been called");
    return *instance;
}

void DSLWriter::SetInstance(std::unique_ptr<DSLWriter> instance) {
    delete static_cast<DSLWriter*>(pthread_getspecific(get_pthread_key()));
    pthread_setspecific(get_pthread_key(), instance.release());
}
#endif

} // namespace dsl

} // namespace SkSL
