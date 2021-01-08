/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLType.h"

#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLConstructor.h"

namespace SkSL {

namespace dsl {

const SkSL::Type& DSLType::skslType() const {
    if (fSkSLType) {
        return *fSkSLType;
    }
    const SkSL::Context& context = DSLWriter::Context();
    switch (fTypeConstant) {
        case kBool:
            return *context.fBool_Type;
        case kBool2:
            return *context.fBool2_Type;
        case kBool3:
            return *context.fBool3_Type;
        case kBool4:
            return *context.fBool4_Type;
        case kHalf:
            return *context.fHalf_Type;
        case kHalf2:
            return *context.fHalf2_Type;
        case kHalf3:
            return *context.fHalf3_Type;
        case kHalf4:
            return *context.fHalf4_Type;
        case kFloat:
            return *context.fFloat_Type;
        case kFloat2:
            return *context.fFloat2_Type;
        case kFloat3:
            return *context.fFloat3_Type;
        case kFloat4:
            return *context.fFloat4_Type;
        case kInt:
            return *context.fInt_Type;
        case kInt2:
            return *context.fInt2_Type;
        case kInt3:
            return *context.fInt3_Type;
        case kInt4:
            return *context.fInt4_Type;
        case kShort:
            return *context.fShort_Type;
        case kShort2:
            return *context.fShort2_Type;
        case kShort3:
            return *context.fShort3_Type;
        case kShort4:
            return *context.fShort4_Type;
        case kVoid:
            return *context.fVoid_Type;
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
    return construct1(*DSLWriter::Context().f ## T ## _Type, std::move(a));                        \
}                                                                                                  \
DSLExpression T ## 2(DSLExpression a) {                                                            \
    return construct1(*DSLWriter::Context().f ## T ## 2_Type, std::move(a));                       \
}                                                                                                  \
DSLExpression T ## 2(DSLExpression a, DSLExpression b) {                                           \
    return construct2(*DSLWriter::Context().f ## T ## 2_Type, std::move(a),                        \
                      std::move(b));                                                               \
}                                                                                                  \
DSLExpression T ## 3(DSLExpression a) {                                                            \
    return construct1(*DSLWriter::Context().f ## T ## 3_Type, std::move(a));                       \
}                                                                                                  \
DSLExpression T ## 3(DSLExpression a, DSLExpression b) {                                           \
    return construct2(*DSLWriter::Context().f ## T ## 3_Type, std::move(a),                        \
                      std::move(b));                                                               \
}                                                                                                  \
DSLExpression T ## 3(DSLExpression a, DSLExpression b, DSLExpression c) {                          \
    return construct3(*DSLWriter::Context().f ## T ## 3_Type, std::move(a),                        \
                      std::move(b), std::move(c));                                                 \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a) {                                                            \
    return construct1(*DSLWriter::Context().f ## T ## 4_Type, std::move(a));                       \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a, DSLExpression b) {                                           \
    return construct2(*DSLWriter::Context().f ## T ## 4_Type, std::move(a),                        \
                      std::move(b));                                                               \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a, DSLExpression b, DSLExpression c) {                          \
    return construct3(*DSLWriter::Context().f ## T ## 4_Type, std::move(a), std::move(b),          \
                      std::move(c));                                                               \
}                                                                                                  \
DSLExpression T ## 4(DSLExpression a, DSLExpression b, DSLExpression c, DSLExpression d) {         \
    return construct4(*DSLWriter::Context().f ## T ## 4_Type, std::move(a), std::move(b),          \
                      std::move(c), std::move(d));                                                 \
}

TYPE(Bool)
TYPE(Float)
TYPE(Half)
TYPE(Int)
TYPE(Short)

#undef TYPE

} // namespace dsl

} // namespace SkSL
