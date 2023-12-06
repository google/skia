/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLUtil_DEFINED
#define GrGLUtil_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"

class SkMatrix;

////////////////////////////////////////////////////////////////////////////////

typedef uint32_t GrGLVersion;
typedef uint32_t GrGLSLVersion;
typedef uint64_t GrGLDriverVersion;

#define GR_GL_VER(major, minor) ((static_cast<uint32_t>(major) << 16) | \
                                 static_cast<uint32_t>(minor))
#define GR_GLSL_VER(major, minor) ((static_cast<uint32_t>(major) << 16) | \
                                    static_cast<uint32_t>(minor))
#define GR_GL_DRIVER_VER(major, minor, point) ((static_cast<uint64_t>(major) << 32) | \
                                               (static_cast<uint64_t>(minor) << 16) | \
                                                static_cast<uint64_t>(point))

#define GR_GL_MAJOR_VER(version) (static_cast<uint32_t>(version) >> 16)
#define GR_GL_MINOR_VER(version) (static_cast<uint32_t>(version) & 0xFFFF)

#define GR_GL_INVALID_VER GR_GL_VER(0, 0)
#define GR_GLSL_INVALID_VER GR_GLSL_VER(0, 0)
#define GR_GL_DRIVER_UNKNOWN_VER GR_GL_DRIVER_VER(0, 0, 0)

static constexpr uint32_t GrGLFormatChannels(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kUnknown:               return 0;
        case GrGLFormat::kRGBA8:                 return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kR8:                    return kRed_SkColorChannelFlag;
        case GrGLFormat::kALPHA8:                return kAlpha_SkColorChannelFlag;
        case GrGLFormat::kLUMINANCE8:            return kGray_SkColorChannelFlag;
        case GrGLFormat::kLUMINANCE8_ALPHA8:     return kGrayAlpha_SkColorChannelFlags;
        case GrGLFormat::kBGRA8:                 return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kRGB565:                return kRGB_SkColorChannelFlags;
        case GrGLFormat::kRGBA16F:               return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kR16F:                  return kRed_SkColorChannelFlag;
        case GrGLFormat::kRGB8:                  return kRGB_SkColorChannelFlags;
        case GrGLFormat::kRGBX8:                 return kRGB_SkColorChannelFlags;
        case GrGLFormat::kRG8:                   return kRG_SkColorChannelFlags;
        case GrGLFormat::kRGB10_A2:              return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kRGBA4:                 return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kSRGB8_ALPHA8:          return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8:  return kRGB_SkColorChannelFlags;
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2:  return kRGB_SkColorChannelFlags;
        case GrGLFormat::kCOMPRESSED_RGB8_BC1:   return kRGB_SkColorChannelFlags;
        case GrGLFormat::kCOMPRESSED_RGBA8_BC1:  return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kR16:                   return kRed_SkColorChannelFlag;
        case GrGLFormat::kRG16:                  return kRG_SkColorChannelFlags;
        case GrGLFormat::kRGBA16:                return kRGBA_SkColorChannelFlags;
        case GrGLFormat::kRG16F:                 return kRG_SkColorChannelFlags;
        case GrGLFormat::kLUMINANCE16F:          return kGray_SkColorChannelFlag;
        case GrGLFormat::kSTENCIL_INDEX8:        return 0;
        case GrGLFormat::kSTENCIL_INDEX16:       return 0;
        case GrGLFormat::kDEPTH24_STENCIL8:      return 0;
    }
    SkUNREACHABLE;
}

