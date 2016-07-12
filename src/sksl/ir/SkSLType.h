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

/**
 * Represents a type, such as int or vec4.
 */
class Type : public Symbol {
public:
    struct Field {
        Field(Modifiers modifiers, std::string name, const Type& type)
        : fModifiers(modifiers)
        , fName(std::move(name))
        , fType(std::move(type)) {}

        const std::string description() const {
            return fType.description() + " " + fName + ";";
        }

        const Modifiers fModifiers;
        const std::string fName;
        const Type& fType;
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
                    size_t alignment = fFields[i].fType.alignment();
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
                    size_t alignment = fFields[i].fType.alignment();
                    if (total % alignment != 0) {
                        total += alignment - total % alignment;
                    }
                    ASSERT(false);
                    ASSERT(total % alignment == 0);
                    total += fFields[i].fType.size();
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
    const Type& toCompound(int columns, int rows) const;

private:
    typedef Symbol INHERITED;

    const Kind fTypeKind;
    const bool fIsNumber = false;
    const Type* fComponentType = nullptr;
    const std::vector<const Type*> fCoercibleTypes = { };
    const int fColumns = -1;
    const int fRows = -1;
    const std::vector<Field> fFields = { };
    const SpvDim_ fDimensions = SpvDim1D;
    const bool fIsDepth = false;
    const bool fIsArrayed = false;
    const bool fIsMultisampled = false;
    const bool fIsSampled = false;
};

extern const Type* kVoid_Type;

extern const Type* kFloat_Type;
extern const Type* kVec2_Type;
extern const Type* kVec3_Type;
extern const Type* kVec4_Type;
extern const Type* kDouble_Type;
extern const Type* kDVec2_Type;
extern const Type* kDVec3_Type;
extern const Type* kDVec4_Type;
extern const Type* kInt_Type;
extern const Type* kIVec2_Type;
extern const Type* kIVec3_Type;
extern const Type* kIVec4_Type;
extern const Type* kUInt_Type;
extern const Type* kUVec2_Type;
extern const Type* kUVec3_Type;
extern const Type* kUVec4_Type;
extern const Type* kBool_Type;
extern const Type* kBVec2_Type;
extern const Type* kBVec3_Type;
extern const Type* kBVec4_Type;

extern const Type* kMat2x2_Type;
extern const Type* kMat2x3_Type;
extern const Type* kMat2x4_Type;
extern const Type* kMat3x2_Type;
extern const Type* kMat3x3_Type;
extern const Type* kMat3x4_Type;
extern const Type* kMat4x2_Type;
extern const Type* kMat4x3_Type;
extern const Type* kMat4x4_Type;

extern const Type* kDMat2x2_Type;
extern const Type* kDMat2x3_Type;
extern const Type* kDMat2x4_Type;
extern const Type* kDMat3x2_Type;
extern const Type* kDMat3x3_Type;
extern const Type* kDMat3x4_Type;
extern const Type* kDMat4x2_Type;
extern const Type* kDMat4x3_Type;
extern const Type* kDMat4x4_Type;

extern const Type* kSampler1D_Type;
extern const Type* kSampler2D_Type;
extern const Type* kSampler3D_Type;
extern const Type* kSamplerCube_Type;
extern const Type* kSampler2DRect_Type;
extern const Type* kSampler1DArray_Type;
extern const Type* kSampler2DArray_Type;
extern const Type* kSamplerCubeArray_Type;
extern const Type* kSamplerBuffer_Type;
extern const Type* kSampler2DMS_Type;
extern const Type* kSampler2DMSArray_Type;

extern const Type* kGSampler1D_Type;
extern const Type* kGSampler2D_Type;
extern const Type* kGSampler3D_Type;
extern const Type* kGSamplerCube_Type;
extern const Type* kGSampler2DRect_Type;
extern const Type* kGSampler1DArray_Type;
extern const Type* kGSampler2DArray_Type;
extern const Type* kGSamplerCubeArray_Type;
extern const Type* kGSamplerBuffer_Type;
extern const Type* kGSampler2DMS_Type;
extern const Type* kGSampler2DMSArray_Type;

extern const Type* kSampler1DShadow_Type;
extern const Type* kSampler2DShadow_Type;
extern const Type* kSamplerCubeShadow_Type;
extern const Type* kSampler2DRectShadow_Type;
extern const Type* kSampler1DArrayShadow_Type;
extern const Type* kSampler2DArrayShadow_Type;
extern const Type* kSamplerCubeArrayShadow_Type;
extern const Type* kGSampler2DArrayShadow_Type;
extern const Type* kGSamplerCubeArrayShadow_Type;

extern const Type* kGenType_Type;
extern const Type* kGenDType_Type;
extern const Type* kGenIType_Type;
extern const Type* kGenUType_Type;
extern const Type* kGenBType_Type;
extern const Type* kMat_Type;
extern const Type* kVec_Type;
extern const Type* kGVec_Type;
extern const Type* kGVec2_Type;
extern const Type* kGVec3_Type;
extern const Type* kGVec4_Type;
extern const Type* kDVec_Type;
extern const Type* kIVec_Type;
extern const Type* kUVec_Type;
extern const Type* kBVec_Type;

extern const Type* kInvalid_Type;

} // namespace

#endif
