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

enum class TypeNumberKind : int8_t {
    kFloat,
    kSigned,
    kUnsigned,
    kNonnumeric
};

/**
 * Represents a type, such as int or float4.
 */
class Type : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kType;

    using Field = SkSL::TypeField;
    using TypeKind = SkSL::TypeKind;
    using NumberKind = SkSL::TypeNumberKind;

    Type(const Type& other) = delete;

    // Create an "other" (special) type with the given name. These types cannot be directly
    // referenced from user code.
    Type(const char* name)
        : INHERITED(-1, TypeData{name, "", TypeKind::kOther}) {}

    // Create an "other" (special) type that supports field access.
    Type(const char* name, std::vector<Field> fields)
        : INHERITED(-1, TypeData{name, "", TypeKind::kOtherStruct, {std::move(fields)}}) {}

    // Create a simple type.
    Type(String name, TypeKind kind)
        : INHERITED(-1, TypeData{"", std::move(name), kind}) {
        this->typeData().fName = StringFragment(this->nameString().c_str(),
                                                this->nameString().length());
    }

    // Create a generic type which maps to the listed types.
    Type(const char* name, std::vector<const Type*> types)
        : INHERITED(-1, TypeData{name, "", TypeKind::kGeneric, {std::move(types)}}) {}

    // Create a struct type with the given fields.
    Type(int offset, String name, std::vector<Field> fields)
        : INHERITED(offset, TypeData{"", std::move(name), TypeKind::kStruct, {std::move(fields)}}) {
        this->typeData().fName = StringFragment(this->nameString().c_str(),
                                                this->nameString().length());
    }

    // Create a scalar type.
    Type(const char* name, NumberKind numberKind, int priority, bool highPrecision = false)
        : INHERITED(-1, TypeData{name, "", TypeKind::kScalar,
                    TypeData::NumberData{numberKind, priority, highPrecision, {}}}) {}

    // Create a scalar type which can be coerced to the listed types.
    Type(const char* name,
         NumberKind numberKind,
         int priority,
         std::vector<const Type*> coercibleTypes)
        : INHERITED(-1, TypeData{name, "", TypeKind::kScalar,
                    TypeData::NumberData{numberKind, priority, false,
                                                std::move(coercibleTypes)}}) {}

    // Create a nullable type.
    Type(String name, TypeKind kind, const Type& componentType)
        : INHERITED(-1, TypeData{"", std::move(name), TypeKind::kNullable,
                    TypeData::DimensionsData{&componentType, 1, 1}}) {
        this->typeData().fName = StringFragment(this->nameString().c_str(),
                                                this->nameString().length());
    }

    // Create a vector type.
    Type(const char* name, const Type& componentType, int columns)
        : Type(name, TypeKind::kVector, componentType, columns) {}

    static constexpr int kUnsizedArray = -1;

    // Create a vector or array type.
    Type(String name, TypeKind kind, const Type& componentType, int columns)
        : INHERITED(-1, TypeData{"", std::move(name), kind,
                    TypeData::DimensionsData{&componentType, columns, 1}}) {
        SkASSERT(this->columns() > 0 || (this->typeKind() == TypeKind::kArray &&
                                         this->columns() == kUnsizedArray));
        this->typeData().fName = StringFragment(this->nameString().c_str(),
                                                this->nameString().length());
    }

    // Create a matrix type.
    Type(const char* name, const Type& componentType, int columns, int rows)
        : INHERITED(-1, TypeData{name, "", TypeKind::kMatrix,
                    TypeData::DimensionsData{&componentType, columns, rows}}) {}

    // Create a texture type.
    Type(const char* name, SpvDim_ dimensions, bool isDepth, bool isArrayed, bool isMultisampled,
         bool isSampled)
        : INHERITED(-1, TypeData{name, "", TypeKind::kTexture,
                    TypeData::TextureData{nullptr, dimensions, isDepth, isArrayed, isMultisampled,
                                          isSampled}}) {}

    // Create a sampler type.
    Type(const char* name, const Type& textureType)
        : INHERITED(-1, TypeData{name, "sampler type", TypeKind::kSampler,
                    TypeData::TextureData{&textureType, textureType.dimensions(),
                                          textureType.isDepth(), textureType.isArrayed(),
                                          textureType.isMultisampled(),
                                          textureType.isSampled()}}) {}

    StringFragment name() const override {
        return this->typeData().fName;
    }

    const String& nameString() {
        return this->typeData().fNameString;
    }

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
        return this->typeData().fTypeKind;
    }

    /**
     * Returns true if this is a numeric scalar type.
     */
    bool isNumber() const {
        return this->typeKind() == TypeKind::kScalar &&
               this->typeData().fExtra.fNumberData.fNumberKind != NumberKind::kNonnumeric;
    }

    /**
     * Returns true if this is a floating-point scalar type (float or half).
     */
    bool isFloat() const {
        return this->isNumber() &&
               this->typeData().fExtra.fNumberData.fNumberKind == NumberKind::kFloat;
    }

    /**
     * Returns true if this is a signed scalar type (int or short).
     */
    bool isSigned() const {
        return this->isNumber() &&
               this->typeData().fExtra.fNumberData.fNumberKind == NumberKind::kSigned;
    }

    /**
     * Returns true if this is an unsigned scalar type (uint or ushort).
     */
    bool isUnsigned() const {
        return this->isNumber() &&
               this->typeData().fExtra.fNumberData.fNumberKind == NumberKind::kUnsigned;
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
        return this->typeData().fExtra.fNumberData.fPriority;
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
        return *this->typeData().fExtra.fDimensionsData.fComponentType;
    }

    /**
     * For texturesamplers, returns the type of texture it samples (e.g., sampler2D has
     * a texture type of texture2D).
     */
    const Type& textureType() const {
        return *this->typeData().fExtra.fTextureData.fTextureType;
    }

    /**
     * For nullable types, returns the base type, otherwise returns the type itself.
     */
    const Type& nonnullable() const {
        if (this->typeKind() == TypeKind::kNullable) {
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
        switch (this->typeKind()) {
            case TypeKind::kVector:
            case TypeKind::kMatrix:
            case TypeKind::kArray:
                return this->typeData().fExtra.fDimensionsData.fColumns;
            default:
                return 1;
        }
    }

    /**
     * For matrices, returns the number of rows (e.g. mat2x4 returns 4). For vectors and scalars,
     * returns 1. For all other types, causes an SkASSERTion failure.
     */
    int rows() const {
        switch (this->typeKind()) {
            case TypeKind::kVector:
            case TypeKind::kMatrix:
            case TypeKind::kArray:
                return this->typeData().fExtra.fDimensionsData.fRows;
            default:
                return 1;
        }
    }

    const std::vector<Field>& fields() const {
        SkASSERT(this->typeData().fExtraKind == IRNode::TypeData::ExtraKind::kFields);
        return this->typeData().fExtra.fFields;
    }

    /**
     * For generic types, returns the types that this generic type can substitute for. For other
     * types, returns a list of other types that this type can be coerced into.
     */
    const std::vector<const Type*>& coercibleTypes() const {
        switch (this->typeKind()) {
            case TypeKind::kScalar:
                SkASSERT(this->typeData().fExtraKind == TypeData::ExtraKind::kNumberData);
                return this->typeData().fExtra.fNumberData.fCoercibleTypes;
            case TypeKind::kGeneric:
                SkASSERT(this->typeData().fExtraKind == TypeData::ExtraKind::kCoercibleTypes);
                return this->typeData().fExtra.fCoercibleTypes;
            default:
                static std::vector<const Type*> empty;
                return empty;
        }
    }

    SpvDim_ dimensions() const {
        SkASSERT(this->typeKind() == TypeKind::kSampler || this->typeKind() == TypeKind::kTexture);
        return this->typeData().fExtra.fTextureData.fDimensions;
    }

    bool isDepth() const {
        SkASSERT(this->typeKind() == TypeKind::kSampler || this->typeKind() == TypeKind::kTexture);
        return this->typeData().fExtra.fTextureData.fIsDepth;
    }

    bool isArrayed() const {
        SkASSERT(this->typeKind() == TypeKind::kSampler || this->typeKind() == TypeKind::kTexture);
        return this->typeData().fExtra.fTextureData.fIsArrayed;
    }

    bool isMultisampled() const {
        SkASSERT(this->typeKind() == TypeKind::kSampler || this->typeKind() == TypeKind::kTexture);
        return this->typeData().fExtra.fTextureData.fIsMultisampled;
    }

    bool isSampled() const {
        SkASSERT(this->typeKind() == TypeKind::kSampler || this->typeKind() == TypeKind::kTexture);
        return this->typeData().fExtra.fTextureData.fIsSampled;
    }

    bool highPrecision() const {
        switch (this->typeKind()) {
            case TypeKind::kScalar:
                return this->typeData().fExtra.fNumberData.fHighPrecision;
            case TypeKind::kArray:
            case TypeKind::kMatrix:
            case TypeKind::kVector:
                return this->componentType().highPrecision();
            default:
                return false;
        }
    }

    /**
     * Returns the corresponding vector or matrix type with the specified number of columns and
     * rows.
     */
    const Type& toCompound(const Context& context, int columns, int rows) const;

private:
    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
