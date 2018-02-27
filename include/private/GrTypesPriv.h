/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypesPriv_DEFINED
#define GrTypesPriv_DEFINED

#include <chrono>
#include "GrSharedEnums.h"
#include "GrTypes.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"

class GrCaps;

// The old libstdc++ uses the draft name "monotonic_clock" rather than "steady_clock". This might
// not actually be monotonic, depending on how libstdc++ was built. However, this is only currently
// used for idle resource purging so it shouldn't cause a correctness problem.
#if defined(__GLIBCXX__) && (__GLIBCXX__ < 20130000)
using GrStdSteadyClock = std::chrono::monotonic_clock;
#else
using GrStdSteadyClock = std::chrono::steady_clock;
#endif

/** This enum is used to specify the load operation to be used when an
 *  opList/GrGpuCommandBuffer begins execution.
 */
enum class GrLoadOp {
    kLoad,
    kClear,
    kDiscard,
};

/** This enum is used to specify the store operation to be used when an
 *  opList/GrGpuCommandBuffer ends execution.
 */
enum class GrStoreOp {
    kStore,
    kDiscard,
};

/** This enum indicates the type of antialiasing to be performed. */
enum class GrAAType : unsigned {
    /** No antialiasing */
    kNone,
    /** Use fragment shader code to compute a fractional pixel coverage. */
    kCoverage,
    /** Use normal MSAA. */
    kMSAA,
    /**
     * Use "mixed samples" MSAA such that the stencil buffer is multisampled but the color buffer is
     * not.
     */
    kMixedSamples
};

static inline bool GrAATypeIsHW(GrAAType type) {
    switch (type) {
        case GrAAType::kNone:
            return false;
        case GrAAType::kCoverage:
            return false;
        case GrAAType::kMSAA:
            return true;
        case GrAAType::kMixedSamples:
            return true;
    }
    SK_ABORT("Unknown AA Type");
    return false;
}

/** The type of full scene antialiasing supported by a render target. */
enum class GrFSAAType {
    /** No FSAA */
    kNone,
    /** Regular MSAA where each attachment has the same sample count. */
    kUnifiedMSAA,
    /** One color sample, N stencil samples. */
    kMixedSamples,
};

/**
 * Not all drawing code paths support using mixed samples when available and instead use
 * coverage-based aa.
 */
enum class GrAllowMixedSamples : bool { kNo = false, kYes = true };

GrAAType GrChooseAAType(GrAA, GrFSAAType, GrAllowMixedSamples, const GrCaps&);

/**
 * Some pixel configs are inherently clamped to [0,1], while others can hold values outside of that
 * range. This is important for blending - the latter category may require manual clamping.
 */
enum class GrPixelConfigIsClamped : bool {
    kNo = false,   // F16 or F32
    kYes = true,  // Any UNORM type
};

/**
 * Types of shader-language-specific boxed variables we can create. (Currently only GrGLShaderVars,
 * but should be applicable to other shader languages.)
 */
enum GrSLType {
    kVoid_GrSLType,
    kBool_GrSLType,
    kShort_GrSLType,
    kShort2_GrSLType,
    kShort3_GrSLType,
    kShort4_GrSLType,
    kUShort_GrSLType,
    kUShort2_GrSLType,
    kUShort3_GrSLType,
    kUShort4_GrSLType,
    kFloat_GrSLType,
    kFloat2_GrSLType,
    kFloat3_GrSLType,
    kFloat4_GrSLType,
    kFloat2x2_GrSLType,
    kFloat3x3_GrSLType,
    kFloat4x4_GrSLType,
    kHalf_GrSLType,
    kHalf2_GrSLType,
    kHalf3_GrSLType,
    kHalf4_GrSLType,
    kHalf2x2_GrSLType,
    kHalf3x3_GrSLType,
    kHalf4x4_GrSLType,
    kInt_GrSLType,
    kInt2_GrSLType,
    kInt3_GrSLType,
    kInt4_GrSLType,
    kUint_GrSLType,
    kUint2_GrSLType,
    kTexture2DSampler_GrSLType,
    kTextureExternalSampler_GrSLType,
    kTexture2DRectSampler_GrSLType,
    kBufferSampler_GrSLType,
    kTexture2D_GrSLType,
    kSampler_GrSLType,
};

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

    // Default precision is a special tag that means "whatever the default for the program/type
    // combination is". In other words, it maps to the empty string in shader code. There are some
    // scenarios where kDefault is not allowed (as the default precision for a program, or for
    // varyings, for example).
    kDefault_GrSLPrecision,

    // We only consider the "real" precisions here
    kLast_GrSLPrecision = kHigh_GrSLPrecision,
};

