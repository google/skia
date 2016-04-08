/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypesPriv_DEFINED
#define GrTypesPriv_DEFINED

#include "GrTypes.h"
#include "SkRect.h"

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
    kMat22f_GrSLType,
    kMat33f_GrSLType,
    kMat44f_GrSLType,
    kSampler2D_GrSLType,
    kSamplerExternal_GrSLType,
    kSampler2DRect_GrSLType,
    kBool_GrSLType,
    kInt_GrSLType,
    kUint_GrSLType,

    kLast_GrSLType = kUint_GrSLType
};
static const int kGrSLTypeCount = kLast_GrSLType + 1;

enum GrShaderType {
    kVertex_GrShaderType,
    kGeometry_GrShaderType,
    kFragment_GrShaderType,

    kLastkFragment_GrShaderType = kFragment_GrShaderType
};
static const int kGrShaderTypeCount = kLastkFragment_GrShaderType + 1;

enum GrShaderFlags {
    kNone_GrShaderFlags = 0,
    kVertex_GrShaderFlag = 1 << kVertex_GrShaderType,
    kGeometry_GrShaderFlag = 1 << kGeometry_GrShaderType,
    kFragment_GrShaderFlag = 1 << kFragment_GrShaderType
};
GR_MAKE_BITFIELD_OPS(GrShaderFlags);

/**
 * Precisions of shader language variables. Not all shading languages support precisions or actually
 * vary the internal precision based on the qualifiers. These currently only apply to float types (
 * including float vectors and matrices).
 */
enum GrSLPrecision {
    kLow_GrSLPrecision,
    kMedium_GrSLPrecision,
    kHigh_GrSLPrecision,

    // Default precision is medium. This is because on OpenGL ES 2 highp support is not
    // guaranteed. On (non-ES) OpenGL the specifiers have no effect on precision.
    kDefault_GrSLPrecision = kMedium_GrSLPrecision,

    kLast_GrSLPrecision = kHigh_GrSLPrecision
};

static const int kGrSLPrecisionCount = kLast_GrSLPrecision + 1;

/**
 * Gets the vector size of the SLType. Returns -1 for void, matrices, and samplers.
 */
