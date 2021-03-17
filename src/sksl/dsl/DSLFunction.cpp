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
                       std::vector<DSLVar*> params) {
    std::vector<const Variable*> paramVars;
    paramVars.reserve(params.size());
    for (DSLVar* param : params) {
        // This counts as declaring the variable; make sure it hasn't been previously declared and
        // then kill its pending declaration statement. Otherwise the statement will hang around
        // until after the Var is destroyed, which is probably after the End() call and therefore
        // after the Pool's destruction. Freeing a pooled object after the Pool's destruction is a
        // Bad Thing.
        if (param->fDeclared) {
            DSLWriter::ReportError("error: using an already-declared variable as a function "
                                   "parameter\n");
        }
        if (param->fDeclaration && param->fDeclaration->as<VarDeclaration>().value()) {
            DSLWriter::ReportError("error: variables used as function parameters cannot have "
                                   "initial values\n");
        }
        param->fDeclared = true;
        param->fDeclaration = nullptr;
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
    SkASSERTF(!decl->definition(), "function '%s' already defined", decl->description().c_str());
    std::unique_ptr<Statement> body = block.release();
    DSLWriter::IRGenerator().finalizeFunction(*decl, body.get());
    auto function = std::make_unique<SkSL::FunctionDefinition>(/*offset=*/-1, decl,
                                                               /*builtin=*/false, std::move(body));
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
