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
        Field(Modifiers modifiers, std::string name, const Type* type)
        : fModifiers(modifiers)
        , fName(std::move(name))
        , fType(std::move(type)) {}

        const std::string description() const {
            return fType->description() + " " + fName + ";";
        }

        Modifiers fModifiers;
        std::string fName;
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
    Type(std::string name)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kOther_Kind) {}

    // Create a generic type which maps to the listed types.
    Type(std::string name, std::vector<const Type*> types)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kGeneric_Kind)
    , fCoercibleTypes(std::move(types)) {
        ASSERT(fCoercibleTypes.size() == 4);
    }

    // Create a struct type with the given fields.
    Type(std::string name, std::vector<Field> fields)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kStruct_Kind)
    , fFields(std::move(fields)) {}

    // Create a scalar type.
    Type(std::string name, bool isNumber)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kScalar_Kind)
    , fIsNumber(isNumber)
    , fColumns(1)
    , fRows(1) {}

    // Create a scalar type which can be coerced to the listed types.
    Type(std::string name, bool isNumber, std::vector<const Type*> coercibleTypes)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kScalar_Kind)
    , fIsNumber(isNumber)
    , fCoercibleTypes(std::move(coercibleTypes))
    , fColumns(1)
    , fRows(1) {}

    // Create a vector type.
    Type(std::string name, const Type& componentType, int columns)
    : Type(name, kVector_Kind, componentType, columns) {}

    // Create a vector or array type.
    Type(std::string name, Kind kind, const Type& componentType, int columns)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kind)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(1)    
    , fDimensions(SpvDim1D) {}

    // Create a matrix type.
    Type(std::string name, const Type& componentType, int columns, int rows)
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kMatrix_Kind)
    , fComponentType(&componentType)
    , fColumns(columns)
    , fRows(rows)    
    , fDimensions(SpvDim1D) {}

    // Create a sampler type.
    Type(std::string name, SpvDim_ dimensions, bool isDepth, bool isArrayed, bool isMultisampled, 
         bool isSampled) 
    : INHERITED(Position(), kType_Kind, std::move(name))
    , fTypeKind(kSampler_Kind)
    , fDimensions(dimensions)
    , fIsDepth(isDepth)
    , fIsArrayed(isArrayed)
    , fIsMultisampled(isMultisampled)
    , fIsSampled(isSampled) {}

    std::string name() const {
        return fName;
    }

    std::string description() const override {
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

    int dimensions() const {
        ASSERT(fTypeKind == kSampler_Kind);
        return fDimensions;
    }

    bool isDepth() const {
        ASSERT(fTypeKind == kSampler_Kind);
        return fIsDepth;
    }

    bool isArrayed() const {
        ASSERT(fTypeKind == kSampler_Kind);
        return fIsArrayed;
    }

    bool isMultisampled() const {
        ASSERT(fTypeKind == kSampler_Kind);
        return fIsMultisampled;
    }

    bool isSampled() const {
        ASSERT(fTypeKind == kSampler_Kind);
        return fIsSampled;
    }

    static size_t vector_alignment(size_t componentSize, int columns) {
        return componentSize * (columns + columns % 2);
    }

    /**
     * Returns the type's required alignment (when putting this type into a struct, the offset must
     * be a multiple of the alignment).
     */
    size_t alignment() const {
        // See OpenGL Spec 7.6.2.2 Standard Uniform Block Layout
        switch (fTypeKind) {
            case kScalar_Kind:
                return this->size();
            case kVector_Kind:
                return vector_alignment(fComponentType->size(), fColumns);
            case kMatrix_Kind:
                return (vector_alignment(fComponentType->size(), fRows) + 15) & ~15;
            case kArray_Kind:
                // round up to next multiple of 16
                return (fComponentType->alignment() + 15) & ~15;
            case kStruct_Kind: {
                size_t result = 16;
                for (size_t i = 0; i < fFields.size(); i++) {
                    size_t alignment = fFields[i].fType->alignment();
                    if (alignment > result) {
                        result = alignment;
                    }
                }
            }
            default:
                ABORT(("cannot determine size of type " + fName).c_str());
        }
    }

    /**
     * For matrices and arrays, returns the number of bytes from the start of one entry (row, in
     * the case of matrices) to the start of the next.
     */
    size_t stride() const {
        switch (fTypeKind) {
            case kMatrix_Kind: // fall through
            case kArray_Kind:
                return this->alignment();
            default:
                ABORT("type does not have a stride");
        }
    }

    /**
     * Returns the size of this type in bytes.
     */
    size_t size() const {
        switch (fTypeKind) {
            case kScalar_Kind:
                // FIXME need to take precision into account, once we figure out how we want to
                // handle it...
                return 4;
            case kVector_Kind:
                return fColumns * fComponentType->size();
            case kMatrix_Kind:
                return vector_alignment(fComponentType->size(), fRows) * fColumns;
            case kArray_Kind:
                return fColumns * this->stride();
            case kStruct_Kind: {
                size_t total = 0;
                for (size_t i = 0; i < fFields.size(); i++) {
                    size_t alignment = fFields[i].fType->alignment();
                    if (total % alignment != 0) {
                        total += alignment - total % alignment;
                    }
                    ASSERT(false);
                    ASSERT(total % alignment == 0);
                    total += fFields[i].fType->size();
                }
                return total;
            }
            default:
                ABORT(("cannot determine size of type " + fName).c_str());
        }
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
