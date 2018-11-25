/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLUtil.h"
#include "GrTypesPriv.h"
#include "SkMatrix.h"
#include <stdio.h>

void GrGLClearErr(const GrGLInterface* gl) {
    while (GR_GL_NO_ERROR != gl->fFunctions.fGetError()) {}
}

namespace {
const char *get_error_string(uint32_t err) {
    switch (err) {
    case GR_GL_NO_ERROR:
        return "";
    case GR_GL_INVALID_ENUM:
        return "Invalid Enum";
    case GR_GL_INVALID_VALUE:
        return "Invalid Value";
    case GR_GL_INVALID_OPERATION:
        return "Invalid Operation";
    case GR_GL_OUT_OF_MEMORY:
        return "Out of Memory";
    case GR_GL_CONTEXT_LOST:
        return "Context Lost";
    }
    return "Unknown";
}
}

void GrGLCheckErr(const GrGLInterface* gl,
                  const char* location,
                  const char* call) {
    uint32_t err = GR_GL_GET_ERROR(gl);
    if (GR_GL_NO_ERROR != err) {
        SkDebugf("---- glGetError 0x%x(%s)", err, get_error_string(err));
        if (location) {
            SkDebugf(" at\n\t%s", location);
        }
        if (call) {
            SkDebugf("\n\t\t%s", call);
        }
        SkDebugf("\n");
    }
}

///////////////////////////////////////////////////////////////////////////////

#if GR_GL_LOG_CALLS
    bool gLogCallsGL = !!(GR_GL_LOG_CALLS_START);
#endif

#if GR_GL_CHECK_ERROR
    bool gCheckErrorGL = !!(GR_GL_CHECK_ERROR_START);
#endif

///////////////////////////////////////////////////////////////////////////////

GrGLStandard GrGLGetStandardInUseFromString(const char* versionString) {
    if (nullptr == versionString) {
        SkDebugf("nullptr GL version string.");
        return kNone_GrGLStandard;
    }

    int major, minor;

    // check for desktop
    int n = sscanf(versionString, "%d.%d", &major, &minor);
    if (2 == n) {
        return kGL_GrGLStandard;
    }

    // check for ES 1
    char profile[2];
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile+1, &major, &minor);
    if (4 == n) {
        // we no longer support ES1.
        return kNone_GrGLStandard;
    }

    // check for ES2
    n = sscanf(versionString, "OpenGL ES %d.%d", &major, &minor);
    if (2 == n) {
        return kGLES_GrGLStandard;
    }
    return kNone_GrGLStandard;
}