static const int kGrSLPrecisionCount = kLast_GrSLPrecision + 1;

/** Is the shading language type float (including vectors/matrices)? */
static inline bool GrSLTypeIsFloatType(GrSLType type) {
    switch (type) {
        case kFloat_GrSLType:
        case kFloat2_GrSLType:
        case kFloat3_GrSLType:
        case kFloat4_GrSLType:
        case kFloat2x2_GrSLType:
        case kFloat3x3_GrSLType:
        case kFloat4x4_GrSLType:
        case kHalf_GrSLType:
        case kHalf2_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
        case kHalf2x2_GrSLType:
        case kHalf3x3_GrSLType:
        case kHalf4x4_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
        case kBool_GrSLType:
        case kShort_GrSLType:
        case kShort2_GrSLType:
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort_GrSLType:
        case kUShort2_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kInt_GrSLType:
        case kInt2_GrSLType:
        case kInt3_GrSLType:
        case kInt4_GrSLType:
        case kUint_GrSLType:
        case kUint2_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
            return false;
    }
    SK_ABORT("Unexpected type");
    return false;
}

/** If the type represents a single value or vector return the vector length, else -1. */
static inline int GrSLTypeVecLength(GrSLType type) {
    switch (type) {
        case kFloat_GrSLType:
        case kHalf_GrSLType:
        case kBool_GrSLType:
        case kShort_GrSLType:
        case kUShort_GrSLType:
        case kInt_GrSLType:
        case kUint_GrSLType:
            return 1;

        case kFloat2_GrSLType:
        case kHalf2_GrSLType:
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
        case kInt2_GrSLType:
        case kUint2_GrSLType:
            return 2;

        case kFloat3_GrSLType:
        case kHalf3_GrSLType:
        case kShort3_GrSLType:
        case kUShort3_GrSLType:
        case kInt3_GrSLType:
            return 3;

        case kFloat4_GrSLType:
        case kHalf4_GrSLType:
        case kShort4_GrSLType:
        case kUShort4_GrSLType:
        case kInt4_GrSLType:
            return 4;

        case kFloat2x2_GrSLType:
        case kFloat3x3_GrSLType:
        case kFloat4x4_GrSLType:
        case kHalf2x2_GrSLType:
        case kHalf3x3_GrSLType:
        case kHalf4x4_GrSLType:
        case kVoid_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
            return -1;
    }
    SK_ABORT("Unexpected type");
    return -1;
}

static inline bool GrSLTypeIs2DCombinedSamplerType(GrSLType type) {
    switch (type) {
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kFloat_GrSLType:
        case kFloat2_GrSLType:
        case kFloat3_GrSLType:
        case kFloat4_GrSLType:
        case kFloat2x2_GrSLType:
        case kFloat3x3_GrSLType:
        case kFloat4x4_GrSLType:
        case kHalf_GrSLType:
        case kHalf2_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
        case kHalf2x2_GrSLType:
        case kHalf3x3_GrSLType:
        case kHalf4x4_GrSLType:
        case kInt_GrSLType:
        case kInt2_GrSLType:
        case kInt3_GrSLType:
        case kInt4_GrSLType:
        case kUint_GrSLType:
        case kUint2_GrSLType:
        case kBufferSampler_GrSLType:
        case kBool_GrSLType:
        case kShort_GrSLType:
        case kShort2_GrSLType:
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort_GrSLType:
        case kUShort2_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
            return false;
    }
    SK_ABORT("Unexpected type");
    return false;
}

