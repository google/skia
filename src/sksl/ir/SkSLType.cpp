/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLType.h"

#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <utility>

using namespace skia_private;

namespace SkSL {

static constexpr int kMaxStructDepth = 8;

class AliasType final : public Type {
public:
    AliasType(std::string_view name, const Type& targetType)
            : INHERITED(name, targetType.abbreviatedName(), targetType.typeKind())
            , fTargetType(targetType) {}

    const Type& resolve() const override {
        return fTargetType;
    }

    const Type& componentType() const override {
        return fTargetType.componentType();
    }

    NumberKind numberKind() const override {
        return fTargetType.numberKind();
    }

    int priority() const override {
        return fTargetType.priority();
    }

    int columns() const override {
        return fTargetType.columns();
    }

    int rows() const override {
        return fTargetType.rows();
    }

    int bitWidth() const override {
        return fTargetType.bitWidth();
    }

    bool isAllowedInES2() const override {
        return fTargetType.isAllowedInES2();
    }

    size_t slotCount() const override {
        return fTargetType.slotCount();
    }

    const Type& slotType(size_t n) const override {
        return fTargetType.slotType(n);
    }

    SpvDim_ dimensions() const override {
        return fTargetType.dimensions();
    }

    bool isDepth() const override {
        return fTargetType.isDepth();
    }

    bool isArrayedTexture() const override {
        return fTargetType.isArrayedTexture();
    }

    bool isScalar() const override {
        return fTargetType.isScalar();
    }

    bool isLiteral() const override {
        return fTargetType.isLiteral();
    }

    bool isVector() const override {
        return fTargetType.isVector();
    }

    bool isMatrix() const override {
        return fTargetType.isMatrix();
    }

    bool isArray() const override {
        return fTargetType.isArray();
    }

    bool isUnsizedArray() const override {
        return fTargetType.isUnsizedArray();
    }

    bool isStruct() const override {
        return fTargetType.isStruct();
    }

    bool isInterfaceBlock() const override {
        return fTargetType.isInterfaceBlock();
    }

    bool isMultisampled() const override {
        return fTargetType.isMultisampled();
    }

    TextureAccess textureAccess() const override {
        return fTargetType.textureAccess();
    }

    SkSpan<const Type* const> coercibleTypes() const override {
        return fTargetType.coercibleTypes();
    }

private:
    using INHERITED = Type;

    const Type& fTargetType;
};

class ArrayType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kArray;

    ArrayType(std::string_view name, const char* abbrev, const Type& componentType, int count,
              bool isBuiltin)
            : INHERITED(name, abbrev, kTypeKind)
            , fComponentType(componentType)
            , fCount(count)
            , fIsBuiltin(isBuiltin) {
        SkASSERT(count > 0 || count == kUnsizedArray);
        // Disallow multi-dimensional arrays.
        SkASSERT(!componentType.is<ArrayType>());
    }

    bool isArray() const override {
        return true;
    }

    bool isBuiltin() const override {
        return fIsBuiltin;
    }

    bool isOrContainsArray() const override {
        return true;
    }

    bool isOrContainsAtomic() const override {
        return fComponentType.isOrContainsAtomic();
    }

    bool isUnsizedArray() const override {
        return fCount == kUnsizedArray;
    }

    bool isOrContainsUnsizedArray() const override {
        return this->isUnsizedArray() || fComponentType.isOrContainsUnsizedArray();
    }

    const Type& componentType() const override {
        return fComponentType;
    }

    int columns() const override {
        return fCount;
    }

    int bitWidth() const override {
        return fComponentType.bitWidth();
    }

    bool isAllowedInES2() const override {
        return fComponentType.isAllowedInES2();
    }

    bool isAllowedInUniform(Position* errorPosition) const override {
        return fComponentType.isAllowedInUniform(errorPosition);
    }

    size_t slotCount() const override {
        SkASSERT(fCount != kUnsizedArray);
        SkASSERT(fCount > 0);
        return fCount * fComponentType.slotCount();
    }

    const Type& slotType(size_t n) const override {
        SkASSERT(fCount == kUnsizedArray || n < this->slotCount());
        return fComponentType.slotType(n % fComponentType.slotCount());
    }

private:
    using INHERITED = Type;

    const Type& fComponentType;
    int fCount;
    bool fIsBuiltin;
};

class GenericType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kGeneric;

    GenericType(const char* name, SkSpan<const Type* const> coercibleTypes, const Type* slotType)
            : INHERITED(name, "G", kTypeKind)
            , fSlotType(slotType) {
        fNumTypes = coercibleTypes.size();
        SkASSERT(fNumTypes <= std::size(fCoercibleTypes));
        std::copy(coercibleTypes.begin(), coercibleTypes.end(), fCoercibleTypes);
    }

    SkSpan<const Type* const> coercibleTypes() const override {
        return SkSpan(fCoercibleTypes, fNumTypes);
    }

    const Type& slotType(size_t) const override {
        return *fSlotType;
    }