void GrGLGetDriverInfo(GrGLStandard standard,
                       GrGLVendor vendor,
                       const char* rendererString,
                       const char* versionString,
                       GrGLDriver* outDriver,
                       GrGLDriverVersion* outVersion) {
    int major, minor, rev, driverMajor, driverMinor;

    *outDriver = kUnknown_GrGLDriver;
    *outVersion = GR_GL_DRIVER_UNKNOWN_VER;
    // These null checks are for test GL contexts that return nullptr in their
    // glGetString implementation.
    if (!rendererString) {
        rendererString = "";
    }
    if (!versionString) {
        versionString = "";
    }

    static const char kChromium[] = "Chromium";
    char suffix[SK_ARRAY_COUNT(kChromium)];
    if (0 == strcmp(rendererString, kChromium) ||
        (3 == sscanf(versionString, "OpenGL ES %d.%d %8s", &major, &minor, suffix) &&
         0 == strcmp(kChromium, suffix))) {
        *outDriver = kChromium_GrGLDriver;
        return;
    }

    if (standard == kGL_GrGLStandard) {
        if (kNVIDIA_GrGLVendor == vendor) {
            *outDriver = kNVIDIA_GrGLDriver;
            int n = sscanf(versionString, "%d.%d.%d NVIDIA %d.%d",
                           &major, &minor, &rev, &driverMajor, &driverMinor);
            // Some older NVIDIA drivers don't report the driver version.
            if (5 == n) {
                *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor);
            }
            return;
        }
        int n = sscanf(versionString, "%d.%d Mesa %d.%d",
                       &major, &minor, &driverMajor, &driverMinor);
        if (4 != n) {
            n = sscanf(versionString, "%d.%d (Core Profile) Mesa %d.%d",
                       &major, &minor, &driverMajor, &driverMinor);
        }
        if (4 == n) {
            *outDriver = kMesa_GrGLDriver;
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor);
            return;
        }
    }
    else {
        if (kNVIDIA_GrGLVendor == vendor) {
            *outDriver = kNVIDIA_GrGLDriver;
            int n = sscanf(versionString, "OpenGL ES %d.%d NVIDIA %d.%d",
                           &major, &minor, &driverMajor, &driverMinor);
            // Some older NVIDIA drivers don't report the driver version.
            if (4 == n) {
                *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor);
            }
            return;
        }

        int n = sscanf(versionString, "OpenGL ES %d.%d Mesa %d.%d",
                       &major, &minor, &driverMajor, &driverMinor);
        if (4 == n) {
            *outDriver = kMesa_GrGLDriver;
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor);
            return;
        }
        if (0 == strncmp("ANGLE", rendererString, 5)) {
            *outDriver = kANGLE_GrGLDriver;
            n = sscanf(versionString, "OpenGL ES %d.%d (ANGLE %d.%d", &major, &minor, &driverMajor,
                                                                      &driverMinor);
            if (4 == n) {
                *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor);
            }
            return;
        }
    }

    if (kIntel_GrGLVendor == vendor) {
        // We presume we're on the Intel driver since it hasn't identified itself as Mesa.
        *outDriver = kIntel_GrGLDriver;
    }

    if (kQualcomm_GrGLVendor == vendor) {
        *outDriver = kQualcomm_GrGLDriver;
        int n = sscanf(versionString, "OpenGL ES %d.%d V@%d.%d", &major, &minor, &driverMajor,
                       &driverMinor);
        if (4 == n) {
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor);
        }
        return;
    }
}

GrGLVersion GrGLGetVersionFromString(const char* versionString) {
    if (nullptr == versionString) {
        SkDebugf("nullptr GL version string.");
        return GR_GL_INVALID_VER;
    }

    int major, minor;

    // check for mesa
    int mesaMajor, mesaMinor;
    int n = sscanf(versionString, "%d.%d Mesa %d.%d", &major, &minor, &mesaMajor, &mesaMinor);
    if (4 == n) {
        return GR_GL_VER(major, minor);
    }

    n = sscanf(versionString, "%d.%d", &major, &minor);
    if (2 == n) {
        return GR_GL_VER(major, minor);
    }

    char profile[2];
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile+1,
               &major, &minor);
    if (4 == n) {
        return GR_GL_VER(major, minor);
    }

    n = sscanf(versionString, "OpenGL ES %d.%d", &major, &minor);
    if (2 == n) {
        return GR_GL_VER(major, minor);
    }

    return GR_GL_INVALID_VER;
}

GrGLSLVersion GrGLGetGLSLVersionFromString(const char* versionString) {
    if (nullptr == versionString) {
        SkDebugf("nullptr GLSL version string.");
        return GR_GLSL_INVALID_VER;
    }

    int major, minor;

    int n = sscanf(versionString, "%d.%d", &major, &minor);
    if (2 == n) {
        return GR_GLSL_VER(major, minor);
    }

    n = sscanf(versionString, "OpenGL ES GLSL ES %d.%d", &major, &minor);
    if (2 == n) {
        return GR_GLSL_VER(major, minor);
    }

#ifdef SK_BUILD_FOR_ANDROID
    // android hack until the gpu vender updates their drivers
    n = sscanf(versionString, "OpenGL ES GLSL %d.%d", &major, &minor);
    if (2 == n) {
        return GR_GLSL_VER(major, minor);
    }
#endif

    return GR_GLSL_INVALID_VER;
}

