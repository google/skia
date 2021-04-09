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
        case kInt_Type:
            return *context.fTypes.fInt;
        case kInt2_Type:
            return *context.fTypes.fInt2;
        case kInt3_Type:
            return *context.fTypes.fInt3;
        case kInt4_Type:
            return *context.fTypes.fInt4;
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

static DSLExpression construct1(const SkSL::Type& type, DSLExpression a) {
    std::vector<DSLExpression> args;
    args.push_back(std::move(a));
    return DSLWriter::Construct(type, std::move(args));
}

static DSLExpression construct2(const SkSL::Type& type, DSLExpression a,
                                DSLExpression b) {
    std::vector<DSLExpression> args;
    args.push_back(std::move(a));
    args.push_back(std::move(b));
    return DSLWriter::Construct(type, std::move(args));
}

static DSLExpression construct3(const SkSL::Type& type, DSLExpression a,
                                DSLExpression b,
                                DSLExpression c) {
    std::vector<DSLExpression> args;
    args.push_back(std::move(a));
    args.push_back(std::move(b));
    args.push_back(std::move(c));
    return DSLWriter::Construct(type, std::move(args));
}

static DSLExpression construct4(const SkSL::Type& type, DSLExpression a, DSLExpression b,
                                DSLExpression c, DSLExpression d) {
    std::vector<DSLExpression> args;
    args.push_back(std::move(a));
    args.push_back(std::move(b));
    args.push_back(std::move(c));
    args.push_back(std::move(d));
    return DSLWriter::Construct(type, std::move(args));
}

#define TYPE(T)                                                                                    \
DSLExpression T(DSLExpression a) {                                                                 \
    return construct1(*DSLWriter::Context().fTypes.f ## T, std::move(a));                          \
}                                                                                                  \
DSLExpression T ## 2(DSLExpression a) {                                                            \
    return construct1(*DSLWriter::Context().fTypes.f ## T ## 2, std::move(a));                     \
}                                                                                                  \
DSLExpression T ## 2(DSLExpression a, DSLExpression b) {                                           \
    return construct2(*DSLWriter::Context().fTypes.f ## T ## 2, std::move(a),                      \
                      std::move(b));                                                               \
}                                                                                                  \
DSLExpression T ## 3(DSLExpression a) {                                                            \
    return construct1(*DSLWriter::Context().fTypes.f ## T ## 3, std::move(a));                     \
}                                                                                                  \
DSLExpression T ## 3(DSLExpression a, DSLExpression b) {                                           \
    return construct2(*DSLWriter::Context().fTypes.f ## T ## 3, std::move(a),                      \
                      std::move(b));                                                               \
}                                                                                                  \
DSLExpression T ## 3(DSLExpression a, DSLExpression b, DSLExpression c) {                          \
    return construct3(*DSLWriter::Context().fTypes.f ## T ## 3, std::move(a),                      \
                      std::move(b), std::move(c));                                                 \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a) {                                                            \
    return construct1(*DSLWriter::Context().fTypes.f ## T ## 4, std::move(a));                     \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a, DSLExpression b) {                                           \
    return construct2(*DSLWriter::Context().fTypes.f ## T ## 4, std::move(a),                      \
                      std::move(b));                                                               \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a, DSLExpression b, DSLExpression c) {                          \
    return construct3(*DSLWriter::Context().fTypes.f ## T ## 4, std::move(a), std::move(b),        \
                      std::move(c));                                                               \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a, DSLExpression b, DSLExpression c, DSLExpression d) {         \
    return construct4(*DSLWriter::Context().fTypes.f ## T ## 4, std::move(a), std::move(b),        \
                      std::move(c), std::move(d));                                                 \
}

TYPE(Bool)
TYPE(Float)
TYPE(Half)
TYPE(Int)
TYPE(Short)

#undef TYPE

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