static constexpr GrColorFormatDesc GrGLFormatDesc(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kUnknown: return GrColorFormatDesc::MakeInvalid();

        case GrGLFormat::kRGBA8:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kR8:
            return GrColorFormatDesc::MakeR(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kALPHA8:
            return GrColorFormatDesc::MakeAlpha(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kLUMINANCE8:
            return GrColorFormatDesc::MakeGray(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kLUMINANCE8_ALPHA8:
            return GrColorFormatDesc::MakeGrayAlpha(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kBGRA8:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRGB565:
            return GrColorFormatDesc::MakeRGB(5, 6, 5, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRGBA16F:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kFloat);
        case GrGLFormat::kR16F:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kFloat);
        case GrGLFormat::kRGB8:
            return GrColorFormatDesc::MakeRGB(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRGBX8:
            return GrColorFormatDesc::MakeRGB(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRG8:
            return GrColorFormatDesc::MakeRG(8, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRGB10_A2:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRGBA4:
            return GrColorFormatDesc::MakeRGBA(4, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kSRGB8_ALPHA8:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kSRGBUnorm);
        case GrGLFormat::kR16:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRG16:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRGBA16:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kUnorm);
        case GrGLFormat::kRG16F:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kFloat);
        case GrGLFormat::kLUMINANCE16F:
            return GrColorFormatDesc::MakeGray(16, GrColorTypeEncoding::kFloat);

        // Compressed texture formats are not expected to have a description.
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8: return GrColorFormatDesc::MakeInvalid();
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2: return GrColorFormatDesc::MakeInvalid();
        case GrGLFormat::kCOMPRESSED_RGB8_BC1:  return GrColorFormatDesc::MakeInvalid();
        case GrGLFormat::kCOMPRESSED_RGBA8_BC1: return GrColorFormatDesc::MakeInvalid();

        // This type only describes color channels.
        case GrGLFormat::kSTENCIL_INDEX8:   return GrColorFormatDesc::MakeInvalid();
        case GrGLFormat::kSTENCIL_INDEX16:  return GrColorFormatDesc::MakeInvalid();
        case GrGLFormat::kDEPTH24_STENCIL8: return GrColorFormatDesc::MakeInvalid();
    }
    SkUNREACHABLE;
}

/**
 * The Vendor and Renderer enum values are lazily updated as required.
 */
enum class GrGLVendor {
    kARM,
    kGoogle,
    kImagination,
    kIntel,
    kQualcomm,
    kNVIDIA,
    kATI,
    kApple,

    kOther
};

enum class GrGLRenderer {
    kTegra_PreK1,  // Legacy Tegra architecture (pre-K1).
    kTegra,        // Tegra with the same architecture as NVIDIA desktop GPUs (K1+).

    kPowerVR54x,
    kPowerVRBSeries,
    kPowerVRRogue,

    kAdreno3xx,
    kAdreno430,
    kAdreno4xx_other,
    kAdreno530,
    kAdreno5xx_other,
    kAdreno615,  // Pixel3a
    kAdreno620,  // Pixel5
    kAdreno630,  // Pixel3
    kAdreno640,  // Pixel4
    kAdreno6xx_other,

    /** Intel GPU families, ordered by generation **/
    // 6th gen
    kIntelSandyBridge,

    // 7th gen
    kIntelIvyBridge,
    kIntelValleyView,  // aka BayTrail
    kIntelHaswell,

    // 8th gen
    kIntelCherryView,  // aka Braswell
    kIntelBroadwell,

    // 9th gen
    kIntelApolloLake,
    kIntelSkyLake,
    kIntelGeminiLake,
    kIntelKabyLake,
    kIntelCoffeeLake,

    // 11th gen
    kIntelIceLake,

    // 12th gen
    kIntelRocketLake,
    kIntelTigerLake,
    kIntelAlderLake,

    kGalliumLLVM,

    kMali4xx,
    /** G-3x, G-5x, or G-7x */
    kMaliG,
    /** T-6xx, T-7xx, or T-8xx */
    kMaliT,

    kAMDRadeonHD7xxx,     // AMD Radeon HD 7000 Series
    kAMDRadeonR9M3xx,     // AMD Radeon R9 M300 Series
    kAMDRadeonR9M4xx,     // AMD Radeon R9 M400 Series
    kAMDRadeonPro5xxx,    // AMD Radeon Pro 5000 Series
    kAMDRadeonProVegaxx,  // AMD Radeon Pro Vega

    kApple,

    kWebGL,

    kOther
};

enum class GrGLDriver {
    kMesa,
    kNVIDIA,
    kIntel,
    kQualcomm,
    kFreedreno,
    kAndroidEmulator,
    kImagination,
    kARM,
    kApple,
    kUnknown
};

enum class GrGLANGLEBackend {
    kUnknown,
    kD3D9,
    kD3D11,
    kMetal,
    kOpenGL
};

////////////////////////////////////////////////////////////////////////////////

/**
 *  Some drivers want the var-int arg to be zero-initialized on input.
 */
#define GR_GL_INIT_ZERO     0
#define GR_GL_GetIntegerv(gl, e, p)                                            \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetIntegerv(e, p));                                     \
    } while (0)

#define GR_GL_GetFloatv(gl, e, p)                                              \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetFloatv(e, p));                                       \
    } while (0)

#define GR_GL_GetFramebufferAttachmentParameteriv(gl, t, a, pname, p)          \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetFramebufferAttachmentParameteriv(t, a, pname, p));   \
    } while (0)