private:
    using INHERITED = Type;

    const Type* fCoercibleTypes[9];
    const Type* fSlotType;
    size_t fNumTypes;
};

class LiteralType : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kLiteral;

    LiteralType(const char* name, const Type& scalarType, int8_t priority)
            : INHERITED(name, "L", kTypeKind)
            , fScalarType(scalarType)
            , fPriority(priority) {}

    const Type& scalarTypeForLiteral() const override {
        return fScalarType;
    }

    int priority() const override {
        return fPriority;
    }

    int columns() const override {
        return 1;
    }

    int rows() const override {
        return 1;
    }

    NumberKind numberKind() const override {
        return fScalarType.numberKind();
    }

    int bitWidth() const override {
        return fScalarType.bitWidth();
    }

    double minimumValue() const override {
        return fScalarType.minimumValue();
    }

    double maximumValue() const override {
        return fScalarType.maximumValue();
    }

    bool isScalar() const override {
        return true;
    }

    bool isLiteral() const override {
        return true;
    }

    size_t slotCount() const override {
        return 1;
    }

    const Type& slotType(size_t n) const override {
        SkASSERT(n == 0);
        return fScalarType;
    }

private:
    using INHERITED = Type;

    const Type& fScalarType;
    int8_t fPriority;
};


class ScalarType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kScalar;

    ScalarType(std::string_view name, const char* abbrev, NumberKind numberKind, int8_t priority,
               int8_t bitWidth)
            : INHERITED(name, abbrev, kTypeKind)
            , fNumberKind(numberKind)
            , fPriority(priority)
            , fBitWidth(bitWidth) {}

    NumberKind numberKind() const override {
        return fNumberKind;
    }

    int priority() const override {
        return fPriority;
    }

    int bitWidth() const override {
        return fBitWidth;
    }

    int columns() const override {
        return 1;
    }

    int rows() const override {
        return 1;
    }

    bool isScalar() const override {
        return true;
    }

    bool isAllowedInES2() const override {
        return fNumberKind != NumberKind::kUnsigned;
    }

    bool isAllowedInUniform(Position*) const override {
        return fNumberKind != NumberKind::kBoolean;
    }

    size_t slotCount() const override {
        return 1;
    }

    const Type& slotType(size_t n) const override {
        SkASSERT(n == 0);
        return *this;
    }

    using int_limits = std::numeric_limits<int32_t>;
    using short_limits = std::numeric_limits<int16_t>;
    using uint_limits = std::numeric_limits<uint32_t>;
    using ushort_limits = std::numeric_limits<uint16_t>;
    using float_limits = std::numeric_limits<float>;

    /** Returns the maximum value that can fit in the type. */
    double minimumValue() const override {
        switch (this->numberKind()) {
            case NumberKind::kSigned:
                return this->highPrecision() ? int_limits::lowest()
                                             : short_limits::lowest();

            case NumberKind::kUnsigned:
                return 0;

            case NumberKind::kFloat:
            default:
                return float_limits::lowest();
        }
    }

    /** Returns the maximum value that can fit in the type. */
    double maximumValue() const override {
        switch (this->numberKind()) {
            case NumberKind::kSigned:
                return this->highPrecision() ? int_limits::max()
                                             : short_limits::max();

            case NumberKind::kUnsigned:
                return this->highPrecision() ? uint_limits::max()
                                             : ushort_limits::max();

            case NumberKind::kFloat:
            default:
                return float_limits::max();
        }
    }

private:
    using INHERITED = Type;

    NumberKind fNumberKind;
    int8_t fPriority;
    int8_t fBitWidth;
};

class AtomicType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kAtomic;

    AtomicType(std::string_view name, const char* abbrev) : INHERITED(name, abbrev, kTypeKind) {}

    bool isAllowedInES2() const override { return false; }

    bool isAllowedInUniform(Position*) const override { return false; }

    bool isOrContainsAtomic() const override { return true; }

    const Type& slotType(size_t n) const override {
        SkASSERT(n == 0);
        return *this;
    }

private:
    using INHERITED = Type;
};

class MatrixType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kMatrix;

    MatrixType(std::string_view name, const char* abbrev, const Type& componentType,
               int8_t columns, int8_t rows)
            : INHERITED(name, abbrev, kTypeKind)
            , fComponentType(componentType.as<ScalarType>())
            , fColumns(columns)
            , fRows(rows) {
        SkASSERT(columns >= 2 && columns <= 4);
        SkASSERT(rows >= 2 && rows <= 4);
    }

    const ScalarType& componentType() const override {
        return fComponentType;
    }

    int columns() const override {
        return fColumns;
    }

    int rows() const override {
        return fRows;
    }

    int bitWidth() const override {
        return this->componentType().bitWidth();
    }

    bool isMatrix() const override {
        return true;
    }

    bool isAllowedInES2() const override {
        return fColumns == fRows;
    }

    size_t slotCount() const override {
        return fColumns * fRows;
    }

    const Type& slotType(size_t n) const override {
        SkASSERT(n < this->slotCount());
        return fComponentType;
    }