static inline bool GrSLTypeIsCombinedSamplerType(GrSLType type) {
    switch (type) {
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kFloat_GrSLType:
        case kFloat2_GrSLType:
        case kFloat3_GrSLType:
        case kFloat4_GrSLType:
        case kFloat2x2_GrSLType:
        case kFloat3x3_GrSLType:
        case kFloat4x4_GrSLType:
        case kHalf_GrSLType:
        case kHalf2_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
        case kHalf2x2_GrSLType:
        case kHalf3x3_GrSLType:
        case kHalf4x4_GrSLType:
        case kInt_GrSLType:
        case kInt2_GrSLType:
        case kInt3_GrSLType:
        case kInt4_GrSLType:
        case kUint_GrSLType:
        case kUint2_GrSLType:
        case kBool_GrSLType:
        case kShort_GrSLType:
        case kShort2_GrSLType:
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort_GrSLType:
        case kUShort2_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
            return false;
    }
    SK_ABORT("Unexpected type");
    return false;
}

static inline bool GrSLTypeAcceptsPrecision(GrSLType type) {
    switch (type) {
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kBool_GrSLType:
        case kShort_GrSLType:
        case kShort2_GrSLType:
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort_GrSLType:
        case kUShort2_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
        case kFloat_GrSLType:
        case kFloat2_GrSLType:
        case kFloat3_GrSLType:
        case kFloat4_GrSLType:
        case kFloat2x2_GrSLType:
        case kFloat3x3_GrSLType:
        case kFloat4x4_GrSLType:
        case kHalf_GrSLType:
        case kHalf2_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
        case kHalf2x2_GrSLType:
        case kHalf3x3_GrSLType:
        case kHalf4x4_GrSLType:
        case kInt_GrSLType:
        case kInt2_GrSLType:
        case kInt3_GrSLType:
        case kInt4_GrSLType:
        case kUint_GrSLType:
        case kUint2_GrSLType:
            return false;
    }
    SK_ABORT("Unexpected type");
    return false;
}

// temporarily accepting (but ignoring) precision modifiers on the new types; this will be killed
// in a future CL
static inline bool GrSLTypeTemporarilyAcceptsPrecision(GrSLType type) {
    switch (type) {
        case kShort_GrSLType:
        case kUShort_GrSLType:
        case kFloat_GrSLType:
        case kFloat2_GrSLType:
        case kFloat3_GrSLType:
        case kFloat4_GrSLType:
        case kFloat2x2_GrSLType:
        case kFloat3x3_GrSLType:
        case kFloat4x4_GrSLType:
        case kHalf_GrSLType:
        case kHalf2_GrSLType:
        case kHalf3_GrSLType:
        case kHalf4_GrSLType:
        case kHalf2x2_GrSLType:
        case kHalf3x3_GrSLType:
        case kHalf4x4_GrSLType:
        case kInt_GrSLType:
        case kInt2_GrSLType:
        case kInt3_GrSLType:
        case kInt4_GrSLType:
        case kUint_GrSLType:
        case kUint2_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kBool_GrSLType:
        case kShort2_GrSLType:
        case kShort3_GrSLType:
        case kShort4_GrSLType:
        case kUShort2_GrSLType:
        case kUShort3_GrSLType:
        case kUShort4_GrSLType:
            return false;
    }
    SK_ABORT("Unexpected type");
    return false;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Types used to describe format of vertices in arrays.
 */
enum GrVertexAttribType {
    kFloat_GrVertexAttribType = 0,
    kFloat2_GrVertexAttribType,
    kFloat3_GrVertexAttribType,
    kFloat4_GrVertexAttribType,
    kHalf_GrVertexAttribType,
    kHalf2_GrVertexAttribType,
    kHalf3_GrVertexAttribType,
    kHalf4_GrVertexAttribType,

    kInt2_GrVertexAttribType,   // vector of 2 32-bit ints
    kInt3_GrVertexAttribType,   // vector of 3 32-bit ints
    kInt4_GrVertexAttribType,   // vector of 4 32-bit ints

    kUByte_norm_GrVertexAttribType,  // unsigned byte, e.g. coverage, 0 -> 0.0f, 255 -> 1.0f.
    kUByte4_norm_GrVertexAttribType, // vector of 4 unsigned bytes, e.g. colors, 0 -> 0.0f,
                                     // 255 -> 1.0f.

    kShort2_GrVertexAttribType,       // vector of 2 16-bit shorts.
    kUShort2_GrVertexAttribType,      // vector of 2 unsigned shorts. 0 -> 0, 65535 -> 65535.
    kUShort2_norm_GrVertexAttribType, // vector of 2 unsigned shorts. 0 -> 0.0f, 65535 -> 1.0f.

    kInt_GrVertexAttribType,
    kUint_GrVertexAttribType,

    kLast_GrVertexAttribType = kUint_GrVertexAttribType
};
static const int kGrVertexAttribTypeCount = kLast_GrVertexAttribType + 1;

/**
 * Returns the size of the attrib type in bytes.
 */
static inline size_t GrVertexAttribTypeSize(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return sizeof(float);
        case kFloat2_GrVertexAttribType:
            return 2 * sizeof(float);
        case kFloat3_GrVertexAttribType:
            return 3 * sizeof(float);
        case kFloat4_GrVertexAttribType:
            return 4 * sizeof(float);
        case kHalf_GrVertexAttribType:
            return sizeof(float);
        case kHalf2_GrVertexAttribType:
            return 2 * sizeof(float);
        case kHalf3_GrVertexAttribType:
            return 3 * sizeof(float);
        case kHalf4_GrVertexAttribType:
            return 4 * sizeof(float);
        case kInt2_GrVertexAttribType:
            return 2 * sizeof(int32_t);
        case kInt3_GrVertexAttribType:
            return 3 * sizeof(int32_t);
        case kInt4_GrVertexAttribType:
            return 4 * sizeof(int32_t);
        case kUByte_norm_GrVertexAttribType:
            return 1 * sizeof(char);
        case kUByte4_norm_GrVertexAttribType:
            return 4 * sizeof(char);
        case kShort2_GrVertexAttribType:
            return 2 * sizeof(int16_t);
        case kUShort2_GrVertexAttribType: // fall through
        case kUShort2_norm_GrVertexAttribType:
            return 2 * sizeof(uint16_t);
        case kInt_GrVertexAttribType:
            return sizeof(int32_t);
        case kUint_GrVertexAttribType:
            return sizeof(uint32_t);
    }
    SK_ABORT("Unexpected attribute type");
    return 0;
}

