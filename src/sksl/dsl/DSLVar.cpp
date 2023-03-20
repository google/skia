/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLVar.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLSymbol.h"
#include "include/sksl/DSLModifiers.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/SkSLOperator.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <utility>

namespace SkSL {

namespace dsl {

/**
 * DSLVarBase
 */

DSLVarBase::DSLVarBase(VariableStorage storage, DSLType type, std::string_view name,
                       DSLExpression initialValue, Position pos, Position namePos)
    : DSLVarBase(storage, DSLModifiers(), std::move(type), name, std::move(initialValue),
                 pos, namePos) {}

DSLVarBase::DSLVarBase(VariableStorage storage, const DSLModifiers& modifiers, DSLType type,
                       std::string_view name, DSLExpression initialValue, Position pos,
                       Position namePos)
    : fModifiers(std::move(modifiers))
    , fType(std::move(type))
    , fNamePosition(namePos)
    , fName(name)
    , fInitialValue(std::move(initialValue))
    , fPosition(pos)
    , fStorage(storage) {}

void DSLVarBase::swap(DSLVarBase& other) {
    SkASSERT(this->storage() == other.storage());
    std::swap(fModifiers, other.fModifiers);
    std::swap(fType, other.fType);
    std::swap(fDeclaration, other.fDeclaration);
    std::swap(fVar, other.fVar);
    std::swap(fNamePosition, other.fNamePosition);
    std::swap(fName, other.fName);
    std::swap(fInitialValue.fExpression, other.fInitialValue.fExpression);
    std::swap(fInitialized, other.fInitialized);
    std::swap(fPosition, other.fPosition);
}

DSLExpression DSLVarBase::operator[](DSLExpression&& index) {
    return DSLExpression(*this)[std::move(index)];
}

DSLExpression DSLVarBase::assignExpression(DSLExpression expr) {
    return DSLExpression(BinaryExpression::Convert(ThreadContext::Context(), Position(),
            DSLExpression(*this, Position()).release(), SkSL::Operator::Kind::EQ,
            expr.release()));
}

/**
 * DSLVar
 */

DSLVar::DSLVar() : DSLVarBase(SkSL::VariableStorage::kLocal) {}

DSLVar::DSLVar(DSLType type, std::string_view name, DSLExpression initialValue,
               Position pos, Position namePos)
        : INHERITED(SkSL::VariableStorage::kLocal, type, name, std::move(initialValue),
                    pos, namePos) {}

DSLVar::DSLVar(const DSLModifiers& modifiers, DSLType type, std::string_view name,
               DSLExpression initialValue, Position pos, Position namePos)
        : INHERITED(SkSL::VariableStorage::kLocal, modifiers, type, name, std::move(initialValue),
                    pos, namePos) {}

void DSLVar::swap(DSLVar& other) {
    INHERITED::swap(other);
}

/**
 * DSLGlobalVar
 */

DSLGlobalVar::DSLGlobalVar() : DSLVarBase(SkSL::VariableStorage::kGlobal) {}

DSLGlobalVar::DSLGlobalVar(DSLType type, std::string_view name, DSLExpression initialValue,
                           Position pos, Position namePos)
        : INHERITED(SkSL::VariableStorage::kGlobal, type, name, std::move(initialValue),
                    pos, namePos) {}

DSLGlobalVar::DSLGlobalVar(const DSLModifiers& modifiers, DSLType type, std::string_view name,
                           DSLExpression initialValue, Position pos, Position namePos)
        : INHERITED(SkSL::VariableStorage::kGlobal, modifiers, type, name, std::move(initialValue),
                    pos, namePos) {}

DSLGlobalVar::DSLGlobalVar(const char* name)
    : INHERITED(SkSL::VariableStorage::kGlobal, kVoid_Type, name, DSLExpression(),
                Position(), Position()) {
    fName = name;
    SkSL::SymbolTable* symbolTable = ThreadContext::SymbolTable().get();
    SkSL::Symbol* result = symbolTable->findMutable(fName);
    SkASSERTF(result, "could not find '%.*s' in symbol table", (int)fName.length(), fName.data());
    fVar = &result->as<SkSL::Variable>();
    fInitialized = true;
}

void DSLGlobalVar::swap(DSLGlobalVar& other) {
    INHERITED::swap(other);
}

std::unique_ptr<SkSL::Expression> DSLGlobalVar::methodCall(std::string_view methodName,
                                                           Position pos) {
    if (!this->fType.isEffectChild()) {
        ThreadContext::ReportError("type does not support method calls", pos);
        return nullptr;
    }
    return FieldAccess::Convert(ThreadContext::Context(), pos, *ThreadContext::SymbolTable(),
            DSLExpression(*this, pos).release(), methodName);
}

DSLExpression DSLGlobalVar::eval(ExpressionArray args, Position pos) {
    auto method = this->methodCall("eval", pos);
    return DSLExpression(
            method ? SkSL::FunctionCall::Convert(ThreadContext::Context(), pos, std::move(method),
                                                 std::move(args))
                   : nullptr,
            pos);
}

DSLExpression DSLGlobalVar::eval(DSLExpression x, Position pos) {
    ExpressionArray converted;
    converted.push_back(x.release());
    return this->eval(std::move(converted), pos);
}

DSLExpression DSLGlobalVar::eval(DSLExpression x, DSLExpression y, Position pos) {
    ExpressionArray converted;
    converted.push_back(x.release());
    converted.push_back(y.release());
    return this->eval(std::move(converted), pos);
}

/**
 * DSLParameter
 */

DSLParameter::DSLParameter() : DSLVarBase(SkSL::VariableStorage::kParameter) {}

DSLParameter::DSLParameter(DSLType type, std::string_view name, Position pos, Position namePos)
        : INHERITED(SkSL::VariableStorage::kParameter, type, name, DSLExpression(), pos, namePos) {}

DSLParameter::DSLParameter(const DSLModifiers& modifiers, DSLType type, std::string_view name,
                           Position pos, Position namePos)
        : INHERITED(SkSL::VariableStorage::kParameter, modifiers, type, name, DSLExpression(),
                    pos, namePos) {}

void DSLParameter::swap(DSLParameter& other) {
    INHERITED::swap(other);
}

} // namespace dsl

} // namespace SkSL
