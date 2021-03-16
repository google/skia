/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLVar.h"

#include "include/sksl/DSLModifiers.h"
#include "include/sksl/DSLType.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace dsl {

DSLVar::DSLVar(const char* name)
    : fDeclared(true)
    , fVar(nullptr)
    , fRawName(name)
    , fName(name) {
#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    if (!strcmp(name, "sk_SampleCoord")) {
        fName = DSLWriter::CurrentEmitArgs()->fSampleCoord;
        // The actual sk_SampleCoord variable hasn't been created by GrGLSLFPFragmentBuilder yet, so
        // if we attempt to look it up in the symbol table we'll get null. As we are currently
        // converting all DSL code into strings rather than nodes, all we really need is a
        // correctly-named variable with the right type, so we just create a placeholder for it.
        // TODO(skia/11330): we'll need to fix this when switching over to nodes.
        fVar = DSLWriter::SymbolTable()->takeOwnershipOfIRNode(
                       std::make_unique<SkSL::Variable>(
                                  /*offset=*/-1,
                                  DSLWriter::IRGenerator().fModifiers->addToPool(SkSL::Modifiers()),
                                  fName,
                                  DSLWriter::Context().fTypes.fFloat2.get(),
                                  /*builtin=*/true,
                                  SkSL::VariableStorage::kGlobal));
        return;
    }
#endif
    const SkSL::Symbol* result = (*DSLWriter::SymbolTable())[fName];
    SkASSERTF(result, "could not find '%s' in symbol table", fName);
    fVar = &result->as<SkSL::Variable>();
}

DSLVar::DSLVar(DSLType type, const char* name, DSLExpression initialValue)
    : DSLVar(DSLModifiers(), std::move(type), name, std::move(initialValue)) {}

DSLVar::DSLVar(DSLType type, DSLExpression initialValue)
    : DSLVar(type, "var", std::move(initialValue)) {}

DSLVar::DSLVar(DSLModifiers modifiers, DSLType type, DSLExpression initialValue)
    : DSLVar(modifiers, type, "var", std::move(initialValue)) {}

DSLVar::DSLVar(DSLModifiers modifiers, DSLType type, const char* name, DSLExpression initialValue)
    : fDeclared(DSLWriter::Instance().fMarkVarsDeclared)
    , fRawName(name)
    , fName(DSLWriter::Name(name)) {
    Variable::Storage storage = Variable::Storage::kLocal;
#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    if (modifiers.fModifiers.fFlags & Modifiers::kUniform_Flag) {
        storage = Variable::Storage::kGlobal;
        if (DSLWriter::InFragmentProcessor()) {
            const SkSL::Type& skslType = type.skslType();
            GrSLType grslType;
            int count;
            if (skslType.isArray()) {
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
                                                                 this->name(),
                                                                 count,
                                                                 &name).toIndex();
            fName = name;
        }
        fDeclared = true;
    }
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
    DSLWriter::IRGenerator().checkVarDeclaration(/*offset=*/-1, modifiers.fModifiers,
                                                 &type.skslType(), storage);
    std::unique_ptr<SkSL::Variable> var = DSLWriter::IRGenerator().convertVar(/*offset=*/-1,
                                                                              modifiers.fModifiers,
                                                                              &type.skslType(),
                                                                              fName,
                                                                              /*isArray=*/false,
                                                                              /*arraySize=*/nullptr,
                                                                              storage);
    fVar = var.get();
    fDeclaration = DSLWriter::IRGenerator().convertVarDeclaration(std::move(var),
                                                                  initialValue.release());
}

DSLVar::~DSLVar() {
    if (!fDeclared) {
        DSLWriter::ReportError(String::printf("error: variable '%s' was destroyed without being "
                                              "declared\n", fRawName).c_str());
    }
}

DSLPossibleExpression DSLVar::operator[](DSLExpression&& index) {
    return DSLExpression(*this)[std::move(index)];
}

DSLPossibleExpression DSLVar::operator=(DSLExpression expr) {
    return DSLWriter::ConvertBinary(DSLExpression(*this).release(), SkSL::Token::Kind::TK_EQ,
                                    expr.release());
}

} // namespace dsl

} // namespace SkSL
