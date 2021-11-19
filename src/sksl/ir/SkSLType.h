/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TYPE
#define SKSL_TYPE

#include "include/core/SkStringView.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/spirv.h"
#include <algorithm>
#include <climits>
#include <vector>
#include <memory>

namespace SkSL {

class Context;
class SymbolTable;

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
    inline static constexpr Kind kSymbolKind = Kind::kType;
    inline static constexpr int kMaxAbbrevLength = 3;

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
        kGeneric,
        kLiteral,
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
        kBlender,
    };

    enum class NumberKind : int8_t {
        kFloat,
        kSigned,
        kUnsigned,
        kBoolean,
        kNonnumeric
    };

    Type(const Type& other) = delete;

    /** Creates an array type. */
    static std::unique_ptr<Type> MakeArrayType(skstd::string_view name, const Type& componentType,
                                               int columns);

    /** Converts a component type and a size (float, 10) into an array name ("float[10]"). */
    String getArrayName(int arraySize) const;

    /**
     * Create a generic type which maps to the listed types--e.g. $genType is a generic type which
     * can match float, float2, float3 or float4.
     */
    static std::unique_ptr<Type> MakeGenericType(const char* name, std::vector<const Type*> types);

    /** Create a type for literal scalars. */
    static std::unique_ptr<Type> MakeLiteralType(const char* name, const Type& scalarType,
                                                 int8_t priority);

    /** Create a matrix type. */
    static std::unique_ptr<Type> MakeMatrixType(skstd::string_view name, const char* abbrev,
                                                const Type& componentType, int columns,
                                                int8_t rows);

    /** Create a sampler type. */
    static std::unique_ptr<Type> MakeSamplerType(const char* name, const Type& textureType);

    /** Create a scalar type. */
    static std::unique_ptr<Type> MakeScalarType(skstd::string_view name, const char* abbrev,
                                                Type::NumberKind numberKind, int8_t priority,
                                                int8_t bitWidth);

    /**
     * Create a "special" type with the given name, abbreviation, and TypeKind.
     */
    static std::unique_ptr<Type> MakeSpecialType(const char* name, const char* abbrev,
                                                 Type::TypeKind typeKind);

    /** Creates a struct type with the given fields. */
    static std::unique_ptr<Type> MakeStructType(int line,
                                                skstd::string_view name,
                                                std::vector<Field> fields,
                                                bool interfaceBlock = false);

    /** Create a texture type. */
    static std::unique_ptr<Type> MakeTextureType(const char* name, SpvDim_ dimensions,
                                                 bool isDepth, bool isArrayedTexture,
                                                 bool isMultisampled, bool isSampled);

    /** Create a vector type. */
    static std::unique_ptr<Type> MakeVectorType(skstd::string_view name, const char* abbrev,
                                                const Type& componentType, int columns);

    template <typename T>
    bool is() const {
        return this->typeKind() == T::kTypeKind;
    }

    template <typename T>
    const T& as() const {
        SkASSERT(this->is<T>());
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->is<T>());
        return static_cast<T&>(*this);
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
        return !(this->isArray() || this->isStruct());
    }

    String displayName() const {
        return String(this->scalarTypeForLiteral().name());
    }

    String description() const override {
        return this->displayName();
    }

    /** Returns true if the program supports this type. Strict ES2 programs can't use ES3 types. */
    bool isAllowedInES2(const Context& context) const;

    /** Returns true if this type is legal to use in a strict-ES2 program. */
    virtual bool isAllowedInES2() const {
        return true;
    }

    /** Returns true if this type is either private, or contains a private field (recursively). */
    virtual bool isPrivate() const {
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
    virtual NumberKind numberKind() const {
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
        switch (this->numberKind()) {
            case NumberKind::kFloat:
            case NumberKind::kSigned:
            case NumberKind::kUnsigned:
                return true;
            default:
                return false;
        }
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
        switch (this->numberKind()) {
            case NumberKind::kSigned:
            case NumberKind::kUnsigned:
                return true;
            default:
                return false;
        }
    }

    /**
     * Returns true if this is an "opaque type" (an external object which the shader references in
     * some fashion). https://www.khronos.org/opengl/wiki/Data_Type_(GLSL)#Opaque_types
     */
    bool isOpaque() const {
        switch (fTypeKind) {
            case TypeKind::kBlender:
            case TypeKind::kColorFilter:
            case TypeKind::kSampler:
            case TypeKind::kSeparateSampler:
            case TypeKind::kShader:
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
    virtual int priority() const {
        SkDEBUGFAIL("not a number type");
        return -1;
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
    virtual const Type& componentType() const {
        return *this;
    }

    /**
     * For texturesamplers, returns the type of texture it samples (e.g., sampler2D has
     * a texture type of texture2D).
     */
    virtual const Type& textureType() const {
        SkDEBUGFAIL("not a texture type");
        return *this;
    }

    /**
     * For matrices and vectors, returns the number of columns (e.g. both mat3 and float3 return 3).
     * For scalars, returns 1. For arrays, returns either the size of the array (if known) or -1.
     * For all other types, causes an assertion failure.
     */
    virtual int columns() const {
        SkDEBUGFAIL("type does not have columns");
        return -1;
    }

    /**
     * For matrices, returns the number of rows (e.g. mat2x4 returns 4). For vectors and scalars,
     * returns 1. For all other types, causes an assertion failure.
     */
    virtual int rows() const {
        SkDEBUGFAIL("type does not have rows");
        return -1;
    }

    /** For integer types, returns the minimum value that can fit in the type. */
    int64_t minimumValue() const {
        SkASSERT(this->isInteger());
        constexpr int64_t k1 = 1;  // ensures that `1 << n` is evaluated as 64-bit
        return this->isUnsigned() ? 0 : -(k1 << (this->bitWidth() - 1));
    }

    /** For integer types, returns the maximum value that can fit in the type. */
    int64_t maximumValue() const {
        SkASSERT(this->isInteger());
        constexpr int64_t k1 = 1;  // ensures that `1 << n` is evaluated as 64-bit
        return (this->isUnsigned() ? (k1 << this->bitWidth())
                                   : (k1 << (this->bitWidth() - 1))) - 1;
    }

    /**
     * Returns the number of scalars needed to hold this type.
     */
    virtual size_t slotCount() const {
        return 0;
    }

    virtual const std::vector<Field>& fields() const {
        SK_ABORT("Internal error: not a struct");
    }

    /**
     * For generic types, returns the types that this generic type can substitute for.
     */
    virtual const std::vector<const Type*>& coercibleTypes() const {
        SK_ABORT("Internal error: not a generic type");
    }

    virtual SpvDim_ dimensions() const {
        SkASSERT(false);
        return SpvDim1D;
    }

    virtual bool isDepth() const {
        SkASSERT(false);
        return false;
    }

    virtual bool isArrayedTexture() const {
        SkASSERT(false);
        return false;
    }

    bool isVoid() const {
        return fTypeKind == TypeKind::kVoid;
    }

    virtual bool isScalar() const {
        return false;
    }

    virtual bool isLiteral() const {
        return false;
    }

    virtual const Type& scalarTypeForLiteral() const {
        return *this;
    }

    virtual bool isVector() const {
        return false;
    }

    virtual bool isMatrix() const {
        return false;
    }

    virtual bool isArray() const {
        return false;
    }

    virtual bool isStruct() const {
        return false;
    }

    virtual bool isInterfaceBlock() const {
        return false;
    }

    // Is this type something that can be bound & sampled from an SkRuntimeEffect?
    // Includes types that represent stages of the Skia pipeline (colorFilter, shader, blender).
    bool isEffectChild() const {
        return fTypeKind == TypeKind::kColorFilter ||
               fTypeKind == TypeKind::kShader ||
               fTypeKind == TypeKind::kBlender;
    }

    virtual bool isMultisampled() const {
        SkASSERT(false);
        return false;
    }

    virtual bool isSampled() const {
        SkASSERT(false);
        return false;
    }

    bool hasPrecision() const {
        return this->componentType().isNumber() || fTypeKind == TypeKind::kSampler;
    }

    bool highPrecision() const {
        return this->bitWidth() >= 32;
    }

    virtual int bitWidth() const {
        return 0;
    }

    bool isOrContainsArray() const;

    /**
     * Returns true if this type is a struct that is too deeply nested.
     */
    bool isTooDeeplyNested() const;

    /**
     * Returns the corresponding vector or matrix type with the specified number of columns and
     * rows.
     */
    const Type& toCompound(const Context& context, int columns, int rows) const;

    /**
     * Returns a type which honors the precision qualifiers set in Modifiers. e.g., kMediump_Flag
     * when applied to `float2` will return `half2`. Generates an error if the precision qualifiers
     * don't make sense, e.g. `highp bool` or `mediump MyStruct`.
     */
    const Type* applyPrecisionQualifiers(const Context& context,
                                         Modifiers* modifiers,
                                         SymbolTable* symbols,
                                         int line) const;

    /**
     * Coerces the passed-in expression to this type. If the types are incompatible, reports an
     * error and returns null.
     */
    std::unique_ptr<Expression> coerceExpression(std::unique_ptr<Expression> expr,
                                                 const Context& context) const;

    /** Detects any IntLiterals in the expression which can't fit in this type. */
    bool checkForOutOfRangeLiteral(const Context& context, const Expression& expr) const;

    /** Checks if `value` can fit in this type. The type must be scalar. */
    bool checkForOutOfRangeLiteral(const Context& context, double value, int line) const;

    /**
     * Verifies that the expression is a valid constant array size for this type. Returns the array
     * size, or zero if the expression isn't a valid literal value.
     */
    SKSL_INT convertArraySize(const Context& context, std::unique_ptr<Expression> size) const;

protected:
    Type(skstd::string_view name, const char* abbrev, TypeKind kind, int line = -1)
        : INHERITED(line, kSymbolKind, name)
        , fTypeKind(kind) {
        SkASSERT(strlen(abbrev) <= kMaxAbbrevLength);
        strcpy(fAbbreviatedName, abbrev);
    }

private:
    bool isTooDeeplyNested(int limit) const;

    using INHERITED = Symbol;

    char fAbbreviatedName[kMaxAbbrevLength + 1] = {};
    TypeKind fTypeKind;
};

}  // namespace SkSL

#endif
