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
    kOpenGL_GrBackend,
    kVulkan_GrBackend,

    kLast_GrBackend = kVulkan_GrBackend
};
const int kBackendCount = kLast_GrBackend + 1;

/**
 * Backend-specific 3D context handle
 *      GrGLInterface* for OpenGL. If NULL will use the default GL interface.
 *      GrVkBackendContext* for Vulkan.
 */
typedef intptr_t GrBackendContext;

///////////////////////////////////////////////////////////////////////////////

/**
 * Used to control antialiasing in draw calls.
 */
enum class GrAA {
    kYes,
    kNo
};

static inline GrAA GrBoolToAA(bool aa) { return aa ? GrAA::kYes : GrAA::kNo; }

///////////////////////////////////////////////////////////////////////////////

/**
* Geometric primitives used for drawing.
*/
enum GrPrimitiveType {
    kTriangles_GrPrimitiveType,
    kTriangleStrip_GrPrimitiveType,
    kTriangleFan_GrPrimitiveType,
    kPoints_GrPrimitiveType,
    kLines_GrPrimitiveType,     // 1 pix wide only
    kLineStrip_GrPrimitiveType, // 1 pix wide only
    kLast_GrPrimitiveType = kLineStrip_GrPrimitiveType
};

static inline bool GrIsPrimTypeLines(GrPrimitiveType type) {
    return kLines_GrPrimitiveType == type || kLineStrip_GrPrimitiveType == type;
}

static inline bool GrIsPrimTypeTris(GrPrimitiveType type) {
    return kTriangles_GrPrimitiveType == type     ||
           kTriangleStrip_GrPrimitiveType == type ||
           kTriangleFan_GrPrimitiveType == type;
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
     * 8 bit signed integers per-channel. Byte order is b,g,r,a.
     */
    kRGBA_8888_sint_GrPixelConfig,
    /**
     * ETC1 Compressed Data
     */
    kETC1_GrPixelConfig,
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

    kLast_GrPixelConfig = kRGBA_half_GrPixelConfig
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

// Returns true if the pixel config is a GPU-specific compressed format
// representation.
static inline bool GrPixelConfigIsCompressed(GrPixelConfig config) {
    switch (config) {
        case kETC1_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return false;
    }
    SkFAIL("Invalid pixel config");
    return false;
}

/** If the pixel config is compressed, return an equivalent uncompressed format. */
static inline GrPixelConfig GrMakePixelConfigUncompressed(GrPixelConfig config) {
    switch (config) {
        case kETC1_GrPixelConfig:
            return kRGBA_8888_GrPixelConfig;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            SkASSERT(!GrPixelConfigIsCompressed(config));
            return config;
    }
    SkFAIL("Invalid pixel config");
    return config;
}

// Returns true if the pixel config is 32 bits per pixel
static inline bool GrPixelConfigIs8888Unorm(GrPixelConfig config) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kETC1_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return false;
    }
    SkFAIL("Invalid pixel config");
    return false;
}

// Returns true if the color (non-alpha) components represent sRGB values. It does NOT indicate that
// all three color components are present in the config or anything about their order.
static inline bool GrPixelConfigIsSRGB(GrPixelConfig config) {
    switch (config) {
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kETC1_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return false;
    }
    SkFAIL("Invalid pixel config");
    return false;
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
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kETC1_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return kUnknown_GrPixelConfig;
    }
    SkFAIL("Invalid pixel config");
    return kUnknown_GrPixelConfig;
}

static inline size_t GrBytesPerPixel(GrPixelConfig config) {
    SkASSERT(!GrPixelConfigIsCompressed(config));
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
            return 1;
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
            return 2;
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
            return 4;
        case kRGBA_half_GrPixelConfig:
            return 8;
        case kRGBA_float_GrPixelConfig:
            return 16;
        case kRG_float_GrPixelConfig:
            return 8;
        case kUnknown_GrPixelConfig:
        case kETC1_GrPixelConfig:
            return 0;
    }
    SkFAIL("Invalid pixel config");
    return 0;
}

static inline bool GrPixelConfigIsOpaque(GrPixelConfig config) {
    switch (config) {
        case kETC1_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kGray_8_GrPixelConfig:
            return true;
        case kAlpha_8_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kUnknown_GrPixelConfig:
            return false;
    }
    SkFAIL("Invalid pixel config");
    return false;
}

static inline bool GrPixelConfigIsAlphaOnly(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kETC1_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return false;
    }
    SkFAIL("Invalid pixel config.");
    return false;
}

static inline bool GrPixelConfigIsFloatingPoint(GrPixelConfig config) {
    switch (config) {
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return true;
        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kETC1_GrPixelConfig:
            return false;
    }
    SkFAIL("Invalid pixel config");
    return false;
}

static inline bool GrPixelConfigIsSint(GrPixelConfig config) {
    return config == kRGBA_8888_sint_GrPixelConfig;
}

/**
 * Optional bitfield flags that can be set on GrSurfaceDesc (below).
 */
enum GrSurfaceFlags {
    kNone_GrSurfaceFlags            = 0x0,
    /**
     * Creates a texture that can be rendered to as a GrRenderTarget. Use
     * GrTexture::asRenderTarget() to access.
     */
    kRenderTarget_GrSurfaceFlag     = 0x1,
    /**
     * Placeholder for managing zero-copy textures
     */
    kZeroCopy_GrSurfaceFlag         = 0x2,
    /**
     * Indicates that all allocations (color buffer, FBO completeness, etc)
     * should be verified.
     */
    kCheckAllocation_GrSurfaceFlag  = 0x4,
};

