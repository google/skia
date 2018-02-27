/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypes_DEFINED
#define GrTypes_DEFINED

#include "SkMath.h"
#include "SkTypes.h"
#include "GrConfig.h"

////////////////////////////////////////////////////////////////////////////////

/**
 * Defines overloaded bitwise operators to make it easier to use an enum as a
 * bitfield.
 */
#define GR_MAKE_BITFIELD_OPS(X) \
    inline X operator |(X a, X b) { \
        return (X) (+a | +b); \
    } \
    inline X& operator |=(X& a, X b) { \
        return (a = a | b); \
    } \
    inline X operator &(X a, X b) { \
        return (X) (+a & +b); \
    } \
    inline X& operator &=(X& a, X b) { \
        return (a = a & b); \
    } \
    template <typename T> \
    inline X operator &(T a, X b) { \
        return (X) (+a & +b); \
    } \
    template <typename T> \
    inline X operator &(X a, T b) { \
        return (X) (+a & +b); \
    } \

#define GR_DECL_BITFIELD_OPS_FRIENDS(X) \
    friend X operator |(X a, X b); \
    friend X& operator |=(X& a, X b); \
    \
    friend X operator &(X a, X b); \
    friend X& operator &=(X& a, X b); \
    \
    template <typename T> \
    friend X operator &(T a, X b); \
    \
    template <typename T> \
    friend X operator &(X a, T b); \

/**
 * Wraps a C++11 enum that we use as a bitfield, and enables a limited amount of
 * masking with type safety. Instantiated with the ~ operator.
 */
template<typename TFlags> class GrTFlagsMask {
public:
    constexpr explicit GrTFlagsMask(TFlags value) : GrTFlagsMask(static_cast<int>(value)) {}
    constexpr explicit GrTFlagsMask(int value) : fValue(value) {}
    constexpr int value() const { return fValue; }
private:
    const int fValue;
};

// Or-ing a mask always returns another mask.
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator|(GrTFlagsMask<TFlags> a,
                                                                   GrTFlagsMask<TFlags> b) {
    return GrTFlagsMask<TFlags>(a.value() | b.value());
}
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator|(GrTFlagsMask<TFlags> a,
                                                                   TFlags b) {
    return GrTFlagsMask<TFlags>(a.value() | static_cast<int>(b));
}
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator|(TFlags a,
                                                                   GrTFlagsMask<TFlags> b) {
    return GrTFlagsMask<TFlags>(static_cast<int>(a) | b.value());
}
template<typename TFlags> inline GrTFlagsMask<TFlags>& operator|=(GrTFlagsMask<TFlags>& a,
                                                                  GrTFlagsMask<TFlags> b) {
    return (a = a | b);
}

// And-ing two masks returns another mask; and-ing one with regular flags returns flags.
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator&(GrTFlagsMask<TFlags> a,
                                                                   GrTFlagsMask<TFlags> b) {
    return GrTFlagsMask<TFlags>(a.value() & b.value());
}
template<typename TFlags> constexpr TFlags operator&(GrTFlagsMask<TFlags> a, TFlags b) {
    return static_cast<TFlags>(a.value() & static_cast<int>(b));
}
template<typename TFlags> constexpr TFlags operator&(TFlags a, GrTFlagsMask<TFlags> b) {
    return static_cast<TFlags>(static_cast<int>(a) & b.value());
}
template<typename TFlags> inline TFlags& operator&=(TFlags& a, GrTFlagsMask<TFlags> b) {
    return (a = a & b);
}

/**
 * Defines bitwise operators that make it possible to use an enum class as a
 * basic bitfield.
 */
#define GR_MAKE_BITFIELD_CLASS_OPS(X) \
    constexpr GrTFlagsMask<X> operator~(X a) { \
        return GrTFlagsMask<X>(~static_cast<int>(a)); \
    } \
    constexpr X operator|(X a, X b) { \
        return static_cast<X>(static_cast<int>(a) | static_cast<int>(b)); \
    } \
    inline X& operator|=(X& a, X b) { \
        return (a = a | b); \
    } \
    constexpr bool operator&(X a, X b) { \
        return SkToBool(static_cast<int>(a) & static_cast<int>(b)); \
    } \

#define GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(X) \
    friend constexpr GrTFlagsMask<X> operator ~(X); \
    friend constexpr X operator |(X, X); \
    friend X& operator |=(X&, X); \
    friend constexpr bool operator &(X, X);

////////////////////////////////////////////////////////////////////////////////

// compile time versions of min/max
#define GR_CT_MAX(a, b) (((b) < (a)) ? (a) : (b))
#define GR_CT_MIN(a, b) (((b) < (a)) ? (b) : (a))

/**
 *  divide, rounding up
 */
static inline int32_t GrIDivRoundUp(int x, int y) {
    SkASSERT(y > 0);
    return (x + (y-1)) / y;
}
static inline uint32_t GrUIDivRoundUp(uint32_t x, uint32_t y) {
    return (x + (y-1)) / y;
}
static inline size_t GrSizeDivRoundUp(size_t x, size_t y) {
    return (x + (y-1)) / y;
}