GrGLVendor GrGLGetVendorFromString(const char* vendorString) {
    if (vendorString) {
        if (0 == strcmp(vendorString, "ARM")) {
            return kARM_GrGLVendor;
        }
        if (0 == strcmp(vendorString, "Imagination Technologies")) {
            return kImagination_GrGLVendor;
        }
        if (0 == strncmp(vendorString, "Intel ", 6) || 0 == strcmp(vendorString, "Intel")) {
            return kIntel_GrGLVendor;
        }
        if (0 == strcmp(vendorString, "Qualcomm")) {
            return kQualcomm_GrGLVendor;
        }
        if (0 == strcmp(vendorString, "NVIDIA Corporation")) {
            return kNVIDIA_GrGLVendor;
        }
        if (0 == strcmp(vendorString, "ATI Technologies Inc.")) {
            return kATI_GrGLVendor;
        }
    }
    return kOther_GrGLVendor;
}

static bool is_renderer_angle(const char* rendererString) {
    static constexpr char kHeader[] = "ANGLE ";
    static constexpr size_t kHeaderLength = SK_ARRAY_COUNT(kHeader) - 1;
    return 0 == strncmp(rendererString, kHeader, kHeaderLength);
}

GrGLRenderer GrGLGetRendererFromString(const char* rendererString) {
    if (rendererString) {
        if (0 == strcmp(rendererString, "NVIDIA Tegra 3")) {
            return kTegra3_GrGLRenderer;
        } else if (0 == strcmp(rendererString, "NVIDIA Tegra")) {
            return kTegra2_GrGLRenderer;
        }
        int lastDigit;
        int n = sscanf(rendererString, "PowerVR SGX 54%d", &lastDigit);
        if (1 == n && lastDigit >= 0 && lastDigit <= 9) {
            return kPowerVR54x_GrGLRenderer;
        }
        // certain iOS devices also use PowerVR54x GPUs
        static const char kAppleA4Str[] = "Apple A4";
        static const char kAppleA5Str[] = "Apple A5";
        static const char kAppleA6Str[] = "Apple A6";
        if (0 == strncmp(rendererString, kAppleA4Str,
                         SK_ARRAY_COUNT(kAppleA4Str)-1) ||
            0 == strncmp(rendererString, kAppleA5Str,
                         SK_ARRAY_COUNT(kAppleA5Str)-1) ||
            0 == strncmp(rendererString, kAppleA6Str,
                         SK_ARRAY_COUNT(kAppleA6Str)-1)) {
            return kPowerVR54x_GrGLRenderer;
        }
        static const char kPowerVRRogueStr[] = "PowerVR Rogue";
        static const char kAppleA7Str[] = "Apple A7";
        static const char kAppleA8Str[] = "Apple A8";
        if (0 == strncmp(rendererString, kPowerVRRogueStr,
                         SK_ARRAY_COUNT(kPowerVRRogueStr)-1) ||
            0 == strncmp(rendererString, kAppleA7Str,
                         SK_ARRAY_COUNT(kAppleA7Str)-1) ||
            0 == strncmp(rendererString, kAppleA8Str,
                         SK_ARRAY_COUNT(kAppleA8Str)-1)) {
            return kPowerVRRogue_GrGLRenderer;
        }
        int adrenoNumber;
        n = sscanf(rendererString, "Adreno (TM) %d", &adrenoNumber);
        if (1 == n) {
            if (adrenoNumber >= 300) {
                if (adrenoNumber < 400) {
                    return kAdreno3xx_GrGLRenderer;
                }
                if (adrenoNumber < 500) {
                    return kAdreno4xx_GrGLRenderer;
                }
                if (adrenoNumber < 600) {
                    return kAdreno5xx_GrGLRenderer;
                }
            }
        }
        if (0 == strcmp("Intel Iris Pro OpenGL Engine", rendererString)) {
            return kIntelIrisPro_GrGLRenderer;
        }

        int intelNumber;
        n = sscanf(rendererString, "Intel(R) Iris(TM) Graphics %d", &intelNumber);
        if (1 != n) {
            n = sscanf(rendererString, "Intel(R) HD Graphics %d", &intelNumber);
        }
        if (1 == n) {
            if (intelNumber >= 4000 && intelNumber < 5000) {
                return kIntel4xxx_GrGLRenderer;
            }
            if (intelNumber >= 6000 && intelNumber < 7000) {
                return kIntel6xxx_GrGLRenderer;
            }
        }

        // The AMD string can have a somewhat arbitrary preamble (see skbug.com/7195)
        if (const char* amdString = strstr(rendererString, "Radeon")) {
            char amdGeneration, amdTier, amdRevision;
            n = sscanf(amdString, "Radeon (TM) R9 M%c%c%c",
                                       &amdGeneration, &amdTier, &amdRevision);
            if (3 == n) {
                if ('4' == amdGeneration) {
                    return kAMDRadeonR9M4xx_GrGLRenderer;
                }
            }

            char amd0, amd1, amd2;
            n = sscanf(amdString, "Radeon HD 7%c%c%c Series", &amd0, &amd1, &amd2);
            if (3 == n) {
                return kAMDRadeonHD7xxx_GrGLRenderer;
            }
        }

        if (0 == strcmp("Mesa Offscreen", rendererString)) {
            return kOSMesa_GrGLRenderer;
        }
        if (strstr(rendererString, "llvmpipe")) {
            return kGalliumLLVM_GrGLRenderer;
        }
        static const char kMaliTStr[] = "Mali-T";
        if (0 == strncmp(rendererString, kMaliTStr, SK_ARRAY_COUNT(kMaliTStr) - 1)) {
            return kMaliT_GrGLRenderer;
        }
        if (is_renderer_angle(rendererString)) {
            return kANGLE_GrGLRenderer;
        }
    }
    return kOther_GrGLRenderer;
}

