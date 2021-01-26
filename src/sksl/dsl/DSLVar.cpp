/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLVar.h"

#include "src/sksl/SkSLUtil.h"
#include "src/sksl/dsl/DSLModifiers.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace dsl {

DSLVar::DSLVar(const char* name)
    : fName(name) {
    const SkSL::Symbol* result = (*DSLWriter::SymbolTable())[fName];
    SkASSERTF(result, "could not find '%s' in symbol table", fName);
    fVar = &result->as<SkSL::Variable>();
}

DSLVar::DSLVar(DSLType type, const char* name)
    : DSLVar(DSLModifiers(), std::move(type), name) {}

DSLVar::DSLVar(DSLModifiers modifiers, DSLType type, const char* name)
    : fName(DSLWriter::Name(name)) {
    DSLWriter::IRGenerator().checkVarDeclaration(/*offset=*/-1, modifiers.fModifiers,
                                                 &type.skslType(), Variable::Storage::kLocal);
    fDeclaration = DSLWriter::IRGenerator().convertVarDeclaration(/*offset=*/-1,
                                                                  modifiers.fModifiers,
                                                                  &type.skslType(),
                                                                  fName,
                                                                  /*isArray=*/false,
                                                                  /*arraySize=*/nullptr,
                                                                  /*value=*/nullptr,
                                                                  Variable::Storage::kLocal);
    fVar = &fDeclaration->as<SkSL::VarDeclaration>().var();
}

DSLExpression DSLVar::operator[](DSLExpression&& index) {
    return DSLExpression(*this)[std::move(index)];
}

DSLExpression DSLVar::operator=(DSLExpression expr) {
    return DSLWriter::ConvertBinary(DSLExpression(*this).release(), SkSL::Token::Kind::TK_EQ,
                                    expr.release());
}

} // namespace dsl

} // namespace SkSL
