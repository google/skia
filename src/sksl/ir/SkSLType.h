/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIASL_TYPE
#define SKIASL_TYPE

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/spirv.h"
#include <algorithm>
#include <climits>
#include <vector>
#include <memory>

namespace SkSL {

class BuiltinTypes;
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
class Type final : public Symbol {
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
        kMatrix,
        kOther,
        kSampler,
        kSeparateSampler,
        kScalar,
        kStruct,
        kTexture,
        kVector,
        kVoid,
    };

    enum class NumberKind {
        kFloat,
        kSigned,
        kUnsigned,
        kBoolean,
        kNonnumeric
    };

    Type(const Type& other) = delete;

    /** Creates an enum type. */
    static std::unique_ptr<Type> MakeEnumType(String name) {
        return std::unique_ptr<Type>(new Type(std::move(name), TypeKind::kEnum));
    }

    /** Creates a struct type with the given fields. */
    static std::unique_ptr<Type> MakeStructType(int offset, String name,
                                                std::vector<Field> fields) {
        return std::unique_ptr<Type>(new Type(offset, std::move(name), std::move(fields)));
    }

    /** Creates an array type. */
    static constexpr int kUnsizedArray = -1;
    static std::unique_ptr<Type> MakeArrayType(String name, const Type& componentType,
                                               int columns) {
        return std::unique_ptr<Type>(new Type(std::move(name), TypeKind::kArray, componentType,
                                              columns));
    }

    /** Creates a clone of this Type, if needed, and inserts it into a different symbol table. */
    const Type* clone(SymbolTable* symbolTable) const;

    /**
     * Returns true if this type is known to come from BuiltinTypes. If this returns true, the Type
     * will always be available in the root SymbolTable and never needs to be copied to migrate an
     * Expression from one location to another. If it returns false, the Type might not exist in a
     * separate SymbolTable and you'll need to consider copying it.
     */
    bool isInBuiltinTypes() const {
        return !(this->isArray() || this->isStruct() || this->isEnum());
    }

    String displayName() const {
        return this->scalarTypeForLiteral().name();
    }

    String description() const override {
        return this->displayName();
    }

