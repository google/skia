/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/dsl/DSLExpression.h"

#include <string_view>
#include <utility>

namespace SkSL {

class Compiler;
struct Field;
class Position;
class Type;

namespace dsl {

class DSLModifiers;
struct DSLVarBase;

class DSLType {
public:
    DSLType(const SkSL::Type* type, Position pos = {});

    DSLType(std::string_view name, Position pos = {});

    DSLType(std::string_view name, DSLModifiers* modifiers, Position pos = {});

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

    template<typename... Args>
    static DSLExpression Construct(DSLType type, DSLVarBase& var, Args&&... args) {
        DSLExpression argArray[] = {var, args...};
        return Construct(type, SkSpan(argArray));
    }

    template<typename... Args>
    static DSLExpression Construct(DSLType type, DSLExpression expr, Args&&... args) {
        DSLExpression argArray[] = {std::move(expr), std::move(args)...};
        return Construct(type, SkSpan(argArray));
    }

    static DSLExpression Construct(DSLType type, SkSpan<DSLExpression> argArray);

    const SkSL::Type& skslType() const {
        SkASSERT(fSkSLType);
        return *fSkSLType;
    }

private:
    const SkSL::Type* fSkSLType = nullptr;

    friend DSLType Array(const DSLType& base, int count, Position pos);
    friend DSLType Struct(std::string_view name, SkSpan<Field> fields, Position pos);
    friend DSLType StructType(std::string_view name,
                              skia_private::TArray<Field> fields,
                              bool interfaceBlock,
                              Position pos);
    friend DSLType UnsizedArray(const DSLType& base, Position pos);
    friend class DSLCore;
    friend class DSLFunction;
    friend class DSLWriter;
    friend class SkSL::Compiler;
};

DSLType Array(const DSLType& base, int count, Position pos = {});

DSLType UnsizedArray(const DSLType& base, Position pos = {});

/**
 * Creates a StructDefinition at the top level and returns the associated type.
 */
DSLType Struct(std::string_view name, skia_private::TArray<Field> fields, Position pos = {});

/**
 * Creates a struct type and adds it to the current symbol table. Does _not_ create a ProgramElement
 * at the top level, so the type will exist, but won't be represented anywhere in the output.
 * (Use Struct or InterfaceBlock to add a top-level program element.)
 */
DSLType StructType(std::string_view name,
                   skia_private::TArray<Field> fields,
                   bool interfaceBlock,
                   Position pos);

} // namespace dsl

} // namespace SkSL

#endif
