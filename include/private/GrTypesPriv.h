/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypesPriv_DEFINED
#define GrTypesPriv_DEFINED

#include <chrono>
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrSharedEnums.h"
#include "include/private/SkImageInfoPriv.h"
#include "include/private/SkWeakRefCnt.h"

class GrBackendFormat;
class GrCaps;

// The old libstdc++ uses the draft name "monotonic_clock" rather than "steady_clock". This might
// not actually be monotonic, depending on how libstdc++ was built. However, this is only currently
// used for idle resource purging so it shouldn't cause a correctness problem.
#if defined(__GLIBCXX__) && (__GLIBCXX__ < 20130000)
using GrStdSteadyClock = std::chrono::monotonic_clock;
#else
using GrStdSteadyClock = std::chrono::steady_clock;
#endif

/**
 * Pixel configurations. This type conflates texture formats, CPU pixel formats, and
 * premultipliedness. We are moving away from it towards SkColorType and backend API (GL, Vulkan)
 * texture formats in the public API. Right now this mostly refers to texture formats as we're
 * migrating.
 */
enum GrPixelConfig {
    kUnknown_GrPixelConfig,
    kAlpha_8_GrPixelConfig,
    kAlpha_8_as_Alpha_GrPixelConfig,
    kAlpha_8_as_Red_GrPixelConfig,
    kGray_8_GrPixelConfig,
    kGray_8_as_Lum_GrPixelConfig,
    kGray_8_as_Red_GrPixelConfig,
    kRGB_565_GrPixelConfig,
    kRGBA_4444_GrPixelConfig,
    kRGBA_8888_GrPixelConfig,
    kRGB_888_GrPixelConfig,
    kRGB_888X_GrPixelConfig,
    kRG_88_GrPixelConfig,
    kBGRA_8888_GrPixelConfig,
    kSRGBA_8888_GrPixelConfig,
    kRGBA_1010102_GrPixelConfig,
    kRGBA_float_GrPixelConfig,
    kAlpha_half_GrPixelConfig,
    kAlpha_half_as_Lum_GrPixelConfig,
    kAlpha_half_as_Red_GrPixelConfig,
    kRGBA_half_GrPixelConfig,
    kRGBA_half_Clamped_GrPixelConfig,
    kRGB_ETC1_GrPixelConfig,
    kR_16_GrPixelConfig,
    kRG_1616_GrPixelConfig,

    // Experimental (for Y416 and mutant P016/P010)
    kRGBA_16161616_GrPixelConfig,
    kRG_half_GrPixelConfig,

    kLast_GrPixelConfig = kRG_half_GrPixelConfig
};
static const int kGrPixelConfigCnt = kLast_GrPixelConfig + 1;

// Aliases for pixel configs that match skia's byte order.
#ifndef SK_CPU_LENDIAN
#error "Skia gpu currently assumes little endian"
#endif
#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
static const GrPixelConfig kSkia8888_GrPixelConfig = kBGRA_8888_GrPixelConfig;
#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
static const GrPixelConfig kSkia8888_GrPixelConfig = kRGBA_8888_GrPixelConfig;
#else
    #error "SK_*32_SHIFT values must correspond to GL_BGRA or GL_RGBA format."
#endif

/**
 * Geometric primitives used for drawing.
 */
enum class GrPrimitiveType {
    kTriangles,
    kTriangleStrip,
    kPoints,
    kLines,          // 1 pix wide only
    kLineStrip,      // 1 pix wide only
    kLinesAdjacency  // requires geometry shader support.
};
static constexpr int kNumGrPrimitiveTypes = (int)GrPrimitiveType::kLinesAdjacency + 1;

static constexpr bool GrIsPrimTypeLines(GrPrimitiveType type) {
    return GrPrimitiveType::kLines == type ||
           GrPrimitiveType::kLineStrip == type ||
           GrPrimitiveType::kLinesAdjacency == type;
}

static constexpr bool GrIsPrimTypeTris(GrPrimitiveType type) {
    return GrPrimitiveType::kTriangles == type || GrPrimitiveType::kTriangleStrip == type;
}

static constexpr bool GrPrimTypeRequiresGeometryShaderSupport(GrPrimitiveType type) {
    return GrPrimitiveType::kLinesAdjacency == type;
}

enum class GrPrimitiveRestart : bool {
    kNo = false,
    kYes = true
};

/**
 *  Formats for masks, used by the font cache. Important that these are 0-based.
 */
enum GrMaskFormat {
    kA8_GrMaskFormat,    //!< 1-byte per pixel
    kA565_GrMaskFormat,  //!< 2-bytes per pixel, RGB represent 3-channel LCD coverage
    kARGB_GrMaskFormat,  //!< 4-bytes per pixel, color format

    kLast_GrMaskFormat = kARGB_GrMaskFormat
};
static const int kMaskFormatCount = kLast_GrMaskFormat + 1;

/**
 *  Return the number of bytes-per-pixel for the specified mask format.
 */
static inline int GrMaskFormatBytesPerPixel(GrMaskFormat format) {
    SkASSERT(format < kMaskFormatCount);
    // kA8   (0) -> 1
    // kA565 (1) -> 2
    // kARGB (2) -> 4
    static const int sBytesPerPixel[] = {1, 2, 4};
    static_assert(SK_ARRAY_COUNT(sBytesPerPixel) == kMaskFormatCount, "array_size_mismatch");
    static_assert(kA8_GrMaskFormat == 0, "enum_order_dependency");
    static_assert(kA565_GrMaskFormat == 1, "enum_order_dependency");
    static_assert(kARGB_GrMaskFormat == 2, "enum_order_dependency");

    return sBytesPerPixel[(int)format];
}

/**
 * Describes a surface to be created.
 */
struct GrSurfaceDesc {
    GrSurfaceDesc() : fWidth(0), fHeight(0), fConfig(kUnknown_GrPixelConfig) {}

    int                    fWidth;  //!< Width of the texture
    int                    fHeight; //!< Height of the texture

    /**
     * Format of source data of the texture. Not guaranteed to be the same as
     * internal format used by 3D API.
     */
    GrPixelConfig          fConfig;
};

/** Ownership rules for external GPU resources imported into Skia. */
enum GrWrapOwnership {
    /** Skia will assume the client will keep the resource alive and Skia will not free it. */
    kBorrow_GrWrapOwnership,

    /** Skia will assume ownership of the resource and free it. */
    kAdopt_GrWrapOwnership,
};

