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
    static constexpr int kMaxAbbrevLength = 3;

    struct Field {
        Field(Modifiers modifiers, skstd::string_view name, const Type* type)
        : fModifiers(modifiers)
        , fName(name)
        , fType(std::move(type)) {}

        String description() const {
            return fType->displayName() + " " + fName + ";";
        }

        Modifiers fModifiers;
        skstd::string_view fName;
        const Type* fType;
    };

    enum class TypeKind : int8_t {
        kArray,
        kEnum,
        kFragmentProcessor,
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

        // Types that represent stages in the Skia pipeline
        kColorFilter,
        kShader,
    };

    enum class NumberKind : int8_t {
        kFloat,
        kSigned,
        kUnsigned,
        kBoolean,
        kNonnumeric
    };

    Type(const Type& other) = delete;

    ~Type() override;

    /** Creates an enum type. */
    static std::unique_ptr<Type> MakeEnumType(String name) {
        return std::unique_ptr<Type>(new Type(std::move(name), "e", TypeKind::kEnum));
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
        return std::unique_ptr<Type>(new Type(std::move(name), componentType.abbreviatedName(),
                                              TypeKind::kArray, componentType, columns));
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
        return String(this->scalarTypeForLiteral().name());
    }

    String description() const override {
        return this->displayName();
    }

    bool isPrivate() const {
        return this->name().starts_with("$");
    }

    bool operator==(const Type& other) const {
        return this->name() == other.name();
    }

    bool operator!=(const Type& other) const {
        return this->name() != other.name();
    }

    /**
     * Returns an abbreviated name of the type, meant for name-mangling. (e.g. float4x4 -> f44)
     */
    const char* abbreviatedName() const {
        return fAbbreviatedName;
    }

    /**
     * Returns the category (scalar, vector, matrix, etc.) of this type.
     */
    TypeKind typeKind() const {
        return fTypeKind;
    }

    /**
     * Returns the NumberKind of this type (always kNonnumeric for non-scalar values).
     */
    NumberKind numberKind() const {
        if (fTypeKind == TypeKind::kScalar) {
            return fData.fScalarData.fNumberKind;
        }
        return NumberKind::kNonnumeric;
    }

    /**
     * Returns true if this type is a bool.
     */
    bool isBoolean() const {
        return this->numberKind() == NumberKind::kBoolean;
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
        return this->numberKind() == NumberKind::kFloat;
    }

    /**
     * Returns true if this is a signed scalar type (int or short).
     */
    bool isSigned() const {
        return this->numberKind() == NumberKind::kSigned;
    }

    /**
     * Returns true if this is an unsigned scalar type (uint or ushort).
     */
    bool isUnsigned() const {
        return this->numberKind() == NumberKind::kUnsigned;
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
            case TypeKind::kColorFilter:
            case TypeKind::kFragmentProcessor:
            case TypeKind::kOther:
            case TypeKind::kSampler:
            case TypeKind::kSeparateSampler:
            case TypeKind::kShader:
            case TypeKind::kTexture:
            case TypeKind::kVoid:
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
        SkASSERT(this->isScalar());
        return fData.fScalarData.fPriority;
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
        switch (fTypeKind) {
            case TypeKind::kArray:
            case TypeKind::kVector:
                return *fData.fArrayData.fComponentType;
            case TypeKind::kMatrix:
                return *fData.fMatrixData.fComponentType;
            default:
                return *this;
        }
    }

    /**
     * For texturesamplers, returns the type of texture it samples (e.g., sampler2D has
     * a texture type of texture2D).
     */
    const Type& textureType() const {
        SkASSERT(fTypeKind == TypeKind::kSampler);
        return *fData.fTextureData.fTextureType;
    }

    /**
     * For matrices and vectors, returns the number of columns (e.g. both mat3 and float3 return 3).
     * For scalars, returns 1. For arrays, returns either the size of the array (if known) or -1.
     * For all other types, causes an assertion failure.
     */
    int columns() const {
        SkASSERT(this->isScalar() || this->isVector() || this->isMatrix() || this->isArray());
        if (this->isMatrix()) {
            return fData.fMatrixData.fColumns;
        }
        if (this->isScalar()) {
            return 1;
        }
        return fData.fArrayData.fColumns;
    }

    /**
     * For matrices, returns the number of rows (e.g. mat2x4 returns 4). For vectors and scalars,
     * returns 1. For all other types, causes an assertion failure.
     */
    int rows() const {
        SkASSERT(this->isScalar() || this->isVector() || this->isMatrix());
        if (this->isMatrix()) {
            return fData.fMatrixData.fRows;
        }
        return 1;
    }

    /** For integer types, returns the minimum value that can fit in the type. */
    int64_t minimumValue() const {
        SkASSERT(this->isInteger());
        constexpr int64_t k1 = 1;  // ensures that `1 << n` is evaluated as 64-bit
        return this->isUnsigned() ? 0 : -(k1 << (fData.fScalarData.fBitWidth - 1));
    }

    /** For integer types, returns the maximum value that can fit in the type. */
    int64_t maximumValue() const {
        SkASSERT(this->isInteger());
        constexpr int64_t k1 = 1;  // ensures that `1 << n` is evaluated as 64-bit
        return (this->isUnsigned() ? (k1 << fData.fScalarData.fBitWidth)
                                   : (k1 << (fData.fScalarData.fBitWidth - 1))) - 1;
    }

    /**
     * Returns the number of scalars needed to hold this type.
     */
    size_t slotCount() const {
        switch (this->typeKind()) {
            case Type::TypeKind::kColorFilter:
            case Type::TypeKind::kFragmentProcessor:
            case Type::TypeKind::kGeneric:
            case Type::TypeKind::kOther:
            case Type::TypeKind::kSampler:
            case Type::TypeKind::kSeparateSampler:
            case Type::TypeKind::kShader:
            case Type::TypeKind::kTexture:
            case Type::TypeKind::kVoid:
                return 0;

            case Type::TypeKind::kScalar:
            case Type::TypeKind::kEnum:
                return 1;

            case Type::TypeKind::kVector:
                return this->columns();

            case Type::TypeKind::kMatrix:
                return this->columns() * this->rows();

            case Type::TypeKind::kStruct: {
                size_t slots = 0;
                for (const Field& field : this->fields()) {
                    slots += field.fType->slotCount();
                }
                return slots;
            }
            case Type::TypeKind::kArray:
                SkASSERT(this->columns() > 0);
                return this->columns() * this->componentType().slotCount();
        }
        SkUNREACHABLE;
    }

    const std::vector<Field>& fields() const {
        SkASSERT(this->isStruct() && fData.fFields);
        return *fData.fFields;
    }

    /**
     * For generic types, returns the types that this generic type can substitute for.
     */
    const std::vector<const Type*>& coercibleTypes() const {
        SkASSERT(fData.fCoercibleTypes && fData.fCoercibleTypes->size() > 0);
        return *fData.fCoercibleTypes;
    }

    SpvDim_ dimensions() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fData.fTextureData.fDimensions;
    }

    bool isDepth() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fData.fTextureData.fIsDepth;
    }

    bool isArrayedTexture() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fData.fTextureData.fIsArrayed;
    }

    bool isVoid() const {
        return fTypeKind == TypeKind::kVoid;
    }

    bool isScalar() const {
        return fTypeKind == TypeKind::kScalar;
    }

    bool isLiteral() const {
        return this->isScalar() && fData.fScalarData.fScalarTypeForLiteral != nullptr;
    }

    const Type& scalarTypeForLiteral() const {
        return this->isLiteral() ? *fData.fScalarData.fScalarTypeForLiteral : *this;
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

    bool isFragmentProcessor() const {
        return fTypeKind == TypeKind::kFragmentProcessor;
    }

    // Is this type something that can be bound & sampled from an SkRuntimeEffect?
    // Includes types that represent stages of the Skia pipeline (colorFilter and shader).
    bool isEffectChild() const {
        return fTypeKind == TypeKind::kColorFilter || fTypeKind == TypeKind::kShader;
    }

    bool isMultisampled() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fData.fTextureData.fIsMultisampled;
    }

    bool isSampled() const {
        SkASSERT(TypeKind::kSampler == fTypeKind || TypeKind::kTexture == fTypeKind);
        return fData.fTextureData.fIsSampled;
    }

    bool hasPrecision() const {
        return this->componentType().isNumber() || fTypeKind == TypeKind::kSampler;
    }

    bool highPrecision() const {
        return this->bitWidth() >= 32;
    }

    int bitWidth() const {
        const Type& componentType = this->componentType();
        if (&componentType != this) {
            return componentType.bitWidth();
        }
        if (this->isScalar()) {
            return fData.fScalarData.fBitWidth;
        }
        return -1;
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

    /** Detects any IntLiterals in the expression which can't fit in this type. */
    bool checkForOutOfRangeLiteral(const Context& context, const Expression& expr) const;

private:
    friend class BuiltinTypes;

    using INHERITED = Symbol;

    // Constructor for MakeSpecialType.
    Type(const char* name, const char* abbrev, TypeKind kind)
            : INHERITED(/*offset=*/-1, kSymbolKind, name)
            , fTypeKind(kind) {
        SkASSERT(strlen(abbrev) <= kMaxAbbrevLength);
        strcpy(fAbbreviatedName, abbrev);
    }

    // Constructor for MakeEnumType.
    Type(String name, const char* abbrev, TypeKind kind)
            : INHERITED(/*offset=*/-1, kSymbolKind, /*name=*/"")
            , fNameString(std::move(name))
            , fTypeKind(kind) {
        fName = skstd::string_view(fNameString.c_str(), fNameString.length());
        SkASSERT(strlen(abbrev) <= kMaxAbbrevLength);
        strcpy(fAbbreviatedName, abbrev);
    }

    // Constructor for MakeGenericType.
    Type(const char* name, std::vector<const Type*> types)
            : INHERITED(/*offset=*/-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kGeneric) {
        new (&fData.fCoercibleTypes) std::unique_ptr<std::vector<const Type*>>(
                new std::vector<const Type*>(std::move(types)));
        strcpy(fAbbreviatedName, "G");
    }

    // Constructor for MakeScalarType.
    Type(const char* name, const char* abbrev, NumberKind numberKind,
         int8_t priority, int8_t bitWidth)
            : INHERITED(/*offset=*/-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kScalar) {
        SkASSERT(strlen(abbrev) <= kMaxAbbrevLength);
        strcpy(fAbbreviatedName, abbrev);
        fData.fScalarData.fNumberKind = numberKind;
        fData.fScalarData.fBitWidth = bitWidth;
        fData.fScalarData.fPriority = priority;
        fData.fScalarData.fScalarTypeForLiteral = nullptr;
    }

    // Constructor for MakeLiteralType.
    Type(const char* name, const Type& scalarType, int8_t priority)
            : INHERITED(/*offset=*/-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kScalar) {
        strcpy(fAbbreviatedName, "L");
        fData.fScalarData.fNumberKind = scalarType.numberKind();
        fData.fScalarData.fBitWidth = scalarType.bitWidth();
        fData.fScalarData.fPriority = priority;
        fData.fScalarData.fScalarTypeForLiteral = &scalarType;
    }

    // Constructor shared by MakeVectorType and MakeArrayType.
    Type(String name, const char* abbrev, TypeKind kind, const Type& componentType, int columns)
            : INHERITED(/*offset=*/-1, kSymbolKind, /*name=*/"")
            , fNameString(std::move(name))
            , fTypeKind(kind) {
        SkASSERT(strlen(abbrev) <= kMaxAbbrevLength);
        strcpy(fAbbreviatedName, abbrev);
        fData.fArrayData.fComponentType = &componentType;
        fData.fArrayData.fColumns = columns;

        if (this->isArray()) {
            // Allow either explicitly-sized or unsized arrays.
            SkASSERT(this->columns() > 0 || this->columns() == kUnsizedArray);
            // Disallow multi-dimensional arrays.
            SkASSERT(!this->componentType().isArray());
        } else {
            SkASSERT(this->columns() > 0);
        }
        fName = skstd::string_view(fNameString.c_str(), fNameString.length());
    }

    // Constructor for MakeMatrixType.
    Type(const char* name, const char* abbrev, const Type& componentType, int columns, int8_t rows)
            : INHERITED(/*offset=*/-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kMatrix) {
        SkASSERT(strlen(abbrev) <= kMaxAbbrevLength);
        strcpy(fAbbreviatedName, abbrev);
        fData.fMatrixData.fColumns = columns;
        fData.fMatrixData.fRows = rows;
        fData.fMatrixData.fComponentType = &componentType;
    }

    // Constructor for MakeStructType.
    Type(int offset, String name, std::vector<Field> fields)
            : INHERITED(offset, kSymbolKind, "")
            , fNameString(std::move(name))
            , fTypeKind(TypeKind::kStruct) {
        fName = skstd::string_view(fNameString.c_str(), fNameString.length());
        strcpy(fAbbreviatedName, "S");
        new (&fData.fFields) std::unique_ptr<std::vector<Field>>(
                new std::vector<Field>(std::move(fields)));
    }

    // Constructor for MakeTextureType.
    Type(const char* name, SpvDim_ dimensions, bool isDepth, bool isArrayedTexture,
         bool isMultisampled, bool isSampled)
            : INHERITED(/*offset=*/-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kTexture) {
        strcpy(fAbbreviatedName, "T");
        fData.fTextureData.fIsDepth = isDepth;
        fData.fTextureData.fIsArrayed = isArrayedTexture;
        fData.fTextureData.fIsMultisampled = isMultisampled;
        fData.fTextureData.fIsSampled = isSampled;
        fData.fTextureData.fDimensions = dimensions;
    }

    // Constructor for MakeSamplerType.
    Type(const char* name, const Type& textureType)
            : INHERITED(/*offset=*/-1, kSymbolKind, name)
            , fTypeKind(TypeKind::kSampler) {
        strcpy(fAbbreviatedName, "Z");
        fData.fTextureData.fIsDepth = textureType.isDepth();
        fData.fTextureData.fIsArrayed = textureType.isArrayedTexture();
        fData.fTextureData.fIsMultisampled = textureType.isMultisampled();
        fData.fTextureData.fIsSampled = textureType.isSampled();
        fData.fTextureData.fDimensions = textureType.dimensions();
        fData.fTextureData.fTextureType = &textureType;
    }

    union Data {
        Data() {}

        ~Data() {}

        // TypeKind::kScalar
        struct {
            const Type* fScalarTypeForLiteral;
            NumberKind fNumberKind;
            int8_t fPriority = -1;
            int8_t fBitWidth = 0;
        } fScalarData;

        // TypeKind::kVector or kArray
        struct {
            const Type* fComponentType;
            SKSL_INT fColumns;
        } fArrayData;

        // TypeKind::kMatrix
        struct {
            const Type* fComponentType;
            SKSL_INT fColumns;
            int8_t fRows;
        } fMatrixData;

        // TypeKind::kGeneric
        std::unique_ptr<std::vector<const Type*>> fCoercibleTypes;

        // TypeKind::kStruct
        std::unique_ptr<std::vector<Field>> fFields;

        // TypeKind::kTexture or kSampler
        struct {
            const Type* fTextureType;
            SpvDim_ fDimensions;
            bool fIsDepth : 1;
            bool fIsArrayed : 1;
            bool fIsMultisampled : 1;
            bool fIsSampled : 1;
        } fTextureData;
    } fData;

    String fNameString;
    char fAbbreviatedName[kMaxAbbrevLength + 1] = {0};
    TypeKind fTypeKind;
};

}  // namespace SkSL

#endif
