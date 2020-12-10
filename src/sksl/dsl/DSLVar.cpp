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
    : fName(name) {}

DSLVar::DSLVar(DSLType type, const char* name)
    : DSLVar(DSLModifiers(), type, name) {}

DSLVar::DSLVar(DSLModifiers modifiers, DSLType type, const char* name)
    : fName(DSLWriter::Name(name)) {
#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    if (modifiers.fModifiers.fFlags & Modifiers::kUniform_Flag) {
        const SkSL::Type& skslType = type.skslType();
        GrSLType grslType;
        int count;
        if (skslType.typeKind() == SkSL::Type::TypeKind::kArray) {
            SkAssertResult(SkSL::type_to_grsltype(DSLWriter::Context(),
                                                  skslType.componentType(),
                                                  &grslType));
            count = skslType.columns();
            SkASSERT(count > 0);
        } else {
            SkAssertResult(SkSL::type_to_grsltype(DSLWriter::Context(), skslType,
                                                  &grslType));
            count = 0;
        }
        const char* name;
        SkASSERT(DSLWriter::CurrentEmitArgs());
        fUniformHandle = DSLWriter::CurrentEmitArgs()->fUniformHandler->addUniformArray(
                                                                 &DSLWriter::CurrentEmitArgs()->fFp,
                                                                 kFragment_GrShaderFlag,
                                                                 grslType,
                                                                 this->name().c_str(),
                                                                 count,
                                                                 &name);
        fName = name;
    }
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    fOwnedVar = std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                 DSLWriter::Modifiers(modifiers.fModifiers),
                                                 SkSL::StringFragment(fName.c_str()),
                                                 &type.skslType(),
                                                 /*builtin=*/false,
                                                 SkSL::Variable::Storage::kLocal);
    fVar = fOwnedVar.get();
}

const SkSL::Variable* DSLVar::var() const {
    if (fVar) {
        return fVar;
    }
    SkSL::StringFragment name(fName.c_str());
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    if (name == "sk_SampleCoord") {
        name = DSLWriter::CurrentEmitArgs()->fSampleCoord;
    } else if (name == "sk_InColor") {
        name = DSLWriter::CurrentEmitArgs()->fInputColor;
    } else if (name == "sk_OutColor") {
        name = DSLWriter::CurrentEmitArgs()->fOutputColor;
    }
#endif
    const SkSL::Symbol* result = (*DSLWriter::SymbolTable())[name];
    SkASSERTF(result, "could not find '%s' in symbol table", fName.c_str());
    return &result->as<SkSL::Variable>();
}

GrGLSLUniformHandler::UniformHandle DSLVar::uniformHandle() {
    SkASSERT(fVar->modifiers().fFlags & SkSL::Modifiers::kUniform_Flag);
    return fUniformHandle;
}

DSLExpression DSLVar::operator[](DSLExpression&& index) {
    return DSLExpression(std::make_unique<SkSL::IndexExpression>(
                                          DSLWriter::Context(),
                                          DSLExpression(*this).release(),
                                          index.coerceAndRelease(*DSLWriter::Context().fInt_Type)));
}

DSLExpression DSLVar::operator=(DSLExpression&& expr) {
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
