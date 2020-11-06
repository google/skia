/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/Var.h"

#include "src/sksl/dsl/Type.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace skslcode {

Var::Var(const char* name)
    : fName(name) {}

Var::Var(const SkSL::Type& type, const char* name)
    : fName(DSLWriter::Instance().name(name))
    , fVar(DSLWriter::Instance().symbolTable()->add(std::make_unique<SkSL::Variable>(
                                                 /*offset=*/-1,
                                                 DSLWriter::Instance().modifiers(SkSL::Modifiers()),
                                                 SkSL::StringFragment(fName.c_str()),
                                                 &type,
                                                 /*builtin=*/false,
                                                 SkSL::Variable::Storage::kLocal))) {
    DSLWriter::Instance().addVar(fVar);
}

const SkSL::Variable* Var::var() const {
    if (fVar) {
        return fVar;
    }
    SkSL::StringFragment name(fName.c_str());
    SkSL::ParsedModule module = DSLWriter::Instance().compiler().moduleForProgramKind(
            SkSL::Program::kFragment_Kind);
    const SkSL::Symbol* result = (*module.fSymbols)[name];
    SkASSERT(result);
    return &result->as<SkSL::Variable>();
}

Expression Var::operator=(Expression&& expr) {
    const SkSL::Variable* var = this->var();
    return Expression(std::make_unique<SkSL::BinaryExpression>(
                /*offset=*/-1,
                std::make_unique<SkSL::VariableReference>(/*offset=*/-1,
                                                          var,
                                                          SkSL::VariableReference::RefKind::kWrite),
                SkSL::Token::Kind::TK_EQ,
                expr.coerceAndRelease(var->type()),
                &var->type()));
}

} // namespace skslcode