private:
    using INHERITED = Type;

    const ScalarType& fComponentType;
    int8_t fColumns;
    int8_t fRows;
};

class TextureType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kTexture;

    TextureType(const char* name, SpvDim_ dimensions, bool isDepth, bool isArrayed,
                bool isMultisampled, TextureAccess textureAccess)
            : INHERITED(name, "T", kTypeKind)
            , fDimensions(dimensions)
            , fIsDepth(isDepth)
            , fIsArrayed(isArrayed)
            , fIsMultisampled(isMultisampled)
            , fTextureAccess(textureAccess) {}

    SpvDim_ dimensions() const override {
        return fDimensions;
    }

    bool isDepth() const override {
        return fIsDepth;
    }

    bool isArrayedTexture() const override {
        return fIsArrayed;
    }

    bool isMultisampled() const override {
        return fIsMultisampled;
    }

    TextureAccess textureAccess() const override {
        return fTextureAccess;
    }

    const Type& slotType(size_t n) const override {
        SkASSERT(n == 0);
        return *this;
    }

private:
    using INHERITED = Type;

    SpvDim_ fDimensions;
    bool fIsDepth;
    bool fIsArrayed;
    bool fIsMultisampled;
    TextureAccess fTextureAccess;
};

class SamplerType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kSampler;

    SamplerType(const char* name, const Type& textureType)
            : INHERITED(name, "Z", kTypeKind)
            , fTextureType(textureType.as<TextureType>()) {
        // Samplers require sampled texture access.
        SkASSERT(this->textureAccess() == TextureAccess::kSample);
        // Subpass inputs cannot be sampled.
        SkASSERT(this->dimensions() != SpvDimSubpassData);
    }

    const TextureType& textureType() const override {
        return fTextureType;
    }

    SpvDim_ dimensions() const override {
        return fTextureType.dimensions();
    }

    bool isDepth() const override {
        return fTextureType.isDepth();
    }

    bool isArrayedTexture() const override {
        return fTextureType.isArrayedTexture();
    }

    bool isMultisampled() const override {
        return fTextureType.isMultisampled();
    }

    TextureAccess textureAccess() const override {
        return fTextureType.textureAccess();
    }

    const Type& slotType(size_t n) const override {
        SkASSERT(n == 0);
        return *this;
    }

private:
    using INHERITED = Type;

    const TextureType& fTextureType;
};

class StructType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kStruct;

    StructType(Position pos, std::string_view name, TArray<Field> fields, int nestingDepth,
               bool interfaceBlock, bool isBuiltin)
            : INHERITED(std::move(name), "S", kTypeKind, pos)
            , fFields(std::move(fields))
            , fNestingDepth(nestingDepth)
            , fInterfaceBlock(interfaceBlock)
            , fIsBuiltin(isBuiltin) {
        for (const Field& f : fFields) {
            fContainsArray        = fContainsArray        || f.fType->isOrContainsArray();
            fContainsUnsizedArray = fContainsUnsizedArray || f.fType->isOrContainsUnsizedArray();
            fContainsAtomic       = fContainsAtomic       || f.fType->isOrContainsAtomic();
            fIsAllowedInES2       = fIsAllowedInES2       && f.fType->isAllowedInES2();
        }
        for (const Field& f : fFields) {
            Position errorPosition = f.fPosition;
            if (!f.fType->isAllowedInUniform(&errorPosition)) {
                fUniformErrorPosition = errorPosition;
                break;
            }
        }
        if (!fContainsUnsizedArray) {
            for (const Field& f : fFields) {
                fSlotCount += f.fType->slotCount();
            }
        }
    }

    SkSpan<const Field> fields() const override {
        return fFields;
    }

    bool isStruct() const override {
        return true;
    }

    bool isInterfaceBlock() const override {
        return fInterfaceBlock;
    }

    bool isBuiltin() const override {
        return fIsBuiltin;
    }

    bool isAllowedInES2() const override {
        return fIsAllowedInES2;
    }

    bool isAllowedInUniform(Position* errorPosition) const override {
        if (errorPosition != nullptr) {
            *errorPosition = fUniformErrorPosition;
        }
        return !fUniformErrorPosition.valid();
    }

    bool isOrContainsArray() const override {
        return fContainsArray;
    }

    bool isOrContainsUnsizedArray() const override {
        return fContainsUnsizedArray;
    }

    bool isOrContainsAtomic() const override {
        return fContainsAtomic;
    }

    size_t slotCount() const override {
        SkASSERT(!fContainsUnsizedArray);
        return fSlotCount;
    }

    int structNestingDepth() const override {
        return fNestingDepth;
    }

    const Type& slotType(size_t n) const override {
        for (const Field& field : fFields) {
            size_t fieldSlots = field.fType->slotCount();
            if (n < fieldSlots) {
                return field.fType->slotType(n);
            } else {
                n -= fieldSlots;
            }
        }
        SkDEBUGFAIL("slot index out of range");
        return *this;
    }

