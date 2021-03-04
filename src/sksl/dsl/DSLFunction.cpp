/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLFunction.h"

#include "include/sksl/DSLVar.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLReturnStatement.h"

namespace SkSL {

namespace dsl {

void DSLFunction::init(const DSLType& returnType, const char* name,
                       std::vector<const DSLVar*> params) {
    std::vector<const Variable*> paramVars;
    paramVars.reserve(params.size());
    for (const DSLVar* param : params) {
        paramVars.push_back(&DSLWriter::Var(*param));
    }
    SkSL::SymbolTable& symbols = *DSLWriter::SymbolTable();
    fDecl = symbols.add(std::make_unique<SkSL::FunctionDeclaration>(
                                             /*offset=*/-1,
                                             DSLWriter::Modifiers(SkSL::Modifiers()),
                                             DSLWriter::Name(name),
                                             std::move(paramVars), fReturnType,
                                             /*builtin=*/false));
}

void DSLFunction::define(DSLBlock block) {
    SkASSERT(fDecl);
    const SkSL::FunctionDeclaration* decl = static_cast<const SkSL::FunctionDeclaration*>(fDecl);
    SkASSERTF(!decl->definition(), "function already defined");
    auto function = std::make_unique<SkSL::FunctionDefinition>(/*offset=*/-1, decl,
                                                               /*builtin=*/false, block.release());
    DSLWriter::IRGenerator().finalizeFunction(*function);
    if (DSLWriter::Compiler().errorCount()) {
        DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
        DSLWriter::Compiler().setErrorCount(0);
        SkASSERT(!DSLWriter::Compiler().errorCount());
    }
    DSLWriter::ProgramElements().push_back(std::move(function));
}

DSLExpression DSLFunction::call(ExpressionArray args) {
    return DSLWriter::Call(*static_cast<const SkSL::FunctionDeclaration*>(fDecl), std::move(args));
}

} // namespace dsl

} // namespace SkSL
