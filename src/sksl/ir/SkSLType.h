/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIASL_TYPE
#define SKIASL_TYPE

#include "SkSLModifiers.h"
#include "SkSLSymbol.h"
#include "../SkSLPosition.h"
#include "../SkSLUtil.h"
#include "../spirv.h"
#include <vector>
#include <memory>

namespace SkSL {

class Context;

/**
 * Represents a type, such as int or vec4.
 */
class Type : public Symbol {
public:
    struct Field {
        Field(Modifiers modifiers, String name, const Type* type)
        : fModifiers(modifiers)
        , fName(std::move(name))
        , fType(std::move(type)) {}

        const String description() const {
            return fType->description() + " " + fName + ";";
        }

        Modifiers fModifiers;
        String fName;
        const Type* fType;
    };

    enum Kind {
        kScalar_Kind,
        kVector_Kind,
        kMatrix_Kind,
        kArray_Kind,
        kStruct_Kind,
        kGeneric_Kind,
        kSampler_Kind,
        kOther_Kind
    };

    // Create an "other" (special) type with the given name. These types cannot be directly
    // referenced from user code.
    Type(String name)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kOther_Kind) {}

    // Create a generic type which maps to the listed types.
    Type(String name, std::vector<const Type*> types)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kGeneric_Kind)
    , fCoercibleTypes(std::move(types)) {}

    // Create a struct type with the given fields.
    Type(Position position, String name, std::vector<Field> fields)
    : INHERITED(position, kType_Kind, std::move(name))
    , fTypeKind(kStruct_Kind)
    , fFields(std::move(fields)) {}

    // Create a scalar type.
    Type(String name, bool isNumber)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kScalar_Kind)
    , fIsNumber(isNumber)
    , fColumns(1)
    , fRows(1) {}

    // Create a scalar type which can be coerced to the listed types.
    Type(String name, bool isNumber, std::vector<const Type*> coercibleTypes)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kScalar_Kind)
    , fIsNumber(isNumber)
    , fCoercibleTypes(std::move(coercibleTypes))
    , fColumns(1)
    , fRows(1) {}

    // Create a vector type.
    Type(String name, const Type& componentType, int columns)
    : Type(name, kVector_Kind, componentType, columns) {}

    // Create a vector or array type.
    Type(String name, Kind kind, const Type& componentType, int columns)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kind)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(1)
    , fDimensions(SpvDim1D) {}

    // Create a matrix type.
    Type(String name, const Type& componentType, int columns, int rows)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kMatrix_Kind)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(rows)
    , fDimensions(SpvDim1D) {}

    // Create a sampler type.
    Type(String name, SpvDim_ dimensions, bool isDepth, bool isArrayed, bool isMultisampled,
         bool isSampled)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kSampler_Kind)
    , fDimensions(dimensions)
    , fIsDepth(isDepth)
    , fIsArrayed(isArrayed)
    , fIsMultisampled(isMultisampled)
    , fIsSampled(isSampled) {}

    String name() const {
        return fName;
    }

    String description() const override {
        return fName;
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
        return fIsNumber;
    }

    /**
     * Returns true if an instance of this type can be freely coerced (implicitly converted) to
     * another type.
     */
    bool canCoerceTo(const Type& other) const {
        int cost;
        return determineCoercionCost(other, &cost);
    }

    /**
     * Determines the "cost" of coercing (implicitly converting) this type to another type. The cost
     * is a number with no particular meaning other than that lower costs are preferable to higher
     * costs. Returns true if a conversion is possible, false otherwise. The value of the out
     * parameter is undefined if false is returned.
     */
    bool determineCoercionCost(const Type& other, int* outCost) const;

    /**
     * For matrices and vectors, returns the type of individual cells (e.g. mat2 has a component
     * type of kFloat_Type). For all other types, causes an assertion failure.
     */
    const Type& componentType() const {
        ASSERT(fComponentType);
        return *fComponentType;
    }

    /**
     * For matrices and vectors, returns the number of columns (e.g. both mat3 and vec3 return 3).
     * For scalars, returns 1. For arrays, returns either the size of the array (if known) or -1.
     * For all other types, causes an assertion failure.
     */
    int columns() const {
        ASSERT(fTypeKind == kScalar_Kind || fTypeKind == kVector_Kind ||
               fTypeKind == kMatrix_Kind || fTypeKind == kArray_Kind);
        return fColumns;
    }

    /**
     * For matrices, returns the number of rows (e.g. mat2x4 returns 4). For vectors and scalars,
     * returns 1. For all other types, causes an assertion failure.
     */
    int rows() const {
        ASSERT(fRows > 0);
        return fRows;
    }

    const std::vector<Field>& fields() const {
        ASSERT(fTypeKind == kStruct_Kind);
        return fFields;
    }

    /**
     * For generic types, returns the types that this generic type can substitute for. For other
     * types, returns a list of other types that this type can be coerced into.
     */
    const std::vector<const Type*>& coercibleTypes() const {
        ASSERT(fCoercibleTypes.size() > 0);
        return fCoercibleTypes;
    }

    SpvDim_ dimensions() const {
        ASSERT(kSampler_Kind == fTypeKind);
        return fDimensions;
    }

    bool isDepth() const {
        ASSERT(kSampler_Kind == fTypeKind);
        return fIsDepth;
    }

    bool isArrayed() const {
        ASSERT(kSampler_Kind == fTypeKind);
        return fIsArrayed;
    }

    bool isMultisampled() const {
        ASSERT(kSampler_Kind == fTypeKind);
        return fIsMultisampled;
    }

    bool isSampled() const {
        ASSERT(kSampler_Kind == fTypeKind);
        return fIsSampled;
    }

    /**
     * Returns the corresponding vector or matrix type with the specified number of columns and
     * rows.
     */
    const Type& toCompound(const Context& context, int columns, int rows) const;

private:
    typedef Symbol INHERITED;

    const Kind fTypeKind;
    const bool fIsNumber = false;
    const Type* fComponentType = nullptr;
    const std::vector<const Type*> fCoercibleTypes;
    const int fColumns = -1;
    const int fRows = -1;
    const std::vector<Field> fFields;
    const SpvDim_ fDimensions = SpvDim1D;
    const bool fIsDepth = false;
    const bool fIsArrayed = false;
    const bool fIsMultisampled = false;
    const bool fIsSampled = false;
};

} // namespace

#endif