#define GR_GL_GetInternalformativ(gl, t, f, n, s, p)                           \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetInternalformativ(t, f, n, s, p));                    \
    } while (0)

#define GR_GL_GetNamedFramebufferAttachmentParameteriv(gl, fb, a, pname, p)          \
    do {                                                                             \
        *(p) = GR_GL_INIT_ZERO;                                                      \
        GR_GL_CALL(gl, GetNamedFramebufferAttachmentParameteriv(fb, a, pname, p));   \
    } while (0)

#define GR_GL_GetRenderbufferParameteriv(gl, t, pname, p)                      \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetRenderbufferParameteriv(t, pname, p));               \
    } while (0)

#define GR_GL_GetTexLevelParameteriv(gl, t, l, pname, p)                       \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetTexLevelParameteriv(t, l, pname, p));                \
    } while (0)

#define GR_GL_GetShaderPrecisionFormat(gl, st, pt, range, precision)           \
    do {                                                                       \
        (range)[0] = GR_GL_INIT_ZERO;                                          \
        (range)[1] = GR_GL_INIT_ZERO;                                          \
        (*precision) = GR_GL_INIT_ZERO;                                        \
        GR_GL_CALL(gl, GetShaderPrecisionFormat(st, pt, range, precision));    \
    } while (0)

////////////////////////////////////////////////////////////////////////////////

GrGLStandard GrGLGetStandardInUseFromString(const char* versionString);
GrGLSLVersion GrGLGetVersion(const GrGLInterface*);
GrGLSLVersion GrGLGetVersionFromString(const char*);

struct GrGLDriverInfo {
    GrGLStandard      fStandard      = kNone_GrGLStandard;
    GrGLVersion       fVersion       = GR_GL_INVALID_VER;
    GrGLSLVersion     fGLSLVersion   = GR_GLSL_INVALID_VER;
    GrGLVendor        fVendor        = GrGLVendor::kOther;
    GrGLRenderer      fRenderer      = GrGLRenderer::kOther;
    GrGLDriver        fDriver        = GrGLDriver::kUnknown;
    GrGLDriverVersion fDriverVersion = GR_GL_DRIVER_UNKNOWN_VER;

    GrGLANGLEBackend  fANGLEBackend       = GrGLANGLEBackend::kUnknown;
    GrGLVendor        fANGLEVendor        = GrGLVendor::kOther;
    GrGLRenderer      fANGLERenderer      = GrGLRenderer::kOther;
    GrGLDriver        fANGLEDriver        = GrGLDriver::kUnknown;
    GrGLDriverVersion fANGLEDriverVersion = GR_GL_DRIVER_UNKNOWN_VER;

    GrGLVendor        fWebGLVendor        = GrGLVendor::kOther;
    GrGLRenderer      fWebGLRenderer      = GrGLRenderer::kOther;

    // Are we running over the Chrome interprocess command buffer?
    bool fIsOverCommandBuffer = false;

    // Running over virgl guest driver.
    bool fIsRunningOverVirgl = false;
};

GrGLDriverInfo GrGLGetDriverInfo(const GrGLInterface*);

/**
 * Helpers for glGetError()
 */

void GrGLCheckErr(const GrGLInterface* gl,
                  const char* location,
                  const char* call);

////////////////////////////////////////////////////////////////////////////////

/**
 *  GR_STRING makes a string of X where X is expanded before conversion to a string
 *  if X itself contains macros.
 */
