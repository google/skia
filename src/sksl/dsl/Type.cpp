/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/Type.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLConstructor.h"

namespace SkSL {

namespace dsl {

const SkSL::Type& Type::skslType() const {
    if (fSkSLType) {
        return *fSkSLType;
    }
    switch (fTypeConstant) {
        case kBool:
            return *DSLWriter::Instance().context().fBool_Type;
        case kBool2:
            return *DSLWriter::Instance().context().fBool2_Type;
        case kBool3:
            return *DSLWriter::Instance().context().fBool3_Type;
        case kBool4:
            return *DSLWriter::Instance().context().fBool4_Type;
        case kHalf:
            return *DSLWriter::Instance().context().fHalf_Type;
        case kHalf2:
            return *DSLWriter::Instance().context().fHalf2_Type;
        case kHalf3:
            return *DSLWriter::Instance().context().fHalf3_Type;
        case kHalf4:
            return *DSLWriter::Instance().context().fHalf4_Type;
        case kFloat:
            return *DSLWriter::Instance().context().fFloat_Type;
        case kFloat2:
            return *DSLWriter::Instance().context().fFloat2_Type;
        case kFloat3:
            return *DSLWriter::Instance().context().fFloat3_Type;
        case kFloat4:
            return *DSLWriter::Instance().context().fFloat4_Type;
        case kInt:
            return *DSLWriter::Instance().context().fInt_Type;
        case kInt2:
            return *DSLWriter::Instance().context().fInt2_Type;
        case kInt3:
            return *DSLWriter::Instance().context().fInt3_Type;
        case kInt4:
            return *DSLWriter::Instance().context().fInt4_Type;
        case kShort:
            return *DSLWriter::Instance().context().fShort_Type;
        case kShort2:
            return *DSLWriter::Instance().context().fShort2_Type;
        case kShort3:
            return *DSLWriter::Instance().context().fShort3_Type;
        case kShort4:
            return *DSLWriter::Instance().context().fShort4_Type;
        default:
            SkUNREACHABLE;
    }
}

#define SCALAR(T)                                                                                  \
Expression T(Expression expr) {                                                                    \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(expr.release());                                                                \
    DSLWriter& dsl = DSLWriter::Instance();                                                        \
    const SkSL::Type& type = Type(k ## T).skslType();                                              \
    return Expression(dsl.irGenerator().call(/*offset=*/-1,                                        \
                                             std::make_unique<SkSL::TypeReference>(dsl.context(),  \
                                                                                   /*offset=*/-1,  \
                                                                                   &type),         \
                                             std::move(args)));                                    \
}

SCALAR(Bool)
SCALAR(Float)
SCALAR(Half)
SCALAR(Int)
SCALAR(Short)

#define VECTOR2(T)                                                                                 \
SCALAR(T)                                                                                          \
Expression T(Expression x, Expression y) {                                                         \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(x.release());                                                                   \
    args.push_back(y.release());                                                                   \
    DSLWriter& dsl = DSLWriter::Instance();                                                        \
    const SkSL::Type& type = Type(k ## T).skslType();                                              \
    return Expression(dsl.irGenerator().call(/*offset=*/-1,                                        \
                                             std::make_unique<SkSL::TypeReference>(dsl.context(),  \
                                                                                   /*offset=*/-1,  \
                                                                                   &type),         \
                                             std::move(args)));                                    \
}

#define VECTOR3(T)                                                                                 \
VECTOR2(T)                                                                                         \
Expression T(Expression x, Expression y, Expression z) {                                           \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(x.release());                                                                   \
    args.push_back(y.release());                                                                   \
    args.push_back(z.release());                                                                   \
    DSLWriter& dsl = DSLWriter::Instance();                                                        \
    const SkSL::Type& type = Type(k ## T).skslType();                                              \
    return Expression(dsl.irGenerator().call(/*offset=*/-1,                                        \
                                             std::make_unique<SkSL::TypeReference>(dsl.context(),  \
                                                                                   /*offset=*/-1,  \
                                                                                   &type),         \
                                             std::move(args)));                                    \
}

#define VECTOR4(T)                                                                                 \
VECTOR3(T)                                                                                         \
Expression T(Expression x, Expression y, Expression z, Expression w) {                             \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(x.release());                                                                   \
    args.push_back(y.release());                                                                   \
    args.push_back(z.release());                                                                   \
    args.push_back(w.release());                                                                   \
    DSLWriter& dsl = DSLWriter::Instance();                                                        \
    const SkSL::Type& type = Type(k ## T).skslType();                                              \
    return Expression(dsl.irGenerator().call(/*offset=*/-1,                                        \
                                             std::make_unique<SkSL::TypeReference>(dsl.context(),  \
                                                                                   /*offset=*/-1,  \
                                                                                   &type),         \
                                             std::move(args)));                                    \
}

#define VECTOR(T)                                                                                  \
VECTOR2(T ## 2)                                                                                    \
VECTOR3(T ## 3)                                                                                    \
VECTOR4(T ## 4)

VECTOR(Bool)
VECTOR(Float)
VECTOR(Half)
VECTOR(Int)
VECTOR(Short)

#undef SCALAR
#undef VECTOR
#undef VECTOR2
#undef VECTOR3
#undef VECTOR4

Type Array(const Type& base, int count) {
    SkSL::String name = base.skslType().name() + "[" + SkSL::to_string(count) + "]";
    return Type(DSLWriter::Instance().symbolTable()->takeOwnershipOfSymbol(
                                    std::make_unique<SkSL::Type>(name, SkSL::Type::TypeKind::kArray,
                                                                 base.skslType(), count)));
}

} // namespace dsl

} // namespace SkSL