enum class GrWrapCacheable : bool {
    /**
     * The wrapped resource will be removed from the cache as soon as it becomes purgeable. It may
     * still be assigned and found by a unique key, but the presence of the key will not be used to
     * keep the resource alive when it has no references.
     */
    kNo = false,
    /**
     * The wrapped resource is allowed to remain in the GrResourceCache when it has no references
     * but has a unique key. Such resources should only be given unique keys when it is known that
     * the key will eventually be removed from the resource or invalidated via the message bus.
     */
    kYes = true
};

enum class GrBudgetedType : uint8_t {
    /** The resource is budgeted and is subject to purging under budget pressure. */
    kBudgeted,
    /**
     * The resource is unbudgeted and is purged as soon as it has no refs regardless of whether
     * it has a unique or scratch key.
     */
    kUnbudgetedUncacheable,
    /**
     * The resource is unbudgeted and is allowed to remain in the cache with no refs if it
     * has a unique key. Scratch keys are ignored.
     */
    kUnbudgetedCacheable,
};

/**
 * Clips are composed from these objects.
 */
enum GrClipType {
    kRect_ClipType,
    kPath_ClipType
};

enum class GrScissorTest : bool {
    kDisabled = false,
    kEnabled = true
};

struct GrMipLevel {
    const void* fPixels = nullptr;
    size_t fRowBytes = 0;
};

/**
 * This enum is used to specify the load operation to be used when an GrOpsTask/GrOpsRenderPass
 * begins execution.
 */
enum class GrLoadOp {
    kLoad,
    kClear,
    kDiscard,
};

/**
 * This enum is used to specify the store operation to be used when an GrOpsTask/GrOpsRenderPass
 * ends execution.
 */
enum class GrStoreOp {
    kStore,
    kDiscard,
};

/**
 * Used to control antialiasing in draw calls.
 */
enum class GrAA : bool {
    kNo = false,
    kYes = true
};

enum class GrFillRule : bool {
    kNonzero,
    kEvenOdd
};

inline GrFillRule GrFillRuleForSkPath(const SkPath& path) {
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
        case SkPath::kInverseWinding_FillType:
            return GrFillRule::kNonzero;
        case SkPath::kEvenOdd_FillType:
        case SkPath::kInverseEvenOdd_FillType:
            return GrFillRule::kEvenOdd;
    }
    SkUNREACHABLE;
}

/** This enum indicates the type of antialiasing to be performed. */
enum class GrAAType : unsigned {
    /** No antialiasing */
    kNone,
    /** Use fragment shader code or mixed samples to blend with a fractional pixel coverage. */
    kCoverage,
    /** Use normal MSAA. */
    kMSAA
};

static constexpr bool GrAATypeIsHW(GrAAType type) {
    switch (type) {
        case GrAAType::kNone:
            return false;
        case GrAAType::kCoverage:
            return false;
        case GrAAType::kMSAA:
            return true;
    }
    SkUNREACHABLE;
}

/**
 * Some pixel configs are inherently clamped to [0,1], some are allowed to go outside that range,
 * and some are FP but manually clamped in the XP.
 */
enum class GrClampType {
    kAuto,    // Normalized, fixed-point configs
    kManual,  // Clamped FP configs
    kNone,    // Normal (unclamped) FP configs
};

/**
 * A number of rectangle/quadrilateral drawing APIs can control anti-aliasing on a per edge basis.
 * These masks specify which edges are AA'ed. The intent for this is to support tiling with seamless
 * boundaries, where the inner edges are non-AA and the outer edges are AA. Regular draws (where AA
 * is specified by GrAA) is almost equivalent to kNone or kAll, with the exception of how MSAA is
 * handled.
 *
 * When tiling and there is MSAA, mixed edge rectangles are processed with MSAA, so in order for the
 * tiled edges to remain seamless, inner tiles with kNone must also be processed with MSAA. In
 * regular drawing, however, kNone should disable MSAA (if it's supported) to match the expected
 * appearance.
 *
 * Therefore, APIs that use per-edge AA flags also take a GrAA value so that they can differentiate
 * between the regular and tiling use case behaviors. Tiling operations should always pass
 * GrAA::kYes while regular options should pass GrAA based on the SkPaint's anti-alias state.
 */
enum class GrQuadAAFlags {
    kLeft   = SkCanvas::kLeft_QuadAAFlag,
    kTop    = SkCanvas::kTop_QuadAAFlag,
    kRight  = SkCanvas::kRight_QuadAAFlag,
    kBottom = SkCanvas::kBottom_QuadAAFlag,

    kNone = SkCanvas::kNone_QuadAAFlags,
    kAll  = SkCanvas::kAll_QuadAAFlags
};

GR_MAKE_BITFIELD_CLASS_OPS(GrQuadAAFlags)

static inline GrQuadAAFlags SkToGrQuadAAFlags(unsigned flags) {
    return static_cast<GrQuadAAFlags>(flags);
}

/**
 * Types of shader-language-specific boxed variables we can create.
 */
enum GrSLType {
    kVoid_GrSLType,
    kBool_GrSLType,
    kByte_GrSLType,
    kByte2_GrSLType,
    kByte3_GrSLType,
    kByte4_GrSLType,
    kUByte_GrSLType,
    kUByte2_GrSLType,
    kUByte3_GrSLType,
    kUByte4_GrSLType,
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
    kTexture2D_GrSLType,
    kSampler_GrSLType,

    kLast_GrSLType = kSampler_GrSLType
};
static const int kGrSLTypeCount = kLast_GrSLType + 1;

/**
 * The type of texture. Backends other than GL currently only use the 2D value but the type must
 * still be known at the API-neutral layer as it used to determine whether MIP maps, renderability,
 * and sampling parameters are legal for proxies that will be instantiated with wrapped textures.
 */
