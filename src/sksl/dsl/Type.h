/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

#include "src/sksl/dsl/Expression.h"

namespace SkSL {

class Type;

namespace dsl {

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
};

class Type {
public:
    Type(TypeConstant tc)
        : fTypeConstant(tc) {}

    Type(const SkSL::Type* type)
        : fSkSLType(type) {}

    const SkSL::Type& skslType() const;

private:
    const SkSL::Type* fSkSLType = nullptr;

    TypeConstant fTypeConstant;

    friend class Var;
};

#define SCALAR(T) Expression T(Expression expr);

SCALAR(Bool)
SCALAR(Float)
SCALAR(Half)
SCALAR(Int)
SCALAR(Short)

#define VECTOR2(T)                                                                                 \
SCALAR(T)                                                                                          \
Expression T(Expression x, Expression y);

#define VECTOR3(T)                                                                                 \
VECTOR2(T)                                                                                         \
Expression T(Expression x, Expression y, Expression z);

#define VECTOR4(T)                                                                                 \
VECTOR3(T)                                                                                         \
Expression T(Expression x, Expression y, Expression z, Expression w);

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

Type Array(const Type& base, int count);

} // namespace dsl

} // namespace SkSL

#endif