static inline int GrSLTypeVectorCount(GrSLType type) {
    SkASSERT(type >= 0 && type < static_cast<GrSLType>(kGrSLTypeCount));
    static const int kCounts[] = { -1, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, 1, 1, 1 };
    return kCounts[type];

    GR_STATIC_ASSERT(0 == kVoid_GrSLType);
    GR_STATIC_ASSERT(1 == kFloat_GrSLType);
    GR_STATIC_ASSERT(2 == kVec2f_GrSLType);
    GR_STATIC_ASSERT(3 == kVec3f_GrSLType);
    GR_STATIC_ASSERT(4 == kVec4f_GrSLType);
    GR_STATIC_ASSERT(5 == kMat22f_GrSLType);
    GR_STATIC_ASSERT(6 == kMat33f_GrSLType);
    GR_STATIC_ASSERT(7 == kMat44f_GrSLType);
    GR_STATIC_ASSERT(8 == kSampler2D_GrSLType);
    GR_STATIC_ASSERT(9 == kSamplerExternal_GrSLType);
    GR_STATIC_ASSERT(10 == kSampler2DRect_GrSLType);
    GR_STATIC_ASSERT(11 == kBool_GrSLType);
    GR_STATIC_ASSERT(12 == kInt_GrSLType);
    GR_STATIC_ASSERT(13 == kUint_GrSLType);
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

/** Is the shading language type float (including vectors/matrices)? */
static inline bool GrSLTypeIsFloatType(GrSLType type) {
    SkASSERT(type >= 0 && type < static_cast<GrSLType>(kGrSLTypeCount));
    return type >= kFloat_GrSLType && type <= kMat44f_GrSLType;

    GR_STATIC_ASSERT(0 == kVoid_GrSLType);
    GR_STATIC_ASSERT(1 == kFloat_GrSLType);
    GR_STATIC_ASSERT(2 == kVec2f_GrSLType);
    GR_STATIC_ASSERT(3 == kVec3f_GrSLType);
    GR_STATIC_ASSERT(4 == kVec4f_GrSLType);
    GR_STATIC_ASSERT(5 == kMat22f_GrSLType);
    GR_STATIC_ASSERT(6 == kMat33f_GrSLType);
    GR_STATIC_ASSERT(7 == kMat44f_GrSLType);
    GR_STATIC_ASSERT(8 == kSampler2D_GrSLType);
    GR_STATIC_ASSERT(9 == kSamplerExternal_GrSLType);
    GR_STATIC_ASSERT(10 == kSampler2DRect_GrSLType);
    GR_STATIC_ASSERT(11 == kBool_GrSLType);
    GR_STATIC_ASSERT(12 == kInt_GrSLType);
    GR_STATIC_ASSERT(13 == kUint_GrSLType);
    GR_STATIC_ASSERT(14 == kGrSLTypeCount);
}

/** Is the shading language type integral (including vectors/matrices)? */
static inline bool GrSLTypeIsIntType(GrSLType type) {
    SkASSERT(type >= 0 && type < static_cast<GrSLType>(kGrSLTypeCount));
    return type >= kInt_GrSLType;

    GR_STATIC_ASSERT(0 == kVoid_GrSLType);
    GR_STATIC_ASSERT(1 == kFloat_GrSLType);
    GR_STATIC_ASSERT(2 == kVec2f_GrSLType);
    GR_STATIC_ASSERT(3 == kVec3f_GrSLType);
    GR_STATIC_ASSERT(4 == kVec4f_GrSLType);
    GR_STATIC_ASSERT(5 == kMat22f_GrSLType);
    GR_STATIC_ASSERT(6 == kMat33f_GrSLType);
    GR_STATIC_ASSERT(7 == kMat44f_GrSLType);
    GR_STATIC_ASSERT(8 == kSampler2D_GrSLType);
    GR_STATIC_ASSERT(9 == kSamplerExternal_GrSLType);
    GR_STATIC_ASSERT(10 == kSampler2DRect_GrSLType);
    GR_STATIC_ASSERT(11 == kBool_GrSLType);
    GR_STATIC_ASSERT(12 == kInt_GrSLType);
    GR_STATIC_ASSERT(13 == kUint_GrSLType);
    GR_STATIC_ASSERT(14 == kGrSLTypeCount);
}

/** Is the shading language type numeric (including vectors/matrices)? */
static inline bool GrSLTypeIsNumeric(GrSLType type) {
    return GrSLTypeIsFloatType(type) || GrSLTypeIsIntType(type);
}

/** Returns the size in bytes for floating point GrSLTypes. For non floating point type returns 0 */
static inline size_t GrSLTypeSize(GrSLType type) {
    SkASSERT(GrSLTypeIsFloatType(type));
    static const size_t kSizes[] = {
        0,                        // kVoid_GrSLType
        sizeof(float),            // kFloat_GrSLType
        2 * sizeof(float),        // kVec2f_GrSLType
        3 * sizeof(float),        // kVec3f_GrSLType
        4 * sizeof(float),        // kVec4f_GrSLType
        2 * 2 * sizeof(float),    // kMat22f_GrSLType
        3 * 3 * sizeof(float),    // kMat33f_GrSLType
        4 * 4 * sizeof(float),    // kMat44f_GrSLType
        0,                        // kSampler2D_GrSLType
        0,                        // kSamplerExternal_GrSLType
        0,                        // kSampler2DRect_GrSLType
        0,                        // kBool_GrSLType
        0,                        // kInt_GrSLType
        0,                        // kUint_GrSLType
    };
    return kSizes[type];

    GR_STATIC_ASSERT(0 == kVoid_GrSLType);
    GR_STATIC_ASSERT(1 == kFloat_GrSLType);
    GR_STATIC_ASSERT(2 == kVec2f_GrSLType);
    GR_STATIC_ASSERT(3 == kVec3f_GrSLType);
    GR_STATIC_ASSERT(4 == kVec4f_GrSLType);
    GR_STATIC_ASSERT(5 == kMat22f_GrSLType);
    GR_STATIC_ASSERT(6 == kMat33f_GrSLType);
    GR_STATIC_ASSERT(7 == kMat44f_GrSLType);
    GR_STATIC_ASSERT(8 == kSampler2D_GrSLType);
    GR_STATIC_ASSERT(9 == kSamplerExternal_GrSLType);
    GR_STATIC_ASSERT(10 == kSampler2DRect_GrSLType);
    GR_STATIC_ASSERT(11 == kBool_GrSLType);
    GR_STATIC_ASSERT(12 == kInt_GrSLType);
    GR_STATIC_ASSERT(13 == kUint_GrSLType);
    GR_STATIC_ASSERT(14 == kGrSLTypeCount);
}

static inline bool GrSLTypeIsSamplerType(GrSLType type) {
    SkASSERT(type >= 0 && type < static_cast<GrSLType>(kGrSLTypeCount));
    return type >= kSampler2D_GrSLType && type <= kSampler2DRect_GrSLType;

    GR_STATIC_ASSERT(8 == kSampler2D_GrSLType);
    GR_STATIC_ASSERT(9 == kSamplerExternal_GrSLType);
    GR_STATIC_ASSERT(10 == kSampler2DRect_GrSLType);
}

static inline bool GrSLTypeAcceptsPrecision(GrSLType type) {
    return GrSLTypeIsNumeric(type) || GrSLTypeIsSamplerType(type);
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

    kUByte_GrVertexAttribType,   // unsigned byte, e.g. coverage
    kVec4ub_GrVertexAttribType,  // vector of 4 unsigned bytes, e.g. colors

    kVec2us_GrVertexAttribType,   // vector of 2 shorts, e.g. texture coordinates

    kInt_GrVertexAttribType,
    kUint_GrVertexAttribType,
    
    kLast_GrVertexAttribType = kUint_GrVertexAttribType
};
static const int kGrVertexAttribTypeCount = kLast_GrVertexAttribType + 1;

/**
 * Returns the vector size of the type.
 */
static inline int GrVertexAttribTypeVectorCount(GrVertexAttribType type) {
    SkASSERT(type >= 0 && type < kGrVertexAttribTypeCount);
    static const int kCounts[] = { 1, 2, 3, 4, 1, 4, 2, 1, 1 };
    return kCounts[type];

    GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
    GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
    GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
    GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
    GR_STATIC_ASSERT(4 == kUByte_GrVertexAttribType);
    GR_STATIC_ASSERT(5 == kVec4ub_GrVertexAttribType);
    GR_STATIC_ASSERT(6 == kVec2us_GrVertexAttribType);
    GR_STATIC_ASSERT(7 == kInt_GrVertexAttribType);
    GR_STATIC_ASSERT(8 == kUint_GrVertexAttribType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kCounts) == kGrVertexAttribTypeCount);
}

