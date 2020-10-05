/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIASL_TYPE
#define SKIASL_TYPE

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/spirv.h"
#include <algorithm>
#include <climits>
#include <vector>
#include <memory>

namespace SkSL {

class Context;

struct CoercionCost {
    static CoercionCost Free()              { return {    0,    0, false }; }
    static CoercionCost Normal(int cost)    { return { cost,    0, false }; }
    static CoercionCost Narrowing(int cost) { return {    0, cost, false }; }
    static CoercionCost Impossible()        { return {    0,    0,  true }; }

    bool isPossible(bool allowNarrowing) const {
        return !fImpossible && (fNarrowingCost == 0 || allowNarrowing);
    }

    // Addition of two costs. Saturates at Impossible().
    CoercionCost operator+(CoercionCost rhs) const {
        if (fImpossible || rhs.fImpossible) {
            return Impossible();
        }
        return { fNormalCost + rhs.fNormalCost, fNarrowingCost + rhs.fNarrowingCost, false };
    }

    bool operator<(CoercionCost rhs) const {
        return std::tie(    fImpossible,     fNarrowingCost,     fNormalCost) <
               std::tie(rhs.fImpossible, rhs.fNarrowingCost, rhs.fNormalCost);
    }

    int  fNormalCost;
    int  fNarrowingCost;
    bool fImpossible;
};

/**
 * Represents a type, such as int or float4.
 */
class Type : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kType;

    struct Field {
        Field(Modifiers modifiers, StringFragment name, const Type* type)
        : fModifiers(modifiers)
        , fName(name)
        , fType(std::move(type)) {}

        String description() const {
            return fType->displayName() + " " + fName + ";";
        }

        Modifiers fModifiers;
        StringFragment fName;
        const Type* fType;
    };

    enum class TypeKind {
        kArray,
        kEnum,
        kGeneric,
        kNullable,
        kMatrix,
        kOther,
        kSampler,
        kSeparateSampler,
        kScalar,
        kStruct,
        kTexture,
        kVector
    };

    enum class NumberKind {
        kFloat,
        kSigned,
        kUnsigned,
        kNonnumeric
    };

    Type(const Type& other) = delete;

