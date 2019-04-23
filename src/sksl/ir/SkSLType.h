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
#include <climits>
#include <vector>
#include <memory>

namespace SkSL {

class Context;

/**
 * Represents a type, such as int or float4.
 */
class Type : public Symbol {
public:
    struct Field {
        Field(Modifiers modifiers, StringFragment name, const Type* type)
        : fModifiers(modifiers)
        , fName(name)
        , fType(std::move(type)) {}

        const String description() const {
            return fType->description() + " " + fName + ";";
        }

        Modifiers fModifiers;
        StringFragment fName;
        const Type* fType;
    };

    enum Kind {
        kArray_Kind,
        kEnum_Kind,
        kGeneric_Kind,
        kNullable_Kind,
        kMatrix_Kind,
        kOther_Kind,
        kSampler_Kind,
        kScalar_Kind,
        kStruct_Kind,
        kVector_Kind
    };

    enum NumberKind {
        kFloat_NumberKind,
        kSigned_NumberKind,
        kUnsigned_NumberKind,
        kNonnumeric_NumberKind
    };

    // Create an "other" (special) type with the given name. These types cannot be directly
    // referenced from user code.
    Type(const char* name)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(name)
    , fTypeKind(kOther_Kind)
    , fNumberKind(kNonnumeric_NumberKind) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create an "other" (special) type that supports field access.
    Type(const char* name, std::vector<Field> fields)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(name)
    , fTypeKind(kOther_Kind)
    , fNumberKind(kNonnumeric_NumberKind)
    , fFields(std::move(fields)) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a simple type.
    Type(String name, Kind kind)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(std::move(name))
    , fTypeKind(kind)
    , fNumberKind(kNonnumeric_NumberKind) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a generic type which maps to the listed types.
    Type(const char* name, std::vector<const Type*> types)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(name)
    , fTypeKind(kGeneric_Kind)
    , fNumberKind(kNonnumeric_NumberKind)
    , fCoercibleTypes(std::move(types)) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a struct type with the given fields.
    Type(int offset, String name, std::vector<Field> fields)
    : INHERITED(offset, kType_Kind, StringFragment())
    , fNameString(std::move(name))
    , fTypeKind(kStruct_Kind)
    , fNumberKind(kNonnumeric_NumberKind)
    , fFields(std::move(fields)) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a scalar type.
    Type(const char* name, NumberKind numberKind, int priority, bool highPrecision = false)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(name)
    , fTypeKind(kScalar_Kind)
    , fNumberKind(numberKind)
    , fPriority(priority)
    , fColumns(1)
    , fRows(1)
    , fHighPrecision(highPrecision) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a scalar type which can be coerced to the listed types.
    Type(const char* name,
         NumberKind numberKind,
         int priority,
         std::vector<const Type*> coercibleTypes)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(name)
    , fTypeKind(kScalar_Kind)
    , fNumberKind(numberKind)
    , fPriority(priority)
    , fCoercibleTypes(std::move(coercibleTypes))
    , fColumns(1)
    , fRows(1) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a nullable type.
    Type(String name, Kind kind, const Type& componentType)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(std::move(name))
    , fTypeKind(kind)
    , fNumberKind(kNonnumeric_NumberKind)
    , fComponentType(&componentType)
    , fColumns(1)
    , fRows(1)
    , fDimensions(SpvDim1D) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a vector type.
    Type(const char* name, const Type& componentType, int columns)
    : Type(name, kVector_Kind, componentType, columns) {}

    // Create a vector or array type.
    Type(String name, Kind kind, const Type& componentType, int columns)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(std::move(name))
    , fTypeKind(kind)
    , fNumberKind(kNonnumeric_NumberKind)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(1)
    , fDimensions(SpvDim1D) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a matrix type.
    Type(const char* name, const Type& componentType, int columns, int rows)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(name)
    , fTypeKind(kMatrix_Kind)
    , fNumberKind(kNonnumeric_NumberKind)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(rows)
    , fDimensions(SpvDim1D) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    // Create a sampler type.
    Type(const char* name, SpvDim_ dimensions, bool isDepth, bool isArrayed, bool isMultisampled,
         bool isSampled)
    : INHERITED(-1, kType_Kind, StringFragment())
    , fNameString(name)
    , fTypeKind(kSampler_Kind)
    , fNumberKind(kNonnumeric_NumberKind)
    , fDimensions(dimensions)
    , fIsDepth(isDepth)
    , fIsArrayed(isArrayed)
    , fIsMultisampled(isMultisampled)
    , fIsSampled(isSampled) {
        fName.fChars = fNameString.c_str();
        fName.fLength = fNameString.size();
    }