/**
 * converts a GrVertexAttribType to a GrSLType
 */
static inline GrSLType GrVertexAttribTypeToSLType(GrVertexAttribType type) {
    switch (type) {
        case kShort2_GrVertexAttribType:
            return kShort2_GrSLType;
        case kUShort2_GrVertexAttribType:
            return kUShort2_GrSLType;
        case kUShort2_norm_GrVertexAttribType:
            return kFloat2_GrSLType;
        case kUByte_norm_GrVertexAttribType:   // fall through
        case kFloat_GrVertexAttribType:
            return kFloat_GrSLType;
        case kFloat2_GrVertexAttribType:
            return kFloat2_GrSLType;
        case kFloat3_GrVertexAttribType:
            return kFloat3_GrSLType;
        case kFloat4_GrVertexAttribType:
            return kFloat4_GrSLType;
        case kHalf_GrVertexAttribType:
            return kHalf_GrSLType;
        case kHalf2_GrVertexAttribType:
            return kHalf2_GrSLType;
        case kHalf3_GrVertexAttribType:
            return kHalf3_GrSLType;
        case kHalf4_GrVertexAttribType:
        case kUByte4_norm_GrVertexAttribType:
            return kHalf4_GrSLType;
        case kInt2_GrVertexAttribType:
            return kInt2_GrSLType;
        case kInt3_GrVertexAttribType:
            return kInt3_GrSLType;
        case kInt4_GrVertexAttribType:
            return kInt4_GrSLType;
        case kInt_GrVertexAttribType:
            return kInt_GrSLType;
        case kUint_GrVertexAttribType:
            return kUint_GrSLType;
    }
    SK_ABORT("Unsupported type conversion");
    return kVoid_GrSLType;
}

//////////////////////////////////////////////////////////////////////////////

static const int kGrClipEdgeTypeCnt = (int) GrClipEdgeType::kLast + 1;

static inline bool GrProcessorEdgeTypeIsFill(const GrClipEdgeType edgeType) {
    return (GrClipEdgeType::kFillAA == edgeType || GrClipEdgeType::kFillBW == edgeType);
}

static inline bool GrProcessorEdgeTypeIsInverseFill(const GrClipEdgeType edgeType) {
    return (GrClipEdgeType::kInverseFillAA == edgeType ||
            GrClipEdgeType::kInverseFillBW == edgeType);
}