private:
    using INHERITED = Type;

    TArray<Field> fFields;
    size_t fSlotCount = 0;
    int fNestingDepth = 0;
    Position fUniformErrorPosition = {};
    bool fInterfaceBlock = false;
    bool fContainsArray = false;
    bool fContainsUnsizedArray = false;
    bool fContainsAtomic = false;
    bool fIsBuiltin = false;
    bool fIsAllowedInES2 = true;
};

class VectorType final : public Type {
public:
    inline static constexpr TypeKind kTypeKind = TypeKind::kVector;

    VectorType(std::string_view name, const char* abbrev, const Type& componentType,
               int8_t columns)
            : INHERITED(name, abbrev, kTypeKind)
            , fComponentType(componentType.as<ScalarType>())
            , fColumns(columns) {
        SkASSERT(columns >= 2 && columns <= 4);
    }

    const ScalarType& componentType() const override {
        return fComponentType;
    }

    int columns() const override {
        return fColumns;
    }

    int rows() const override {
        return 1;
    }

    int bitWidth() const override {
        return this->componentType().bitWidth();
    }

    bool isVector() const override {
        return true;
    }

    bool isAllowedInES2() const override {
        return fComponentType.isAllowedInES2();
    }

    bool isAllowedInUniform(Position* errorPosition) const override {
        return fComponentType.isAllowedInUniform(errorPosition);
    }

    size_t slotCount() const override {
        return fColumns;
    }

    const Type& slotType(size_t n) const override {
        SkASSERT(n < SkToSizeT(fColumns));
        return fComponentType;
    }

private:
    using INHERITED = Type;

    const ScalarType& fComponentType;
    int8_t fColumns;
};

std::string Type::getArrayName(int arraySize) const {
    std::string_view name = this->name();
    if (arraySize == kUnsizedArray) {
        return String::printf("%.*s[]", (int)name.size(), name.data());
    }
    return String::printf("%.*s[%d]", (int)name.size(), name.data(), arraySize);
}

std::unique_ptr<Type> Type::MakeAliasType(std::string_view name, const Type& targetType) {
    return std::make_unique<AliasType>(std::move(name), targetType);
}

std::unique_ptr<Type> Type::MakeArrayType(const Context& context,
                                          std::string_view name,
                                          const Type& componentType,
                                          int columns) {
    return std::make_unique<ArrayType>(std::move(name), componentType.abbreviatedName(),
                                       componentType, columns, context.fConfig->fIsBuiltinCode);
}

std::unique_ptr<Type> Type::MakeGenericType(const char* name,
                                            SkSpan<const Type* const> types,
                                            const Type* slotType) {
    return std::make_unique<GenericType>(name, types, slotType);
}

std::unique_ptr<Type> Type::MakeLiteralType(const char* name, const Type& scalarType,
                                            int8_t priority) {
    return std::make_unique<LiteralType>(name, scalarType, priority);
}

std::unique_ptr<Type> Type::MakeMatrixType(std::string_view name, const char* abbrev,
                                           const Type& componentType, int columns, int8_t rows) {
    return std::make_unique<MatrixType>(name, abbrev, componentType, columns, rows);
}

std::unique_ptr<Type> Type::MakeSamplerType(const char* name, const Type& textureType) {
    return std::make_unique<SamplerType>(name, textureType);
}

std::unique_ptr<Type> Type::MakeSpecialType(const char* name, const char* abbrev,
                                            Type::TypeKind typeKind) {
    return std::unique_ptr<Type>(new Type(name, abbrev, typeKind));
}

std::unique_ptr<Type> Type::MakeScalarType(std::string_view name, const char* abbrev,
                                           Type::NumberKind numberKind, int8_t priority,
                                           int8_t bitWidth) {
    return std::make_unique<ScalarType>(name, abbrev, numberKind, priority, bitWidth);
}

std::unique_ptr<Type> Type::MakeAtomicType(std::string_view name, const char* abbrev) {
    return std::make_unique<AtomicType>(name, abbrev);
}

