/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypesPriv_DEFINED
#define GrTypesPriv_DEFINED

#include <chrono>
#include "GrTypes.h"
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
    SkFAIL("Unknown AA Type");
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
enum class GrAllowMixedSamples { kNo, kYes };

GrAAType GrChooseAAType(GrAA, GrFSAAType, GrAllowMixedSamples, const GrCaps&);

/**
 * Types of shader-language-specific boxed variables we can create. (Currently only GrGLShaderVars,
 * but should be applicable to other shader languages.)
 */
enum GrSLType {
    kVoid_GrSLType,
    kBool_GrSLType,
    kInt_GrSLType,
    kUint_GrSLType,
    kFloat_GrSLType,
    kVec2f_GrSLType,
    kVec3f_GrSLType,
    kVec4f_GrSLType,
    kVec2i_GrSLType,
    kVec3i_GrSLType,
    kVec4i_GrSLType,
    kMat22f_GrSLType,
    kMat33f_GrSLType,
    kMat44f_GrSLType,
    kTexture2DSampler_GrSLType,
    kITexture2DSampler_GrSLType,
    kTextureExternalSampler_GrSLType,
    kTexture2DRectSampler_GrSLType,
    kBufferSampler_GrSLType,
    kTexture2D_GrSLType,
    kSampler_GrSLType,
    kImageStorage2D_GrSLType,
    kIImageStorage2D_GrSLType,
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
        case kVec2f_GrSLType:
        case kVec3f_GrSLType:
        case kVec4f_GrSLType:
        case kMat22f_GrSLType:
        case kMat33f_GrSLType:
        case kMat44f_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kITexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
        case kBool_GrSLType:
        case kInt_GrSLType:
        case kUint_GrSLType:
        case kVec2i_GrSLType:
        case kVec3i_GrSLType:
        case kVec4i_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kImageStorage2D_GrSLType:
        case kIImageStorage2D_GrSLType:
            return false;
    }
    SkFAIL("Unexpected type");
    return false;
}

static inline bool GrSLTypeIs2DCombinedSamplerType(GrSLType type) {
    switch (type) {
        case kTexture2DSampler_GrSLType:
        case kITexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kFloat_GrSLType:
        case kVec2f_GrSLType:
        case kVec3f_GrSLType:
        case kVec4f_GrSLType:
        case kVec2i_GrSLType:
        case kVec3i_GrSLType:
        case kVec4i_GrSLType:
        case kMat22f_GrSLType:
        case kMat33f_GrSLType:
        case kMat44f_GrSLType:
        case kBufferSampler_GrSLType:
        case kInt_GrSLType:
        case kUint_GrSLType:
        case kBool_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kImageStorage2D_GrSLType:
        case kIImageStorage2D_GrSLType:
            return false;
    }
    SkFAIL("Unexpected type");
    return false;
}

static inline bool GrSLTypeIsCombinedSamplerType(GrSLType type) {
    switch (type) {
        case kTexture2DSampler_GrSLType:
        case kITexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kFloat_GrSLType:
        case kVec2f_GrSLType:
        case kVec3f_GrSLType:
        case kVec4f_GrSLType:
        case kVec2i_GrSLType:
        case kVec3i_GrSLType:
        case kVec4i_GrSLType:
        case kMat22f_GrSLType:
        case kMat33f_GrSLType:
        case kMat44f_GrSLType:
        case kInt_GrSLType:
        case kUint_GrSLType:
        case kBool_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kImageStorage2D_GrSLType:
        case kIImageStorage2D_GrSLType:
            return false;
    }
    SkFAIL("Unexpected type");
    return false;
}

static inline bool GrSLTypeIsImageStorage(GrSLType type) {
    switch (type) {
        case kImageStorage2D_GrSLType:
        case kIImageStorage2D_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kFloat_GrSLType:
        case kVec2f_GrSLType:
        case kVec3f_GrSLType:
        case kVec4f_GrSLType:
        case kVec2i_GrSLType:
        case kVec3i_GrSLType:
        case kVec4i_GrSLType:
        case kMat22f_GrSLType:
        case kMat33f_GrSLType:
        case kMat44f_GrSLType:
        case kInt_GrSLType:
        case kUint_GrSLType:
        case kBool_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kITexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
            return false;
    }
    SkFAIL("Unexpected type");
    return false;
}