static inline bool GrProcessorEdgeTypeIsAA(const GrClipEdgeType edgeType) {
    return (GrClipEdgeType::kFillBW != edgeType &&
            GrClipEdgeType::kInverseFillBW != edgeType);
}

static inline GrClipEdgeType GrInvertProcessorEdgeType(const GrClipEdgeType edgeType) {
    switch (edgeType) {
        case GrClipEdgeType::kFillBW:
            return GrClipEdgeType::kInverseFillBW;
        case GrClipEdgeType::kFillAA:
            return GrClipEdgeType::kInverseFillAA;
        case GrClipEdgeType::kInverseFillBW:
            return GrClipEdgeType::kFillBW;
        case GrClipEdgeType::kInverseFillAA:
            return GrClipEdgeType::kFillAA;
        case GrClipEdgeType::kHairlineAA:
            SK_ABORT("Hairline fill isn't invertible.");
    }
    return GrClipEdgeType::kFillAA;  // suppress warning.
}

/**
 * Indicates the type of pending IO operations that can be recorded for gpu resources.
 */
enum GrIOType {
    kRead_GrIOType,
    kWrite_GrIOType,
    kRW_GrIOType
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

// Flags shared between GrRenderTarget and GrRenderTargetProxy
enum class GrRenderTargetFlags {
    kNone               = 0,

    // For internal resources:
    //    this is enabled whenever MSAA is enabled and GrCaps reports mixed samples are supported
    // For wrapped resources:
    //    this is disabled for FBO0
    //    but, otherwise, is enabled whenever MSAA is enabled and GrCaps reports mixed samples
    //        are supported
    kMixedSampled       = 1 << 0,

    // For internal resources:
    //    this is enabled whenever GrCaps reports window rect support
    // For wrapped resources1
    //    this is disabled for FBO0
    //    but, otherwise, is enabled whenever GrCaps reports window rect support
    kWindowRectsSupport = 1 << 1
};
GR_MAKE_BITFIELD_CLASS_OPS(GrRenderTargetFlags)

#ifdef SK_DEBUG
// Takes a pointer to a GrCaps, and will suppress prints if required
#define GrCapsDebugf(caps, ...)      \
    if (!(caps)->suppressPrints()) { \
        SkDebugf(__VA_ARGS__);       \
    }
#else
#define GrCapsDebugf(caps, ...)
#endif

/**
 * Specifies if the holder owns the backend, OpenGL or Vulkan, object.
 */
enum class GrBackendObjectOwnership : bool {
    /** Holder does not destroy the backend object. */
    kBorrowed = false,
    /** Holder destroys the backend object. */
    kOwned = true
};

template <typename T>
T* const* unique_ptr_address_as_pointer_address(std::unique_ptr<T> const* up) {
    static_assert(sizeof(T*) == sizeof(std::unique_ptr<T>), "unique_ptr not expected size.");
    return reinterpret_cast<T* const*>(up);
}

/*
 * Object for CPU-GPU synchronization
 */
typedef uint64_t GrFence;

/**
 * Used to include or exclude specific GPU path renderers for testing purposes.
 */
enum class GpuPathRenderers {
    kNone              = 0, // Always use sofware masks and/or GrDefaultPathRenderer.
    kDashLine          = 1 << 0,
    kStencilAndCover   = 1 << 1,
    kMSAA              = 1 << 2,
    kAAConvex          = 1 << 3,
    kAALinearizing     = 1 << 4,
    kSmall             = 1 << 5,
    kCoverageCounting  = 1 << 6,
    kTessellating      = 1 << 7,