GR_MAKE_BITFIELD_OPS(GrSurfaceFlags)

// opaque type for 3D API object handles
typedef intptr_t GrBackendObject;

/**
 * Some textures will be stored such that the upper and left edges of the content meet at the
 * the origin (in texture coord space) and for other textures the lower and left edges meet at
 * the origin. kDefault_GrSurfaceOrigin sets textures to TopLeft, and render targets
 * to BottomLeft.
 */

enum GrSurfaceOrigin {
    kDefault_GrSurfaceOrigin,         // DEPRECATED; to be removed
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
    , fOrigin(kDefault_GrSurfaceOrigin)
    , fWidth(0)
    , fHeight(0)
    , fConfig(kUnknown_GrPixelConfig)
    , fSampleCnt(0)
    , fIsMipMapped(false) {
    }

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
     * The number of samples per pixel or 0 to disable full scene AA. This only
     * applies if the kRenderTarget_GrSurfaceFlag is set. The actual number
     * of samples may not exactly match the request. The request will be rounded
     * up to the next supported sample count, or down if it is larger than the
     * max supported count.
     */
    int                    fSampleCnt;
    bool                   fIsMipMapped; //!< Indicates if the texture has mipmaps
};

// Legacy alias
typedef GrSurfaceDesc GrTextureDesc;

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

    /** Skia will assume ownership of the resource, free it, and reuse it within the cache. */
    kAdoptAndCache_GrWrapOwnership,
};

/**
 * Gr can wrap an existing texture created by the client with a GrTexture
 * object. The client is responsible for ensuring that the texture lives at
 * least as long as the GrTexture object wrapping it. We require the client to
 * explicitly provide information about the texture, such as width, height,
 * and pixel config, rather than querying the 3D APIfor these values. We expect
 * these to be immutable even if the 3D API doesn't require this (OpenGL).
 *
 * Textures that are also render targets are supported as well. Gr will manage
 * any ancillary 3D API (stencil buffer, FBO id, etc) objects necessary for
 * Gr to draw into the render target. To access the render target object
 * call GrTexture::asRenderTarget().
 *
 * If in addition to the render target flag, the caller also specifies a sample
 * count Gr will create an MSAA buffer that resolves into the texture. Gr auto-
 * resolves when it reads from the texture. The client can explictly resolve
 * using the GrRenderTarget interface.
 *
 * Note: These flags currently form a subset of GrTexture's flags.
 */

enum GrBackendTextureFlags {
    /**
     * No flags enabled
     */
    kNone_GrBackendTextureFlag             = 0,
    /**
     * Indicates that the texture is also a render target, and thus should have
     * a GrRenderTarget object.
     */
    kRenderTarget_GrBackendTextureFlag     = kRenderTarget_GrSurfaceFlag,
};
GR_MAKE_BITFIELD_OPS(GrBackendTextureFlags)

struct GrBackendTextureDesc {
    GrBackendTextureDesc() { memset(this, 0, sizeof(*this)); }
    GrBackendTextureFlags           fFlags;
    GrSurfaceOrigin                 fOrigin;
    int                             fWidth;         //<! width in pixels
    int                             fHeight;        //<! height in pixels
    GrPixelConfig                   fConfig;        //<! color format
    /**
     * If the render target flag is set and sample count is greater than 0
     * then Gr will create an MSAA buffer that resolves to the texture.
     */
    int                             fSampleCnt;
    /**
     * Handle to the 3D API object.
     * OpenGL: Texture ID.
     * Vulkan: GrVkImageInfo*
     */
    GrBackendObject                 fTextureHandle;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Gr can wrap an existing render target created by the client in the 3D API
 * with a GrRenderTarget object. The client is responsible for ensuring that the
 * underlying 3D API object lives at least as long as the GrRenderTarget object
 * wrapping it. We require the client to explicitly provide information about
 * the target, such as width, height, and pixel config rather than querying the
 * 3D API for these values. We expect these properties to be immutable even if
 * the 3D API doesn't require this (OpenGL).
 */

struct GrBackendRenderTargetDesc {
    GrBackendRenderTargetDesc() { memset(this, 0, sizeof(*this)); }
    int                             fWidth;         //<! width in pixels
    int                             fHeight;        //<! height in pixels
    GrPixelConfig                   fConfig;        //<! color format
    GrSurfaceOrigin                 fOrigin;        //<! pixel origin
    /**
     * The number of samples per pixel. Gr uses this to influence decisions
     * about applying other forms of anti-aliasing.
     */
    int                             fSampleCnt;
    /**
     * Number of bits of stencil per-pixel.
     */
    int                             fStencilBits;
    /**
     * Handle to the 3D API object.
     * OpenGL: FBO ID
     * Vulkan: GrVkImageInfo*
     */
    GrBackendObject                 fRenderTargetHandle;
};

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
 * Returns the data size for the given compressed pixel config
 */
static inline size_t GrCompressedFormatDataSize(GrPixelConfig config,
                                                int width, int height) {
    SkASSERT(GrPixelConfigIsCompressed(config));

    switch (config) {
        case kETC1_GrPixelConfig:
            SkASSERT((width & 3) == 0);
            SkASSERT((height & 3) == 0);
            return (width >> 2) * (height >> 2) * 8;

        case kUnknown_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            SkFAIL("Unknown compressed pixel config");
            return 4 * width * height;
    }

    SkFAIL("Invalid pixel config");
    return 4 * width * height;
}

/**
 * This value translates to reseting all the context state for any backend.
 */
static const uint32_t kAll_GrBackendState = 0xffffffff;

#endif