    // Create an "other" (special) type with the given name. These types cannot be directly
    // referenced from user code.
    Type(const char* name)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kOther)
    , fNumberKind(NumberKind::kNonnumeric) {}

    // Create an "other" (special) type that supports field access.
    Type(const char* name, std::vector<Field> fields)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kOther)
    , fNumberKind(NumberKind::kNonnumeric)
    , fFields(std::move(fields)) {}

    // Create a simple type.
    Type(String name, TypeKind kind)
    : INHERITED(-1, kSymbolKind, "")
    , fNameString(std::move(name))
    , fTypeKind(kind)
    , fNumberKind(NumberKind::kNonnumeric) {
        this->symbolData().fName = StringFragment(fNameString.c_str(), fNameString.length());
    }

    // Create a generic type which maps to the listed types.
    Type(const char* name, std::vector<const Type*> types)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kGeneric)
    , fNumberKind(NumberKind::kNonnumeric)
    , fCoercibleTypes(std::move(types)) {}

    // Create a struct type with the given fields.
    Type(int offset, String name, std::vector<Field> fields)
    : INHERITED(offset, kSymbolKind, "")
    , fNameString(std::move(name))
    , fTypeKind(TypeKind::kStruct)
    , fNumberKind(NumberKind::kNonnumeric)
    , fFields(std::move(fields)) {
        this->symbolData().fName = StringFragment(fNameString.c_str(), fNameString.length());
    }

    // Create a scalar type.
    Type(const char* name, NumberKind numberKind, int priority, bool highPrecision = false)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kScalar)
    , fNumberKind(numberKind)
    , fPriority(priority)
    , fColumns(1)
    , fRows(1)
    , fHighPrecision(highPrecision) {}

    // Create a scalar type which can be coerced to the listed types.
    Type(const char* name,
         NumberKind numberKind,
         int priority,
         std::vector<const Type*> coercibleTypes)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kScalar)
    , fNumberKind(numberKind)
    , fPriority(priority)
    , fCoercibleTypes(std::move(coercibleTypes))
    , fColumns(1)
    , fRows(1) {}

    // Create a nullable type.
    Type(String name, TypeKind kind, const Type& componentType)
    : INHERITED(-1, kSymbolKind, "")
    , fNameString(std::move(name))
    , fTypeKind(kind)
    , fNumberKind(NumberKind::kNonnumeric)
    , fComponentType(&componentType)
    , fColumns(1)
    , fRows(1)
    , fDimensions(SpvDim1D) {
        this->symbolData().fName = StringFragment(fNameString.c_str(), fNameString.length());
    }

    // Create a vector type.
    Type(const char* name, const Type& componentType, int columns)
    : Type(name, TypeKind::kVector, componentType, columns) {}

    static constexpr int kUnsizedArray = -1;

    // Create a vector or array type.
    Type(String name, TypeKind kind, const Type& componentType, int columns)
    : INHERITED(-1, kSymbolKind, "")
    , fNameString(std::move(name))
    , fTypeKind(kind)
    , fNumberKind(NumberKind::kNonnumeric)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(1)
    , fDimensions(SpvDim1D) {
        SkASSERT(fColumns > 0 || (fTypeKind == TypeKind::kArray && fColumns == kUnsizedArray));
        this->symbolData().fName = StringFragment(fNameString.c_str(), fNameString.length());
    }

    // Create a matrix type.
    Type(const char* name, const Type& componentType, int columns, int rows)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kMatrix)
    , fNumberKind(NumberKind::kNonnumeric)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(rows)
    , fDimensions(SpvDim1D) {}

    // Create a texture type.
    Type(const char* name, SpvDim_ dimensions, bool isDepth, bool isArrayed, bool isMultisampled,
         bool isSampled)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kTexture)
    , fNumberKind(NumberKind::kNonnumeric)
    , fDimensions(dimensions)
    , fIsDepth(isDepth)
    , fIsArrayed(isArrayed)
    , fIsMultisampled(isMultisampled)
    , fIsSampled(isSampled) {}

    // Create a sampler type.
    Type(const char* name, const Type& textureType)
    : INHERITED(-1, kSymbolKind, name)
    , fTypeKind(TypeKind::kSampler)
    , fNumberKind(NumberKind::kNonnumeric)
    , fDimensions(textureType.dimensions())
    , fIsDepth(textureType.isDepth())
    , fIsArrayed(textureType.isArrayed())
    , fIsMultisampled(textureType.isMultisampled())
    , fIsSampled(textureType.isSampled())
    , fTextureType(&textureType) {}

    String displayName() const {
        StringFragment name = this->name();
        if (name == "$floatLiteral") {
            return "float";
        }
        if (name == "$intLiteral") {
            return "int";
        }
        return name;
    }

    String description() const override {
        return this->displayName();
    }

    bool operator==(const Type& other) const {
        return this->name() == other.name();
    }

    bool operator!=(const Type& other) const {
        return this->name() != other.name();
    }

    /**
     * Returns the category (scalar, vector, matrix, etc.) of this type.
     */
    TypeKind typeKind() const {
        return fTypeKind;
    }

    /**
     * Returns true if this is a numeric scalar type.
     */
    bool isNumber() const {
        return fNumberKind != NumberKind::kNonnumeric;
    }

    /**
     * Returns true if this is a floating-point scalar type (float or half).
     */
    bool isFloat() const {
        return fNumberKind == NumberKind::kFloat;
    }

    /**
     * Returns true if this is a signed scalar type (int or short).
     */
    bool isSigned() const {
        return fNumberKind == NumberKind::kSigned;
    }

    /**
     * Returns true if this is an unsigned scalar type (uint or ushort).
     */
    bool isUnsigned() const {
        return fNumberKind == NumberKind::kUnsigned;
    }

    /**
     * Returns true if this is a signed or unsigned integer.
     */
    bool isInteger() const {
        return isSigned() || isUnsigned();
    }

    /**
     * Returns the "priority" of a number type, in order of float > half > int > short.
     * When operating on two number types, the result is the higher-priority type.
     */
    int priority() const {
        return fPriority;
    }

    /**
     * Returns true if an instance of this type can be freely coerced (implicitly converted) to
     * another type.
     */
    bool canCoerceTo(const Type& other, bool allowNarrowing) const {
        return this->coercionCost(other).isPossible(allowNarrowing);
    }

    /**
     * Determines the "cost" of coercing (implicitly converting) this type to another type. The cost
     * is a number with no particular meaning other than that lower costs are preferable to higher
     * costs. Returns INT_MAX if the coercion is not possible.
     */
    CoercionCost coercionCost(const Type& other) const;

    /**
     * For matrices and vectors, returns the type of individual cells (e.g. mat2 has a component
     * type of kFloat_Type). For all other types, causes an SkASSERTion failure.
     */
    const Type& componentType() const {
        SkASSERT(fComponentType);
        return *fComponentType;
    }

    /**
     * For texturesamplers, returns the type of texture it samples (e.g., sampler2D has
     * a texture type of texture2D).
     */
    const Type& textureType() const {
        SkASSERT(fTextureType);
        return *fTextureType;
    }

    /**
     * For nullable types, returns the base type, otherwise returns the type itself.
     */
    const Type& nonnullable() const {
        if (fTypeKind == TypeKind::kNullable) {
            return this->componentType();
        }
        return *this;
    }

    /**
     * For matrices and vectors, returns the number of columns (e.g. both mat3 and float3return 3).
     * For scalars, returns 1. For arrays, returns either the size of the array (if known) or -1.
     * For all other types, causes an SkASSERTion failure.
     */
    int columns() const {
        SkASSERT(fTypeKind == TypeKind::kScalar || fTypeKind == TypeKind::kVector ||
                 fTypeKind == TypeKind::kMatrix || fTypeKind == TypeKind::kArray);
        return fColumns;
    }

    /**
     * For matrices, returns the number of rows (e.g. mat2x4 returns 4). For vectors and scalars,
     * returns 1. For all other types, causes an SkASSERTion failure.
     */
    int rows() const {
        SkASSERT(fRows > 0);
        return fRows;
    }

    const std::vector<Field>& fields() const {
        SkASSERT(fTypeKind == TypeKind::kStruct || fTypeKind == TypeKind::kOther);
        return fFields;
    }

    /**
     * For generic types, returns the types that this generic type can substitute for. For other
     * types, returns a list of other types that this type can be coerced into.
     */
    const std::vector<const Type*>& coercibleTypes() const {
        SkASSERT(fCoercibleTypes.size() > 0);
        return fCoercibleTypes;
    }

    SpvDim_ dimensions() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fDimensions;
    }

    bool isDepth() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fIsDepth;
    }

    bool isArrayed() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fIsArrayed;
    }

    bool isMultisampled() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fIsMultisampled;
    }

    bool isSampled() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fIsSampled;
    }

    bool highPrecision() const {
        if (fComponentType) {
            return fComponentType->highPrecision();
        }
        return fHighPrecision;
    }

    /**
     * Returns the corresponding vector or matrix type with the specified number of columns and
     * rows.
     */
    const Type& toCompound(const Context& context, int columns, int rows) const;

private:
    using INHERITED = Symbol;

    String fNameString;
    TypeKind fTypeKind;
    // always kNonnumeric_NumberKind for non-scalar values
    NumberKind fNumberKind;
    int fPriority = -1;
    const Type* fComponentType = nullptr;
    std::vector<const Type*> fCoercibleTypes;
    int fColumns = -1;
    int fRows = -1;
    std::vector<Field> fFields;
    SpvDim_ fDimensions = SpvDim1D;
    bool fIsDepth = false;
    bool fIsArrayed = false;
    bool fIsMultisampled = false;
    bool fIsSampled = false;
    bool fHighPrecision = false;
    const Type* fTextureType = nullptr;
};

}  // namespace SkSL

#endif
