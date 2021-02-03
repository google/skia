/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/mock/GrMockCaps.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSLCore.h"

#if !SKSL_USE_THREAD_LOCAL
#include <pthread.h>
#endif // !SKSL_USE_THREAD_LOCAL

namespace SkSL {

namespace dsl {

DSLWriter::DSLWriter(SkSL::Compiler* compiler)
    : fCompiler(compiler) {
    SkSL::ParsedModule module = fCompiler->moduleForProgramKind(SkSL::Program::kFragment_Kind);
    SkSL::IRGenerator& ir = *fCompiler->fIRGenerator;
    ir.fSymbolTable = module.fSymbols;
    ir.fSettings = &fSettings;
    ir.pushSymbolTable();
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

const SkSL::Modifiers* DSLWriter::Modifiers(SkSL::Modifiers modifiers) {
    return IRGenerator().fModifiers->addToPool(modifiers);
}

const char* DSLWriter::Name(const char* name) {
    if (ManglingEnabled()) {
        auto mangled =
                std::make_unique<String>(Instance().fMangler.uniqueName(name, SymbolTable().get()));
        const SkSL::String* s = SymbolTable()->takeOwnershipOfString(std::move(mangled));
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
    instance.fStack.pop();
    IRGenerator().popSymbolTable();
}
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

std::unique_ptr<SkSL::Expression> DSLWriter::Check(std::unique_ptr<SkSL::Expression> expr) {
    if (expr == nullptr) {
        if (DSLWriter::Compiler().errorCount()) {
            DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
        }
    }
    return expr;
}

DSLExpression DSLWriter::Coerce(std::unique_ptr<Expression> left, const SkSL::Type& type) {
    return DSLExpression(Check(IRGenerator().coerce(std::move(left), type)));
}

DSLExpression DSLWriter::Construct(const SkSL::Type& type, std::vector<DSLExpression> rawArgs) {
    SkSL::ExpressionArray args;
    args.reserve_back(rawArgs.size());

    for (DSLExpression& arg : rawArgs) {
        args.push_back(arg.release());
    }
    return DSLExpression(DSLWriter::IRGenerator().call(
                                         /*offset=*/-1,
                                         std::make_unique<SkSL::TypeReference>(DSLWriter::Context(),
                                                                               /*offset=*/-1,
                                                                               &type),
                                         std::move(args)));
}

DSLExpression DSLWriter::ConvertBinary(std::unique_ptr<Expression> left, Token::Kind op,
                                       std::unique_ptr<Expression> right) {
    return DSLExpression(Check(IRGenerator().convertBinaryExpression(std::move(left), op,
                                                                     std::move(right))));
}

DSLExpression DSLWriter::ConvertIndex(std::unique_ptr<Expression> base,
                                      std::unique_ptr<Expression> index) {
    return DSLExpression(Check(IRGenerator().convertIndex(std::move(base), std::move(index))));
}

DSLExpression DSLWriter::ConvertPostfix(std::unique_ptr<Expression> expr, Token::Kind op) {
    return DSLExpression(Check(IRGenerator().convertPostfixExpression(std::move(expr), op)));
}

DSLExpression DSLWriter::ConvertPrefix(Token::Kind op, std::unique_ptr<Expression> expr) {
    return DSLExpression(Check(IRGenerator().convertPrefixExpression(op, std::move(expr))));
}

void DSLWriter::ReportError(const char* msg) {
    if (Instance().fErrorHandler) {
        Instance().fErrorHandler->handleError(msg);
    } else {
        SK_ABORT("%sNo SkSL DSL error handler configured, treating this as a fatal error\n", msg);
    }
}

const SkSL::Variable& DSLWriter::Var(const DSLVar& var) {
    return *var.var();
}

thread_local DSLWriter* writer_instance = nullptr;

} // namespace dsl

} // namespace SkSL