void GrGLGetANGLEInfoFromString(const char* rendererString, GrGLANGLEBackend* backend,
                                GrGLANGLEVendor* vendor, GrGLANGLERenderer* renderer) {
    *backend = GrGLANGLEBackend::kUnknown;
    *vendor = GrGLANGLEVendor::kUnknown;
    *renderer = GrGLANGLERenderer::kUnknown;
    if (!is_renderer_angle(rendererString)) {
        return;
    }
    if (strstr(rendererString, "Intel")) {
        *vendor = GrGLANGLEVendor::kIntel;

        const char* modelStr;
        int modelNumber;
        if ((modelStr = strstr(rendererString, "HD Graphics")) &&
            (1 == sscanf(modelStr, "HD Graphics %i", &modelNumber) ||
             1 == sscanf(modelStr, "HD Graphics P%i", &modelNumber))) {
            switch (modelNumber) {
                case 4000:
                case 2500:
                    *renderer = GrGLANGLERenderer::kIvyBridge;
                    break;
                case 510:
                case 515:
                case 520:
                case 530:
                    *renderer = GrGLANGLERenderer::kSkylake;
                    break;
            }
        } else if ((modelStr = strstr(rendererString, "Iris")) &&
                   (1 == sscanf(modelStr, "Iris(TM) Graphics %i", &modelNumber) ||
                    1 == sscanf(modelStr, "Iris(TM) Pro Graphics %i", &modelNumber) ||
                    1 == sscanf(modelStr, "Iris(TM) Pro Graphics P%i", &modelNumber))) {
            switch (modelNumber) {
                case 540:
                case 550:
                case 555:
                case 580:
                    *renderer = GrGLANGLERenderer::kSkylake;
                    break;
            }
        }
    }
    if (strstr(rendererString, "Direct3D11")) {
        *backend = GrGLANGLEBackend::kD3D11;
    } else if (strstr(rendererString, "Direct3D9")) {
        *backend = GrGLANGLEBackend::kD3D9;
    } else if (strstr(rendererString, "OpenGL")) {
        *backend = GrGLANGLEBackend::kOpenGL;
    }
}