enum class GrTextureType {
    kNone,
    k2D,
    /* Rectangle uses unnormalized texture coordinates. */
    kRectangle,
    kExternal
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
GR_MAKE_BITFIELD_OPS(GrShaderFlags)

/** Is the shading language type float (including vectors/matrices)? */
static constexpr bool GrSLTypeIsFloatType(GrSLType type) {
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
        case kBool_GrSLType:
        case kByte_GrSLType:
        case kByte2_GrSLType:
        case kByte3_GrSLType:
        case kByte4_GrSLType:
        case kUByte_GrSLType:
        case kUByte2_GrSLType:
        case kUByte3_GrSLType:
        case kUByte4_GrSLType:
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
    SkUNREACHABLE;
}

/** If the type represents a single value or vector return the vector length, else -1. */
static constexpr int GrSLTypeVecLength(GrSLType type) {
    switch (type) {
        case kFloat_GrSLType:
        case kHalf_GrSLType:
        case kBool_GrSLType:
        case kByte_GrSLType:
        case kUByte_GrSLType:
        case kShort_GrSLType:
        case kUShort_GrSLType:
        case kInt_GrSLType:
        case kUint_GrSLType:
            return 1;

        case kFloat2_GrSLType:
        case kHalf2_GrSLType:
        case kByte2_GrSLType:
        case kUByte2_GrSLType:
        case kShort2_GrSLType:
        case kUShort2_GrSLType:
        case kInt2_GrSLType:
        case kUint2_GrSLType:
            return 2;

        case kFloat3_GrSLType:
        case kHalf3_GrSLType:
        case kByte3_GrSLType:
        case kUByte3_GrSLType:
        case kShort3_GrSLType:
        case kUShort3_GrSLType:
        case kInt3_GrSLType:
            return 3;

        case kFloat4_GrSLType:
        case kHalf4_GrSLType:
        case kByte4_GrSLType:
        case kUByte4_GrSLType:
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
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
            return -1;
    }
    SkUNREACHABLE;
}

static inline GrSLType GrSLCombinedSamplerTypeForTextureType(GrTextureType type) {
    switch (type) {
        case GrTextureType::k2D:
            return kTexture2DSampler_GrSLType;
        case GrTextureType::kRectangle:
            return kTexture2DRectSampler_GrSLType;
        case GrTextureType::kExternal:
            return kTextureExternalSampler_GrSLType;
        default:
            SK_ABORT("Unexpected texture type");
    }
}

/** Rectangle and external textures only support the clamp wrap mode and do not support
 *  MIP maps.
 */
static inline bool GrTextureTypeHasRestrictedSampling(GrTextureType type) {
    switch (type) {
        case GrTextureType::k2D:
            return false;
        case GrTextureType::kRectangle:
            return true;
        case GrTextureType::kExternal:
            return true;
        default:
            SK_ABORT("Unexpected texture type");
    }
}

static constexpr bool GrSLTypeIsCombinedSamplerType(GrSLType type) {
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
        case kBool_GrSLType:
        case kByte_GrSLType:
        case kByte2_GrSLType:
        case kByte3_GrSLType:
        case kByte4_GrSLType:
        case kUByte_GrSLType:
        case kUByte2_GrSLType:
        case kUByte3_GrSLType:
        case kUByte4_GrSLType:
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
    SkUNREACHABLE;
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


    kByte_GrVertexAttribType,  // signed byte
    kByte2_GrVertexAttribType, // vector of 2 8-bit signed bytes
    kByte3_GrVertexAttribType, // vector of 3 8-bit signed bytes
    kByte4_GrVertexAttribType, // vector of 4 8-bit signed bytes
    kUByte_GrVertexAttribType,  // unsigned byte
    kUByte2_GrVertexAttribType, // vector of 2 8-bit unsigned bytes
    kUByte3_GrVertexAttribType, // vector of 3 8-bit unsigned bytes
    kUByte4_GrVertexAttribType, // vector of 4 8-bit unsigned bytes

    kUByte_norm_GrVertexAttribType,  // unsigned byte, e.g. coverage, 0 -> 0.0f, 255 -> 1.0f.
    kUByte4_norm_GrVertexAttribType, // vector of 4 unsigned bytes, e.g. colors, 0 -> 0.0f,
                                     // 255 -> 1.0f.

    kShort2_GrVertexAttribType,       // vector of 2 16-bit shorts.
    kShort4_GrVertexAttribType,       // vector of 4 16-bit shorts.

    kUShort2_GrVertexAttribType,      // vector of 2 unsigned shorts. 0 -> 0, 65535 -> 65535.
    kUShort2_norm_GrVertexAttribType, // vector of 2 unsigned shorts. 0 -> 0.0f, 65535 -> 1.0f.

    kInt_GrVertexAttribType,
    kUint_GrVertexAttribType,

    kUShort_norm_GrVertexAttribType,

    // Experimental (for Y416)
    kUShort4_norm_GrVertexAttribType, // vector of 4 unsigned shorts. 0 -> 0.0f, 65535 -> 1.0f.

    kLast_GrVertexAttribType = kUShort4_norm_GrVertexAttribType
};
static const int kGrVertexAttribTypeCount = kLast_GrVertexAttribType + 1;

//////////////////////////////////////////////////////////////////////////////

static const int kGrClipEdgeTypeCnt = (int) GrClipEdgeType::kLast + 1;

static constexpr bool GrProcessorEdgeTypeIsFill(const GrClipEdgeType edgeType) {
    return (GrClipEdgeType::kFillAA == edgeType || GrClipEdgeType::kFillBW == edgeType);
}

static constexpr bool GrProcessorEdgeTypeIsInverseFill(const GrClipEdgeType edgeType) {
    return (GrClipEdgeType::kInverseFillAA == edgeType ||
            GrClipEdgeType::kInverseFillBW == edgeType);
}

static constexpr bool GrProcessorEdgeTypeIsAA(const GrClipEdgeType edgeType) {
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
enum class GrGpuBufferType {
    kVertex,
    kIndex,
    kXferCpuToGpu,
    kXferGpuToCpu,
};
static const int kGrGpuBufferTypeCount = static_cast<int>(GrGpuBufferType::kXferGpuToCpu) + 1;

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

// Flags shared between the GrSurface & GrSurfaceProxy class hierarchies
enum class GrInternalSurfaceFlags {
    kNone                           = 0,

    // Texture-level

    // Means the pixels in the texture are read-only. Cannot also be a GrRenderTarget[Proxy].
    kReadOnly                       = 1 << 0,

    // RT-level

    // This flag is for use with GL only. It tells us that the internal render target wraps FBO 0.
    kGLRTFBOIDIs0                   = 1 << 1,

    // This means the render target is multisampled, and internally holds a non-msaa texture for
    // resolving into. The render target resolves itself by blitting into this internal texture.
    // (asTexture() might or might not return the internal texture, but if it does, we always
    // resolve the render target before accessing this texture's data.)
    kRequiresManualMSAAResolve      = 1 << 2,
};

GR_MAKE_BITFIELD_CLASS_OPS(GrInternalSurfaceFlags)

// 'GR_MAKE_BITFIELD_CLASS_OPS' defines the & operator on GrInternalSurfaceFlags to return bool.
// We want to find the bitwise & with these masks, so we declare them as ints.
constexpr static int kGrInternalTextureFlagsMask = static_cast<int>(
        GrInternalSurfaceFlags::kReadOnly);

constexpr static int kGrInternalRenderTargetFlagsMask = static_cast<int>(
        GrInternalSurfaceFlags::kGLRTFBOIDIs0 | GrInternalSurfaceFlags::kRequiresManualMSAAResolve);

constexpr static int kGrInternalTextureRenderTargetFlagsMask =
        kGrInternalTextureFlagsMask | kGrInternalRenderTargetFlagsMask;

#ifdef SK_DEBUG
// Takes a pointer to a GrCaps, and will suppress prints if required
#define GrCapsDebugf(caps, ...)  if (!(caps)->suppressPrints()) SkDebugf(__VA_ARGS__)
#else
#define GrCapsDebugf(caps, ...) do {} while (0)
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
    kNone              = 0, // Always use software masks and/or GrDefaultPathRenderer.
    kDashLine          = 1 << 0,
    kStencilAndCover   = 1 << 1,
    kCoverageCounting  = 1 << 2,
    kAAHairline        = 1 << 3,
    kAAConvex          = 1 << 4,
    kAALinearizing     = 1 << 5,
    kSmall             = 1 << 6,
    kTessellating      = 1 << 7,

    kAll               = (kTessellating | (kTessellating - 1)),
    kDefault           = kAll & ~kCoverageCounting

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

static constexpr GrPixelConfig GrCompressionTypePixelConfig(SkImage::CompressionType compression) {
    switch (compression) {
        case SkImage::kETC1_CompressionType: return kRGB_ETC1_GrPixelConfig;
    }
    SkUNREACHABLE;
}

static constexpr size_t GrBytesPerPixel(GrPixelConfig config) {
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
        case kRG_88_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Lum_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kR_16_GrPixelConfig:
            return 2;
        case kRGBA_8888_GrPixelConfig:
        case kRGB_888_GrPixelConfig:  // Assuming GPUs store this 4-byte aligned.
        case kRGB_888X_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRG_1616_GrPixelConfig:
            return 4;
        case kRGBA_half_GrPixelConfig:
        case kRGBA_half_Clamped_GrPixelConfig:
            return 8;
        case kRGBA_float_GrPixelConfig:
            return 16;
        case kUnknown_GrPixelConfig:
        case kRGB_ETC1_GrPixelConfig:
            return 0;

        // Experimental (for Y416 and mutant P016/P010)
        case kRGBA_16161616_GrPixelConfig:
            return 8;
        case kRG_half_GrPixelConfig:
            return 4;
    }
    SkUNREACHABLE;
}

static constexpr bool GrPixelConfigIsOpaque(GrPixelConfig config) {
    switch (config) {
        case kRGB_565_GrPixelConfig:
        case kRGB_888_GrPixelConfig:
        case kRGB_888X_GrPixelConfig:
        case kRG_88_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_ETC1_GrPixelConfig:
        case kR_16_GrPixelConfig:
        case kRG_1616_GrPixelConfig:
        case kRG_half_GrPixelConfig: // Experimental (for mutant P016/P010)
            return true;
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Lum_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
        case kRGBA_half_Clamped_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRGBA_16161616_GrPixelConfig: // Experimental (for Y416)
        case kUnknown_GrPixelConfig:
            return false;
    }
    SkUNREACHABLE;
}

static constexpr bool GrPixelConfigIsAlphaOnly(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Lum_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kRGB_888_GrPixelConfig:
        case kRGB_888X_GrPixelConfig:
        case kRG_88_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
        case kRGBA_half_Clamped_GrPixelConfig:
        case kRGB_ETC1_GrPixelConfig:
        case kR_16_GrPixelConfig:
        case kRG_1616_GrPixelConfig:
        // Experimental (for Y416 and mutant P016/P010)
        case kRGBA_16161616_GrPixelConfig:
        case kRG_half_GrPixelConfig:
            return false;
    }
    SkUNREACHABLE;
}