/**
 * Returns the size of the attrib type in bytes.
 */
static inline size_t GrVertexAttribTypeSize(GrVertexAttribType type) {
    static const size_t kSizes[] = {
        sizeof(float),          // kFloat_GrVertexAttribType
        2*sizeof(float),        // kVec2f_GrVertexAttribType
        3*sizeof(float),        // kVec3f_GrVertexAttribType
        4*sizeof(float),        // kVec4f_GrVertexAttribType
        1*sizeof(char),         // kUByte_GrVertexAttribType
        4*sizeof(char),         // kVec4ub_GrVertexAttribType
        2*sizeof(int16_t),      // kVec2us_GrVertexAttribType
        sizeof(int32_t),        // kInt_GrVertexAttribType
        sizeof(uint32_t)        // kUint_GrVertexAttribType
    };
    return kSizes[type];

    GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
    GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
    GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
    GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
    GR_STATIC_ASSERT(4 == kUByte_GrVertexAttribType);
    GR_STATIC_ASSERT(5 == kVec4ub_GrVertexAttribType);
    GR_STATIC_ASSERT(6 == kVec2us_GrVertexAttribType);
    GR_STATIC_ASSERT(7 == kInt_GrVertexAttribType);
    GR_STATIC_ASSERT(8 == kUint_GrVertexAttribType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kSizes) == kGrVertexAttribTypeCount);
}

/**
 * Is the attrib type integral?
 */
