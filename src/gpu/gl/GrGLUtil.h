/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLUtil_DEFINED
#define GrGLUtil_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/gl/GrGLDefines.h"

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

#define GR_GL_INVALID_VER GR_GL_VER(0, 0)
#define GR_GLSL_INVALID_VER GR_GLSL_VER(0, 0)
#define GR_GL_DRIVER_UNKNOWN_VER GR_GL_DRIVER_VER(0, 0, 0)

/**
 * The Vendor and Renderer enum values are lazily updated as required.
 */
enum GrGLVendor {
    kARM_GrGLVendor,
    kGoogle_GrGLVendor,
    kImagination_GrGLVendor,
    kIntel_GrGLVendor,
    kQualcomm_GrGLVendor,
    kNVIDIA_GrGLVendor,
    kATI_GrGLVendor,

    kOther_GrGLVendor
};

enum GrGLRenderer {
    kTegra_PreK1_GrGLRenderer,  // Legacy Tegra architecture (pre-K1).
    kTegra_GrGLRenderer,  // Tegra with the same architecture as NVIDIA desktop GPUs (K1+).
    kPowerVR54x_GrGLRenderer,
    kPowerVRRogue_GrGLRenderer,
    kAdreno3xx_GrGLRenderer,
    kAdreno430_GrGLRenderer,
    kAdreno4xx_other_GrGLRenderer,
    kAdreno5xx_GrGLRenderer,
    kOSMesa_GrGLRenderer,
    kGoogleSwiftShader_GrGLRenderer,

    /** Intel GPU families, ordered by generation **/
    // 6th gen
    kIntelSandyBridge_GrGLRenderer,

    // 7th gen
    kIntelIvyBridge_GrGLRenderer,
    kIntelValleyView_GrGLRenderer, // aka BayTrail
    kIntelHaswell_GrGLRenderer,

    // 8th gen
    kIntelCherryView_GrGLRenderer, // aka Braswell
    kIntelBroadwell_GrGLRenderer,

    // 9th gen
    kIntelApolloLake_GrGLRenderer,
    kIntelSkyLake_GrGLRenderer,
    kIntelGeminiLake_GrGLRenderer,
    kIntelKabyLake_GrGLRenderer,
    kIntelCoffeeLake_GrGLRenderer,

    // 11th gen
    kIntelIceLake_GrGLRenderer,

    kGalliumLLVM_GrGLRenderer,
    kMali4xx_GrGLRenderer,
    /** T-6xx, T-7xx, or T-8xx */
    kMaliT_GrGLRenderer,
    kANGLE_GrGLRenderer,

    kAMDRadeonHD7xxx_GrGLRenderer,  // AMD Radeon HD 7000 Series
    kAMDRadeonR9M4xx_GrGLRenderer,  // AMD Radeon R9 M400 Series

    kOther_GrGLRenderer
};

enum GrGLDriver {
    kMesa_GrGLDriver,
    kChromium_GrGLDriver,
    kNVIDIA_GrGLDriver,
    kIntel_GrGLDriver,
    kANGLE_GrGLDriver,
    kSwiftShader_GrGLDriver,
    kQualcomm_GrGLDriver,
    kUnknown_GrGLDriver
};

enum class GrGLANGLEBackend {
    kUnknown,
    kD3D9,
    kD3D11,
    kOpenGL
};

enum class GrGLANGLEVendor {
    kUnknown,
    kIntel
};

