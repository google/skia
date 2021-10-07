/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLType.h"

#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLStructDefinition.h"

namespace SkSL {

namespace dsl {

static const SkSL::Type* verify_type(const Context& context,
                                     const SkSL::Type* type,
                                     bool allowPrivateTypes,
                                     PositionInfo pos) {
    if (!context.fConfig->fIsBuiltinCode) {
        if (!allowPrivateTypes && type->isPrivate()) {
            context.fErrors->error("type '" + String(type->name()) + "' is private", pos);
            return context.fTypes.fPoison.get();
        }
        if (!type->isAllowedInES2(context)) {
            context.fErrors->error("type '" + String(type->name()) + "' is not supported", pos);
            return context.fTypes.fPoison.get();
        }
    }
    return type;
}

static const SkSL::Type* find_type(const Context& context,
                                   skstd::string_view name,
                                   PositionInfo pos) {
    const Symbol* symbol = (*ThreadContext::SymbolTable())[name];
    if (!symbol) {
        context.fErrors->error(String::printf("no symbol named '%.*s'",
                                              (int)name.length(), name.data()), pos);
        return context.fTypes.fPoison.get();
    }
    if (!symbol->is<SkSL::Type>()) {
        context.fErrors->error(String::printf("symbol '%.*s' is not a type",
                                              (int)name.length(), name.data()), pos);
        return context.fTypes.fPoison.get();
    }
    const SkSL::Type* type = &symbol->as<SkSL::Type>();
    return verify_type(context, type, /*allowPrivateTypes=*/false, pos);
}

static const SkSL::Type* find_type(const Context& context,
                                   skstd::string_view name,
                                   Modifiers* modifiers,
                                   PositionInfo pos) {
    const Type* type = find_type(context, name, pos);
    type = type->applyPrecisionQualifiers(context, modifiers, ThreadContext::SymbolTable().get(),
                                          pos.line());
    ThreadContext::ReportErrors(pos);
    return type;
}

static const SkSL::Type* get_type_from_type_constant(const Context& context, TypeConstant tc) {
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

DSLType::DSLType(skstd::string_view name)
        : fSkSLType(find_type(ThreadContext::Context(), name, PositionInfo())) {}

DSLType::DSLType(skstd::string_view name, DSLModifiers* modifiers, PositionInfo position)
        : fSkSLType(find_type(ThreadContext::Context(), name, &modifiers->fModifiers, position)) {}

DSLType::DSLType(const SkSL::Type* type)
        : fSkSLType(verify_type(ThreadContext::Context(), type, /*allowPrivateTypes=*/true,
                                PositionInfo())) {}

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

const SkSL::Type& DSLType::skslType() const {
    if (fSkSLType) {
        return *fSkSLType;
    }
    const Context& context = ThreadContext::Context();
    return *verify_type(context,
                        get_type_from_type_constant(context, fTypeConstant),
                        /*allowPrivateTypes=*/true,
                        PositionInfo());
}

DSLPossibleExpression DSLType::Construct(DSLType type, SkSpan<DSLExpression> argArray) {
    SkSL::ExpressionArray skslArgs;
    skslArgs.reserve_back(argArray.size());

    for (DSLExpression& arg : argArray) {
        if (!arg.hasValue()) {
            return DSLPossibleExpression(nullptr);
        }
        skslArgs.push_back(arg.release());
    }
    return SkSL::Constructor::Convert(ThreadContext::Context(), /*line=*/-1, type.skslType(),
            std::move(skslArgs));
}

DSLType Array(const DSLType& base, int count, PositionInfo pos) {
    count = base.skslType().convertArraySize(ThreadContext::Context(),
            DSLExpression(count, pos).release());
    ThreadContext::ReportErrors(pos);
    if (!count) {
        return DSLType(kPoison_Type);
    }
    return ThreadContext::SymbolTable()->addArrayDimension(&base.skslType(), count);
}

DSLType Struct(skstd::string_view name, SkSpan<DSLField> fields, PositionInfo pos) {
    std::vector<SkSL::Type::Field> skslFields;
    skslFields.reserve(fields.size());
    for (const DSLField& field : fields) {
        if (field.fModifiers.fModifiers.fFlags != Modifiers::kNo_Flag) {
            String desc = field.fModifiers.fModifiers.description();
            desc.pop_back();  // remove trailing space
            ThreadContext::ReportError("modifier '" + desc + "' is not permitted on a struct field",
                    field.fPosition);
        }

        const SkSL::Type& type = field.fType.skslType();
        if (type.isOpaque()) {
            ThreadContext::ReportError("opaque type '" + type.displayName() +
                    "' is not permitted in a struct", field.fPosition);
        }
        skslFields.emplace_back(field.fModifiers.fModifiers, field.fName, &type);
    }
    const SkSL::Type* result = ThreadContext::SymbolTable()->add(Type::MakeStructType(pos.line(),
            name, skslFields));
    if (result->isTooDeeplyNested()) {
        ThreadContext::ReportError("struct '" + String(name) + "' is too deeply nested", pos);
    }
    ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::StructDefinition>(/*line=*/-1,
            *result));
    return result;
}

} // namespace dsl

} // namespace SkSL