std::unique_ptr<Type> Type::MakeStructType(const Context& context,
                                           Position pos,
                                           std::string_view name,
                                           TArray<Field> fields,
                                           bool interfaceBlock) {
    const char* const structOrIB  = interfaceBlock ? "interface block" : "struct";
    const char* const aStructOrIB = interfaceBlock ? "an interface block" : "a struct";

    if (fields.empty()) {
        context.fErrors->error(pos, std::string(structOrIB) + " '" + std::string(name) +
                                    "' must contain at least one field");
    }
    size_t slots = 0;

    THashSet<std::string_view> fieldNames;
    for (const Field& field : fields) {
        // Add this field name to our set; if the set doesn't grow, we found a duplicate.
        int numFieldNames = fieldNames.count();
        fieldNames.add(field.fName);
        if (fieldNames.count() == numFieldNames) {
            context.fErrors->error(field.fPosition, "field '" + std::string(field.fName) +
                                                    "' was already defined in the same " +
                                                    std::string(structOrIB) + " ('" +
                                                    std::string(name) + "')");
        }
        if (field.fModifierFlags != ModifierFlag::kNone) {
            std::string desc = field.fModifierFlags.description();
            context.fErrors->error(field.fPosition, "modifier '" + desc + "' is not permitted on " +
                                                    std::string(aStructOrIB) + " field");
        }
        if (field.fLayout.fFlags & LayoutFlag::kBinding) {
            context.fErrors->error(field.fPosition, "layout qualifier 'binding' is not permitted "
                                                    "on " + std::string(aStructOrIB) + " field");
        }
        if (field.fLayout.fFlags & LayoutFlag::kSet) {
            context.fErrors->error(field.fPosition, "layout qualifier 'set' is not permitted on " +
                                                    std::string(aStructOrIB) + " field");
        }

        if (field.fType->isVoid()) {
            context.fErrors->error(field.fPosition, "type 'void' is not permitted in " +
                                                    std::string(aStructOrIB));
        }
        if (field.fType->isOpaque() && !field.fType->isAtomic()) {
            context.fErrors->error(field.fPosition, "opaque type '" + field.fType->displayName() +
                                                    "' is not permitted in " +
                                                    std::string(aStructOrIB));
        }
        if (field.fType->isOrContainsUnsizedArray()) {
            if (!interfaceBlock) {
                // Reject unsized arrays anywhere in structs.
                context.fErrors->error(field.fPosition, "unsized arrays are not permitted here");
            }
        } else {
            // If we haven't already exceeded the struct size limit...
            if (slots < kVariableSlotLimit) {
                // ... see if this field causes us to exceed the size limit.
                slots = SkSafeMath::Add(slots, field.fType->slotCount());
                if (slots >= kVariableSlotLimit) {
                    context.fErrors->error(pos, std::string(structOrIB) + " is too large");
                }
            }
        }
    }
    int nestingDepth = 0;
    for (const Field& field : fields) {
        nestingDepth = std::max(nestingDepth, field.fType->structNestingDepth());
    }
    if (nestingDepth >= kMaxStructDepth) {
        context.fErrors->error(pos, std::string(structOrIB) + " '" + std::string(name) +
                                    "' is too deeply nested");
    }
    return std::make_unique<StructType>(pos, name, std::move(fields), nestingDepth + 1,
                                        interfaceBlock, context.fConfig->fIsBuiltinCode);
}

std::unique_ptr<Type> Type::MakeTextureType(const char* name, SpvDim_ dimensions, bool isDepth,
                                            bool isArrayedTexture, bool isMultisampled,
                                            TextureAccess textureAccess) {
    return std::make_unique<TextureType>(name, dimensions, isDepth, isArrayedTexture,
                                         isMultisampled, textureAccess);
}

std::unique_ptr<Type> Type::MakeVectorType(std::string_view name, const char* abbrev,
                                           const Type& componentType, int columns) {
    return std::make_unique<VectorType>(name, abbrev, componentType, columns);
}

CoercionCost Type::coercionCost(const Type& other) const {
    if (this->matches(other)) {
        return CoercionCost::Free();
    }
    if (this->typeKind() == other.typeKind() &&
        (this->isVector() || this->isMatrix() || this->isArray())) {
        // Vectors/matrices/arrays of the same size can be coerced if their component type can be.
        if (this->isMatrix() && (this->rows() != other.rows())) {
            return CoercionCost::Impossible();
        }
        if (this->columns() != other.columns()) {
            return CoercionCost::Impossible();
        }
        return this->componentType().coercionCost(other.componentType());
    }
    if (this->isNumber() && other.isNumber()) {
        if (this->isLiteral() && this->isInteger()) {
            return CoercionCost::Free();
        } else if (this->numberKind() != other.numberKind()) {
            return CoercionCost::Impossible();
        } else if (other.priority() >= this->priority()) {
            return CoercionCost::Normal(other.priority() - this->priority());
        } else {
            return CoercionCost::Narrowing(this->priority() - other.priority());
        }
    }
    if (fTypeKind == TypeKind::kGeneric) {
        SkSpan<const Type* const> types = this->coercibleTypes();
        for (size_t i = 0; i < types.size(); i++) {
            if (types[i]->matches(other)) {
                return CoercionCost::Normal((int) i + 1);
            }
        }
    }
    return CoercionCost::Impossible();
}

const Type* Type::applyQualifiers(const Context& context,
                                  ModifierFlags* modifierFlags,
                                  Position pos) const {
    const Type* type;
    type = this->applyPrecisionQualifiers(context, modifierFlags, pos);
    type = type->applyAccessQualifiers(context, modifierFlags, pos);
    return type;
}

