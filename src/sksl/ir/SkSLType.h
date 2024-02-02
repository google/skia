/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TYPE
#define SKSL_TYPE

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/spirv.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

namespace SkSL {

class Context;
class Expression;
class SymbolTable;
class Type;

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

    bool operator<=(CoercionCost rhs) const {
        return std::tie(    fImpossible,     fNarrowingCost,     fNormalCost) <=
               std::tie(rhs.fImpossible, rhs.fNarrowingCost, rhs.fNormalCost);
    }

    int  fNormalCost;
    int  fNarrowingCost;
    bool fImpossible;
};

/**
 * Represents a single field in a struct type.
 */
struct Field {
    Field(Position pos, Layout layout, ModifierFlags flags, std::string_view name, const Type* type)
            : fPosition(pos)
            , fLayout(layout)
            , fModifierFlags(flags)
            , fName(name)
            , fType(type) {}

    std::string description() const;

    Position fPosition;
    Layout fLayout;
    ModifierFlags fModifierFlags;
    std::string_view fName;
    const Type* fType;
};

/**
 * Represents a type, such as int or float4.
 */
class Type : public Symbol {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kType;
    inline static constexpr int kMaxAbbrevLength = 3;
    // Represents unspecified array dimensions, as in `int[]`.
    inline static constexpr int kUnsizedArray = -1;

    enum class TypeKind : int8_t {
        kArray,
        kAtomic,
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

    enum class TextureAccess : int8_t {
        kSample,  // `kSample` access level allows both sampling and reading
        kRead,
        kWrite,
        kReadWrite,
    };

    Type(const Type& other) = delete;

    /** Creates an array type. `columns` may be kUnsizedArray. */
    static std::unique_ptr<Type> MakeArrayType(const Context& context, std::string_view name,
                                               const Type& componentType, int columns);

    /** Converts a component type and a size (float, 10) into an array name ("float[10]"). */
    std::string getArrayName(int arraySize) const;

    /**
     * Creates an alias which maps to another type.
     */
    static std::unique_ptr<Type> MakeAliasType(std::string_view name, const Type& targetType);

    /**
     * Create a generic type which maps to the listed types--e.g. $genType is a generic type which
     * can match float, float2, float3 or float4.
     */
    static std::unique_ptr<Type> MakeGenericType(const char* name,
                                                 SkSpan<const Type* const> types,
                                                 const Type* slotType);

    /** Create a type for literal scalars. */
    static std::unique_ptr<Type> MakeLiteralType(const char* name, const Type& scalarType,
                                                 int8_t priority);

    /** Create a matrix type. */
    static std::unique_ptr<Type> MakeMatrixType(std::string_view name, const char* abbrev,
                                                const Type& componentType, int columns,
                                                int8_t rows);

    /** Create a sampler type. */
    static std::unique_ptr<Type> MakeSamplerType(const char* name, const Type& textureType);

    /** Create a scalar type. */
    static std::unique_ptr<Type> MakeScalarType(std::string_view name, const char* abbrev,
                                                Type::NumberKind numberKind, int8_t priority,
                                                int8_t bitWidth);

    /**
     * Create a "special" type with the given name, abbreviation, and TypeKind.
     */
    static std::unique_ptr<Type> MakeSpecialType(const char* name, const char* abbrev,
                                                 Type::TypeKind typeKind);

    /**
     * Creates a struct type with the given fields. Reports an error if the struct is not
     * well-formed.
     */
    static std::unique_ptr<Type> MakeStructType(const Context& context,
                                                Position pos,
                                                std::string_view name,
                                                skia_private::TArray<Field> fields,
                                                bool interfaceBlock = false);

    /** Create a texture type. */
    static std::unique_ptr<Type> MakeTextureType(const char* name, SpvDim_ dimensions, bool isDepth,
                                                 bool isArrayedTexture, bool isMultisampled,
                                                 TextureAccess textureAccess);

    /** Create a vector type. */
    static std::unique_ptr<Type> MakeVectorType(std::string_view name, const char* abbrev,
                                                const Type& componentType, int columns);

