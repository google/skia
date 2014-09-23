/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypesPriv_DEFINED
#define GrTypesPriv_DEFINED

#include "GrTypes.h"
#include "SkTArray.h"

/**
 * Types of shader-language-specific boxed variables we can create. (Currently only GrGLShaderVars,
 * but should be applicable to other shader languages.)
 */
enum GrSLType {
    kVoid_GrSLType,
    kFloat_GrSLType,
    kVec2f_GrSLType,
    kVec3f_GrSLType,
    kVec4f_GrSLType,
    kMat33f_GrSLType,
    kMat44f_GrSLType,
    kSampler2D_GrSLType,

    kLast_GrSLType = kSampler2D_GrSLType
};
static const int kGrSLTypeCount = kLast_GrSLType + 1;

/**
 * Gets the vector size of the SLType. Returns -1 for void, matrices, and samplers.
 */
static inline int GrSLTypeVectorCount(GrSLType type) {
    SkASSERT(type >= 0 && type < static_cast<GrSLType>(kGrSLTypeCount));
    static const int kCounts[] = { -1, 1, 2, 3, 4, -1, -1, -1 };
    return kCounts[type];

    GR_STATIC_ASSERT(0 == kVoid_GrSLType);
    GR_STATIC_ASSERT(1 == kFloat_GrSLType);
    GR_STATIC_ASSERT(2 == kVec2f_GrSLType);
    GR_STATIC_ASSERT(3 == kVec3f_GrSLType);
    GR_STATIC_ASSERT(4 == kVec4f_GrSLType);
    GR_STATIC_ASSERT(5 == kMat33f_GrSLType);
    GR_STATIC_ASSERT(6 == kMat44f_GrSLType);
    GR_STATIC_ASSERT(7 == kSampler2D_GrSLType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kCounts) == kGrSLTypeCount);
}

/** Return the type enum for a vector of floats of length n (1..4),
 e.g. 1 -> kFloat_GrSLType, 2 -> kVec2_GrSLType, ... */
static inline GrSLType GrSLFloatVectorType(int count) {
    SkASSERT(count > 0 && count <= 4);
    return (GrSLType)(count);

    GR_STATIC_ASSERT(kFloat_GrSLType == 1);
    GR_STATIC_ASSERT(kVec2f_GrSLType == 2);
    GR_STATIC_ASSERT(kVec3f_GrSLType == 3);
    GR_STATIC_ASSERT(kVec4f_GrSLType == 4);
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Types used to describe format of vertices in arrays.
  */
enum GrVertexAttribType {
    kFloat_GrVertexAttribType = 0,
    kVec2f_GrVertexAttribType,
    kVec3f_GrVertexAttribType,
    kVec4f_GrVertexAttribType,
    kVec4ub_GrVertexAttribType,   // vector of 4 unsigned bytes, e.g. colors

    kLast_GrVertexAttribType = kVec4ub_GrVertexAttribType
};
static const int kGrVertexAttribTypeCount = kLast_GrVertexAttribType + 1;

/**
 * Returns the vector size of the type.
 */
static inline int GrVertexAttribTypeVectorCount(GrVertexAttribType type) {
    SkASSERT(type >= 0 && type < kGrVertexAttribTypeCount);
    static const int kCounts[] = { 1, 2, 3, 4, 4 };
    return kCounts[type];

    GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
    GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
    GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
    GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
    GR_STATIC_ASSERT(4 == kVec4ub_GrVertexAttribType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kCounts) == kGrVertexAttribTypeCount);
}

/**
 * Returns the size of the attrib type in bytes.
 */
static inline size_t GrVertexAttribTypeSize(GrVertexAttribType type) {
    SkASSERT(type >= 0 && type < kGrVertexAttribTypeCount);
    static const size_t kSizes[] = {
        sizeof(float),          // kFloat_GrVertexAttribType
        2*sizeof(float),        // kVec2f_GrVertexAttribType
        3*sizeof(float),        // kVec3f_GrVertexAttribType
        4*sizeof(float),        // kVec4f_GrVertexAttribType
        4*sizeof(char)          // kVec4ub_GrVertexAttribType
    };
    return kSizes[type];

    GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
    GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
    GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
    GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
    GR_STATIC_ASSERT(4 == kVec4ub_GrVertexAttribType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kSizes) == kGrVertexAttribTypeCount);
}

/**
 * Semantic bindings for vertex attributes. kEffect means that the attribute is input to a
 * GrProcessor. Each binding other than kEffect may not appear more than once in the current set of
 * attributes. kPosition must be appear for exactly one attribute.
 */
