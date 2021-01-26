/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>

namespace SkSL {

class Type;

namespace dsl {

class DSLExpression;

enum TypeConstant : uint8_t {
    kBool,
    kBool2,
    kBool3,
    kBool4,
    kHalf,
    kHalf2,
    kHalf3,
    kHalf4,
    kFloat,
    kFloat2,
    kFloat3,
    kFloat4,
    kInt,
    kInt2,
    kInt3,
    kInt4,
    kShort,
    kShort2,
    kShort3,
    kShort4,
    kVoid,
};

class DSLType {
public:
    DSLType(TypeConstant tc)
        : fTypeConstant(tc) {}

    DSLType(const SkSL::Type* type)
        : fSkSLType(type) {}

private:
    const SkSL::Type& skslType() const;

    const SkSL::Type* fSkSLType = nullptr;

    TypeConstant fTypeConstant;

    friend DSLType Array(const DSLType& base, int count);
    friend class DSLFunction;
    friend class DSLVar;
};

#define TYPE(T)                                                                                    \
    DSLExpression T(DSLExpression expr);                                                           \
    DSLExpression T##2(DSLExpression expr);                                                        \
    DSLExpression T##2(DSLExpression x, DSLExpression y);                                          \
    DSLExpression T##3(DSLExpression expr);                                                        \
    DSLExpression T##3(DSLExpression x, DSLExpression y);                                          \
    DSLExpression T##3(DSLExpression x, DSLExpression y, DSLExpression z);                         \
    DSLExpression T##4(DSLExpression expr);                                                        \
    DSLExpression T##4(DSLExpression x, DSLExpression y);                                          \
    DSLExpression T##4(DSLExpression x, DSLExpression y, DSLExpression z);                         \
    DSLExpression T##4(DSLExpression x, DSLExpression y, DSLExpression z, DSLExpression w);

TYPE(Bool)
TYPE(Float)
TYPE(Half)
TYPE(Int)
TYPE(Short)

#undef TYPE

DSLType Array(const DSLType& base, int count);

} // namespace dsl

} // namespace SkSL

#endif