    /** Create an atomic type. */
    static std::unique_ptr<Type> MakeAtomicType(std::string_view name, const char* abbrev);

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
    const Type* clone(const Context& context, SymbolTable* symbolTable) const;

    /**
     * Returns true if this type is known to come from BuiltinTypes, or is declared in a module. If
     * this returns true, the Type will always be available in the root SymbolTable and never needs
     * to be copied to migrate an Expression from one location to another. If it returns false, the
     * Type might not exist in a separate SymbolTable and you'll need to consider cloning it.
     */
    virtual bool isBuiltin() const {
        return true;
    }

    std::string displayName() const {
        return std::string(this->scalarTypeForLiteral().name());
    }

    std::string description() const override {
        return this->displayName();
    }

    /** Returns true if the program supports this type. Strict ES2 programs can't use ES3 types. */
    bool isAllowedInES2(const Context& context) const;

    /** Returns true if this type is legal to use in a strict-ES2 program. */
    virtual bool isAllowedInES2() const {
        return true;
    }

    /**
     * Returns true if this type is legal to use as a uniform. If false is returned, the
     * `errorPosition` field may be populated; if it is, this position can be used to emit an extra
     * diagnostic "caused by: <a field>" for nested types.
     * Note that runtime effects enforce additional, much stricter rules about uniforms; these
     * limitations are not handled here.
     */
    virtual bool isAllowedInUniform(Position* errorPosition = nullptr) const {
        // We don't allow samplers, textures or atomics to be marked as uniforms.
        // This rules out all opaque types.
        return !this->isOpaque();
    }

    /** If this is an alias, returns the underlying type, otherwise returns this. */
    virtual const Type& resolve() const {
        return *this;
    }