static inline bool GrSLTypeAcceptsPrecision(GrSLType type) {
    switch (type) {
        case kInt_GrSLType:
        case kUint_GrSLType:
        case kFloat_GrSLType:
        case kVec2f_GrSLType:
        case kVec3f_GrSLType:
        case kVec4f_GrSLType:
        case kVec2i_GrSLType:
        case kVec3i_GrSLType:
        case kVec4i_GrSLType:
        case kMat22f_GrSLType:
        case kMat33f_GrSLType:
        case kMat44f_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kITexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kBufferSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kImageStorage2D_GrSLType:
        case kIImageStorage2D_GrSLType:
            return true;

        case kVoid_GrSLType:
        case kBool_GrSLType:
            return false;
    }
    SkFAIL("Unexpected type");
    return false;
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

    kVec2i_GrVertexAttribType,   // vector of 2 32-bit ints
    kVec3i_GrVertexAttribType,   // vector of 3 32-bit ints
    kVec4i_GrVertexAttribType,   // vector of 4 32-bit ints

    kUByte_GrVertexAttribType,   // unsigned byte, e.g. coverage
    kVec4ub_GrVertexAttribType,  // vector of 4 unsigned bytes, e.g. colors

    kVec2us_GrVertexAttribType,  // vector of 2 shorts, e.g. texture coordinates

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
        case kVec2f_GrVertexAttribType:
            return 2 * sizeof(float);
        case kVec3f_GrVertexAttribType:
            return 3 * sizeof(float);
        case kVec4f_GrVertexAttribType:
            return 4 * sizeof(float);
        case kVec2i_GrVertexAttribType:
            return 2 * sizeof(int32_t);
        case kVec3i_GrVertexAttribType:
            return 3 * sizeof(int32_t);
        case kVec4i_GrVertexAttribType:
            return 4 * sizeof(int32_t);
        case kUByte_GrVertexAttribType:
            return 1 * sizeof(char);
        case kVec4ub_GrVertexAttribType:
            return 4 * sizeof(char);
        case kVec2us_GrVertexAttribType:
            return 2 * sizeof(int16_t);
        case kInt_GrVertexAttribType:
            return sizeof(int32_t);
        case kUint_GrVertexAttribType:
            return sizeof(uint32_t);
    }
    SkFAIL("Unexpected attribute type");
    return 0;
}

/**
 * Is the attrib type integral?
 */
static inline bool GrVertexAttribTypeIsIntType(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return false;
        case kVec2f_GrVertexAttribType:
            return false;
        case kVec3f_GrVertexAttribType:
            return false;
        case kVec4f_GrVertexAttribType:
            return false;
        case kVec2i_GrVertexAttribType:
            return true;
        case kVec3i_GrVertexAttribType:
            return true;
        case kVec4i_GrVertexAttribType:
            return true;
        case kUByte_GrVertexAttribType:
            return false;
        case kVec4ub_GrVertexAttribType:
            return false;
        case kVec2us_GrVertexAttribType:
            return false;
        case kInt_GrVertexAttribType:
            return true;
        case kUint_GrVertexAttribType:
            return true;
    }
    SkFAIL("Unexpected attribute type");
    return false;
}

/**
 * converts a GrVertexAttribType to a GrSLType
 */
static inline GrSLType GrVertexAttribTypeToSLType(GrVertexAttribType type) {
    switch (type) {
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
        case kVec2i_GrVertexAttribType:
            return kVec2i_GrSLType;
        case kVec3i_GrVertexAttribType:
            return kVec3i_GrSLType;
        case kVec4i_GrVertexAttribType:
            return kVec4i_GrSLType;
        case kInt_GrVertexAttribType:
            return kInt_GrSLType;
        case kUint_GrVertexAttribType:
            return kUint_GrSLType;
    }
    SkFAIL("Unsupported type conversion");
    return kVoid_GrSLType;
}

//////////////////////////////////////////////////////////////////////////////

enum class GrImageStorageFormat {
    kRGBA8,
    kRGBA8i,
    kRGBA16f,
    kRGBA32f,
};

/**
 * Describes types of caching and compiler optimizations allowed for certain variable types
 * (currently only image storages).
 **/
enum class GrSLMemoryModel {
    /** No special restrctions on memory accesses or compiler optimizations */
    kNone,
    /** Cache coherent across shader invocations */
    kCoherent,
    /**
     * Disallows compiler from eliding loads or stores that appear redundant in a single
     * invocation. Implies coherent.
     */
    kVolatile
};

/**
 * If kYes then the memory backing the varialble is only accessed via the variable. This is
 * currently only used with image storages.
 */
enum class GrSLRestrict {
    kYes,
    kNo,
};

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
    return (kFillBW_GrProcessorEdgeType != edgeType &&
            kInverseFillBW_GrProcessorEdgeType != edgeType);
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
    return kFillAA_GrProcessorEdgeType;  // suppress warning.
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

template <typename T> T * const * sk_sp_address_as_pointer_address(sk_sp<T> const * sp) {
    static_assert(sizeof(T*) == sizeof(sk_sp<T>), "sk_sp not expected size.");
    return reinterpret_cast<T * const *>(sp);
}

/*
 * Object for CPU-GPU synchronization
 */
typedef uint64_t GrFence;

#endif
