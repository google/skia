/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>

namespace SkSL {

class Statement;
class Type;

namespace dsl {

class DSLStatement;

enum TypeConstant : uint8_t {
    kBool,
    kBool2,
    kBool3,
    kBool4,
    kHalf,
    kHalf2,
    kHalf3,
    kHalf4,
    kHalf2x2,
    kHalf3x2,
    kHalf4x2,
    kHalf2x3,
    kHalf3x3,
    kHalf4x3,
    kHalf2x4,
    kHalf3x4,
    kHalf4x4,
    kFloat,
    kFloat2,
    kFloat3,
    kFloat4,
    kFloat2x2,
    kFloat3x2,
    kFloat4x2,
    kFloat2x3,
    kFloat3x3,
    kFloat4x3,
    kFloat2x4,
    kFloat3x4,
    kFloat4x4,
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

    friend DSLExpression dsl_construct(const SkSL::Type& type, std::vector<DSLExpression> rawArgs);
    friend DSLType Array(const DSLType& base, int count);

    friend class DSLFunction;
    friend class DSLVar;
};

DSLExpression dsl_construct(const SkSL::Type& type, std::vector<DSLExpression> rawArgs);

template<typename... Args>
DSLExpression dsl_construct(const SkSL::Type& type, Args... args) {
    std::vector<DSLExpression> argVec;
    (argVec.push_back(args.release()), ...);
    return dsl_construct(type, std::move(argVec));
}

#define CONSTRUCT(T)                                                                               \
    template<typename... Args>                                                                     \
    DSLExpression T(Args&&... args) {                                                              \
        return dsl_construct(*DSLWriter::Context().f ## T ## _Type,                                \
                             DSLExpression(std::move(args))...);                                   \
    }

#define TYPE(T)                                                                                    \
    CONSTRUCT(T)                                                                                   \
    CONSTRUCT(T ## 2)                                                                              \
    CONSTRUCT(T ## 3)                                                                              \
    CONSTRUCT(T ## 4)

#define MATRIX_TYPE(T)                                                                             \
    CONSTRUCT(T ## 2x2)                                                                            \
    CONSTRUCT(T ## 3x2)                                                                            \
    CONSTRUCT(T ## 4x2)                                                                            \
    CONSTRUCT(T ## 2x3)                                                                            \
    CONSTRUCT(T ## 3x3)                                                                            \
    CONSTRUCT(T ## 4x3)                                                                            \
    CONSTRUCT(T ## 2x4)                                                                            \
    CONSTRUCT(T ## 3x4)                                                                            \
    CONSTRUCT(T ## 4x4)

TYPE(Bool)
TYPE(Float)
TYPE(Half)
TYPE(Int)
TYPE(Short)
MATRIX_TYPE(Float)
MATRIX_TYPE(Half)

#undef TYPE
#undef TYPE_FRIEND

DSLType Array(const DSLType& base, int count);

} // namespace dsl

} // namespace SkSL

#endif