#define GR_STRING(X) GR_STRING_IMPL(X)
#define GR_STRING_IMPL(X) #X

/**
 *  Creates a string of the form "<filename>(<linenumber>) : "
 */
#define GR_FILE_AND_LINE_STR __FILE__ "(" GR_STRING(__LINE__) ") : "

/**
 * Macros for using GrGLInterface to make GL calls
 */

// Conditionally checks glGetError based on compile-time and run-time flags.
#if GR_GL_CHECK_ERROR
    extern bool gCheckErrorGL;
#define GR_GL_CHECK_ERROR_IMPL(IFACE, X)                 \
    do {                                                 \
        if (gCheckErrorGL) {                             \
            IFACE->checkError(GR_FILE_AND_LINE_STR, #X); \
        }                                                \
    } while (false)
#else
#define GR_GL_CHECK_ERROR_IMPL(IFACE, X) \
    do {                                 \
    } while (false)
#endif

// internal macro to conditionally log the gl call using SkDebugf based on
// compile-time and run-time flags.
#if GR_GL_LOG_CALLS
    extern bool gLogCallsGL;
    #define GR_GL_LOG_CALLS_IMPL(X)                             \
        if (gLogCallsGL)                                        \
            SkDebugf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
#else
    #define GR_GL_LOG_CALLS_IMPL(X)
#endif

// makes a GL call on the interface and does any error checking and logging
#define GR_GL_CALL(IFACE, X)                                    \
    do {                                                        \
        GR_GL_CALL_NOERRCHECK(IFACE, X);                        \
        GR_GL_CHECK_ERROR_IMPL(IFACE, X);                       \
    } while (false)

// Variant of above that always skips the error check. This is useful when
// the caller wants to do its own glGetError() call and examine the error value.
#define GR_GL_CALL_NOERRCHECK(IFACE, X)                         \
    do {                                                        \
        (IFACE)->fFunctions.f##X;                               \
        GR_GL_LOG_CALLS_IMPL(X);                                \
    } while (false)

// same as GR_GL_CALL but stores the return value of the gl call in RET
#define GR_GL_CALL_RET(IFACE, RET, X)                           \
    do {                                                        \
        GR_GL_CALL_RET_NOERRCHECK(IFACE, RET, X);               \
        GR_GL_CHECK_ERROR_IMPL(IFACE, X);                       \
    } while (false)

// same as GR_GL_CALL_RET but always skips the error check.
#define GR_GL_CALL_RET_NOERRCHECK(IFACE, RET, X)                \
    do {                                                        \
        (RET) = (IFACE)->fFunctions.f##X;                       \
        GR_GL_LOG_CALLS_IMPL(X);                                \
    } while (false)

static constexpr GrGLFormat GrGLFormatFromGLEnum(GrGLenum glFormat) {
    switch (glFormat) {
        case GR_GL_RGBA8:                return GrGLFormat::kRGBA8;
        case GR_GL_R8:                   return GrGLFormat::kR8;
        case GR_GL_ALPHA8:               return GrGLFormat::kALPHA8;
        case GR_GL_LUMINANCE8:           return GrGLFormat::kLUMINANCE8;
        case GR_GL_LUMINANCE8_ALPHA8:    return GrGLFormat::kLUMINANCE8_ALPHA8;
        case GR_GL_BGRA8:                return GrGLFormat::kBGRA8;
        case GR_GL_RGB565:               return GrGLFormat::kRGB565;
        case GR_GL_RGBA16F:              return GrGLFormat::kRGBA16F;
        case GR_GL_LUMINANCE16F:         return GrGLFormat::kLUMINANCE16F;
        case GR_GL_R16F:                 return GrGLFormat::kR16F;
        case GR_GL_RGB8:                 return GrGLFormat::kRGB8;
        case GR_GL_RGBX8:                return GrGLFormat::kRGBX8;
        case GR_GL_RG8:                  return GrGLFormat::kRG8;
        case GR_GL_RGB10_A2:             return GrGLFormat::kRGB10_A2;
        case GR_GL_RGBA4:                return GrGLFormat::kRGBA4;
        case GR_GL_SRGB8_ALPHA8:         return GrGLFormat::kSRGB8_ALPHA8;
        case GR_GL_COMPRESSED_ETC1_RGB8: return GrGLFormat::kCOMPRESSED_ETC1_RGB8;
        case GR_GL_COMPRESSED_RGB8_ETC2: return GrGLFormat::kCOMPRESSED_RGB8_ETC2;
        case GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return GrGLFormat::kCOMPRESSED_RGB8_BC1;
        case GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return GrGLFormat::kCOMPRESSED_RGBA8_BC1;
        case GR_GL_R16:                  return GrGLFormat::kR16;
        case GR_GL_RG16:                 return GrGLFormat::kRG16;
        case GR_GL_RGBA16:               return GrGLFormat::kRGBA16;
        case GR_GL_RG16F:                return GrGLFormat::kRG16F;
        case GR_GL_STENCIL_INDEX8:       return GrGLFormat::kSTENCIL_INDEX8;
        case GR_GL_STENCIL_INDEX16:      return GrGLFormat::kSTENCIL_INDEX16;
        case GR_GL_DEPTH24_STENCIL8:     return GrGLFormat::kDEPTH24_STENCIL8;


        default:                         return GrGLFormat::kUnknown;
    }
}

/** Returns either the sized internal format or compressed internal format of the GrGLFormat. */
static constexpr GrGLenum GrGLFormatToEnum(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kRGBA8:                return GR_GL_RGBA8;
        case GrGLFormat::kR8:                   return GR_GL_R8;
        case GrGLFormat::kALPHA8:               return GR_GL_ALPHA8;
        case GrGLFormat::kLUMINANCE8:           return GR_GL_LUMINANCE8;
        case GrGLFormat::kLUMINANCE8_ALPHA8:    return GR_GL_LUMINANCE8_ALPHA8;
        case GrGLFormat::kBGRA8:                return GR_GL_BGRA8;
        case GrGLFormat::kRGB565:               return GR_GL_RGB565;
        case GrGLFormat::kRGBA16F:              return GR_GL_RGBA16F;
        case GrGLFormat::kLUMINANCE16F:         return GR_GL_LUMINANCE16F;
        case GrGLFormat::kR16F:                 return GR_GL_R16F;
        case GrGLFormat::kRGB8:                 return GR_GL_RGB8;
        case GrGLFormat::kRGBX8:                return GR_GL_RGBX8;
        case GrGLFormat::kRG8:                  return GR_GL_RG8;
        case GrGLFormat::kRGB10_A2:             return GR_GL_RGB10_A2;
        case GrGLFormat::kRGBA4:                return GR_GL_RGBA4;
        case GrGLFormat::kSRGB8_ALPHA8:         return GR_GL_SRGB8_ALPHA8;
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8: return GR_GL_COMPRESSED_ETC1_RGB8;
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2: return GR_GL_COMPRESSED_RGB8_ETC2;
        case GrGLFormat::kCOMPRESSED_RGB8_BC1:  return GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case GrGLFormat::kCOMPRESSED_RGBA8_BC1: return GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case GrGLFormat::kR16:                  return GR_GL_R16;
        case GrGLFormat::kRG16:                 return GR_GL_RG16;
        case GrGLFormat::kRGBA16:               return GR_GL_RGBA16;
        case GrGLFormat::kRG16F:                return GR_GL_RG16F;
        case GrGLFormat::kSTENCIL_INDEX8:       return GR_GL_STENCIL_INDEX8;
        case GrGLFormat::kSTENCIL_INDEX16:      return GR_GL_STENCIL_INDEX16;
        case GrGLFormat::kDEPTH24_STENCIL8:     return GR_GL_DEPTH24_STENCIL8;
        case GrGLFormat::kUnknown:              return 0;
    }
    SkUNREACHABLE;
}

