/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLType.h"

#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLStructDefinition.h"

namespace SkSL {

namespace dsl {

static const Type* find_type(skstd::string_view name) {
    const Symbol* symbol = (*DSLWriter::SymbolTable())[name];
    if (!symbol) {
        DSLWriter::ReportError(String::printf("no symbol named '%.*s'",
                                              (int)name.length(), name.data()).c_str());
        return nullptr;
    }
    if (!symbol->is<Type>()) {
        DSLWriter::ReportError(String::printf("symbol '%.*s' is not a type",
                                              (int)name.length(), name.data()).c_str());
        return nullptr;
    }
    return &symbol->as<Type>();
}

static const Type* find_type(skstd::string_view name, const Modifiers& modifiers) {
    const Type* type = find_type(name);
    if (!type) {
        return nullptr;
    }
    return type->applyPrecisionQualifiers(DSLWriter::Context(),
                                          modifiers,
                                          DSLWriter::SymbolTable().get(),
                                          /*offset=*/-1);
}

DSLType::DSLType(skstd::string_view name)
        : fSkSLType(find_type(name)) {}

DSLType::DSLType(skstd::string_view name, const DSLModifiers& modifiers)
        : fSkSLType(find_type(name, modifiers.fModifiers)) {}

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

const SkSL::Type& DSLType::skslType() const {
    if (fSkSLType) {
        return *fSkSLType;
    }
    const SkSL::Context& context = DSLWriter::Context();
    switch (fTypeConstant) {
        case kBool_Type:
            return *context.fTypes.fBool;
        case kBool2_Type:
            return *context.fTypes.fBool2;
        case kBool3_Type:
            return *context.fTypes.fBool3;
        case kBool4_Type:
            return *context.fTypes.fBool4;
        case kHalf_Type:
            return *context.fTypes.fHalf;
        case kHalf2_Type:
            return *context.fTypes.fHalf2;
        case kHalf3_Type:
            return *context.fTypes.fHalf3;
        case kHalf4_Type:
            return *context.fTypes.fHalf4;
        case kHalf2x2_Type:
            return *context.fTypes.fHalf2x2;
        case kHalf3x2_Type:
            return *context.fTypes.fHalf3x2;
        case kHalf4x2_Type:
            return *context.fTypes.fHalf4x2;
        case kHalf2x3_Type:
            return *context.fTypes.fHalf2x3;
        case kHalf3x3_Type:
            return *context.fTypes.fHalf3x3;
        case kHalf4x3_Type:
            return *context.fTypes.fHalf4x3;
        case kHalf2x4_Type:
            return *context.fTypes.fHalf2x4;
        case kHalf3x4_Type:
            return *context.fTypes.fHalf3x4;
        case kHalf4x4_Type:
            return *context.fTypes.fHalf4x4;
        case kFloat_Type:
            return *context.fTypes.fFloat;
        case kFloat2_Type:
            return *context.fTypes.fFloat2;
        case kFloat3_Type:
            return *context.fTypes.fFloat3;
        case kFloat4_Type:
            return *context.fTypes.fFloat4;
        case kFloat2x2_Type:
            return *context.fTypes.fFloat2x2;
        case kFloat3x2_Type:
            return *context.fTypes.fFloat3x2;
        case kFloat4x2_Type:
            return *context.fTypes.fFloat4x2;
        case kFloat2x3_Type:
            return *context.fTypes.fFloat2x3;
        case kFloat3x3_Type:
            return *context.fTypes.fFloat3x3;
        case kFloat4x3_Type:
            return *context.fTypes.fFloat4x3;
        case kFloat2x4_Type:
            return *context.fTypes.fFloat2x4;
        case kFloat3x4_Type:
            return *context.fTypes.fFloat3x4;
        case kFloat4x4_Type:
            return *context.fTypes.fFloat4x4;
        case kInt_Type:
            return *context.fTypes.fInt;
        case kInt2_Type:
            return *context.fTypes.fInt2;
        case kInt3_Type:
            return *context.fTypes.fInt3;
        case kInt4_Type:
            return *context.fTypes.fInt4;
        case kShader_Type:
            return *context.fTypes.fShader;
        case kShort_Type:
            return *context.fTypes.fShort;
        case kShort2_Type:
            return *context.fTypes.fShort2;
        case kShort3_Type:
            return *context.fTypes.fShort3;
        case kShort4_Type:
            return *context.fTypes.fShort4;
        case kUInt_Type:
            return *context.fTypes.fUInt;
        case kUInt2_Type:
            return *context.fTypes.fUInt2;
        case kUInt3_Type:
            return *context.fTypes.fUInt3;
        case kUInt4_Type:
            return *context.fTypes.fUInt4;
        case kUShort_Type:
            return *context.fTypes.fUShort;
        case kUShort2_Type:
            return *context.fTypes.fUShort2;
        case kUShort3_Type:
            return *context.fTypes.fUShort3;
        case kUShort4_Type:
            return *context.fTypes.fUShort4;
        case kVoid_Type:
            return *context.fTypes.fVoid;
        default:
            SkUNREACHABLE;
    }
}

DSLExpression DSLType::Construct(DSLType type, SkSpan<DSLExpression> argArray) {
    return DSLWriter::Construct(type.skslType(), std::move(argArray));
}

DSLType Array(const DSLType& base, int count) {
    if (count <= 0) {
        DSLWriter::ReportError("error: array size must be positive\n");
    }
    if (base.isArray()) {
        DSLWriter::ReportError("error: multidimensional arrays are not permitted\n");
        return base;
    }
    return DSLWriter::SymbolTable()->addArrayDimension(&base.skslType(), count);
}

DSLType Struct(skstd::string_view name, SkSpan<DSLField> fields) {
    std::vector<SkSL::Type::Field> skslFields;
    skslFields.reserve(fields.size());
    for (const DSLField& field : fields) {
        skslFields.emplace_back(field.fModifiers.fModifiers, field.fName, &field.fType.skslType());
    }
    const SkSL::Type* result = DSLWriter::SymbolTable()->add(Type::MakeStructType(/*offset=*/-1,
                                                                                  name,
                                                                                  skslFields));
    DSLWriter::ProgramElements().push_back(std::make_unique<SkSL::StructDefinition>(/*offset=*/-1,
                                                                                    *result));
    return result;
}

} // namespace dsl

} // namespace SkSL