    const String& name() const {
        return fNameString;
    }

    String description() const override {
        if (fNameString == "$floatLiteral") {
            return "float";
        }
        if (fNameString == "$intLiteral") {
            return "int";
        }
        return fNameString;
    }

    bool operator==(const Type& other) const {
        return fName == other.fName;
    }

    bool operator!=(const Type& other) const {
        return fName != other.fName;
    }

    /**
     * Returns the category (scalar, vector, matrix, etc.) of this type.
     */
    Kind kind() const {
        return fTypeKind;
    }

    /**
     * Returns true if this is a numeric scalar type.
     */
    bool isNumber() const {
        return fNumberKind != kNonnumeric_NumberKind;
    }

    /**
     * Returns true if this is a floating-point scalar type (float, half, or double).
     */
    bool isFloat() const {
        return fNumberKind == kFloat_NumberKind;
    }

    /**
     * Returns true if this is a signed scalar type (int or short).
     */
    bool isSigned() const {
        return fNumberKind == kSigned_NumberKind;
    }

    /**
     * Returns true if this is an unsigned scalar type (uint or ushort).
     */
    bool isUnsigned() const {
        return fNumberKind == kUnsigned_NumberKind;
    }

    /**
     * Returns true if this is a signed or unsigned integer.
     */
    bool isInteger() const {
        return isSigned() || isUnsigned();
    }

    /**
     * Returns the "priority" of a number type, in order of double > float > half > int > short.
     * When operating on two number types, the result is the higher-priority type.
     */
    int priority() const {
        return fPriority;
    }

    /**
     * Returns true if an instance of this type can be freely coerced (implicitly converted) to
     * another type.
     */
    bool canCoerceTo(const Type& other) const {
        return coercionCost(other) != INT_MAX;
    }

    /**
     * Determines the "cost" of coercing (implicitly converting) this type to another type. The cost
     * is a number with no particular meaning other than that lower costs are preferable to higher
     * costs. Returns INT_MAX if the coercion is not possible.
     */
    int coercionCost(const Type& other) const;

    /**
     * For matrices and vectors, returns the type of individual cells (e.g. mat2 has a component
     * type of kFloat_Type). For all other types, causes an SkASSERTion failure.
     */
    const Type& componentType() const {
        SkASSERT(fComponentType);
        return *fComponentType;
    }

    /**
     * For nullable types, returns the base type, otherwise returns the type itself.
     */
    const Type& nonnullable() const {
        if (fTypeKind == kNullable_Kind) {
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
        SkASSERT(fTypeKind == kScalar_Kind || fTypeKind == kVector_Kind ||
                 fTypeKind == kMatrix_Kind || fTypeKind == kArray_Kind);
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
        SkASSERT(fTypeKind == kStruct_Kind || fTypeKind == kOther_Kind);
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
        SkASSERT(kSampler_Kind == fTypeKind);
        return fDimensions;
    }

    bool isDepth() const {
        SkASSERT(kSampler_Kind == fTypeKind);
        return fIsDepth;
    }

    bool isArrayed() const {
        SkASSERT(kSampler_Kind == fTypeKind);
        return fIsArrayed;
    }

    bool isMultisampled() const {
        SkASSERT(kSampler_Kind == fTypeKind);
        return fIsMultisampled;
    }

    bool isSampled() const {
        SkASSERT(kSampler_Kind == fTypeKind);
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
    typedef Symbol INHERITED;

    String fNameString;
    Kind fTypeKind;
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
};

} // namespace

#endif