static constexpr size_t GrGLFormatBytesPerBlock(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kRGBA8:                return 4;
        case GrGLFormat::kR8:                   return 1;
        case GrGLFormat::kALPHA8:               return 1;
        case GrGLFormat::kLUMINANCE8:           return 1;
        case GrGLFormat::kLUMINANCE8_ALPHA8:    return 2;
        case GrGLFormat::kBGRA8:                return 4;
        case GrGLFormat::kRGB565:               return 2;
        case GrGLFormat::kRGBA16F:              return 8;
        case GrGLFormat::kLUMINANCE16F:         return 2;
        case GrGLFormat::kR16F:                 return 2;
        // We assume the GPU stores this format 4 byte aligned
        case GrGLFormat::kRGB8:                 return 4;
        case GrGLFormat::kRGBX8:                return 4;
        case GrGLFormat::kRG8:                  return 2;
        case GrGLFormat::kRGB10_A2:             return 4;
        case GrGLFormat::kRGBA4:                return 2;
        case GrGLFormat::kSRGB8_ALPHA8:         return 4;
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8: return 8;
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2: return 8;
        case GrGLFormat::kCOMPRESSED_RGB8_BC1:  return 8;
        case GrGLFormat::kCOMPRESSED_RGBA8_BC1: return 8;
        case GrGLFormat::kR16:                  return 2;
        case GrGLFormat::kRG16:                 return 4;
        case GrGLFormat::kRGBA16:               return 8;
        case GrGLFormat::kRG16F:                return 4;
        case GrGLFormat::kSTENCIL_INDEX8:       return 1;
        case GrGLFormat::kSTENCIL_INDEX16:      return 2;
        case GrGLFormat::kDEPTH24_STENCIL8:     return 4;
        case GrGLFormat::kUnknown:              return 0;
    }
    SkUNREACHABLE;
}