// compile time, evaluates Y multiple times
#define GR_CT_DIV_ROUND_UP(X, Y) (((X) + ((Y)-1)) / (Y))

/**
 *  align up
 */
static inline uint32_t GrUIAlignUp(uint32_t x, uint32_t alignment) {
    return GrUIDivRoundUp(x, alignment) * alignment;
}
static inline size_t GrSizeAlignUp(size_t x, size_t alignment) {
    return GrSizeDivRoundUp(x, alignment) * alignment;
}

// compile time, evaluates A multiple times
#define GR_CT_ALIGN_UP(X, A) (GR_CT_DIV_ROUND_UP((X),(A)) * (A))

/**
 * amount of pad needed to align up
 */
static inline uint32_t GrUIAlignUpPad(uint32_t x, uint32_t alignment) {
    return (alignment - x % alignment) % alignment;
}
static inline size_t GrSizeAlignUpPad(size_t x, size_t alignment) {
    return (alignment - x % alignment) % alignment;
}

/**
 *  align down
 */
static inline uint32_t GrUIAlignDown(uint32_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}
static inline size_t GrSizeAlignDown(size_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Possible 3D APIs that may be used by Ganesh.
 */
enum GrBackend {
    kMetal_GrBackend,
    kOpenGL_GrBackend,
    kVulkan_GrBackend,
    /**
     * Mock is a backend that does not draw anything. It is used for unit tests
     * and to measure CPU overhead.
     */
    kMock_GrBackend,
};

/**
 * Backend-specific 3D context handle
 *      OpenGL: const GrGLInterface*. If null will use the result of GrGLMakeNativeInterface().
 *      Vulkan: GrVkBackendContext*.
 *      Mock: const GrMockOptions* or null for default constructed GrMockContextOptions.
 */
typedef intptr_t GrBackendContext;

///////////////////////////////////////////////////////////////////////////////

/**
 * Used to control antialiasing in draw calls.
 */
enum class GrAA : bool {
    kNo = false,
    kYes = true
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Used to say whether a texture has mip levels allocated or not.
 */
enum class GrMipMapped : bool {
    kNo = false,
    kYes = true
};

///////////////////////////////////////////////////////////////////////////////

/**
* Geometric primitives used for drawing.
*/
enum class GrPrimitiveType {
    kTriangles,
    kTriangleStrip,
    kTriangleFan,
    kPoints,
    kLines,     // 1 pix wide only
    kLineStrip, // 1 pix wide only
    kLinesAdjacency // requires geometry shader support.
};
static constexpr int kNumGrPrimitiveTypes = (int) GrPrimitiveType::kLinesAdjacency + 1;

static constexpr bool GrIsPrimTypeLines(GrPrimitiveType type) {
    return GrPrimitiveType::kLines == type ||
           GrPrimitiveType::kLineStrip == type ||
           GrPrimitiveType::kLinesAdjacency == type;
}

static constexpr bool GrIsPrimTypeTris(GrPrimitiveType type) {
    return GrPrimitiveType::kTriangles == type     ||
           GrPrimitiveType::kTriangleStrip == type ||
           GrPrimitiveType::kTriangleFan == type;
}

static constexpr bool GrPrimTypeRequiresGeometryShaderSupport(GrPrimitiveType type) {
    return GrPrimitiveType::kLinesAdjacency == type;
}

/**
 *  Formats for masks, used by the font cache.
 *  Important that these are 0-based.
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
    static const int sBytesPerPixel[] = { 1, 2, 4 };
    static_assert(SK_ARRAY_COUNT(sBytesPerPixel) == kMaskFormatCount, "array_size_mismatch");
    static_assert(kA8_GrMaskFormat == 0, "enum_order_dependency");
    static_assert(kA565_GrMaskFormat == 1, "enum_order_dependency");
    static_assert(kARGB_GrMaskFormat == 2, "enum_order_dependency");

    return sBytesPerPixel[(int) format];
}

/**
 * Pixel configurations.
 */
enum GrPixelConfig {
    kUnknown_GrPixelConfig,
    kAlpha_8_GrPixelConfig,
    kGray_8_GrPixelConfig,
    kRGB_565_GrPixelConfig,
    /**
     * Premultiplied
     */
    kRGBA_4444_GrPixelConfig,
    /**
     * Premultiplied. Byte order is r,g,b,a.
     */
    kRGBA_8888_GrPixelConfig,
    /**
     * Premultiplied. Byte order is b,g,r,a.
     */
    kBGRA_8888_GrPixelConfig,
    /**
     * Premultiplied and sRGB. Byte order is r,g,b,a.
     */
    kSRGBA_8888_GrPixelConfig,
    /**
     * Premultiplied and sRGB. Byte order is b,g,r,a.
     */
    kSBGRA_8888_GrPixelConfig,

    /**
     * Premultiplied.
     */
    kRGBA_1010102_GrPixelConfig,

    /**
     * Byte order is r, g, b, a.  This color format is 32 bits per channel
     */
    kRGBA_float_GrPixelConfig,
    /**
     * Byte order is r, g.  This color format is 32 bits per channel
     */
    kRG_float_GrPixelConfig,

    /**
     * This color format is a single 16 bit float channel
     */
    kAlpha_half_GrPixelConfig,

    /**
    * Byte order is r, g, b, a.  This color format is 16 bits per channel
    */
    kRGBA_half_GrPixelConfig,

    kPrivateConfig1_GrPixelConfig,
    kPrivateConfig2_GrPixelConfig,
    kPrivateConfig3_GrPixelConfig,
    kPrivateConfig4_GrPixelConfig,
    kPrivateConfig5_GrPixelConfig,

    kLast_GrPixelConfig = kPrivateConfig5_GrPixelConfig
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
 * Optional bitfield flags that can be set on GrSurfaceDesc (below).
 */
enum GrSurfaceFlags {
    kNone_GrSurfaceFlags = 0x0,
    /**
     * Creates a texture that can be rendered to as a GrRenderTarget. Use
     * GrTexture::asRenderTarget() to access.
     */
    kRenderTarget_GrSurfaceFlag = 0x1,
    /**
     * Clears to zero on creation. It will cause creation failure if initial data is supplied to the
     * texture. This only affects the base level if the texture is created with MIP levels.
     */
    kPerformInitialClear_GrSurfaceFlag = 0x2
};

GR_MAKE_BITFIELD_OPS(GrSurfaceFlags)

// opaque type for 3D API object handles
typedef intptr_t GrBackendObject;

/**
 * Some textures will be stored such that the upper and left edges of the content meet at the
 * the origin (in texture coord space) and for other textures the lower and left edges meet at
 * the origin.
 */

enum GrSurfaceOrigin {
    kTopLeft_GrSurfaceOrigin,
    kBottomLeft_GrSurfaceOrigin,
};

struct GrMipLevel {
    const void* fPixels;
    size_t fRowBytes;
};

/**
 * Describes a surface to be created.
 */
struct GrSurfaceDesc {
    GrSurfaceDesc()
            : fFlags(kNone_GrSurfaceFlags)
            , fOrigin(kTopLeft_GrSurfaceOrigin)
            , fWidth(0)
            , fHeight(0)
            , fConfig(kUnknown_GrPixelConfig)
            , fSampleCnt(1) {}

    GrSurfaceFlags         fFlags;  //!< bitfield of TextureFlags
    GrSurfaceOrigin        fOrigin; //!< origin of the texture
    int                    fWidth;  //!< Width of the texture
    int                    fHeight; //!< Height of the texture

    /**
     * Format of source data of the texture. Not guaranteed to be the same as
     * internal format used by 3D API.
     */
    GrPixelConfig          fConfig;

    /**
     * The number of samples per pixel. Zero is treated equivalently to 1. This only
     * applies if the kRenderTarget_GrSurfaceFlag is set. The actual number
     * of samples may not exactly match the request. The request will be rounded
     * up to the next supported sample count. A value larger than the largest
     * supported sample count will fail.
     */
    int                    fSampleCnt;
};

/**
 * Clips are composed from these objects.
 */
enum GrClipType {
    kRect_ClipType,
    kPath_ClipType
};

///////////////////////////////////////////////////////////////////////////////

/** Ownership rules for external GPU resources imported into Skia. */
enum GrWrapOwnership {
    /** Skia will assume the client will keep the resource alive and Skia will not free it. */
    kBorrow_GrWrapOwnership,

    /** Skia will assume ownership of the resource and free it. */
    kAdopt_GrWrapOwnership,
};

///////////////////////////////////////////////////////////////////////////////

/**
 * The GrContext's cache of backend context state can be partially invalidated.
 * These enums are specific to the GL backend and we'd add a new set for an alternative backend.
 */
enum GrGLBackendState {
    kRenderTarget_GrGLBackendState     = 1 << 0,
    kTextureBinding_GrGLBackendState   = 1 << 1,
    // View state stands for scissor and viewport
    kView_GrGLBackendState             = 1 << 2,
    kBlend_GrGLBackendState            = 1 << 3,
    kMSAAEnable_GrGLBackendState       = 1 << 4,
    kVertex_GrGLBackendState           = 1 << 5,
    kStencil_GrGLBackendState          = 1 << 6,
    kPixelStore_GrGLBackendState       = 1 << 7,
    kProgram_GrGLBackendState          = 1 << 8,
    kFixedFunction_GrGLBackendState    = 1 << 9,
    kMisc_GrGLBackendState             = 1 << 10,
    kPathRendering_GrGLBackendState    = 1 << 11,
    kALL_GrGLBackendState              = 0xffff
};

/**
 * This value translates to reseting all the context state for any backend.
 */
static const uint32_t kAll_GrBackendState = 0xffffffff;

// Enum used as return value when flush with semaphores so the client knows whether the
// semaphores were submitted to GPU or not.
enum class GrSemaphoresSubmitted : bool {
    kNo = false,
    kYes = true
};

#endif