enum class GrGLANGLERenderer {
    kUnknown,
    kSandyBridge,
    kIvyBridge,
    kSkylake
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

/**
 * Helpers for glGetString()
 */

// these variants assume caller already has a string from glGetString()
GrGLVersion GrGLGetVersionFromString(const char* versionString);
GrGLStandard GrGLGetStandardInUseFromString(const char* versionString);
GrGLSLVersion GrGLGetGLSLVersionFromString(const char* versionString);
GrGLVendor GrGLGetVendorFromString(const char* vendorString);
GrGLRenderer GrGLGetRendererFromStrings(const char* rendererString, const GrGLExtensions&);
void GrGLGetANGLEInfoFromString(const char* rendererString, GrGLANGLEBackend*,
                                GrGLANGLEVendor*, GrGLANGLERenderer*);

void GrGLGetDriverInfo(GrGLStandard standard,
                       GrGLVendor vendor,
                       const char* rendererString,
                       const char* versionString,
                       GrGLDriver* outDriver,
                       GrGLDriverVersion* outVersion);

// these variants call glGetString()
GrGLVersion GrGLGetVersion(const GrGLInterface*);
GrGLSLVersion GrGLGetGLSLVersion(const GrGLInterface*);
GrGLVendor GrGLGetVendor(const GrGLInterface*);
GrGLRenderer GrGLGetRenderer(const GrGLInterface*);

/**
 * Helpers for glGetError()
 */

void GrGLCheckErr(const GrGLInterface* gl,
                  const char* location,
                  const char* call);

void GrGLClearErr(const GrGLInterface* gl);

////////////////////////////////////////////////////////////////////////////////

/**
 * Macros for using GrGLInterface to make GL calls
 */

// internal macro to conditionally call glGetError based on compile-time and
// run-time flags.
#if GR_GL_CHECK_ERROR
    extern bool gCheckErrorGL;
    #define GR_GL_CHECK_ERROR_IMPL(IFACE, X)                    \
        if (gCheckErrorGL)                                      \
            GrGLCheckErr(IFACE, GR_FILE_AND_LINE_STR, #X)
#else
    #define GR_GL_CHECK_ERROR_IMPL(IFACE, X)
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

// call glGetError without doing a redundant error check or logging.
#define GR_GL_GET_ERROR(IFACE) (IFACE)->fFunctions.fGetError()

static constexpr GrGLFormat GrGLFormatFromGLEnum(GrGLenum glFormat) {
    switch (glFormat) {
        case GR_GL_RGBA8:                return GrGLFormat::kRGBA8;
        case GR_GL_R8:                   return GrGLFormat::kR8;
        case GR_GL_ALPHA8:               return GrGLFormat::kALPHA8;
        case GR_GL_LUMINANCE8:           return GrGLFormat::kLUMINANCE8;
        case GR_GL_BGRA8:                return GrGLFormat::kBGRA8;
        case GR_GL_RGB565:               return GrGLFormat::kRGB565;
        case GR_GL_RGBA16F:              return GrGLFormat::kRGBA16F;
        case GR_GL_LUMINANCE16F:         return GrGLFormat::kLUMINANCE16F;
        case GR_GL_R16F:                 return GrGLFormat::kR16F;
        case GR_GL_RGB8:                 return GrGLFormat::kRGB8;
        case GR_GL_RG8:                  return GrGLFormat::kRG8;
        case GR_GL_RGB10_A2:             return GrGLFormat::kRGB10_A2;
        case GR_GL_RGBA4:                return GrGLFormat::kRGBA4;
        case GR_GL_RGBA32F:              return GrGLFormat::kRGBA32F;
        case GR_GL_SRGB8_ALPHA8:         return GrGLFormat::kSRGB8_ALPHA8;
        case GR_GL_COMPRESSED_RGB8_ETC2: return GrGLFormat::kCOMPRESSED_RGB8_ETC2;
        case GR_GL_COMPRESSED_ETC1_RGB8: return GrGLFormat::kCOMPRESSED_ETC1_RGB8;
        case GR_GL_R16:                  return GrGLFormat::kR16;
        case GR_GL_RG16:                 return GrGLFormat::kRG16;
        case GR_GL_RGBA16:               return GrGLFormat::kRGBA16;
        case GR_GL_RG16F:                return GrGLFormat::kRG16F;

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
        case GrGLFormat::kBGRA8:                return GR_GL_BGRA8;
        case GrGLFormat::kRGB565:               return GR_GL_RGB565;
        case GrGLFormat::kRGBA16F:              return GR_GL_RGBA16F;
        case GrGLFormat::kLUMINANCE16F:         return GR_GL_LUMINANCE16F;
        case GrGLFormat::kR16F:                 return GR_GL_R16F;
        case GrGLFormat::kRGB8:                 return GR_GL_RGB8;
        case GrGLFormat::kRG8:                  return GR_GL_RG8;
        case GrGLFormat::kRGB10_A2:             return GR_GL_RGB10_A2;
        case GrGLFormat::kRGBA4:                return GR_GL_RGBA4;
        case GrGLFormat::kRGBA32F:              return GR_GL_RGBA32F;
        case GrGLFormat::kSRGB8_ALPHA8:         return GR_GL_SRGB8_ALPHA8;
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2: return GR_GL_COMPRESSED_RGB8_ETC2;
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8: return GR_GL_COMPRESSED_ETC1_RGB8;
        case GrGLFormat::kR16:                  return GR_GL_R16;
        case GrGLFormat::kRG16:                 return GR_GL_RG16;
        case GrGLFormat::kRGBA16:               return GR_GL_RGBA16;
        case GrGLFormat::kRG16F:                return GR_GL_RG16F;
        case GrGLFormat::kUnknown:              return 0;
    }
    SkUNREACHABLE;
}

#if GR_TEST_UTILS
static constexpr const char* GrGLFormatToStr(GrGLenum glFormat) {
    switch (glFormat) {
        case GR_GL_RGBA8:                return "RGBA8";
        case GR_GL_R8:                   return "R8";
        case GR_GL_ALPHA8:               return "ALPHA8";
        case GR_GL_LUMINANCE8:           return "LUMINANCE8";
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
        case GR_GL_COMPRESSED_RGB8_ETC2: return "ETC2";
        case GR_GL_COMPRESSED_ETC1_RGB8: return "ETC1";
        case GR_GL_R16:                  return "R16";
        case GR_GL_RG16:                 return "RG16";
        case GR_GL_RGBA16:               return "RGBA16";
        case GR_GL_RG16F:                return "RG16F";

        default:                         return "Unknown";
    }
}
#endif

GrGLenum GrToGLStencilFunc(GrStencilTest test);

/**
 * Returns true if the format is compressed.
 */
bool GrGLFormatIsCompressed(GrGLFormat);

/**
 * Maps a GrGLFormat into the CompressionType enum if appropriate.
 */
bool GrGLFormatToCompressionType(GrGLFormat, SkImage::CompressionType*);

size_t GrGLBytesPerFormat(GrGLFormat);

#endif
