/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLPosition.h"  // IWYU pragma: keep

#include <string_view>

namespace SkSL {

class Type;
struct Modifiers;

namespace dsl {

class DSLType {
public:
    DSLType(const SkSL::Type* type, Position pos = {});

    DSLType(std::string_view name, Position pos = {});

    DSLType(std::string_view name, Position overallPos,
            SkSL::Modifiers* modifiers, Position modifiersPos);

    static DSLType Invalid();
    static DSLType Poison();
    static DSLType Void();

    /**
     * Returns true if the SkSL type is non-null.
     */
    bool hasValue() const { return fSkSLType != nullptr; }

    /**
     * Returns true if this type is a bool.
     */
    bool isBoolean() const;

    /**
     * Returns true if this is a numeric scalar type.
     */
    bool isNumber() const;

    /**
     * Returns true if this is a floating-point scalar type (float or half).
     */
    bool isFloat() const;

    /**
     * Returns true if this is a signed scalar type (int or short).
     */
    bool isSigned() const;

    /**
     * Returns true if this is an unsigned scalar type (uint or ushort).
     */
    bool isUnsigned() const;

    /**
     * Returns true if this is a signed or unsigned integer.
     */
    bool isInteger() const;

    /**
     * Returns true if this is a scalar type.
     */
    bool isScalar() const;

    /**
     * Returns true if this is a vector type.
     */
    bool isVector() const;

    /**
     * Returns true if this is a matrix type.
     */
    bool isMatrix() const;

    /**
     * Returns true if this is a array type.
     */
    bool isArray() const;

    /**
     * Returns true if this is a struct type.
     */
    bool isStruct() const;

    /**
     * Returns true if this is an interface block
     */
    bool isInterfaceBlock() const;

    /**
     * Returns true if this is a Skia object type (shader, colorFilter, blender).
     */
    bool isEffectChild() const;

    const SkSL::Type& skslType() const {
        SkASSERT(fSkSLType);
        return *fSkSLType;
    }

private:
    const SkSL::Type* fSkSLType = nullptr;

    friend DSLType Array(const DSLType& base, int count, Position pos);
    friend DSLType UnsizedArray(const DSLType& base, Position pos);
};

DSLType Array(const DSLType& base, int count, Position pos = {});

DSLType UnsizedArray(const DSLType& base, Position pos = {});

} // namespace dsl

} // namespace SkSL

#endif