    kAll               = (kTessellating | (kTessellating - 1)),
    kDefault           = kAll
};

/**
 * Used to describe the current state of Mips on a GrTexture
 */
enum class  GrMipMapsStatus {
    kNotAllocated, // Mips have not been allocated
    kDirty,        // Mips are allocated but the full mip tree does not have valid data
    kValid,        // All levels fully allocated and have valid data in them
};

GR_MAKE_BITFIELD_CLASS_OPS(GpuPathRenderers)

/**
 * We want to extend the GrPixelConfig enum to add cases for dealing with alpha_8 which is
 * internally either alpha8 or red8. Also for Gray_8 which can be luminance_8 or red_8.
 */
static constexpr GrPixelConfig kAlpha_8_as_Alpha_GrPixelConfig = kPrivateConfig1_GrPixelConfig;
static constexpr GrPixelConfig kAlpha_8_as_Red_GrPixelConfig = kPrivateConfig2_GrPixelConfig;
static constexpr GrPixelConfig kAlpha_half_as_Red_GrPixelConfig = kPrivateConfig3_GrPixelConfig;
static constexpr GrPixelConfig kGray_8_as_Lum_GrPixelConfig = kPrivateConfig4_GrPixelConfig;
static constexpr GrPixelConfig kGray_8_as_Red_GrPixelConfig = kPrivateConfig5_GrPixelConfig;

/**
 * Refers to the encoding of a GPU buffer as it will be interpreted by the GPU when sampling and
 * blending.
 */
enum class GrSRGBEncoded : bool { kNo = false, kYes = true };

/**
 * Describes whether pixel data encoding should be converted to/from linear/sRGB encoding.
 */
enum class GrSRGBConversion {
    kNone,
    kSRGBToLinear,
    kLinearToSRGB,
};

/**
 * Utility functions for GrPixelConfig
 */

// Returns whether the config's color channels are sRGB encoded.
static inline GrSRGBEncoded GrPixelConfigIsSRGBEncoded(GrPixelConfig config) {
    switch (config) {
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
            return GrSRGBEncoded::kYes;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return GrSRGBEncoded::kNo;
    }
    SK_ABORT("Invalid pixel config");
    return GrSRGBEncoded::kNo;
}

static inline bool GrPixelConfigIsSRGB(GrPixelConfig config) {
    return GrSRGBEncoded::kYes == GrPixelConfigIsSRGBEncoded(config);
}
// Takes a config and returns the equivalent config with the R and B order
// swapped if such a config exists. Otherwise, kUnknown_GrPixelConfig
static inline GrPixelConfig GrPixelConfigSwapRAndB(GrPixelConfig config) {
    switch (config) {
        case kBGRA_8888_GrPixelConfig:
            return kRGBA_8888_GrPixelConfig;
        case kRGBA_8888_GrPixelConfig:
            return kBGRA_8888_GrPixelConfig;
        case kSBGRA_8888_GrPixelConfig:
            return kSRGBA_8888_GrPixelConfig;
        case kSRGBA_8888_GrPixelConfig:
            return kSBGRA_8888_GrPixelConfig;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return kUnknown_GrPixelConfig;
    }
    SK_ABORT("Invalid pixel config");
    return kUnknown_GrPixelConfig;
}

static inline size_t GrBytesPerPixel(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
            return 1;
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
            return 2;
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
            return 4;
        case kRGBA_half_GrPixelConfig:
            return 8;
        case kRGBA_float_GrPixelConfig:
            return 16;
        case kRG_float_GrPixelConfig:
            return 8;
        case kUnknown_GrPixelConfig:
            return 0;
    }
    SK_ABORT("Invalid pixel config");
    return 0;
}

static inline bool GrPixelConfigIsOpaque(GrPixelConfig config) {
    switch (config) {
        case kRGB_565_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRG_float_GrPixelConfig:
            return true;
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kUnknown_GrPixelConfig:
            return false;
    }
    SK_ABORT("Invalid pixel config");
    return false;
}

static inline bool GrPixelConfigIsAlphaOnly(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return false;
    }
    SK_ABORT("Invalid pixel config.");
    return false;
}

static inline bool GrPixelConfigIsFloatingPoint(GrPixelConfig config) {
    switch (config) {
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
            return false;
    }
    SK_ABORT("Invalid pixel config");
    return false;
}

static inline bool GrPixelConfigIsUnorm(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return false;
    }
    SK_ABORT("Invalid pixel config.");
    return false;
}

/**
 * Precision qualifier that should be used with a sampler.
 */
static inline GrSLPrecision GrSLSamplerPrecision(GrPixelConfig config) {
    switch (config) {
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
            return kLow_GrSLPrecision;
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
            return kHigh_GrSLPrecision;
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
            return kMedium_GrSLPrecision;
    }
    SK_ABORT("Unexpected type");
    return kHigh_GrSLPrecision;
}