const Type* Type::applyPrecisionQualifiers(const Context& context,
                                           ModifierFlags* modifierFlags,
                                           Position pos) const {
    ModifierFlags precisionQualifiers = *modifierFlags & (ModifierFlag::kHighp |
                                                          ModifierFlag::kMediump |
                                                          ModifierFlag::kLowp);
    if (precisionQualifiers == ModifierFlag::kNone) {
        // No precision qualifiers here. Return the type as-is.
        return this;
    }

    if (!ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
        // We want to discourage precision modifiers internally. Instead, use the type that
        // corresponds to the precision you need. (e.g. half vs float, short vs int)
        context.fErrors->error(pos, "precision qualifiers are not allowed");
        return context.fTypes.fPoison.get();
    }

    if (SkPopCount(precisionQualifiers.value()) > 1) {
        context.fErrors->error(pos, "only one precision qualifier can be used");
        return context.fTypes.fPoison.get();
    }

    // We're going to return a whole new type, so the modifier bits can be cleared out.
    *modifierFlags &= ~(ModifierFlag::kHighp |
                        ModifierFlag::kMediump |
                        ModifierFlag::kLowp);

    const Type& component = this->componentType();
    if (component.highPrecision()) {
        if (precisionQualifiers & ModifierFlag::kHighp) {
            // Type is already high precision, and we are requesting high precision. Return as-is.
            return this;
        }

        // SkSL doesn't support low precision, so `lowp` is interpreted as medium precision.
        // Ascertain the mediump equivalent type for this type, if any.
        const Type* mediumpType;
        switch (component.numberKind()) {
            case Type::NumberKind::kFloat:
                mediumpType = context.fTypes.fHalf.get();
                break;

            case Type::NumberKind::kSigned:
                mediumpType = context.fTypes.fShort.get();
                break;

            case Type::NumberKind::kUnsigned:
                mediumpType = context.fTypes.fUShort.get();
                break;

            default:
                mediumpType = context.fTypes.fPoison.get();
                break;
        }

        if (mediumpType) {
            // Convert the mediump component type into the final vector/matrix/array type as needed.
            return this->isArray()
                    ? context.fSymbolTable->addArrayDimension(context, mediumpType, this->columns())
                    : &mediumpType->toCompound(context, this->columns(), this->rows());
        }
    }

    context.fErrors->error(pos, "type '" + this->displayName() +
                                "' does not support precision qualifiers");
    return context.fTypes.fPoison.get();
}

const Type* Type::applyAccessQualifiers(const Context& context,
                                        ModifierFlags* modifierFlags,
                                        Position pos) const {
    ModifierFlags accessQualifiers = *modifierFlags & (ModifierFlag::kReadOnly |
                                                       ModifierFlag::kWriteOnly);

    // We're going to return a whole new type, so the modifier bits must be cleared out.
    *modifierFlags &= ~(ModifierFlag::kReadOnly |
                        ModifierFlag::kWriteOnly);

    if (this->matches(*context.fTypes.fTexture2D)) {
        // We require all texture2Ds to be qualified with `readonly` or `writeonly`.
        // (Read-write textures are not yet supported in WGSL.)
        if (accessQualifiers == ModifierFlag::kReadOnly) {
            return context.fTypes.fReadOnlyTexture2D.get();
        }
        if (accessQualifiers == ModifierFlag::kWriteOnly) {
            return context.fTypes.fWriteOnlyTexture2D.get();
        }
        context.fErrors->error(
                pos,
                accessQualifiers
                        ? "'readonly' and 'writeonly' qualifiers cannot be combined"
                        : "'texture2D' requires a 'readonly' or 'writeonly' access qualifier");
        return this;
    }

    if (accessQualifiers) {
        context.fErrors->error(pos, "type '" + this->displayName() + "' does not support "
                                    "qualifier '" + accessQualifiers.description() + "'");
    }

    return this;
}