    /** Returns true if these types are equal after alias resolution. */
    bool matches(const Type& other) const {
        return this->resolve().name() == other.resolve().name();
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
            case TypeKind::kAtomic:
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
     * Returns true if this is a storage texture.
     */
    bool isStorageTexture() const {
        return fTypeKind == TypeKind::kTexture && this->dimensions() != SpvDimSubpassData;
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
     * For matrix types, returns the type of a single column (`m[n]`). Asserts for all other types.
     */
    const Type& columnType(const Context& context) const {
        return this->componentType().toCompound(context, this->rows(), /*rows=*/1);
    }

    /**
     * For texture samplers, returns the type of texture it samples (e.g., sampler2D has
     * a texture type of texture2D).
     */
    virtual const Type& textureType() const {
        SkDEBUGFAIL("not a sampler type");
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

    /** Returns the minimum value that can fit in the type. */
    virtual double minimumValue() const {
        SkDEBUGFAIL("type does not have a minimum value");
        return -INFINITY;
    }

    virtual double maximumValue() const {
        SkDEBUGFAIL("type does not have a maximum value");
        return +INFINITY;
    }

    /**
     * Returns the number of scalars needed to hold this type.
     */
    virtual size_t slotCount() const {
        return 0;
    }

    /**
     * Returns the type of the value in the nth slot. For scalar, vector and matrix types, should
     * always match `componentType()`.
     */
    virtual const Type& slotType(size_t) const {
        return *this;
    }

    virtual SkSpan<const Field> fields() const {
        SK_ABORT("Internal error: not a struct");
    }

    /**
     * For generic types, returns the types that this generic type can substitute for.
     */
    virtual SkSpan<const Type* const> coercibleTypes() const {
        SkDEBUGFAIL("Internal error: not a generic type");
        return {};
    }

    virtual SpvDim_ dimensions() const {
        SkDEBUGFAIL("Internal error: not a texture type");
        return SpvDim1D;
    }

    virtual bool isDepth() const {
        SkDEBUGFAIL("Internal error: not a texture type");
        return false;
    }

    virtual bool isArrayedTexture() const {
        SkDEBUGFAIL("Internal error: not a texture type");
        return false;
    }

    bool isVoid() const {
        return fTypeKind == TypeKind::kVoid;
    }

    bool isGeneric() const {
        return fTypeKind == TypeKind::kGeneric;
    }

    bool isSampler() const {
        return fTypeKind == TypeKind::kSampler;
    }

    bool isAtomic() const {
        return this->typeKind() == TypeKind::kAtomic;
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

    virtual bool isUnsizedArray() const {
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
        SkDEBUGFAIL("not a texture type");
        return false;
    }

    virtual TextureAccess textureAccess() const {
        SkDEBUGFAIL("not a texture type");
        return TextureAccess::kSample;
    }

    bool hasPrecision() const {
        return this->componentType().isNumber() || this->isSampler();
    }

    bool highPrecision() const {
        return this->bitWidth() >= 32;
    }

    virtual int bitWidth() const {
        return 0;
    }

    virtual bool isOrContainsArray() const {
        return false;
    }

    virtual bool isOrContainsUnsizedArray() const {
        return false;
    }

    virtual bool isOrContainsAtomic() const {
        return false;
    }

    /**
     * Returns the corresponding vector or matrix type with the specified number of columns and
     * rows.
     */
    const Type& toCompound(const Context& context, int columns, int rows) const;

    /**
     * Returns a type which honors the precision and access-level qualifiers set in ModifierFlags.
     * For example:
     *  - Modifier `mediump` + Type `float2`:     Type `half2`
     *  - Modifier `readonly` + Type `texture2D`: Type `readonlyTexture2D`
     * Generates an error if the qualifiers don't make sense (`highp bool`, `writeonly MyStruct`)
     */
    const Type* applyQualifiers(const Context& context,
                                ModifierFlags* modifierFlags,
                                Position pos) const;

    /**
     * Coerces the passed-in expression to this type. If the types are incompatible, reports an
     * error and returns null.
     */
    std::unique_ptr<Expression> coerceExpression(std::unique_ptr<Expression> expr,
                                                 const Context& context) const;

    /** Detects any IntLiterals in the expression which can't fit in this type. */
    bool checkForOutOfRangeLiteral(const Context& context, const Expression& expr) const;

    /** Checks if `value` can fit in this type. The type must be scalar. */
    bool checkForOutOfRangeLiteral(const Context& context, double value, Position pos) const;

    /**
     * Reports errors and returns false if this type cannot be used as the base type for an array.
     */
    bool checkIfUsableInArray(const Context& context, Position arrayPos) const;

    /**
     * Verifies that the expression is a valid constant array size for this type. Returns the array
     * size, or reports errors and returns zero if the expression isn't a valid literal value.
     */
    SKSL_INT convertArraySize(const Context& context,
                              Position arrayPos,
                              std::unique_ptr<Expression> size) const;

    SKSL_INT convertArraySize(const Context& context,
                              Position arrayPos,
                              Position sizePos,
                              SKSL_INT size) const;

protected:
    Type(std::string_view name, const char* abbrev, TypeKind kind, Position pos = Position())
            : INHERITED(pos, kIRNodeKind, name)
            , fTypeKind(kind) {
        SkASSERT(strlen(abbrev) <= kMaxAbbrevLength);
        strcpy(fAbbreviatedName, abbrev);
    }

    const Type* applyPrecisionQualifiers(const Context& context,
                                         ModifierFlags* modifierFlags,
                                         Position pos) const;

    const Type* applyAccessQualifiers(const Context& context,
                                      ModifierFlags* modifierFlags,
                                      Position pos) const;

    /** Only structs and arrays can be created in code; all other types exist in the root. */
    bool isInRootSymbolTable() const {
        return !(this->isArray() || this->isStruct());
    }

    /** If the type is a struct, returns the depth of the struct's most deeply-nested field. */
    virtual int structNestingDepth() const {
        return 0;
    }

private:
    using INHERITED = Symbol;

    char fAbbreviatedName[kMaxAbbrevLength + 1] = {};
    TypeKind fTypeKind;
};

}  // namespace SkSL

#endif