static constexpr bool GrPixelConfigIsFloatingPoint(GrPixelConfig config) {
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
        case kRGB_888_GrPixelConfig:
        case kRGB_888X_GrPixelConfig:
        case kRG_88_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGB_ETC1_GrPixelConfig:
        case kR_16_GrPixelConfig:
        case kRG_1616_GrPixelConfig:
        case kRGBA_16161616_GrPixelConfig: // Experimental (for Y416)
            return false;
        case kRGBA_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Lum_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
        case kRGBA_half_Clamped_GrPixelConfig:
        case kRG_half_GrPixelConfig: // Experimental (for mutant P016/P010)
            return true;
    }
    SkUNREACHABLE;
}

static constexpr GrClampType GrPixelConfigClampType(GrPixelConfig config) {
    if (!GrPixelConfigIsFloatingPoint(config)) {
        return GrClampType::kAuto;
    }
    return kRGBA_half_Clamped_GrPixelConfig == config ? GrClampType::kManual : GrClampType::kNone;
}

/**
 * Returns true if the pixel config is a GPU-specific compressed format
 * representation.
 */
static constexpr bool GrPixelConfigIsCompressed(GrPixelConfig config) {
    switch (config) {
        case kRGB_ETC1_GrPixelConfig:
            return true;
        default:
            return false;
    }
    SkUNREACHABLE;
}

/**
 * Returns the data size for the given compressed pixel config
 */
static inline size_t GrCompressedFormatDataSize(GrPixelConfig config,
                                                int width, int height) {
    SkASSERT(GrPixelConfigIsCompressed(config));

    switch (config) {
        case kRGB_ETC1_GrPixelConfig:
            SkASSERT((width & 3) == 0);
            SkASSERT((height & 3) == 0);
            return (width >> 2) * (height >> 2) * 8;

        default:
            SK_ABORT("Unknown compressed pixel config");
    }

    SK_ABORT("Invalid pixel config");
}

/**
 * Like SkColorType this describes a layout of pixel data in CPU memory. It specifies the channels,
 * their type, and width. This exists so that the GPU backend can have private types that have no
 * analog in the public facing SkColorType enum and omit types not implemented in the GPU backend.
 * It does not refer to a texture format and the mapping to texture formats may be many-to-many.
 * It does not specify the sRGB encoding of the stored values. The components are listed in order of
 * where they appear in memory. In other words the first component listed is in the low bits and
 * the last component in the high bits.
 */
