/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLModifiers.h"

#include <cstdint>

namespace SkSL {

class Type;

namespace dsl {

class DSLExpression;
class DSLField;

enum TypeConstant : uint8_t {
    kBool_Type,
    kBool2_Type,
    kBool3_Type,
    kBool4_Type,
    kHalf_Type,
    kHalf2_Type,
    kHalf3_Type,
    kHalf4_Type,
    kHalf2x2_Type,
    kHalf3x2_Type,
    kHalf4x2_Type,
    kHalf2x3_Type,
    kHalf3x3_Type,
    kHalf4x3_Type,
    kHalf2x4_Type,
    kHalf3x4_Type,
    kHalf4x4_Type,
    kFloat_Type,
    kFloat2_Type,
    kFloat3_Type,
    kFloat4_Type,
    kFragmentProcessor_Type,
    kFloat2x2_Type,
    kFloat3x2_Type,
    kFloat4x2_Type,
    kFloat2x3_Type,
    kFloat3x3_Type,
    kFloat4x3_Type,
    kFloat2x4_Type,
    kFloat3x4_Type,
    kFloat4x4_Type,
    kInt_Type,
    kInt2_Type,
    kInt3_Type,
    kInt4_Type,
    kShader_Type,
    kShort_Type,
    kShort2_Type,
    kShort3_Type,
    kShort4_Type,
    kVoid_Type,
};

class DSLType {
public:
    DSLType(TypeConstant tc)
        : fTypeConstant(tc) {}

    DSLType(const SkSL::Type* type)
        : fSkSLType(type) {}

    template<typename... Args>
    static DSLExpression Construct(DSLType type, Args&&... args) {
        SkTArray<DSLExpression> argArray;
        argArray.reserve_back(sizeof...(args));
        CollectArgs(argArray, std::forward<Args>(args)...);
        return Construct(type, std::move(argArray));
    }

    static DSLExpression Construct(DSLType type, SkTArray<DSLExpression> argArray);

private:
    const SkSL::Type& skslType() const;

    const SkSL::Type* fSkSLType = nullptr;

    static void CollectArgs(SkTArray<DSLExpression>& args) {}

    template<class... RemainingArgs>
    static void CollectArgs(SkTArray<DSLExpression>& args, DSLVar& var,
                            RemainingArgs&&... remaining) {
        args.push_back(var);
        CollectArgs(args, std::forward<RemainingArgs>(remaining)...);
    }

    template<class... RemainingArgs>
    static void CollectArgs(SkTArray<DSLExpression>& args, DSLExpression expr,
                            RemainingArgs&&... remaining) {
        args.push_back(std::move(expr));
        CollectArgs(args, std::forward<RemainingArgs>(remaining)...);
    }

    TypeConstant fTypeConstant;

    friend DSLType Array(const DSLType& base, int count);
    friend DSLType Struct(const char* name, SkTArray<DSLField> fields);
    friend class DSLFunction;
    friend class DSLVar;
    friend class DSLWriter;
};

#define TYPE(T)                                                                                    \
    template<typename... Args>                                                                     \
    DSLExpression T(Args&&... args) {                                                              \
        return DSLType::Construct(k ## T ## _Type, std::forward<Args>(args)...);                   \
    }

#define VECTOR_TYPE(T)                                                                             \
    TYPE(T)                                                                                        \
    TYPE(T ## 2)                                                                                   \
    TYPE(T ## 3)                                                                                   \
    TYPE(T ## 4)

#define MATRIX_TYPE(T)                                                                             \
    TYPE(T ## 2x2)                                                                                 \
    TYPE(T ## 3x2)                                                                                 \
    TYPE(T ## 4x2)                                                                                 \
    TYPE(T ## 2x3)                                                                                 \
    TYPE(T ## 3x3)                                                                                 \
    TYPE(T ## 4x3)                                                                                 \
    TYPE(T ## 2x4)                                                                                 \
    TYPE(T ## 3x4)                                                                                 \
    TYPE(T ## 4x4)

VECTOR_TYPE(Bool)
VECTOR_TYPE(Float)
VECTOR_TYPE(Half)
VECTOR_TYPE(Int)
VECTOR_TYPE(Short)

MATRIX_TYPE(Float)
MATRIX_TYPE(Half)

#undef TYPE
#undef VECTOR_TYPE
#undef MATRIX_TYPE

DSLType Array(const DSLType& base, int count);

class DSLField {
public:
    DSLField(const DSLType type, const char* name)
        : DSLField(DSLModifiers(), type, name) {}

private:
    DSLField(DSLModifiers modifiers, const DSLType type, const char* name)
        : fModifiers(modifiers)
        , fType(type)
        , fName(name) {}

    DSLModifiers fModifiers;
    const DSLType fType;
    const char* fName;

    friend DSLType Struct(const char* name, SkTArray<DSLField> fields);
};

DSLType Struct(const char* name, SkTArray<DSLField> fields);

template<typename... Field>
DSLType Struct(const char* name, Field... fields) {
    SkTArray<DSLField> fieldTypes;
    fieldTypes.reserve_back(sizeof...(fields));
    // in C++17, we could just do:
    // (fieldTypes.push_back(std::move(fields)), ...);
    int unused[] = {0, (fieldTypes.push_back(std::move(fields)), 0)...};
    static_cast<void>(unused);

    return Struct(name, std::move(fieldTypes));
}

} // namespace dsl

} // namespace SkSL

#endif
