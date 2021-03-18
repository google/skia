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
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"

#if !SKSL_USE_THREAD_LOCAL
#include <pthread.h>
#endif // !SKSL_USE_THREAD_LOCAL

namespace SkSL {

namespace dsl {

DSLWriter::DSLWriter(SkSL::Compiler* compiler)
    : fCompiler(compiler) {
    SkSL::ParsedModule module = fCompiler->moduleForProgramKind(SkSL::ProgramKind::kFragment);
    fConfig.fKind = SkSL::ProgramKind::kFragment;

    SkSL::IRGenerator& ir = *fCompiler->fIRGenerator;
    fOldSymbolTable = ir.fSymbolTable;
    fOldConfig = fCompiler->fContext->fConfig;
    ir.fSymbolTable = module.fSymbols;
    fCompiler->fContext->fConfig = &fConfig;
    ir.pushSymbolTable();
    if (compiler->context().fCaps.useNodePools()) {
        fPool = Pool::Create();
        fPool->attachToThread();
    }
}

DSLWriter::~DSLWriter() {
    SkSL::IRGenerator& ir = *fCompiler->fIRGenerator;
    ir.fSymbolTable = fOldSymbolTable;
    fCompiler->fContext->fConfig = fOldConfig;
    fProgramElements.clear();
    if (fPool) {
        fPool->detachFromThread();
    }
}

SkSL::IRGenerator& DSLWriter::IRGenerator() {
    return *Compiler().fIRGenerator;
}

const SkSL::Context& DSLWriter::Context() {
    return IRGenerator().fContext;
}

const std::shared_ptr<SkSL::SymbolTable>& DSLWriter::SymbolTable() {
    return IRGenerator().fSymbolTable;
}

void DSLWriter::Reset() {
    IRGenerator().popSymbolTable();
    IRGenerator().pushSymbolTable();
    ProgramElements().clear();
}

const SkSL::Modifiers* DSLWriter::Modifiers(SkSL::Modifiers modifiers) {
    return IRGenerator().fModifiers->addToPool(modifiers);
}

const char* DSLWriter::Name(const char* name) {
    if (ManglingEnabled()) {
        const String* s = SymbolTable()->takeOwnershipOfString(
                Instance().fMangler.uniqueName(name, SymbolTable().get()));
        return s->c_str();
    }
    return name;
}

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
void DSLWriter::StartFragmentProcessor(GrGLSLFragmentProcessor* processor,
                                       GrGLSLFragmentProcessor::EmitArgs* emitArgs) {
    Instance().fStack.push({processor, emitArgs});
    IRGenerator().pushSymbolTable();
}

void DSLWriter::EndFragmentProcessor() {
    DSLWriter& instance = Instance();
    SkASSERT(!instance.fStack.empty());
    CurrentEmitArgs()->fFragBuilder->fDeclarations.reset();
    instance.fStack.pop();
    IRGenerator().popSymbolTable();
}

GrGLSLUniformHandler::UniformHandle DSLWriter::VarUniformHandle(const DSLVar& var) {
    return GrGLSLUniformHandler::UniformHandle(var.fUniformHandle);
}
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

std::unique_ptr<SkSL::Expression> DSLWriter::Call(const FunctionDeclaration& function,
                                                  ExpressionArray arguments) {
    return IRGenerator().call(/*offset=*/-1, function, std::move(arguments));
}

std::unique_ptr<SkSL::Expression> DSLWriter::Check(std::unique_ptr<SkSL::Expression> expr) {
    if (DSLWriter::Compiler().errorCount()) {
        DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
        DSLWriter::Compiler().setErrorCount(0);
    }
    return expr;
}

DSLPossibleExpression DSLWriter::Coerce(std::unique_ptr<Expression> left, const SkSL::Type& type) {
    return IRGenerator().coerce(std::move(left), type);
}

DSLPossibleExpression DSLWriter::Construct(const SkSL::Type& type,
                                           std::vector<DSLExpression> rawArgs) {
    SkSL::ExpressionArray args;
    args.reserve_back(rawArgs.size());

    for (DSLExpression& arg : rawArgs) {
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
                                                          const char* name) {
    return FieldAccess::Convert(Context(), std::move(base), name);
}

std::unique_ptr<SkSL::Expression> DSLWriter::ConvertIndex(std::unique_ptr<Expression> base,
                                                          std::unique_ptr<Expression> index) {
    return IndexExpression::Convert(Context(), std::move(base), std::move(index));
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
                                              SkTArray<SkSL::StatementArray> caseStatements) {
    StatementArray caseBlocks;
    caseBlocks.resize(caseStatements.count());
    for (int index = 0; index < caseStatements.count(); ++index) {
        caseBlocks[index] = std::make_unique<SkSL::Block>(/*offset=*/-1,
                                                          std::move(caseStatements[index]),
                                                          /*symbols=*/nullptr,
                                                          /*isScope=*/false);
    }

    return SwitchStatement::Convert(Context(), /*offset=*/-1, /*isStatic=*/false, std::move(value),
                                    std::move(caseValues), std::move(caseBlocks),
                                    IRGenerator().fSymbolTable);
}

void DSLWriter::ReportError(const char* msg, PositionInfo* info) {
    if (info && !info->file_name()) {
        info = nullptr;
    }
    if (Instance().fErrorHandler) {
        Instance().fErrorHandler->handleError(msg, info);
    } else if (info) {
        SK_ABORT("%s: %d: %sNo SkSL DSL error handler configured, treating this as a fatal error\n",
                 info->file_name(), info->line(), msg);
    } else {
        SK_ABORT("%sNo SkSL DSL error handler configured, treating this as a fatal error\n", msg);
    }
}

const SkSL::Variable& DSLWriter::Var(const DSLVar& var) {
    return *var.fVar;
}

void DSLWriter::MarkDeclared(DSLVar& var) {
    SkASSERT(!var.fDeclared);
    var.fDeclared = true;
}

#if !SK_SUPPORT_GPU || defined(SKSL_STANDALONE)

DSLWriter& DSLWriter::Instance() {
    SkUNREACHABLE;
}

void DSLWriter::SetInstance(std::unique_ptr<DSLWriter> instance) {
    SkDEBUGFAIL("unimplemented");
}

#elif SKSL_USE_THREAD_LOCAL

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