enum class GrColorType {
    kUnknown,
    kAlpha_8,
    kBGR_565,
    kABGR_4444,  // This name differs from SkColorType. kARGB_4444_SkColorType is misnamed.
    kRGBA_8888,
    kRGBA_8888_SRGB,
    kRGB_888x,
    kRG_88,
    kBGRA_8888,
    kRGBA_1010102,
    kGray_8,
    kAlpha_F16,
    kRGBA_F16,
    kRGBA_F16_Clamped,
    kRGBA_F32,

    // Unusual formats that come up after reading back in cases where we are reassigning the meaning
    // of a texture format's channels to use for a particular color format but have to read back the
    // data to a full RGBA quadruple. (e.g. using a R8 texture format as A8 color type but the API
    // only supports reading to RGBA8.) None of these have SkColorType equivalents.
    kAlpha_8xxx,
    kAlpha_F32xxx,
    kGray_8xxx,

    kR_16,          // Not in SkColorType
    kRG_1616,       // Not in SkColorType

    // Experimental (for Y416 and mutant P016/P010)
    kRGBA_16161616, // Not in SkColorType
    kRG_F16,        // Not in SkColorType

    kLast = kRG_F16
};

static const int kGrColorTypeCnt = static_cast<int>(GrColorType::kLast) + 1;

static constexpr SkColorType GrColorTypeToSkColorType(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:          return kUnknown_SkColorType;
        case GrColorType::kAlpha_8:          return kAlpha_8_SkColorType;
        case GrColorType::kBGR_565:          return kRGB_565_SkColorType;
        case GrColorType::kABGR_4444:        return kARGB_4444_SkColorType;
        case GrColorType::kRGBA_8888:        return kRGBA_8888_SkColorType;
        // Once we a kRGBA_8888_SRGB_SkColorType we should return that here.
        case GrColorType::kRGBA_8888_SRGB:   return kRGBA_8888_SkColorType;
        case GrColorType::kRGB_888x:         return kRGB_888x_SkColorType;
        case GrColorType::kRG_88:            return kUnknown_SkColorType;
        case GrColorType::kBGRA_8888:        return kBGRA_8888_SkColorType;
        case GrColorType::kRGBA_1010102:     return kRGBA_1010102_SkColorType;
        case GrColorType::kGray_8:           return kGray_8_SkColorType;
        case GrColorType::kAlpha_F16:        return kUnknown_SkColorType;
        case GrColorType::kRGBA_F16:         return kRGBA_F16_SkColorType;
        case GrColorType::kRGBA_F16_Clamped: return kRGBA_F16Norm_SkColorType;
        case GrColorType::kRGBA_F32:         return kRGBA_F32_SkColorType;
        case GrColorType::kAlpha_8xxx:       return kUnknown_SkColorType;
        case GrColorType::kAlpha_F32xxx:     return kUnknown_SkColorType;
        case GrColorType::kGray_8xxx:        return kUnknown_SkColorType;
        case GrColorType::kR_16:             return kUnknown_SkColorType;
        case GrColorType::kRG_1616:          return kUnknown_SkColorType;
        // Experimental (for Y416 and mutant P016/P010)
        case GrColorType::kRGBA_16161616:    return kUnknown_SkColorType;
        case GrColorType::kRG_F16:           return kUnknown_SkColorType;
    }
    SkUNREACHABLE;
}

static constexpr GrColorType SkColorTypeToGrColorType(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return GrColorType::kUnknown;
        case kAlpha_8_SkColorType:      return GrColorType::kAlpha_8;
        case kRGB_565_SkColorType:      return GrColorType::kBGR_565;
        case kARGB_4444_SkColorType:    return GrColorType::kABGR_4444;
        case kRGBA_8888_SkColorType:    return GrColorType::kRGBA_8888;
        case kRGB_888x_SkColorType:     return GrColorType::kRGB_888x;
        case kBGRA_8888_SkColorType:    return GrColorType::kBGRA_8888;
        case kGray_8_SkColorType:       return GrColorType::kGray_8;
        case kRGBA_F16Norm_SkColorType: return GrColorType::kRGBA_F16_Clamped;
        case kRGBA_F16_SkColorType:     return GrColorType::kRGBA_F16;
        case kRGBA_1010102_SkColorType: return GrColorType::kRGBA_1010102;
        case kRGB_101010x_SkColorType:  return GrColorType::kUnknown;
        case kRGBA_F32_SkColorType:     return GrColorType::kRGBA_F32;
    }
    SkUNREACHABLE;
}

// This is a temporary means of mapping an SkColorType and format to a
// GrColorType::kRGBA_8888_SRGB. Once we have an SRGB SkColorType this can go away.
GrColorType SkColorTypeAndFormatToGrColorType(const GrCaps* caps,
                                              SkColorType skCT,
                                              const GrBackendFormat& format);

static constexpr uint32_t GrColorTypeComponentFlags(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:          return 0;
        case GrColorType::kAlpha_8:          return kAlpha_SkColorTypeComponentFlag;
        case GrColorType::kBGR_565:          return kRGB_SkColorTypeComponentFlags;
        case GrColorType::kABGR_4444:        return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kRGBA_8888:        return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kRGBA_8888_SRGB:   return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kRGB_888x:         return kRGB_SkColorTypeComponentFlags;
        case GrColorType::kRG_88:            return kRed_SkColorTypeComponentFlag |
                                                    kGreen_SkColorTypeComponentFlag;
        case GrColorType::kBGRA_8888:        return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kRGBA_1010102:     return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kGray_8:           return kGray_SkColorTypeComponentFlag;
        case GrColorType::kAlpha_F16:        return kAlpha_SkColorTypeComponentFlag;
        case GrColorType::kRGBA_F16:         return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kRGBA_F16_Clamped: return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kRGBA_F32:         return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kAlpha_8xxx:       return kAlpha_SkColorTypeComponentFlag;
        case GrColorType::kAlpha_F32xxx:     return kAlpha_SkColorTypeComponentFlag;
        case GrColorType::kGray_8xxx:        return kGray_SkColorTypeComponentFlag;
        case GrColorType::kR_16:             return kRed_SkColorTypeComponentFlag;
        case GrColorType::kRG_1616:          return kRed_SkColorTypeComponentFlag |
                                                    kGreen_SkColorTypeComponentFlag;
        // Experimental (for Y416 and mutant P016/P010)
        case GrColorType::kRGBA_16161616:    return kRGBA_SkColorTypeComponentFlags;
        case GrColorType::kRG_F16:           return kRed_SkColorTypeComponentFlag |
                                                    kGreen_SkColorTypeComponentFlag;
    }
    SkUNREACHABLE;
}