enum GrVertexAttribBinding {
    kPosition_GrVertexAttribBinding,    // required, must have vector count of 2
    kLocalCoord_GrVertexAttribBinding,  // must have vector count of 2
    kColor_GrVertexAttribBinding,       // must have vector count of 4
    kCoverage_GrVertexAttribBinding,    // must have vector count of 4

    kLastFixedFunction_GrVertexAttribBinding = kCoverage_GrVertexAttribBinding,

    kGeometryProcessor_GrVertexAttribBinding,      // vector length must agree with
                                        // GrProcessor::vertexAttribType() for each effect input to
                                        // which the attribute is mapped by GrDrawState::setEffect()
    kLast_GrVertexAttribBinding = kGeometryProcessor_GrVertexAttribBinding
};

static const int kGrVertexAttribBindingCnt = kLast_GrVertexAttribBinding + 1;
static const int kGrFixedFunctionVertexAttribBindingCnt =
    kLastFixedFunction_GrVertexAttribBinding + 1;

static inline int GrFixedFunctionVertexAttribVectorCount(GrVertexAttribBinding binding) {
    SkASSERT(binding >= 0 && binding < kGrFixedFunctionVertexAttribBindingCnt);
    static const int kVecCounts[] = { 2, 2, 4, 4 };

    return kVecCounts[binding];

    GR_STATIC_ASSERT(0 == kPosition_GrVertexAttribBinding);
    GR_STATIC_ASSERT(1 == kLocalCoord_GrVertexAttribBinding);
    GR_STATIC_ASSERT(2 == kColor_GrVertexAttribBinding);
    GR_STATIC_ASSERT(3 == kCoverage_GrVertexAttribBinding);
    GR_STATIC_ASSERT(kGrFixedFunctionVertexAttribBindingCnt == SK_ARRAY_COUNT(kVecCounts));
}

struct GrVertexAttrib {
    inline void set(GrVertexAttribType type, size_t offset, GrVertexAttribBinding binding) {
        fType = type;
        fOffset = offset;
        fBinding = binding;
    }
    bool operator==(const GrVertexAttrib& other) const {
        return fType == other.fType && fOffset == other.fOffset && fBinding == other.fBinding;
    };
    bool operator!=(const GrVertexAttrib& other) const { return !(*this == other); }

    GrVertexAttribType      fType;
    size_t                  fOffset;
    GrVertexAttribBinding   fBinding;
};

template <int N> class GrVertexAttribArray : public SkSTArray<N, GrVertexAttrib, true> {};

//////////////////////////////////////////////////////////////////////////////

/**
* We have coverage effects that clip rendering to the edge of some geometric primitive.
* This enum specifies how that clipping is performed. Not all factories that take a
* GrProcessorEdgeType will succeed with all values and it is up to the caller to check for
* a NULL return.
*/
enum GrPrimitiveEdgeType {
    kFillBW_GrProcessorEdgeType,
    kFillAA_GrProcessorEdgeType,
    kInverseFillBW_GrProcessorEdgeType,
    kInverseFillAA_GrProcessorEdgeType,
    kHairlineAA_GrProcessorEdgeType,

    kLast_GrProcessorEdgeType = kHairlineAA_GrProcessorEdgeType
};

static const int kGrProcessorEdgeTypeCnt = kLast_GrProcessorEdgeType + 1;

static inline bool GrProcessorEdgeTypeIsFill(const GrPrimitiveEdgeType edgeType) {
    return (kFillAA_GrProcessorEdgeType == edgeType || kFillBW_GrProcessorEdgeType == edgeType);
}

static inline bool GrProcessorEdgeTypeIsInverseFill(const GrPrimitiveEdgeType edgeType) {
    return (kInverseFillAA_GrProcessorEdgeType == edgeType ||
            kInverseFillBW_GrProcessorEdgeType == edgeType);
}

static inline bool GrProcessorEdgeTypeIsAA(const GrPrimitiveEdgeType edgeType) {
    return (kFillBW_GrProcessorEdgeType != edgeType && kInverseFillBW_GrProcessorEdgeType != edgeType);
}

static inline GrPrimitiveEdgeType GrInvertProcessorEdgeType(const GrPrimitiveEdgeType edgeType) {
    switch (edgeType) {
        case kFillBW_GrProcessorEdgeType:
            return kInverseFillBW_GrProcessorEdgeType;
        case kFillAA_GrProcessorEdgeType:
            return kInverseFillAA_GrProcessorEdgeType;
        case kInverseFillBW_GrProcessorEdgeType:
            return kFillBW_GrProcessorEdgeType;
        case kInverseFillAA_GrProcessorEdgeType:
            return kFillAA_GrProcessorEdgeType;
        case kHairlineAA_GrProcessorEdgeType:
            SkFAIL("Hairline fill isn't invertible.");
    }
    return kFillAA_GrProcessorEdgeType; // suppress warning.
}

#endif
