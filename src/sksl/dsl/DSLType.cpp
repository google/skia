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
        case kFragmentProcessor_Type:
            return *context.fTypes.fFragmentProcessor;
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
        case kVoid_Type:
            return *context.fTypes.fVoid;
        default:
            SkUNREACHABLE;
    }
}

DSLExpression DSLType::Construct(TypeConstant type, SkTArray<DSLExpression> argArray) {
    return DSLWriter::Construct(DSLType(type).skslType(), std::move(argArray));
}

DSLType Array(const DSLType& base, int count) {
    SkASSERT(count >= 1);
    return DSLWriter::SymbolTable()->addArrayDimension(&base.skslType(), count);
}

DSLType Struct(const char* name, SkTArray<DSLField> fields) {
    std::vector<SkSL::Type::Field> skslFields;
    skslFields.reserve(fields.count());
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
