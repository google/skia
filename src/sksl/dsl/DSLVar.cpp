/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLVar.h"

#include "src/sksl/SkSLUtil.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace dsl {

DSLVar::DSLVar(const char* name)
    : fName(name) {}

DSLVar::DSLVar(DSLType type, const char* name)
    : fName(DSLWriter::Name(name)) {
    fOwnedVar = std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                 DSLWriter::Modifiers(Modifiers()),
                                                 fName,
                                                 &type.skslType(),
                                                 /*builtin=*/false,
                                                 SkSL::Variable::Storage::kLocal);
    fVar = fOwnedVar.get();
}

const SkSL::Variable* DSLVar::var() const {
    if (!fVar) {
        const SkSL::Symbol* result = (*DSLWriter::SymbolTable())[fName];
        SkASSERTF(result, "could not find '%s' in symbol table", fName);
        fVar = &result->as<SkSL::Variable>();
    }
    return fVar;
}

DSLExpression DSLVar::operator=(DSLExpression expr) {
    const SkSL::Variable* var = this->var();
    return DSLExpression(std::make_unique<SkSL::BinaryExpression>(
                /*offset=*/-1,
                std::make_unique<SkSL::VariableReference>(/*offset=*/-1,
                                                          var,
                                                          SkSL::VariableReference::RefKind::kWrite),
                SkSL::Token::Kind::TK_EQ,
                expr.coerceAndRelease(var->type()),
                &var->type()));
}

} // namespace dsl

} // namespace SkSL