const Type& Type::toCompound(const Context& context, int columns, int rows) const {
    SkASSERT(this->isScalar());
    if (columns == 1 && rows == 1) {
        return *this;
    }
    if (this->matches(*context.fTypes.fFloat) || this->matches(*context.fTypes.fFloatLiteral)) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fFloat;
                    case 2: return *context.fTypes.fFloat2;
                    case 3: return *context.fTypes.fFloat3;
                    case 4: return *context.fTypes.fFloat4;
                    default: SK_ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fTypes.fFloat2x2;
                    case 3: return *context.fTypes.fFloat3x2;
                    case 4: return *context.fTypes.fFloat4x2;
                    default: SK_ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fTypes.fFloat2x3;
                    case 3: return *context.fTypes.fFloat3x3;
                    case 4: return *context.fTypes.fFloat4x3;
                    default: SK_ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fTypes.fFloat2x4;
                    case 3: return *context.fTypes.fFloat3x4;
                    case 4: return *context.fTypes.fFloat4x4;
                    default: SK_ABORT("unsupported matrix column count (%d)", columns);
                }
            default: SK_ABORT("unsupported row count (%d)", rows);
        }
    } else if (this->matches(*context.fTypes.fHalf)) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fHalf;
                    case 2: return *context.fTypes.fHalf2;
                    case 3: return *context.fTypes.fHalf3;
                    case 4: return *context.fTypes.fHalf4;
                    default: SK_ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return *context.fTypes.fHalf2x2;
                    case 3: return *context.fTypes.fHalf3x2;
                    case 4: return *context.fTypes.fHalf4x2;
                    default: SK_ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return *context.fTypes.fHalf2x3;
                    case 3: return *context.fTypes.fHalf3x3;
                    case 4: return *context.fTypes.fHalf4x3;
                    default: SK_ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return *context.fTypes.fHalf2x4;
                    case 3: return *context.fTypes.fHalf3x4;
                    case 4: return *context.fTypes.fHalf4x4;
                    default: SK_ABORT("unsupported matrix column count (%d)", columns);
                }
            default: SK_ABORT("unsupported row count (%d)", rows);
        }
    } else if (this->matches(*context.fTypes.fInt) || this->matches(*context.fTypes.fIntLiteral)) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fInt;
                    case 2: return *context.fTypes.fInt2;
                    case 3: return *context.fTypes.fInt3;
                    case 4: return *context.fTypes.fInt4;
                    default: SK_ABORT("unsupported vector column count (%d)", columns);
                }
            default: SK_ABORT("unsupported row count (%d)", rows);
        }
    } else if (this->matches(*context.fTypes.fShort)) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fShort;
                    case 2: return *context.fTypes.fShort2;
                    case 3: return *context.fTypes.fShort3;
                    case 4: return *context.fTypes.fShort4;
                    default: SK_ABORT("unsupported vector column count (%d)", columns);
                }
            default: SK_ABORT("unsupported row count (%d)", rows);
        }
    } else if (this->matches(*context.fTypes.fUInt)) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fUInt;
                    case 2: return *context.fTypes.fUInt2;
                    case 3: return *context.fTypes.fUInt3;
                    case 4: return *context.fTypes.fUInt4;
                    default: SK_ABORT("unsupported vector column count (%d)", columns);
                }
            default: SK_ABORT("unsupported row count (%d)", rows);
        }
    } else if (this->matches(*context.fTypes.fUShort)) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fUShort;
                    case 2: return *context.fTypes.fUShort2;
                    case 3: return *context.fTypes.fUShort3;
                    case 4: return *context.fTypes.fUShort4;
                    default: SK_ABORT("unsupported vector column count (%d)", columns);
                }
            default: SK_ABORT("unsupported row count (%d)", rows);
        }
    } else if (this->matches(*context.fTypes.fBool)) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 1: return *context.fTypes.fBool;
                    case 2: return *context.fTypes.fBool2;
                    case 3: return *context.fTypes.fBool3;
                    case 4: return *context.fTypes.fBool4;
                    default: SK_ABORT("unsupported vector column count (%d)", columns);
                }
            default: SK_ABORT("unsupported row count (%d)", rows);
        }
    }
    SkDEBUGFAILF("unsupported toCompound type %s", this->description().c_str());
    return *context.fTypes.fVoid;
}

const Type* Type::clone(const Context& context, SymbolTable* symbolTable) const {
    // Many types are built-ins, and exist in every SymbolTable by default.
    if (this->isInRootSymbolTable()) {
        return this;
    }
    // If we are compiling a program, and the type comes from the program's module, it is safe to
    // assume that the type is in-scope anywhere in the program without actually recursing through
    // the SymbolTable hierarchy to prove it.
    if (!context.fConfig->fIsBuiltinCode && this->isBuiltin()) {
        return this;
    }
    // Even if the type isn't a built-in, it might already exist in the SymbolTable. Search by name.
    const Symbol* existingSymbol = symbolTable->find(this->name());
    if (existingSymbol != nullptr) {
        const Type* existingType = &existingSymbol->as<Type>();
        SkASSERT(existingType->typeKind() == this->typeKind());
        return existingType;
    }
    // This type actually needs to be cloned into the destination SymbolTable.
    switch (this->typeKind()) {
        case TypeKind::kArray: {
            return symbolTable->addArrayDimension(context, &this->componentType(), this->columns());
        }
        case TypeKind::kStruct: {
            // We are cloning an existing struct, so there's no need to call MakeStructType and
            // fully error-check it again.
            const std::string* name = symbolTable->takeOwnershipOfString(std::string(this->name()));
            SkSpan<const Field> fieldSpan = this->fields();
            return symbolTable->add(
                    context,
                    std::make_unique<StructType>(this->fPosition,
                                                 *name,
                                                 TArray<Field>(fieldSpan.data(), fieldSpan.size()),
                                                 this->structNestingDepth(),
                                                 /*interfaceBlock=*/this->isInterfaceBlock(),
                                                 /*isBuiltin=*/context.fConfig->fIsBuiltinCode));
        }
        default:
            SkDEBUGFAILF("don't know how to clone type '%s'", this->description().c_str());
            return nullptr;
    }
}