/**
 * Describes the encoding of channel data in a GrColorType.
 */
enum class GrColorTypeEncoding {
    kUnorm,
    kSRGBUnorm,
    // kSnorm,
    kFloat,
    // kSint
    // kUint
};

/**
 * Describes a GrColorType by how many bits are used for each color component and how they are
 * encoded. Currently all the non-zero channels share a single GrColorTypeEncoding. This could be
 * expanded to store separate encodings and to indicate which bits belong to which components.
 */
struct GrColorTypeDesc {
public:
    static constexpr GrColorTypeDesc MakeRGBA(int rgba, GrColorTypeEncoding e) {
        return {rgba, rgba, rgba, rgba, 0, e};
    }

    static constexpr GrColorTypeDesc MakeRGBA(int rgb, int a, GrColorTypeEncoding e) {
        return {rgb, rgb, rgb, a, 0, e};
    }

    static constexpr GrColorTypeDesc MakeRGB(int rgb, GrColorTypeEncoding e) {
        return {rgb, rgb, rgb, 0, 0, e};
    }

    static constexpr GrColorTypeDesc MakeRGB(int r, int g, int b, GrColorTypeEncoding e) {
        return {r, g, b, 0, 0, e};
    }

    static constexpr GrColorTypeDesc MakeAlpha(int a, GrColorTypeEncoding e) {
        return {0, 0, 0, a, 0, e};
    }

    static constexpr GrColorTypeDesc MakeR(int r, GrColorTypeEncoding e) {
        return {r, 0, 0, 0, 0, e};
    }

    static constexpr GrColorTypeDesc MakeRG(int rg, GrColorTypeEncoding e) {
        return {rg, rg, 0, 0, 0, e};
    }

    static constexpr GrColorTypeDesc MakeGray(int grayBits, GrColorTypeEncoding e) {
        return {0, 0, 0, 0, grayBits, e};
    }

    static constexpr GrColorTypeDesc MakeInvalid() { return {}; }

    constexpr int r() const { return fRBits; }
    constexpr int g() const { return fGBits; }
    constexpr int b() const { return fBBits; }
    constexpr int a() const { return fABits; }
    constexpr int operator[](int c) const {
        switch (c) {
            case 0: return this->r();
            case 1: return this->g();
            case 2: return this->b();
            case 3: return this->a();
        }
        SkUNREACHABLE;
    }

    constexpr int gray() const { return fGrayBits; }

    constexpr GrColorTypeEncoding encoding() const { return fEncoding; }

private:
    int fRBits = 0;
    int fGBits = 0;
    int fBBits = 0;
    int fABits = 0;
    int fGrayBits = 0;
    GrColorTypeEncoding fEncoding = GrColorTypeEncoding::kUnorm;

    constexpr GrColorTypeDesc() = default;

    constexpr GrColorTypeDesc(int r, int g, int b, int a, int gray, GrColorTypeEncoding encoding)
            : fRBits(r), fGBits(g), fBBits(b), fABits(a), fGrayBits(gray), fEncoding(encoding) {
        SkASSERT(r >= 0 && g >= 0 && b >= 0 && a >= 0 && gray >= 0);
        SkASSERT(!gray || (!r && !g && !b));
        SkASSERT(r || g || b || a || gray);
    }
};

