/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkMatrix.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/gl/GrGLUtil.h"
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

    // WebGL might look like "OpenGL ES 2.0 (WebGL 1.0 (OpenGL ES 2.0 Chromium))"
    int esMajor, esMinor;
    n = sscanf(versionString, "OpenGL ES %d.%d (WebGL %d.%d", &esMajor, &esMinor, &major, &minor);
    if (4 == n) {
        return kWebGL_GrGLStandard;
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
    int major, minor, rev, driverMajor, driverMinor, driverPoint;

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

    if (GR_IS_GR_GL(standard)) {
        if (kNVIDIA_GrGLVendor == vendor) {
            *outDriver = kNVIDIA_GrGLDriver;
            int n = sscanf(versionString, "%d.%d.%d NVIDIA %d.%d",
                           &major, &minor, &rev, &driverMajor, &driverMinor);
            // Some older NVIDIA drivers don't report the driver version.
            if (5 == n) {
                *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
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
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            return;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (kNVIDIA_GrGLVendor == vendor) {
            *outDriver = kNVIDIA_GrGLDriver;
            int n = sscanf(versionString, "OpenGL ES %d.%d NVIDIA %d.%d",
                           &major, &minor, &driverMajor, &driverMinor);
            // Some older NVIDIA drivers don't report the driver version.
            if (4 == n) {
                *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
            return;
        }

        int n = sscanf(versionString, "OpenGL ES %d.%d Mesa %d.%d",
                       &major, &minor, &driverMajor, &driverMinor);
        if (4 == n) {
            *outDriver = kMesa_GrGLDriver;
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            return;
        }
        if (0 == strncmp("ANGLE", rendererString, 5)) {
            *outDriver = kANGLE_GrGLDriver;
            n = sscanf(versionString, "OpenGL ES %d.%d (ANGLE %d.%d", &major, &minor, &driverMajor,
                                                                      &driverMinor);
            if (4 == n) {
                *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
            return;
        }
    }

    if (kGoogle_GrGLVendor == vendor) {
        // Swiftshader is the only Google vendor at the moment
        *outDriver = kSwiftShader_GrGLDriver;

        // Swiftshader has a strange version string: w.x.y.z  Going to arbitrarily ignore
        // y and assume w,x and z are major, minor, point.
        // As of writing, version is 4.0.0.6
        int n = sscanf(versionString, "OpenGL ES %d.%d SwiftShader %d.%d.0.%d", &major, &minor,
                       &driverMajor, &driverMinor, &driverPoint);
        if (5 == n) {
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, driverPoint);
        }
        return;
    }

    if (kIntel_GrGLVendor == vendor) {
        // We presume we're on the Intel driver since it hasn't identified itself as Mesa.
        *outDriver = kIntel_GrGLDriver;

        //This is how the macOS version strings are structured. This might be different on different
        // OSes.
        int n = sscanf(versionString, "%d.%d INTEL-%d.%d.%d", &major, &minor, &driverMajor,
                       &driverMinor, &driverPoint);
        if (5 == n) {
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, driverPoint);
        }
    }

    if (kQualcomm_GrGLVendor == vendor) {
        *outDriver = kQualcomm_GrGLDriver;
        int n = sscanf(versionString, "OpenGL ES %d.%d V@%d.%d", &major, &minor, &driverMajor,
                       &driverMinor);
        if (4 == n) {
            *outVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
        }
        return;
    }
    static constexpr char kEmulatorPrefix[] = "Android Emulator OpenGL ES Translator";
    if (0 == strncmp(kEmulatorPrefix, rendererString, strlen(kEmulatorPrefix))) {
        *outDriver = kAndroidEmulator_GrGLDriver;
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

    // WebGL might look like "OpenGL ES 2.0 (WebGL 1.0 (OpenGL ES 2.0 Chromium))"
    int esMajor, esMinor;
    n = sscanf(versionString, "OpenGL ES %d.%d (WebGL %d.%d", &esMajor, &esMinor, &major, &minor);
    if (4 == n) {
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
        if (0 == strcmp(vendorString, "Google Inc.")) {
            return kGoogle_GrGLVendor;
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
    return rendererString && 0 == strncmp(rendererString, kHeader, kHeaderLength);
}

GrGLRenderer GrGLGetRendererFromStrings(const char* rendererString,
                                        const GrGLExtensions& extensions) {
    if (rendererString) {
        static const char kTegraStr[] = "NVIDIA Tegra";
        if (0 == strncmp(rendererString, kTegraStr, SK_ARRAY_COUNT(kTegraStr) - 1)) {
            // Tegra strings are not very descriptive. We distinguish between the modern and legacy
            // architectures by the presence of NV_path_rendering.
            return extensions.has("GL_NV_path_rendering") ? kTegra_GrGLRenderer
                                                          : kTegra_PreK1_GrGLRenderer;
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
                    return adrenoNumber >= 430
                            ? kAdreno430_GrGLRenderer : kAdreno4xx_other_GrGLRenderer;
                }
                if (adrenoNumber < 600) {
                    return kAdreno5xx_GrGLRenderer;
                }
                if (adrenoNumber == 615) {
                    return kAdreno615_GrGLRenderer;
                }
                if (adrenoNumber == 630) {
                    return kAdreno630_GrGLRenderer;
                }
            }
        }
        if (0 == strcmp("Google SwiftShader", rendererString)) {
            return kGoogleSwiftShader_GrGLRenderer;
        }

        if (const char* intelString = strstr(rendererString, "Intel")) {
            // These generic strings seem to always come from Haswell: Iris 5100 or Iris Pro 5200
            if (0 == strcmp("Intel Iris OpenGL Engine", intelString) ||
                0 == strcmp("Intel Iris Pro OpenGL Engine", intelString)) {
                return kIntelHaswell_GrGLRenderer;
            }
            if (strstr(intelString, "Sandybridge")) {
                return kIntelSandyBridge_GrGLRenderer;
            }
            if (strstr(intelString, "Bay Trail")) {
                return kIntelValleyView_GrGLRenderer;
            }
            // There are many possible intervening strings here:
            // 'Intel(R)' is a common prefix
            // 'Iris' may appear, followed by '(R)' or '(TM)'
            // 'Iris' can then be followed by 'Graphics', 'Pro Graphics', or 'Plus Graphics'
            // If 'Iris' isn't there, we might have 'HD Graphics' or 'UHD Graphics'
            //
            // In all cases, though, we end with 'Graphics ', an optional 'P', and a number,
            // so just skip to that and handle two cases:
            if (const char* intelGfxString = strstr(intelString, "Graphics")) {
                int intelNumber;
                if (sscanf(intelGfxString, "Graphics %d", &intelNumber) ||
                    sscanf(intelGfxString, "Graphics P%d", &intelNumber)) {

                    if (intelNumber == 2000 || intelNumber == 3000) {
                        return kIntelSandyBridge_GrGLRenderer;
                    }
                    if (intelNumber == 2500 || intelNumber == 4000) {
                        return kIntelIvyBridge_GrGLRenderer;
                    }
                    if (intelNumber >= 4200 && intelNumber <= 5200) {
                        return kIntelHaswell_GrGLRenderer;
                    }
                    if (intelNumber >= 400 && intelNumber <= 405) {
                        return kIntelCherryView_GrGLRenderer;
                    }
                    if (intelNumber >= 5300 && intelNumber <= 6300) {
                        return kIntelBroadwell_GrGLRenderer;
                    }
                    if (intelNumber >= 500 && intelNumber <= 505) {
                        return kIntelApolloLake_GrGLRenderer;
                    }
                    if (intelNumber >= 510 && intelNumber <= 580) {
                        return kIntelSkyLake_GrGLRenderer;
                    }
                    if (intelNumber >= 600 && intelNumber <= 605) {
                        return kIntelGeminiLake_GrGLRenderer;
                    }
                    // 610 and 630 are reused from KabyLake to CoffeeLake. The CoffeeLake variants
                    // are "UHD Graphics", while the KabyLake ones are "HD Graphics"
                    if (intelNumber == 610 || intelNumber == 630) {
                        return strstr(intelString, "UHD") ? kIntelCoffeeLake_GrGLRenderer
                                                          : kIntelKabyLake_GrGLRenderer;
                    }
                    if (intelNumber >= 610 && intelNumber <= 650) {
                        return kIntelKabyLake_GrGLRenderer;
                    }
                    if (intelNumber == 655) {
                        return kIntelCoffeeLake_GrGLRenderer;
                    }
                    if (intelNumber >= 910 && intelNumber <= 950) {
                        return kIntelIceLake_GrGLRenderer;
                    }
                }
            }
        }

        // The AMD string can have a somewhat arbitrary preamble (see skbug.com/7195)
        static constexpr char kRadeonStr[] = "Radeon ";
        if (const char* amdString = strstr(rendererString, kRadeonStr)) {
            amdString += strlen(kRadeonStr);
            char amdGeneration, amdTier, amdRevision;
            // Sometimes there is a (TM) and sometimes not.
            static constexpr char kTMStr[] = "(TM) ";
            if (!strncmp(amdString, kTMStr, strlen(kTMStr))) {
                amdString += strlen(kTMStr);
            }
            n = sscanf(amdString, "R9 M%c%c%c", &amdGeneration, &amdTier, &amdRevision);
            if (3 == n) {
                if ('3' == amdGeneration) {
                    return kAMDRadeonR9M3xx_GrGLRenderer;
                } else if ('4' == amdGeneration) {
                    return kAMDRadeonR9M4xx_GrGLRenderer;
                }
            }

            char amd0, amd1, amd2;
            n = sscanf(amdString, "HD 7%c%c%c Series", &amd0, &amd1, &amd2);
            if (3 == n) {
                return kAMDRadeonHD7xxx_GrGLRenderer;
            }
        }

        if (strstr(rendererString, "llvmpipe")) {
            return kGalliumLLVM_GrGLRenderer;
        }
        static const char kMaliTStr[] = "Mali-T";
        if (0 == strncmp(rendererString, kMaliTStr, SK_ARRAY_COUNT(kMaliTStr) - 1)) {
            return kMaliT_GrGLRenderer;
        }
        int mali400Num;
        if (1 == sscanf(rendererString, "Mali-%d", &mali400Num) && mali400Num >= 400 &&
            mali400Num < 500) {
            return kMali4xx_GrGLRenderer;
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
                case 2000:
                case 3000:
                    *renderer = GrGLANGLERenderer::kSandyBridge;
                    break;
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
    const GrGLubyte* rendererString;
    GR_GL_CALL_RET(gl, rendererString, GetString(GR_GL_RENDERER));

    return GrGLGetRendererFromStrings((const char*)rendererString, gl->fExtensions);
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

bool GrGLFormatIsCompressed(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
            return true;

        case GrGLFormat::kRGBA8:
        case GrGLFormat::kR8:
        case GrGLFormat::kALPHA8:
        case GrGLFormat::kLUMINANCE8:
        case GrGLFormat::kBGRA8:
        case GrGLFormat::kRGB565:
        case GrGLFormat::kRGBA16F:
        case GrGLFormat::kR16F:
        case GrGLFormat::kLUMINANCE16F:
        case GrGLFormat::kRGB8:
        case GrGLFormat::kRG8:
        case GrGLFormat::kRGB10_A2:
        case GrGLFormat::kRGBA4:
        case GrGLFormat::kSRGB8_ALPHA8:
        case GrGLFormat::kR16:
        case GrGLFormat::kRG16:
        case GrGLFormat::kRGBA16:
        case GrGLFormat::kRG16F:
        case GrGLFormat::kUnknown:
            return false;
    }
    SkUNREACHABLE;
}

bool GrGLFormatToCompressionType(GrGLFormat format, SkImage::CompressionType* compressionType) {
    switch (format) {
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
            *compressionType = SkImage::kETC1_CompressionType;
            return true;

        case GrGLFormat::kRGBA8:
        case GrGLFormat::kR8:
        case GrGLFormat::kALPHA8:
        case GrGLFormat::kLUMINANCE8:
        case GrGLFormat::kBGRA8:
        case GrGLFormat::kRGB565:
        case GrGLFormat::kRGBA16F:
        case GrGLFormat::kR16F:
        case GrGLFormat::kLUMINANCE16F:
        case GrGLFormat::kRGB8:
        case GrGLFormat::kRG8:
        case GrGLFormat::kRGB10_A2:
        case GrGLFormat::kRGBA4:
        case GrGLFormat::kSRGB8_ALPHA8:
        case GrGLFormat::kR16:
        case GrGLFormat::kRG16:
        case GrGLFormat::kRGBA16:
        case GrGLFormat::kRG16F:
        case GrGLFormat::kUnknown:
            return false;
    }
    SkUNREACHABLE;
}

