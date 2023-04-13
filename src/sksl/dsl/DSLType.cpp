/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLType.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLString.h"
#include "include/private/SkSLSymbol.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#include <memory>
#include <string>
#include <vector>

namespace SkSL {

struct Modifiers;

namespace dsl {

static const SkSL::Type* verify_type(const Context& context,
                                     const SkSL::Type* type,
                                     bool allowGenericTypes,
                                     Position pos) {
    if (!context.fConfig->fIsBuiltinCode && type) {
        if (!allowGenericTypes && (type->isGeneric() || type->isLiteral())) {
            context.fErrors->error(pos, "type '" + std::string(type->name()) + "' is generic");
            return context.fTypes.fPoison.get();
        }
        if (!type->isAllowedInES2(context)) {
            context.fErrors->error(pos, "type '" + std::string(type->name()) +"' is not supported");
            return context.fTypes.fPoison.get();
        }
    }
    return type;
}

static const SkSL::Type* find_type(const Context& context,
                                   Position pos,
                                   std::string_view name) {
    const Symbol* symbol = ThreadContext::SymbolTable()->find(name);
    if (!symbol) {
        context.fErrors->error(pos, String::printf("no symbol named '%.*s'",
                                                   (int)name.length(), name.data()));
        return context.fTypes.fPoison.get();
    }
    if (!symbol->is<SkSL::Type>()) {
        context.fErrors->error(pos, String::printf("symbol '%.*s' is not a type",
                                                   (int)name.length(), name.data()));
        return context.fTypes.fPoison.get();
    }
    const SkSL::Type* type = &symbol->as<SkSL::Type>();
    return verify_type(context, type, /*allowGenericTypes=*/false, pos);
}

static const SkSL::Type* find_type(const Context& context,
                                   Position overallPos,
                                   std::string_view name,
                                   Position modifiersPos,
                                   Modifiers* modifiers) {
    const Type* type = find_type(context, overallPos, name);
    return type->applyQualifiers(context, modifiers, ThreadContext::SymbolTable().get(),
                                 modifiersPos);
}

static const SkSL::Type* get_type_from_type_constant(TypeConstant tc) {
    const Context& context = ThreadContext::Context();
    switch (tc) {
        case kBool_Type:
            return context.fTypes.fBool.get();
        case kBool2_Type:
            return context.fTypes.fBool2.get();
        case kBool3_Type:
            return context.fTypes.fBool3.get();
        case kBool4_Type:
            return context.fTypes.fBool4.get();
        case kHalf_Type:
            return context.fTypes.fHalf.get();
        case kHalf2_Type:
            return context.fTypes.fHalf2.get();
        case kHalf3_Type:
            return context.fTypes.fHalf3.get();
        case kHalf4_Type:
            return context.fTypes.fHalf4.get();
        case kHalf2x2_Type:
            return context.fTypes.fHalf2x2.get();
        case kHalf3x2_Type:
            return context.fTypes.fHalf3x2.get();
        case kHalf4x2_Type:
            return context.fTypes.fHalf4x2.get();
        case kHalf2x3_Type:
            return context.fTypes.fHalf2x3.get();
        case kHalf3x3_Type:
            return context.fTypes.fHalf3x3.get();
        case kHalf4x3_Type:
            return context.fTypes.fHalf4x3.get();
        case kHalf2x4_Type:
            return context.fTypes.fHalf2x4.get();
        case kHalf3x4_Type:
            return context.fTypes.fHalf3x4.get();
        case kHalf4x4_Type:
            return context.fTypes.fHalf4x4.get();
        case kFloat_Type:
            return context.fTypes.fFloat.get();
        case kFloat2_Type:
            return context.fTypes.fFloat2.get();
        case kFloat3_Type:
            return context.fTypes.fFloat3.get();
        case kFloat4_Type:
            return context.fTypes.fFloat4.get();
        case kFloat2x2_Type:
            return context.fTypes.fFloat2x2.get();
        case kFloat3x2_Type:
            return context.fTypes.fFloat3x2.get();
        case kFloat4x2_Type:
            return context.fTypes.fFloat4x2.get();
        case kFloat2x3_Type:
            return context.fTypes.fFloat2x3.get();
        case kFloat3x3_Type:
            return context.fTypes.fFloat3x3.get();
        case kFloat4x3_Type:
            return context.fTypes.fFloat4x3.get();
        case kFloat2x4_Type:
            return context.fTypes.fFloat2x4.get();
        case kFloat3x4_Type:
            return context.fTypes.fFloat3x4.get();
        case kFloat4x4_Type:
            return context.fTypes.fFloat4x4.get();
        case kInt_Type:
            return context.fTypes.fInt.get();
        case kInt2_Type:
            return context.fTypes.fInt2.get();
        case kInt3_Type:
            return context.fTypes.fInt3.get();
        case kInt4_Type:
            return context.fTypes.fInt4.get();
        case kShader_Type:
            return context.fTypes.fShader.get();
        case kShort_Type:
            return context.fTypes.fShort.get();
        case kShort2_Type:
            return context.fTypes.fShort2.get();
        case kShort3_Type:
            return context.fTypes.fShort3.get();
        case kShort4_Type:
            return context.fTypes.fShort4.get();
        case kUInt_Type:
            return context.fTypes.fUInt.get();
        case kUInt2_Type:
            return context.fTypes.fUInt2.get();
        case kUInt3_Type:
            return context.fTypes.fUInt3.get();
        case kUInt4_Type:
            return context.fTypes.fUInt4.get();
        case kUShort_Type:
            return context.fTypes.fUShort.get();
        case kUShort2_Type:
            return context.fTypes.fUShort2.get();
        case kUShort3_Type:
            return context.fTypes.fUShort3.get();
        case kUShort4_Type:
            return context.fTypes.fUShort4.get();
        case kVoid_Type:
            return context.fTypes.fVoid.get();
        case kPoison_Type:
            return context.fTypes.fPoison.get();
        default:
            SkUNREACHABLE;
    }
}

DSLType::DSLType(TypeConstant tc, Position pos)
        : fSkSLType(verify_type(ThreadContext::Context(),
                                get_type_from_type_constant(tc),
                                /*allowGenericTypes=*/false,
                                pos)) {}

DSLType::DSLType(std::string_view name, Position pos)
        : fSkSLType(find_type(ThreadContext::Context(), pos, name)) {}

DSLType::DSLType(std::string_view name, DSLModifiers* modifiers, Position pos)
        : fSkSLType(find_type(ThreadContext::Context(),
                              pos,
                              name,
                              modifiers->fPosition,
                              &modifiers->fModifiers)) {}

DSLType::DSLType(const SkSL::Type* type, Position pos)
        : fSkSLType(verify_type(ThreadContext::Context(), type, /*allowGenericTypes=*/true, pos)) {}

DSLType DSLType::Invalid() {
    return DSLType(ThreadContext::Context().fTypes.fInvalid.get(), Position());
}

bool DSLType::isBoolean() const {
    return this->skslType().isBoolean();
}

bool DSLType::isNumber() const {
    return this->skslType().isNumber();
}

bool DSLType::isFloat() const {
    return this->skslType().isFloat();
}

bool DSLType::isSigned() const {
    return this->skslType().isSigned();
}

bool DSLType::isUnsigned() const {
    return this->skslType().isUnsigned();
}

bool DSLType::isInteger() const {
    return this->skslType().isInteger();
}

bool DSLType::isScalar() const {
    return this->skslType().isScalar();
}

bool DSLType::isVector() const {
    return this->skslType().isVector();
}

bool DSLType::isMatrix() const {
    return this->skslType().isMatrix();
}

bool DSLType::isArray() const {
    return this->skslType().isArray();
}

bool DSLType::isStruct() const {
    return this->skslType().isStruct();
}

bool DSLType::isEffectChild() const {
    return this->skslType().isEffectChild();
}

DSLExpression DSLType::Construct(DSLType type, SkSpan<DSLExpression> argArray) {
    SkSL::ExpressionArray skslArgs;
    skslArgs.reserve_back(argArray.size());

    for (DSLExpression& arg : argArray) {
        if (!arg.hasValue()) {
            return DSLExpression();
        }
        skslArgs.push_back(arg.release());
    }
    return DSLExpression(SkSL::Constructor::Convert(ThreadContext::Context(), Position(),
                                                    type.skslType(), std::move(skslArgs)));
}

DSLType Array(const DSLType& base, int count, Position pos) {
    count = base.skslType().convertArraySize(ThreadContext::Context(), pos,
                                             DSLExpression(count, pos).release());
    if (!count) {
        return DSLType(kPoison_Type);
    }
    return DSLType(ThreadContext::SymbolTable()->addArrayDimension(&base.skslType(), count), pos);
}

DSLType UnsizedArray(const DSLType& base, Position pos) {
    if (!base.skslType().checkIfUsableInArray(ThreadContext::Context(), pos)) {
        return DSLType(kPoison_Type);
    }
    return ThreadContext::SymbolTable()->addArrayDimension(&base.skslType(),
                                                           SkSL::Type::kUnsizedArray);
}

DSLType StructType(std::string_view name,
                   SkSpan<DSLField> fields,
                   bool interfaceBlock,
                   Position pos) {
    std::vector<SkSL::Type::Field> skslFields;
    skslFields.reserve(fields.size());
    for (const DSLField& field : fields) {
        skslFields.emplace_back(field.fPosition, field.fModifiers.fModifiers, field.fName,
                                &field.fType.skslType());
    }
    std::unique_ptr<Type> newType = Type::MakeStructType(ThreadContext::Context(), pos, name,
                                                         std::move(skslFields), interfaceBlock);
    return DSLType(ThreadContext::SymbolTable()->add(std::move(newType)), pos);
}

DSLType Struct(std::string_view name, SkSpan<DSLField> fields, Position pos) {
    DSLType result = StructType(name, fields, /*interfaceBlock=*/false, pos);
    ThreadContext::ProgramElements().push_back(
            std::make_unique<SkSL::StructDefinition>(pos, result.skslType()));
    return result;
}

} // namespace dsl

} // namespace SkSL