static constexpr GrColorTypeDesc GrGetColorTypeDesc(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:
            return GrColorTypeDesc::MakeInvalid();
        case GrColorType::kAlpha_8:
            return GrColorTypeDesc::MakeAlpha(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kBGR_565:
            return GrColorTypeDesc::MakeRGB(5, 6, 5, GrColorTypeEncoding::kUnorm);
        case GrColorType::kABGR_4444:
            return GrColorTypeDesc::MakeRGBA(4, GrColorTypeEncoding::kUnorm);
        case GrColorType::kRGBA_8888:
            return GrColorTypeDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kRGBA_8888_SRGB:
            return GrColorTypeDesc::MakeRGBA(8, GrColorTypeEncoding::kSRGBUnorm);
        case GrColorType::kRGB_888x:
            return GrColorTypeDesc::MakeRGB(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kRG_88:
            return GrColorTypeDesc::MakeRG(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kBGRA_8888:
            return GrColorTypeDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kRGBA_1010102:
            return GrColorTypeDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
        case GrColorType::kGray_8:
            return GrColorTypeDesc::MakeGray(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kAlpha_F16:
            return GrColorTypeDesc::MakeAlpha(16, GrColorTypeEncoding::kFloat);
        case GrColorType::kRGBA_F16:
            return GrColorTypeDesc::MakeRGBA(16, GrColorTypeEncoding::kFloat);
        case GrColorType::kRGBA_F16_Clamped:
            return GrColorTypeDesc::MakeRGBA(16, GrColorTypeEncoding::kFloat);
        case GrColorType::kRGBA_F32:
            return GrColorTypeDesc::MakeRGBA(32, GrColorTypeEncoding::kFloat);
        case GrColorType::kAlpha_8xxx:
            return GrColorTypeDesc::MakeAlpha(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kAlpha_F32xxx:
            return GrColorTypeDesc::MakeAlpha(32, GrColorTypeEncoding::kFloat);
        case GrColorType::kGray_8xxx:
            return GrColorTypeDesc::MakeGray(8, GrColorTypeEncoding::kUnorm);
        case GrColorType::kR_16:
            return GrColorTypeDesc::MakeR(16, GrColorTypeEncoding::kUnorm);
        case GrColorType::kRG_1616:
            return GrColorTypeDesc::MakeRG(16, GrColorTypeEncoding::kUnorm);
        case GrColorType::kRGBA_16161616:
            return GrColorTypeDesc::MakeRGBA(16, GrColorTypeEncoding::kUnorm);
        case GrColorType::kRG_F16:
            return GrColorTypeDesc::MakeRG(16, GrColorTypeEncoding::kFloat);
    }
    SkUNREACHABLE;
}

static constexpr GrClampType GrColorTypeClampType(GrColorType colorType) {
    if (GrGetColorTypeDesc(colorType).encoding() == GrColorTypeEncoding::kUnorm ||
        GrGetColorTypeDesc(colorType).encoding() == GrColorTypeEncoding::kSRGBUnorm) {
        return GrClampType::kAuto;
    }
    return GrColorType::kRGBA_F16_Clamped == colorType ? GrClampType::kManual : GrClampType::kNone;
}

// Consider a color type "wider" than n if it has more than n bits for any its representable
// channels.
static constexpr bool GrColorTypeIsWiderThan(GrColorType colorType, int n) {
    SkASSERT(n > 0);
    auto desc = GrGetColorTypeDesc(colorType);
    return (desc.r() && desc.r() > n )||
           (desc.g() && desc.g() > n) ||
           (desc.b() && desc.b() > n) ||
           (desc.a() && desc.a() > n) ||
           (desc.gray() && desc.gray() > n);
}

static constexpr bool GrColorTypeIsAlphaOnly(GrColorType ct) {
    return kAlpha_SkColorTypeComponentFlag == GrColorTypeComponentFlags(ct);
}

static constexpr bool GrColorTypeHasAlpha(GrColorType ct) {
    return kAlpha_SkColorTypeComponentFlag & GrColorTypeComponentFlags(ct);
}

static constexpr size_t GrColorTypeBytesPerPixel(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:          return 0;
        case GrColorType::kAlpha_8:          return 1;
        case GrColorType::kBGR_565:          return 2;
        case GrColorType::kABGR_4444:        return 2;
        case GrColorType::kRGBA_8888:        return 4;
        case GrColorType::kRGBA_8888_SRGB:   return 4;
        case GrColorType::kRGB_888x:         return 4;
        case GrColorType::kRG_88:            return 2;
        case GrColorType::kBGRA_8888:        return 4;
        case GrColorType::kRGBA_1010102:     return 4;
        case GrColorType::kGray_8:           return 1;
        case GrColorType::kAlpha_F16:        return 2;
        case GrColorType::kRGBA_F16:         return 8;
        case GrColorType::kRGBA_F16_Clamped: return 8;
        case GrColorType::kRGBA_F32:         return 16;
        case GrColorType::kAlpha_8xxx:       return 4;
        case GrColorType::kAlpha_F32xxx:     return 16;
        case GrColorType::kGray_8xxx:        return 4;
        case GrColorType::kR_16:             return 2;
        case GrColorType::kRG_1616:          return 4;
        // Experimental (for Y416 and mutant P016/P010)
        case GrColorType::kRGBA_16161616:    return 8;
        case GrColorType::kRG_F16:           return 4;
    }
    SkUNREACHABLE;
}

static constexpr GrColorType GrPixelConfigToColorType(GrPixelConfig config) {
    switch (config) {
        case kUnknown_GrPixelConfig:
            return GrColorType::kUnknown;
        case kAlpha_8_GrPixelConfig:
            return GrColorType::kAlpha_8;
        case kGray_8_GrPixelConfig:
            return GrColorType::kGray_8;
        case kRGB_565_GrPixelConfig:
            return GrColorType::kBGR_565;
        case kRGBA_4444_GrPixelConfig:
            return GrColorType::kABGR_4444;
        case kRGBA_8888_GrPixelConfig:
            return GrColorType::kRGBA_8888;
        case kRGB_888_GrPixelConfig:
            return GrColorType::kRGB_888x;
        case kRGB_888X_GrPixelConfig:
            return GrColorType::kRGB_888x;
        case kRG_88_GrPixelConfig:
            return GrColorType::kRG_88;
        case kBGRA_8888_GrPixelConfig:
            return GrColorType::kBGRA_8888;
        case kSRGBA_8888_GrPixelConfig:
            return GrColorType::kRGBA_8888_SRGB;
        case kRGBA_1010102_GrPixelConfig:
            return GrColorType::kRGBA_1010102;
        case kRGBA_float_GrPixelConfig:
            return GrColorType::kRGBA_F32;
        case kAlpha_half_GrPixelConfig:
            return GrColorType::kAlpha_F16;
        case kRGBA_half_GrPixelConfig:
            return GrColorType::kRGBA_F16;
        case kRGBA_half_Clamped_GrPixelConfig:
            return GrColorType::kRGBA_F16_Clamped;
        case kRGB_ETC1_GrPixelConfig:
            // We may need a roughly equivalent color type for a compressed texture. This should be
            // the logical format for decompressing the data into.
            return GrColorType::kRGB_888x;
        case kAlpha_8_as_Alpha_GrPixelConfig:
            return GrColorType::kAlpha_8;
        case kAlpha_8_as_Red_GrPixelConfig:
            return GrColorType::kAlpha_8;
        case kAlpha_half_as_Lum_GrPixelConfig:  // fall through
        case kAlpha_half_as_Red_GrPixelConfig:
            return GrColorType::kAlpha_F16;
        case kGray_8_as_Lum_GrPixelConfig:
            return GrColorType::kGray_8;
        case kGray_8_as_Red_GrPixelConfig:
            return GrColorType::kGray_8;
        case kR_16_GrPixelConfig:
            return GrColorType::kR_16;
        case kRG_1616_GrPixelConfig:
            return GrColorType::kRG_1616;

        // Experimental (for Y416 and mutant P016/P010)
        case kRGBA_16161616_GrPixelConfig:
            return GrColorType::kRGBA_16161616;
        case kRG_half_GrPixelConfig:
            return GrColorType::kRG_F16;
    }
    SkUNREACHABLE;
}

static constexpr GrPixelConfig GrColorTypeToPixelConfig(GrColorType colorType) {
    switch (colorType) {
        case GrColorType::kUnknown:          return kUnknown_GrPixelConfig;
        case GrColorType::kAlpha_8:          return kAlpha_8_GrPixelConfig;
        case GrColorType::kGray_8:           return kGray_8_GrPixelConfig;
        case GrColorType::kBGR_565:          return kRGB_565_GrPixelConfig;
        case GrColorType::kABGR_4444:        return kRGBA_4444_GrPixelConfig;
        case GrColorType::kRGBA_8888:        return kRGBA_8888_GrPixelConfig;
        case GrColorType::kRGBA_8888_SRGB:   return kSRGBA_8888_GrPixelConfig;
        case GrColorType::kRGB_888x:         return kRGB_888_GrPixelConfig;
        case GrColorType::kRG_88:            return kRG_88_GrPixelConfig;
        case GrColorType::kBGRA_8888:        return kBGRA_8888_GrPixelConfig;
        case GrColorType::kRGBA_1010102:     return kRGBA_1010102_GrPixelConfig;
        case GrColorType::kRGBA_F32:         return kRGBA_float_GrPixelConfig;
        case GrColorType::kAlpha_F16:        return kAlpha_half_GrPixelConfig;
        case GrColorType::kRGBA_F16:         return kRGBA_half_GrPixelConfig;
        case GrColorType::kRGBA_F16_Clamped: return kRGBA_half_Clamped_GrPixelConfig;
        case GrColorType::kAlpha_8xxx:       return kUnknown_GrPixelConfig;
        case GrColorType::kAlpha_F32xxx:     return kUnknown_GrPixelConfig;
        case GrColorType::kGray_8xxx:        return kUnknown_GrPixelConfig;
        case GrColorType::kR_16:             return kR_16_GrPixelConfig;
        case GrColorType::kRG_1616:          return kRG_1616_GrPixelConfig;

        // Experimental (for Y416 and mutant P016/P010)
        case GrColorType::kRGBA_16161616:    return kRGBA_16161616_GrPixelConfig;
        case GrColorType::kRG_F16:           return kRG_half_GrPixelConfig;
    }
    SkUNREACHABLE;
}

/**
 * Ref-counted object that calls a callback from its destructor.
 */
class GrRefCntedCallback : public SkRefCnt {
public:
    using Context = void*;
    using Callback = void (*)(Context);

    GrRefCntedCallback(Callback proc, Context ctx) : fReleaseProc(proc), fReleaseCtx(ctx) {
        SkASSERT(proc);
    }
    ~GrRefCntedCallback() override { fReleaseProc ? fReleaseProc(fReleaseCtx) : void(); }

    Context context() const { return fReleaseCtx; }

private:
    Callback fReleaseProc;
    Context fReleaseCtx;
};

/**
 * Indicates "resolutions" that need to be done on a texture before it can be sampled from.
 */
enum class GrTextureResolveFlags {
    kNone = 0,
    kMipMaps = 1 << 0,  // Regenerate all mipmap levels.
    // TODO: kMSAA = 1 << 1  // Blit the MSAA render buffer into a standard texture.
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTextureResolveFlags)

#if GR_TEST_UTILS || defined(SK_ENABLE_DUMP_GPU)
static constexpr const char* GrBackendApiToStr(GrBackendApi api) {
    switch (api) {
        case GrBackendApi::kMetal:  return "Metal";
        case GrBackendApi::kDawn:   return "Dawn";
        case GrBackendApi::kOpenGL: return "OpenGL";
        case GrBackendApi::kVulkan: return "Vulkan";
        case GrBackendApi::kMock:   return "Mock";
    }
    SkUNREACHABLE;
}

static constexpr const char* GrPixelConfigToStr(GrPixelConfig config) {
    switch (config) {
        case kUnknown_GrPixelConfig:           return "Unknown";
        case kAlpha_8_GrPixelConfig:           return "Alpha8";
        case kAlpha_8_as_Alpha_GrPixelConfig:  return "Alpha8_asAlpha";
        case kAlpha_8_as_Red_GrPixelConfig:    return "Alpha8_asRed";
        case kGray_8_GrPixelConfig:            return "Gray8";
        case kGray_8_as_Lum_GrPixelConfig:     return "Gray8_asLum";
        case kGray_8_as_Red_GrPixelConfig:     return "Gray8_asRed";
        case kRGB_565_GrPixelConfig:           return "RGB565";
        case kRGBA_4444_GrPixelConfig:         return "RGBA444";
        case kRGBA_8888_GrPixelConfig:         return "RGBA8888";
        case kRGB_888_GrPixelConfig:           return "RGB888";
        case kRGB_888X_GrPixelConfig:          return "RGB888X";
        case kRG_88_GrPixelConfig:             return "RG88";
        case kBGRA_8888_GrPixelConfig:         return "BGRA8888";
        case kSRGBA_8888_GrPixelConfig:        return "SRGBA8888";
        case kRGBA_1010102_GrPixelConfig:      return "RGBA1010102";
        case kRGBA_float_GrPixelConfig:        return "RGBAFloat";
        case kAlpha_half_GrPixelConfig:        return "AlphaHalf";
        case kAlpha_half_as_Lum_GrPixelConfig: return "AlphaHalf_asLum";
        case kAlpha_half_as_Red_GrPixelConfig: return "AlphaHalf_asRed";
        case kRGBA_half_GrPixelConfig:         return "RGBAHalf";
        case kRGBA_half_Clamped_GrPixelConfig: return "RGBAHalfClamped";
        case kRGB_ETC1_GrPixelConfig:          return "RGBETC1";
        case kR_16_GrPixelConfig:              return "R16";
        case kRG_1616_GrPixelConfig:           return "RG1616";
        case kRGBA_16161616_GrPixelConfig:     return "RGBA16161616";
        case kRG_half_GrPixelConfig:           return "RGHalf";
    }
    SkUNREACHABLE;
}

static constexpr const char* GrColorTypeToStr(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:          return "kUnknown";
        case GrColorType::kAlpha_8:          return "kAlpha_8";
        case GrColorType::kBGR_565:          return "kRGB_565";
        case GrColorType::kABGR_4444:        return "kARGB_4444";
        case GrColorType::kRGBA_8888:        return "kRGBA_8888";
        case GrColorType::kRGBA_8888_SRGB:   return "kRGBA_8888_SRGB";
        case GrColorType::kRGB_888x:         return "kRGB_888x";
        case GrColorType::kRG_88:            return "kRG_88";
        case GrColorType::kBGRA_8888:        return "kBGRA_8888";
        case GrColorType::kRGBA_1010102:     return "kRGBA_1010102";
        case GrColorType::kGray_8:           return "kGray_8";
        case GrColorType::kAlpha_F16:        return "kAlpha_F16";
        case GrColorType::kRGBA_F16:         return "kRGBA_F16";
        case GrColorType::kRGBA_F16_Clamped: return "kRGBA_F16_Clamped";
        case GrColorType::kRGBA_F32:         return "kRGBA_F32";
        case GrColorType::kAlpha_8xxx:       return "kAlpha_8xxx";
        case GrColorType::kAlpha_F32xxx:     return "kAlpha_F32xxx";
        case GrColorType::kGray_8xxx:        return "kGray_8xxx";
        case GrColorType::kR_16:             return "kR_16";
        case GrColorType::kRG_1616:          return "kRG_1616";
        case GrColorType::kRGBA_16161616:    return "kRGBA_16161616";
        case GrColorType::kRG_F16:           return "kRG_F16";
    }
    SkUNREACHABLE;
}
#endif

#endif
