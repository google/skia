/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/Var.h"

#include "src/sksl/SkSLUtil.h"
#include "src/sksl/dsl/Type.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace dsl {

Var::Var(const char* name)
    : fName(name) {}

Var::Var(Type type, const char* name)
    : Var(Modifiers(), type, name) {}

Var::Var(Modifiers modifiers, Type type, const char* name)
    : fName(DSLWriter::Instance().name(name)) {
#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    if (modifiers.fModifiers.fFlags & Modifiers::kUniform_Flag) {
        const SkSL::Type& skslType = type.skslType();
        GrSLType grslType;
        int count;
        if (skslType.typeKind() == SkSL::Type::TypeKind::kArray) {
            SkAssertResult(SkSL::type_to_grsltype(DSLWriter::Instance().context(),
                                                  skslType.componentType(),
                                                  &grslType));
            count = skslType.columns();
            SkASSERT(count > 0);
        } else {
            SkAssertResult(SkSL::type_to_grsltype(DSLWriter::Instance().context(), skslType,
                                                  &grslType));
            count = 0;
        }
        const char* name;
        SkASSERT(DSLWriter::Instance().currentEmitArgs());
        fUniformHandle = DSLWriter::Instance().currentEmitArgs()->fUniformHandler->addUniformArray(
                                                      &DSLWriter::Instance().currentEmitArgs()->fFp,
                                                      kFragment_GrShaderFlag,
                                                      grslType,
                                                      this->name().c_str(),
                                                      count,
                                                      &name);
        fName = name;
    }
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    fVar = DSLWriter::Instance().symbolTable()->add(std::make_unique<SkSL::Variable>(
                                              /*offset=*/-1,
                                              DSLWriter::Instance().modifiers(modifiers.fModifiers),
                                              SkSL::StringFragment(fName.c_str()),
                                              &type.skslType(),
                                              /*builtin=*/false,
                                              SkSL::Variable::Storage::kLocal));
}

const SkSL::Variable* Var::var() const {
    if (fVar) {
        return fVar;
    }
    SkSL::StringFragment name(fName.c_str());
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    if (name == "sk_SampleCoord") {
        name = DSLWriter::Instance().currentEmitArgs()->fSampleCoord;
    } else if (name == "sk_InColor") {
        name = DSLWriter::Instance().currentEmitArgs()->fInputColor;
    } else if (name == "sk_OutColor") {
        name = DSLWriter::Instance().currentEmitArgs()->fOutputColor;
    }
#endif
    const SkSL::Symbol* result = (*DSLWriter::Instance().symbolTable())[name];
    SkASSERTF(result, "could not find '%s' in symbol table", fName.c_str());
    return &result->as<SkSL::Variable>();
}

Expression Var::operator[](Expression&& index) {
    return Expression(std::make_unique<SkSL::IndexExpression>(
                               DSLWriter::Instance().context(),
                               Expression(*this).release(),
                               index.coerceAndRelease(*DSLWriter::Instance().context().fInt_Type)));
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

} // namespace dsl

} // namespace SkSL