std::unique_ptr<Expression> Type::coerceExpression(std::unique_ptr<Expression> expr,
                                                   const Context& context) const {
    if (!expr || expr->isIncomplete(context)) {
        return nullptr;
    }
    if (expr->type().matches(*this)) {
        return expr;
    }

    const Position pos = expr->fPosition;
    const ProgramSettings& settings = context.fConfig->fSettings;
    if (!expr->coercionCost(*this).isPossible(settings.fAllowNarrowingConversions)) {
        context.fErrors->error(pos, "expected '" + this->displayName() + "', but found '" +
                                    expr->type().displayName() + "'");
        return nullptr;
    }

    if (this->isScalar()) {
        return ConstructorScalarCast::Make(context, pos, *this, std::move(expr));
    }
    if (this->isVector() || this->isMatrix()) {
        return ConstructorCompoundCast::Make(context, pos, *this, std::move(expr));
    }
    if (this->isArray()) {
        return ConstructorArrayCast::Make(context, pos, *this, std::move(expr));
    }
    context.fErrors->error(pos, "cannot construct '" + this->displayName() + "'");
    return nullptr;
}

bool Type::isAllowedInES2(const Context& context) const {
    return !context.fConfig->strictES2Mode() || this->isAllowedInES2();
}

bool Type::checkForOutOfRangeLiteral(const Context& context, const Expression& expr) const {
    bool foundError = false;
    const Type& baseType = this->componentType();
    if (baseType.isNumber()) {
        // Replace constant expressions with their corresponding values.
        const Expression* valueExpr = ConstantFolder::GetConstantValueForVariable(expr);
        if (valueExpr->supportsConstantValues()) {
            // Iterate over every constant subexpression in the value.
            int numSlots = valueExpr->type().slotCount();
            for (int slot = 0; slot < numSlots; ++slot) {
                std::optional<double> slotVal = valueExpr->getConstantValue(slot);
                // Check for Literal values that are out of range for the base type.
                if (slotVal.has_value() &&
                    baseType.checkForOutOfRangeLiteral(context, *slotVal, valueExpr->fPosition)) {
                    foundError = true;
                }
            }
        }
    }

    // We don't need range checks for floats or booleans; any matched-type value is acceptable.
    return foundError;
}

bool Type::checkForOutOfRangeLiteral(const Context& context, double value, Position pos) const {
    SkASSERT(this->isScalar());
    if (!this->isNumber()) {
       return false;
    }
    if (value >= this->minimumValue() && value <= this->maximumValue()) {
        return false;
    }
    // We found a value that can't fit in our type. Flag it as an error.
    context.fErrors->error(pos, SkSL::String::printf("value is out of range for type '%s': %.0f",
                                                     this->displayName().c_str(),
                                                     value));
    return true;
}

bool Type::checkIfUsableInArray(const Context& context, Position arrayPos) const {
    if (this->isArray()) {
        context.fErrors->error(arrayPos, "multi-dimensional arrays are not supported");
        return false;
    }
    if (this->isVoid()) {
        context.fErrors->error(arrayPos, "type 'void' may not be used in an array");
        return false;
    }
    if (this->isOpaque() && !this->isAtomic()) {
        context.fErrors->error(arrayPos, "opaque type '" + std::string(this->name()) +
                                         "' may not be used in an array");
        return false;
    }
    return true;
}

SKSL_INT Type::convertArraySize(const Context& context,
                                Position arrayPos,
                                std::unique_ptr<Expression> size) const {
    size = context.fTypes.fInt->coerceExpression(std::move(size), context);
    if (!size) {
        return 0;
    }
    SKSL_INT count;
    if (!ConstantFolder::GetConstantInt(*size, &count)) {
        context.fErrors->error(size->fPosition, "array size must be an integer");
        return 0;
    }
    return this->convertArraySize(context, arrayPos, size->fPosition, count);
}

SKSL_INT Type::convertArraySize(const Context& context,
                                Position arrayPos,
                                Position sizePos,
                                SKSL_INT size) const {
    if (!this->checkIfUsableInArray(context, arrayPos)) {
        // `checkIfUsableInArray` will generate an error for us.
        return 0;
    }
    if (size <= 0) {
        context.fErrors->error(sizePos, "array size must be positive");
        return 0;
    }
    // We can't get a meaningful slot count if the interior type contains an unsized array; we'll
    // assert if we try. Unsized arrays should only occur in a handful of limited cases (e.g. an
    // interface block with a trailing buffer), and will never be valid in a runtime effect.
    if (!this->isOrContainsUnsizedArray()) {
        if (SkSafeMath::Mul(this->slotCount(), size) > kVariableSlotLimit) {
            context.fErrors->error(sizePos, "array size is too large");
            return 0;
        }
    }
    return size;
}

std::string Field::description() const {
    return fLayout.paddedDescription() + fModifierFlags.paddedDescription() + fType->displayName() +
           ' ' + std::string(fName) + ';';
}

}  // namespace SkSL