static inline GrPixelConfigIsClamped GrGetPixelConfigIsClamped(GrPixelConfig config) {
    return GrPixelConfigIsFloatingPoint(config) ? GrPixelConfigIsClamped::kNo
                                                : GrPixelConfigIsClamped::kYes;
}

/**
 * Like SkColorType this describes a layout of pixel data in CPU memory. It specifies the channels,
 * their type, and width. This exists so that the GPU backend can have private types that have no
 * analog in the public facing SkColorType enum and omit types not implemented in the GPU backend.
 * It does not refer to a texture format and the mapping to texture formats may be many-to-many.
 * It does not specify the sRGB encding of the stored values.
 */
enum class GrColorType {
    kUnknown,
    kAlpha_8,
    kRGB_565,
    kABGR_4444,  // This name differs from SkColorType. kARGB_4444_SkColorType is misnamed.
    kRGBA_8888,
    kBGRA_8888,
    kRGBA_1010102,
    kGray_8,
    kAlpha_F16,
    kRGBA_F16,
    kRG_F32,
    kRGBA_F32,
};

static inline SkColorType GrColorTypeToSkColorType(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:      return kUnknown_SkColorType;
        case GrColorType::kAlpha_8:      return kAlpha_8_SkColorType;
        case GrColorType::kRGB_565:      return kRGB_565_SkColorType;
        case GrColorType::kABGR_4444:    return kARGB_4444_SkColorType;
        case GrColorType::kRGBA_8888:    return kRGBA_8888_SkColorType;
        case GrColorType::kBGRA_8888:    return kBGRA_8888_SkColorType;
        case GrColorType::kRGBA_1010102: return kRGBA_1010102_SkColorType;
        case GrColorType::kGray_8:       return kGray_8_SkColorType;
        case GrColorType::kAlpha_F16:    return kUnknown_SkColorType;
        case GrColorType::kRGBA_F16:     return kRGBA_F16_SkColorType;
        case GrColorType::kRG_F32:       return kUnknown_SkColorType;
        case GrColorType::kRGBA_F32:     return kUnknown_SkColorType;
    }
    SK_ABORT("Invalid GrColorType");
    return kUnknown_SkColorType;
}

static inline GrColorType SkColorTypeToGrColorType(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return GrColorType::kUnknown;
        case kAlpha_8_SkColorType:      return GrColorType::kAlpha_8;
        case kRGB_565_SkColorType:      return GrColorType::kRGB_565;
        case kARGB_4444_SkColorType:    return GrColorType::kABGR_4444;
        case kRGBA_8888_SkColorType:    return GrColorType::kRGBA_8888;
        case kBGRA_8888_SkColorType:    return GrColorType::kBGRA_8888;
        case kRGB_888x_SkColorType:     return GrColorType::kUnknown;
        case kGray_8_SkColorType:       return GrColorType::kGray_8;
        case kRGBA_F16_SkColorType:     return GrColorType::kRGBA_F16;
        case kRGBA_1010102_SkColorType: return GrColorType::kRGBA_1010102;
        case kRGB_101010x_SkColorType:  return GrColorType::kUnknown;
    }
    SK_ABORT("Invalid SkColorType");
    return GrColorType::kUnknown;
}

static inline int GrColorTypeBytesPerPixel(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:      return 0;
        case GrColorType::kAlpha_8:      return 1;
        case GrColorType::kRGB_565:      return 2;
        case GrColorType::kABGR_4444:    return 2;
        case GrColorType::kRGBA_8888:    return 4;
        case GrColorType::kBGRA_8888:    return 4;
        case GrColorType::kRGBA_1010102: return 4;
        case GrColorType::kGray_8:       return 1;
        case GrColorType::kAlpha_F16:    return 2;
        case GrColorType::kRGBA_F16:     return 8;
        case GrColorType::kRG_F32:       return 8;
        case GrColorType::kRGBA_F32:     return 16;
    }
    SK_ABORT("Invalid GrColorType");
    return 0;
}