static constexpr int GrGLFormatStencilBits(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kSTENCIL_INDEX8:
            return 8;
        case GrGLFormat::kSTENCIL_INDEX16:
            return 16;
        case GrGLFormat::kDEPTH24_STENCIL8:
            return 8;
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
        case GrGLFormat::kCOMPRESSED_RGB8_BC1:
        case GrGLFormat::kCOMPRESSED_RGBA8_BC1:
        case GrGLFormat::kRGBA8:
        case GrGLFormat::kR8:
        case GrGLFormat::kALPHA8:
        case GrGLFormat::kLUMINANCE8:
        case GrGLFormat::kLUMINANCE8_ALPHA8:
        case GrGLFormat::kBGRA8:
        case GrGLFormat::kRGB565:
        case GrGLFormat::kRGBA16F:
        case GrGLFormat::kR16F:
        case GrGLFormat::kLUMINANCE16F:
        case GrGLFormat::kRGB8:
        case GrGLFormat::kRGBX8:
        case GrGLFormat::kRG8:
        case GrGLFormat::kRGB10_A2:
        case GrGLFormat::kRGBA4:
        case GrGLFormat::kSRGB8_ALPHA8:
        case GrGLFormat::kR16:
        case GrGLFormat::kRG16:
        case GrGLFormat::kRGBA16:
        case GrGLFormat::kRG16F:
        case GrGLFormat::kUnknown:
            return 0;
    }
    SkUNREACHABLE;
}

static constexpr bool GrGLFormatIsPackedDepthStencil(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kDEPTH24_STENCIL8:
            return true;
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
        case GrGLFormat::kCOMPRESSED_RGB8_BC1:
        case GrGLFormat::kCOMPRESSED_RGBA8_BC1:
        case GrGLFormat::kRGBA8:
        case GrGLFormat::kR8:
        case GrGLFormat::kALPHA8:
        case GrGLFormat::kLUMINANCE8:
        case GrGLFormat::kLUMINANCE8_ALPHA8:
        case GrGLFormat::kBGRA8:
        case GrGLFormat::kRGB565:
        case GrGLFormat::kRGBA16F:
        case GrGLFormat::kR16F:
        case GrGLFormat::kLUMINANCE16F:
        case GrGLFormat::kRGB8:
        case GrGLFormat::kRGBX8:
        case GrGLFormat::kRG8:
        case GrGLFormat::kRGB10_A2:
        case GrGLFormat::kRGBA4:
        case GrGLFormat::kSRGB8_ALPHA8:
        case GrGLFormat::kR16:
        case GrGLFormat::kRG16:
        case GrGLFormat::kRGBA16:
        case GrGLFormat::kRG16F:
        case GrGLFormat::kSTENCIL_INDEX8:
        case GrGLFormat::kSTENCIL_INDEX16:
        case GrGLFormat::kUnknown:
            return false;
    }
    SkUNREACHABLE;
}