static inline bool GrVertexAttribTypeIsIntType(GrVertexAttribType type) {
    SkASSERT(type >= 0 && type < static_cast<GrVertexAttribType>(kGrVertexAttribTypeCount));
    return type >= kInt_GrVertexAttribType;

    GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
    GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
    GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
    GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
    GR_STATIC_ASSERT(4 == kUByte_GrVertexAttribType);
    GR_STATIC_ASSERT(5 == kVec4ub_GrVertexAttribType);
    GR_STATIC_ASSERT(6 == kVec2us_GrVertexAttribType);
    GR_STATIC_ASSERT(7 == kInt_GrVertexAttribType);
    GR_STATIC_ASSERT(8 == kUint_GrVertexAttribType);
    GR_STATIC_ASSERT(9 == kGrVertexAttribTypeCount);
}

/**
 * converts a GrVertexAttribType to a GrSLType
 */
static inline GrSLType GrVertexAttribTypeToSLType(GrVertexAttribType type) {
    switch (type) {
        default:
            SkFAIL("Unsupported type conversion");
            return kVoid_GrSLType;
        case kUByte_GrVertexAttribType:
        case kFloat_GrVertexAttribType:
            return kFloat_GrSLType;
        case kVec2us_GrVertexAttribType:
        case kVec2f_GrVertexAttribType:
            return kVec2f_GrSLType;
        case kVec3f_GrVertexAttribType:
            return kVec3f_GrSLType;
        case kVec4ub_GrVertexAttribType:
        case kVec4f_GrVertexAttribType:
            return kVec4f_GrSLType;
        case kInt_GrVertexAttribType:
            return kInt_GrSLType;
        case kUint_GrVertexAttribType:
            return kUint_GrSLType;
    }
}

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

/**
 * Indicates the type of pending IO operations that can be recorded for gpu resources.
 */
enum GrIOType {
    kRead_GrIOType,
    kWrite_GrIOType,
    kRW_GrIOType
};

struct GrScissorState {
    GrScissorState() : fEnabled(false) {}
    void set(const SkIRect& rect) { fRect = rect; fEnabled = true; }
    bool operator==(const GrScissorState& other) const {
        return fEnabled == other.fEnabled &&
                (false == fEnabled || fRect == other.fRect);
    }
    bool operator!=(const GrScissorState& other) const { return !(*this == other); }

    bool enabled() const { return fEnabled; }
    const SkIRect& rect() const { return fRect; }

private:
    bool    fEnabled;
    SkIRect fRect;
};

/**
* Indicates the type of data that a GPU buffer will be used for.
*/
enum GrBufferType {
    kVertex_GrBufferType,
    kIndex_GrBufferType,
    kTexel_GrBufferType,
    kDrawIndirect_GrBufferType,
    kXferCpuToGpu_GrBufferType,
    kXferGpuToCpu_GrBufferType,

    kLast_GrBufferType = kXferGpuToCpu_GrBufferType
};
static const int kGrBufferTypeCount = kLast_GrBufferType + 1;

static inline bool GrBufferTypeIsVertexOrIndex(GrBufferType type) {
    SkASSERT(type >= 0 && type < kGrBufferTypeCount);
    return type <= kIndex_GrBufferType;

    GR_STATIC_ASSERT(0 == kVertex_GrBufferType);
    GR_STATIC_ASSERT(1 == kIndex_GrBufferType);
}

/**
* Provides a performance hint regarding the frequency at which a data store will be accessed.
*/
enum GrAccessPattern {
    /** Data store will be respecified repeatedly and used many times. */
    kDynamic_GrAccessPattern,
    /** Data store will be specified once and used many times. (Thus disqualified from caching.) */
    kStatic_GrAccessPattern,
    /** Data store will be specified once and used at most a few times. (Also can't be cached.) */
    kStream_GrAccessPattern,

    kLast_GrAccessPattern = kStream_GrAccessPattern
};


#ifdef SK_DEBUG
// Takes a pointer to a GrCaps, and will suppress prints if required
#define GrCapsDebugf(caps, ...)         \
    if (!caps->suppressPrints()) {      \
        SkDebugf(__VA_ARGS__);          \
    }
#else
#define GrCapsDebugf(caps, ...)
#endif

#endif