static inline int GrColorTypeIsAlphaOnly(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:      return false;
        case GrColorType::kAlpha_8:      return true;
        case GrColorType::kRGB_565:      return false;
        case GrColorType::kABGR_4444:    return false;
        case GrColorType::kRGBA_8888:    return false;
        case GrColorType::kBGRA_8888:    return false;
        case GrColorType::kRGBA_1010102: return false;
        case GrColorType::kGray_8:       return false;
        case GrColorType::kAlpha_F16:    return true;
        case GrColorType::kRGBA_F16:     return false;
        case GrColorType::kRG_F32:       return false;
        case GrColorType::kRGBA_F32:     return false;
    }
    SK_ABORT("Invalid GrColorType");
    return false;
}

static inline GrColorType GrPixelConfigToColorTypeAndEncoding(GrPixelConfig config,
                                                              GrSRGBEncoded* srgbEncoded) {
    SkASSERT(srgbEncoded);
    switch (config) {
        case kUnknown_GrPixelConfig:
            return GrColorType::kUnknown;
        case kAlpha_8_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kAlpha_8;
        case kGray_8_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kGray_8;
        case kRGB_565_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kRGB_565;
        case kRGBA_4444_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kABGR_4444;
        case kRGBA_8888_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kRGBA_8888;
        case kBGRA_8888_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kBGRA_8888;
        case kSRGBA_8888_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kYes;
            return GrColorType::kRGBA_8888;
        case kSBGRA_8888_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kYes;
            return GrColorType::kBGRA_8888;
        case kRGBA_1010102_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kRGBA_1010102;
        case kRGBA_float_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kRGBA_F32;
        case kRG_float_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kRG_F32;
        case kAlpha_half_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kAlpha_F16;
        case kRGBA_half_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kRGBA_F16;
        case kAlpha_8_as_Alpha_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kAlpha_8;
        case kAlpha_8_as_Red_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kAlpha_8;
        case kAlpha_half_as_Red_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kAlpha_F16;
        case kGray_8_as_Lum_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kGray_8;
        case kGray_8_as_Red_GrPixelConfig:
            *srgbEncoded = GrSRGBEncoded::kNo;
            return GrColorType::kGray_8;
    }
    SK_ABORT("Invalid GrPixelConfig");
    return GrColorType::kUnknown;
}

static inline GrColorType GrPixelConfigToColorType(GrPixelConfig config) {
    GrSRGBEncoded bogusEncoded;
    return GrPixelConfigToColorTypeAndEncoding(config, &bogusEncoded);
}

static inline GrPixelConfig GrColorTypeToPixelConfig(GrColorType config,
                                                     GrSRGBEncoded srgbEncoded) {
    switch (config) {
        case GrColorType::kUnknown:
            return kUnknown_GrPixelConfig;
        case GrColorType::kAlpha_8:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kAlpha_8_GrPixelConfig;

        case GrColorType::kGray_8:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kGray_8_GrPixelConfig;

        case GrColorType::kRGB_565:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kRGB_565_GrPixelConfig;

        case GrColorType::kABGR_4444:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kRGBA_4444_GrPixelConfig;

        case GrColorType::kRGBA_8888:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kSRGBA_8888_GrPixelConfig
                                                        : kRGBA_8888_GrPixelConfig;

        case GrColorType::kBGRA_8888:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kSBGRA_8888_GrPixelConfig
                                                        : kBGRA_8888_GrPixelConfig;

        case GrColorType::kRGBA_1010102:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kRGBA_1010102_GrPixelConfig;

        case GrColorType::kRGBA_F32:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kRGBA_float_GrPixelConfig;

        case GrColorType::kRG_F32:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kRG_float_GrPixelConfig;

        case GrColorType::kAlpha_F16:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kAlpha_half_GrPixelConfig;

        case GrColorType::kRGBA_F16:
            return (GrSRGBEncoded::kYes == srgbEncoded) ? kUnknown_GrPixelConfig
                                                        : kRGBA_half_GrPixelConfig;
    }
    SK_ABORT("Invalid GrColorType");
    return kUnknown_GrPixelConfig;
}

class GrReleaseProcHelper : public SkRefCnt {
public:
    // These match the definitions in SkImage, from whence they came
    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);

    GrReleaseProcHelper(ReleaseProc proc, ReleaseCtx ctx) : fReleaseProc(proc), fReleaseCtx(ctx) {}
    ~GrReleaseProcHelper() override {
        fReleaseProc(fReleaseCtx);
    }

private:
    ReleaseProc fReleaseProc;
    ReleaseCtx  fReleaseCtx;
};

#endif