static constexpr bool GrGLFormatIsSRGB(GrGLFormat format) {
    switch (format) {
    case GrGLFormat::kSRGB8_ALPHA8:
        return true;
    case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
    case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
    case GrGLFormat::kCOMPRESSED_RGB8_BC1:
    case GrGLFormat::kCOMPRESSED_RGBA8_BC1:
    case GrGLFormat::kRGBA8:
    case GrGLFormat::kR8:
    case GrGLFormat::kALPHA8:
    case GrGLFormat::kLUMINANCE8:
    case GrGLFormat::kLUMINANCE8_ALPHA8:
    case GrGLFormat::kBGRA8:
    case GrGLFormat::kRGB565:
    case GrGLFormat::kRGBA16F:
    case GrGLFormat::kR16F:
    case GrGLFormat::kLUMINANCE16F:
    case GrGLFormat::kRGB8:
    case GrGLFormat::kRGBX8:
    case GrGLFormat::kRG8:
    case GrGLFormat::kRGB10_A2:
    case GrGLFormat::kRGBA4:
    case GrGLFormat::kR16:
    case GrGLFormat::kRG16:
    case GrGLFormat::kRGBA16:
    case GrGLFormat::kRG16F:
    case GrGLFormat::kSTENCIL_INDEX8:
    case GrGLFormat::kSTENCIL_INDEX16:
    case GrGLFormat::kDEPTH24_STENCIL8:
    case GrGLFormat::kUnknown:
        return false;
    }
    SkUNREACHABLE;
}

#if defined(SK_DEBUG) || defined(GR_TEST_UTILS)
static constexpr const char* GrGLFormatToStr(GrGLenum glFormat) {
    switch (glFormat) {
        case GR_GL_RGBA8:                return "RGBA8";
        case GR_GL_R8:                   return "R8";
        case GR_GL_ALPHA8:               return "ALPHA8";
        case GR_GL_LUMINANCE8:           return "LUMINANCE8";
        case GR_GL_LUMINANCE8_ALPHA8:    return "LUMINANCE8_ALPHA8";
        case GR_GL_BGRA8:                return "BGRA8";
        case GR_GL_RGB565:               return "RGB565";
        case GR_GL_RGBA16F:              return "RGBA16F";
        case GR_GL_LUMINANCE16F:         return "LUMINANCE16F";
        case GR_GL_R16F:                 return "R16F";
        case GR_GL_RGB8:                 return "RGB8";
        case GR_GL_RG8:                  return "RG8";
        case GR_GL_RGB10_A2:             return "RGB10_A2";
        case GR_GL_RGBA4:                return "RGBA4";
        case GR_GL_RGBA32F:              return "RGBA32F";
        case GR_GL_SRGB8_ALPHA8:         return "SRGB8_ALPHA8";
        case GR_GL_COMPRESSED_ETC1_RGB8: return "ETC1";
        case GR_GL_COMPRESSED_RGB8_ETC2: return "ETC2";
        case GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return "RGB8_BC1";
        case GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return "RGBA8_BC1";
        case GR_GL_R16:                  return "R16";
        case GR_GL_RG16:                 return "RG16";
        case GR_GL_RGBA16:               return "RGBA16";
        case GR_GL_RG16F:                return "RG16F";
        case GR_GL_STENCIL_INDEX8:       return "STENCIL_INDEX8";
        case GR_GL_STENCIL_INDEX16:      return "STENCIL_INDEX16";
        case GR_GL_DEPTH24_STENCIL8:     return "DEPTH24_STENCIL8";

        default:                         return "Unknown";
    }
}
#endif

GrGLenum GrToGLStencilFunc(GrStencilTest test);

/**
 * Returns true if the format is compressed.
 */
bool GrGLFormatIsCompressed(GrGLFormat);

#endif