    bool isPrivate() const {
        return this->name().startsWith("$");
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

    NumberKind numberKind() const {
        return fNumberKind;
    }

    /**
     * Returns true if this type is a bool.
     */
    bool isBoolean() const {
        return fNumberKind == NumberKind::kBoolean;
    }

    /**
     * Returns true if this is a numeric scalar type.
     */
    bool isNumber() const {
        return this->isFloat() || this->isInteger();
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
        return this->isSigned() || this->isUnsigned();
    }

    /**
     * Returns true if this is an "opaque type" (an external object which the shader references in
     * some fashion), or void. https://www.khronos.org/opengl/wiki/Data_Type_(GLSL)#Opaque_types
     */
    bool isOpaque() const {
        switch (fTypeKind) {
            case TypeKind::kOther:
            case TypeKind::kVoid:
            case TypeKind::kSampler:
            case TypeKind::kSeparateSampler:
            case TypeKind::kTexture:
                return true;
            default:
                return false;
        }
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
     * type of Float). For arrays, returns the base type. For all other types, returns the type
     * itself.
     */
    const Type& componentType() const {
        if (fComponentType) {
            return *fComponentType;
        }
        return *this;
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
     * For matrices and vectors, returns the number of columns (e.g. both mat3 and float3 return 3).
     * For scalars, returns 1. For arrays, returns either the size of the array (if known) or -1.
     * For all other types, causes an SkASSERTion failure.
     */
    int columns() const {
        SkASSERT(this->isScalar() || this->isVector() || this->isMatrix() || this->isArray());
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
        SkASSERT(this->isStruct());
        return fFields;
    }

    /**
     * For generic types, returns the types that this generic type can substitute for.
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

    bool isArrayedTexture() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fIsArrayed;
    }

    bool isVoid() const {
        return fTypeKind == TypeKind::kVoid;
    }

    bool isScalar() const {
        return fTypeKind == TypeKind::kScalar;
    }

    bool isLiteral() const {
        return fScalarTypeForLiteral != nullptr;
    }

    const Type& scalarTypeForLiteral() const {
        return fScalarTypeForLiteral ? *fScalarTypeForLiteral : *this;
    }

    bool isVector() const {
        return fTypeKind == TypeKind::kVector;
    }

    bool isMatrix() const {
        return fTypeKind == TypeKind::kMatrix;
    }

    bool isArray() const {
        return fTypeKind == TypeKind::kArray;
    }

    bool isStruct() const {
        return fTypeKind == TypeKind::kStruct;
    }

    bool isEnum() const {
        return fTypeKind == TypeKind::kEnum;
    }

    bool isMultisampled() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fIsMultisampled;
    }

    bool isSampled() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fIsSampled;
    }

    bool hasPrecision() const {
        return this->componentType().isNumber() || fTypeKind == TypeKind::kSampler;
    }

    bool highPrecision() const {
        if (fComponentType) {
            return fComponentType->highPrecision();
        }
        return fHighPrecision;
    }

    bool isOrContainsArray() const;

    /**
     * Returns the corresponding vector or matrix type with the specified number of columns and
     * rows.
     */
    const Type& toCompound(const Context& context, int columns, int rows) const;

    /**
     * Coerces the passed-in expression to this type. If the types are incompatible, reports an
     * error and returns null.
     */
    std::unique_ptr<Expression> coerceExpression(std::unique_ptr<Expression> expr,
                                                 const Context& context) const;

private:
    friend class BuiltinTypes;

    using INHERITED = Symbol;

    // Constructor for MakeOtherType.
    Type(const char* name)
            : INHERITED(-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kOther)
            , fNumberKind(NumberKind::kNonnumeric) {}

    // Constructor for MakeVoidType, MakeEnumType and MakeSeparateSamplerType.
    Type(String name, TypeKind kind)
            : INHERITED(-1, kSymbolKind, "")
            , fNameString(std::move(name))
            , fTypeKind(kind)
            , fNumberKind(NumberKind::kNonnumeric) {
        fName = StringFragment(fNameString.c_str(), fNameString.length());
    }

    // Constructor for MakeGenericType.
    Type(const char* name, std::vector<const Type*> types)
            : INHERITED(-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kGeneric)
            , fNumberKind(NumberKind::kNonnumeric)
            , fCoercibleTypes(std::move(types)) {}

    // Constructor for MakeScalarType.
    Type(const char* name, NumberKind numberKind, int priority, bool highPrecision)
            : INHERITED(-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kScalar)
            , fNumberKind(numberKind)
            , fPriority(priority)
            , fColumns(1)
            , fRows(1)
            , fHighPrecision(highPrecision) {}

    // Constructor for MakeLiteralType.
    Type(const char* name, const Type& scalarType, int priority)
            : INHERITED(-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kScalar)
            , fNumberKind(scalarType.numberKind())
            , fPriority(priority)
            , fColumns(1)
            , fRows(1)
            , fHighPrecision(scalarType.highPrecision())
            , fScalarTypeForLiteral(&scalarType) {}

    // Constructor shared by MakeVectorType and MakeArrayType.
    Type(String name, TypeKind kind, const Type& componentType, int columns)
            : INHERITED(-1, kSymbolKind, "")
            , fNameString(std::move(name))
            , fTypeKind(kind)
            , fNumberKind(NumberKind::kNonnumeric)
            , fComponentType(&componentType)
            , fColumns(columns)
            , fRows(1)
            , fDimensions(SpvDim1D) {
        if (this->isArray()) {
            // Allow either explicitly-sized or unsized arrays.
            SkASSERT(this->columns() > 0 || this->columns() == kUnsizedArray);
            // Disallow multi-dimensional arrays.
            SkASSERT(!this->componentType().isArray());
        } else {
            SkASSERT(this->columns() > 0);
        }
        fName = StringFragment(fNameString.c_str(), fNameString.length());
    }

    // Constructor for MakeMatrixType.
    Type(const char* name, const Type& componentType, int columns, int rows)
            : INHERITED(-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kMatrix)
            , fNumberKind(NumberKind::kNonnumeric)
            , fComponentType(&componentType)
            , fColumns(columns)
            , fRows(rows)
            , fDimensions(SpvDim1D) {}

    // Constructor for MakeStructType.
    Type(int offset, String name, std::vector<Field> fields)
            : INHERITED(offset, kSymbolKind, "")
            , fNameString(std::move(name))
            , fTypeKind(TypeKind::kStruct)
            , fNumberKind(NumberKind::kNonnumeric)
            , fFields(std::move(fields)) {
        fName = StringFragment(fNameString.c_str(), fNameString.length());
    }

    // Constructor for MakeTextureType.
    Type(const char* name, SpvDim_ dimensions, bool isDepth, bool isArrayedTexture,
         bool isMultisampled, bool isSampled)
            : INHERITED(-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kTexture)
            , fNumberKind(NumberKind::kNonnumeric)
            , fDimensions(dimensions)
            , fIsDepth(isDepth)
            , fIsArrayed(isArrayedTexture)
            , fIsMultisampled(isMultisampled)
            , fIsSampled(isSampled) {}

    // Constructor for MakeSamplerType.
    Type(const char* name, const Type& textureType)
            : INHERITED(-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kSampler)
            , fNumberKind(NumberKind::kNonnumeric)
            , fDimensions(textureType.dimensions())
            , fIsDepth(textureType.isDepth())
            , fIsArrayed(textureType.isArrayedTexture())
            , fIsMultisampled(textureType.isMultisampled())
            , fIsSampled(textureType.isSampled())
            , fTextureType(&textureType) {}

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
    const Type* fScalarTypeForLiteral = nullptr;
};

}  // namespace SkSL

#endif