GrGLVersion GrGLGetVersion(const GrGLInterface* gl) {
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_VERSION));
    return GrGLGetVersionFromString((const char*) v);
}

GrGLSLVersion GrGLGetGLSLVersion(const GrGLInterface* gl) {
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
    return GrGLGetGLSLVersionFromString((const char*) v);
}

GrGLVendor GrGLGetVendor(const GrGLInterface* gl) {
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_VENDOR));
    return GrGLGetVendorFromString((const char*) v);
}

GrGLRenderer GrGLGetRenderer(const GrGLInterface* gl) {
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_RENDERER));
    return GrGLGetRendererFromString((const char*) v);
}

GrGLenum GrToGLStencilFunc(GrStencilTest test) {
    static const GrGLenum gTable[kGrStencilTestCount] = {
        GR_GL_ALWAYS,           // kAlways
        GR_GL_NEVER,            // kNever
        GR_GL_GREATER,          // kGreater
        GR_GL_GEQUAL,           // kGEqual
        GR_GL_LESS,             // kLess
        GR_GL_LEQUAL,           // kLEqual
        GR_GL_EQUAL,            // kEqual
        GR_GL_NOTEQUAL,         // kNotEqual
    };
    GR_STATIC_ASSERT(0 == (int)GrStencilTest::kAlways);
    GR_STATIC_ASSERT(1 == (int)GrStencilTest::kNever);
    GR_STATIC_ASSERT(2 == (int)GrStencilTest::kGreater);
    GR_STATIC_ASSERT(3 == (int)GrStencilTest::kGEqual);
    GR_STATIC_ASSERT(4 == (int)GrStencilTest::kLess);
    GR_STATIC_ASSERT(5 == (int)GrStencilTest::kLEqual);
    GR_STATIC_ASSERT(6 == (int)GrStencilTest::kEqual);
    GR_STATIC_ASSERT(7 == (int)GrStencilTest::kNotEqual);
    SkASSERT(test < (GrStencilTest)kGrStencilTestCount);

    return gTable[(int)test];
}

GrPixelConfig GrGLSizedFormatToPixelConfig(GrGLenum sizedFormat) {
    switch (sizedFormat) {
        case GR_GL_R8:
            return kAlpha_8_as_Red_GrPixelConfig;
        case GR_GL_ALPHA8:
            return kAlpha_8_as_Alpha_GrPixelConfig;
        case GR_GL_RGBA8:
            return kRGBA_8888_GrPixelConfig;
        case GR_GL_BGRA8:
            return kBGRA_8888_GrPixelConfig;
        case GR_GL_SRGB8_ALPHA8:
            return kSRGBA_8888_GrPixelConfig;
        case GR_GL_RGBA8I:
            return kRGBA_8888_sint_GrPixelConfig;
        case GR_GL_RGB565:
            return kRGB_565_GrPixelConfig;
        case GR_GL_RGB5:
            return kRGB_565_GrPixelConfig;
        case GR_GL_RGBA4:
            return kRGBA_4444_GrPixelConfig;
        case GR_GL_LUMINANCE8:
            return kGray_8_GrPixelConfig;
        case GR_GL_RGBA32F:
            return kRGBA_float_GrPixelConfig;
        case GR_GL_RG32F:
            return kRG_float_GrPixelConfig;
        case GR_GL_R16F:
            return kAlpha_half_as_Red_GrPixelConfig;
        case GR_GL_RGBA16F:
            return kRGBA_half_GrPixelConfig;
        default:
            return kUnknown_GrPixelConfig;
    }
}

