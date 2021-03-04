/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

#include "include/sksl/DSLModifiers.h"

#include <cstdint>

namespace SkSL {

class Type;

namespace dsl {

class DSLExpression;
class DSLField;

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
    friend DSLType Struct(const char* name, SkTArray<DSLField> fields);
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
