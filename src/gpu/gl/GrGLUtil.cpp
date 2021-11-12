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

///////////////////////////////////////////////////////////////////////////////

#if GR_GL_LOG_CALLS
    bool gLogCallsGL = !!(GR_GL_LOG_CALLS_START);
#endif

#if GR_GL_CHECK_ERROR
    bool gCheckErrorGL = !!(GR_GL_CHECK_ERROR_START);
#endif

///////////////////////////////////////////////////////////////////////////////

GrGLStandard GrGLGetStandardInUseFromString(const char* versionString) {
    if (!versionString) {
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

GrGLVersion GrGLGetVersionFromString(const char* versionString) {
    if (!versionString) {
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
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile + 1, &major, &minor);
    if (4 == n) {
        return GR_GL_VER(major, minor);
    }

    n = sscanf(versionString, "OpenGL ES %d.%d", &major, &minor);
    if (2 == n) {
        return GR_GL_VER(major, minor);
    }

    return GR_GL_INVALID_VER;
}

GrGLVersion GrGLGetVersion(const GrGLInterface* gl) {
    SkASSERT(gl);
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_VERSION));
    return GrGLGetVersionFromString((const char*)v);
}

static GrGLSLVersion get_glsl_version(const char* versionString) {
    SkASSERT(versionString);
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

static GrGLVendor get_vendor(const char* vendorString) {
    SkASSERT(vendorString);
    if (0 == strcmp(vendorString, "ARM")) {
        return GrGLVendor::kARM;
    }
    if (0 == strcmp(vendorString, "Google Inc.")) {
        return GrGLVendor::kGoogle;
    }
    if (0 == strcmp(vendorString, "Imagination Technologies")) {
        return GrGLVendor::kImagination;
    }
    if (0 == strncmp(vendorString, "Intel ", 6) || 0 == strcmp(vendorString, "Intel")) {
        return GrGLVendor::kIntel;
    }
    if (0 == strcmp(vendorString, "Qualcomm") || 0 == strcmp(vendorString, "freedreno")) {
        return GrGLVendor::kQualcomm;
    }
    if (0 == strcmp(vendorString, "NVIDIA Corporation")) {
        return GrGLVendor::kNVIDIA;
    }
    if (0 == strcmp(vendorString, "ATI Technologies Inc.")) {
        return GrGLVendor::kATI;
    }
    return GrGLVendor::kOther;
}

static GrGLRenderer get_renderer(const char* rendererString, const GrGLExtensions& extensions) {
    SkASSERT(rendererString);
    static const char kTegraStr[] = "NVIDIA Tegra";
    if (0 == strncmp(rendererString, kTegraStr, SK_ARRAY_COUNT(kTegraStr) - 1)) {
        // Tegra strings are not very descriptive. We distinguish between the modern and legacy
        // architectures by the presence of NV_path_rendering.
        return extensions.has("GL_NV_path_rendering") ? GrGLRenderer::kTegra
                                                      : GrGLRenderer::kTegra_PreK1;
    }
    int lastDigit;
    int n = sscanf(rendererString, "PowerVR SGX 54%d", &lastDigit);
    if (1 == n && lastDigit >= 0 && lastDigit <= 9) {
        return GrGLRenderer::kPowerVR54x;
    }
    // certain iOS devices also use PowerVR54x GPUs
    static const char kAppleA4Str[] = "Apple A4";
    static const char kAppleA5Str[] = "Apple A5";
    static const char kAppleA6Str[] = "Apple A6";
    if (0 == strncmp(rendererString, kAppleA4Str, SK_ARRAY_COUNT(kAppleA4Str) - 1) ||
        0 == strncmp(rendererString, kAppleA5Str, SK_ARRAY_COUNT(kAppleA5Str) - 1) ||
        0 == strncmp(rendererString, kAppleA6Str, SK_ARRAY_COUNT(kAppleA6Str) - 1)) {
        return GrGLRenderer::kPowerVR54x;
    }
    static const char kPowerVRRogueStr[] = "PowerVR Rogue";
    static const char kAppleA7Str[] = "Apple A7";
    static const char kAppleA8Str[] = "Apple A8";
    if (0 == strncmp(rendererString, kPowerVRRogueStr, SK_ARRAY_COUNT(kPowerVRRogueStr) - 1) ||
        0 == strncmp(rendererString, kAppleA7Str, SK_ARRAY_COUNT(kAppleA7Str) - 1) ||
        0 == strncmp(rendererString, kAppleA8Str, SK_ARRAY_COUNT(kAppleA8Str) - 1)) {
        return GrGLRenderer::kPowerVRRogue;
    }
    int adrenoNumber;
    n = sscanf(rendererString, "Adreno (TM) %d", &adrenoNumber);
    if (n < 1) {
        // retry with freedreno driver
        n = sscanf(rendererString, "FD%d", &adrenoNumber);
    }
    if (1 == n) {
        if (adrenoNumber >= 300) {
            if (adrenoNumber < 400) {
                return GrGLRenderer::kAdreno3xx;
            }
            if (adrenoNumber < 500) {
                return adrenoNumber >= 430 ? GrGLRenderer::kAdreno430
                                           : GrGLRenderer::kAdreno4xx_other;
            }
            if (adrenoNumber < 600) {
                return adrenoNumber == 530 ? GrGLRenderer::kAdreno530
                                           : GrGLRenderer::kAdreno5xx_other;
            }
            if (adrenoNumber < 700) {
                if (adrenoNumber == 615) {
                    return GrGLRenderer::kAdreno615;
                }
                if (adrenoNumber == 620) {
                    return GrGLRenderer::kAdreno620;
                }
                if (adrenoNumber == 630) {
                    return GrGLRenderer::kAdreno630;
                }
                if (adrenoNumber == 640) {
                    return GrGLRenderer::kAdreno640;
                }
                return GrGLRenderer::kAdreno6xx_other;
            }
        }
    }
    if (0 == strcmp("Google SwiftShader", rendererString)) {
        return GrGLRenderer::kGoogleSwiftShader;
    }

    if (const char* intelString = strstr(rendererString, "Intel")) {
        // These generic strings seem to always come from Haswell: Iris 5100 or Iris Pro 5200
        if (0 == strcmp("Intel Iris OpenGL Engine", intelString) ||
            0 == strcmp("Intel Iris Pro OpenGL Engine", intelString)) {
            return GrGLRenderer::kIntelHaswell;
        }
        if (strstr(intelString, "Sandybridge")) {
            return GrGLRenderer::kIntelSandyBridge;
        }
        if (strstr(intelString, "Bay Trail")) {
            return GrGLRenderer::kIntelValleyView;
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
                    return GrGLRenderer::kIntelSandyBridge;
                }
                if (intelNumber == 2500 || intelNumber == 4000) {
                    return GrGLRenderer::kIntelIvyBridge;
                }
                if (intelNumber >= 4200 && intelNumber <= 5200) {
                    return GrGLRenderer::kIntelHaswell;
                }
                if (intelNumber >= 400 && intelNumber <= 405) {
                    return GrGLRenderer::kIntelCherryView;
                }
                if (intelNumber >= 5300 && intelNumber <= 6300) {
                    return GrGLRenderer::kIntelBroadwell;
                }
                if (intelNumber >= 500 && intelNumber <= 505) {
                    return GrGLRenderer::kIntelApolloLake;
                }
                if (intelNumber >= 510 && intelNumber <= 580) {
                    return GrGLRenderer::kIntelSkyLake;
                }
                if (intelNumber >= 600 && intelNumber <= 605) {
                    return GrGLRenderer::kIntelGeminiLake;
                }
                // 610 and 630 are reused from KabyLake to CoffeeLake. The CoffeeLake variants
                // are "UHD Graphics", while the KabyLake ones are "HD Graphics"
                if (intelNumber == 610 || intelNumber == 630) {
                    return strstr(intelString, "UHD") ? GrGLRenderer::kIntelCoffeeLake
                                                      : GrGLRenderer::kIntelKabyLake;
                }
                if (intelNumber >= 610 && intelNumber <= 650) {
                    return GrGLRenderer::kIntelKabyLake;
                }
                if (intelNumber == 655) {
                    return GrGLRenderer::kIntelCoffeeLake;
                }
                if (intelNumber >= 910 && intelNumber <= 950) {
                    return GrGLRenderer::kIntelIceLake;
                }
            }
        }
    }

    // The AMD string can have a somewhat arbitrary preamble (see skbug.com/7195)
    static constexpr char kRadeonStr[] = "Radeon ";
    if (const char* amdString = strstr(rendererString, kRadeonStr)) {
        amdString += strlen(kRadeonStr);
        // Sometimes there is a (TM) and sometimes not.
        static constexpr char kTMStr[] = "(TM) ";
        if (!strncmp(amdString, kTMStr, strlen(kTMStr))) {
            amdString += strlen(kTMStr);
        }

        char amd0, amd1, amd2;
        int amdModel;
        n = sscanf(amdString, "R9 M3%c%c", &amd0, &amd1);
        if (2 == n && isdigit(amd0) && isdigit(amd1)) {
            return GrGLRenderer::kAMDRadeonR9M3xx;
        }

        n = sscanf(amdString, "R9 M4%c%c", &amd0, &amd1);
        if (2 == n && isdigit(amd0) && isdigit(amd1)) {
            return GrGLRenderer::kAMDRadeonR9M4xx;
        }

        n = sscanf(amdString, "HD 7%c%c%c Series", &amd0, &amd1, &amd2);
        if (3 == n && isdigit(amd0) && isdigit(amd1) && isdigit(amd2)) {
            return GrGLRenderer::kAMDRadeonHD7xxx;
        }

        n = sscanf(amdString, "Pro 5%c%c%c", &amd0, &amd1, &amd2);
        if (3 == n && isdigit(amd0) && isdigit(amd1) && isdigit(amd2)) {
            return GrGLRenderer::kAMDRadeonPro5xxx;
        }

        n = sscanf(amdString, "Pro Vega %i", &amdModel);
        if (1 == n) {
            return GrGLRenderer::kAMDRadeonProVegaxx;
        }
    }

    if (strstr(rendererString, "llvmpipe")) {
        return GrGLRenderer::kGalliumLLVM;
    }
    if (strstr(rendererString, "virgl")) {
        return GrGLRenderer::kVirgl;
    }
    static const char kMaliGStr[] = "Mali-G";
    if (0 == strncmp(rendererString, kMaliGStr, SK_ARRAY_COUNT(kMaliGStr) - 1)) {
        return GrGLRenderer::kMaliG;
    }
    static const char kMaliTStr[] = "Mali-T";
    if (0 == strncmp(rendererString, kMaliTStr, SK_ARRAY_COUNT(kMaliTStr) - 1)) {
        return GrGLRenderer::kMaliT;
    }
    int mali400Num;
    if (1 == sscanf(rendererString, "Mali-%d", &mali400Num) && mali400Num >= 400 &&
        mali400Num < 500) {
        return GrGLRenderer::kMali4xx;
    }
    return GrGLRenderer::kOther;
}

static bool is_commamd_buffer(const char* rendererString, const char* versionString) {
    SkASSERT(rendererString);
    SkASSERT(versionString);

    int major, minor;
    static const char kChromium[] = "Chromium";
    char suffix[SK_ARRAY_COUNT(kChromium)] = {0};
    return (0 == strcmp(rendererString, kChromium) ||
           (3 == sscanf(versionString, "OpenGL ES %d.%d %8s", &major, &minor, suffix) &&
            0 == strcmp(kChromium, suffix)));
}

static std::tuple<GrGLDriver, GrGLDriverVersion> get_driver_and_version(GrGLStandard standard,
                                                                        GrGLVendor vendor,
                                                                        const char* vendorString,
                                                                        const char* rendererString,
                                                                        const char* versionString) {
    SkASSERT(rendererString);
    SkASSERT(versionString);

    GrGLDriver driver               = GrGLDriver::kUnknown;
    GrGLDriverVersion driverVersion = GR_GL_DRIVER_UNKNOWN_VER;

    int major, minor, rev, driverMajor, driverMinor, driverPoint;
    // This is the same on ES and regular GL.
    if (!strcmp(vendorString, "freedreno")) {
        driver = GrGLDriver::kFreedreno;
    } else if (GR_IS_GR_GL(standard)) {
        if (vendor == GrGLVendor::kNVIDIA) {
            driver = GrGLDriver::kNVIDIA;
            int n = sscanf(versionString,
                           "%d.%d.%d NVIDIA %d.%d",
                           &major,
                           &minor,
                           &rev,
                           &driverMajor,
                           &driverMinor);
            // Some older NVIDIA drivers don't report the driver version.
            if (n == 5) {
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        } else {
            int n = sscanf(versionString,
                           "%d.%d Mesa %d.%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor);
            if (n != 4) {
                n = sscanf(versionString,
                           "%d.%d (Core Profile) Mesa %d.%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor);
            }
            if (n == 4) {
                driver = GrGLDriver::kMesa;
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        }
    } else if (standard == kGLES_GrGLStandard) {
        if (vendor == GrGLVendor::kNVIDIA) {
            driver = GrGLDriver::kNVIDIA;
            int n = sscanf(versionString,
                           "OpenGL ES %d.%d NVIDIA %d.%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor);
            // Some older NVIDIA drivers don't report the driver version.
            if (n == 4) {
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        } else if (vendor == GrGLVendor::kImagination) {
            int revision;
            int n = sscanf(versionString,
                           "OpenGL ES %d.%d build %d.%d@%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor,
                           &revision);
            if (n == 5) {
                driver = GrGLDriver::kImagination;
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        } else {
            int n = sscanf(versionString,
                           "OpenGL ES %d.%d Mesa %d.%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor);
            if (n == 4) {
                driver = GrGLDriver::kMesa;
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        }
    }

    if (driver == GrGLDriver::kUnknown) {
        if (vendor == GrGLVendor::kGoogle) {
            // Swiftshader is the only Google vendor at the moment
            driver = GrGLDriver::kSwiftShader;

            // Swiftshader has a strange version string: w.x.y.z  Going to arbitrarily ignore
            // y and assume w,x and z are major, minor, point.
            // As of writing, version is 4.0.0.6
            int n = sscanf(versionString,
                           "OpenGL ES %d.%d SwiftShader %d.%d.0.%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor,
                           &driverPoint);
            if (n == 5) {
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, driverPoint);
            }
        } else if (vendor == GrGLVendor::kIntel) {
            // We presume we're on the Intel driver since it hasn't identified itself as Mesa.
            driver = GrGLDriver::kIntel;

            // This is how the macOS version strings are structured. This might be different on
            // different
            //  OSes.
            int n = sscanf(versionString,
                           "%d.%d INTEL-%d.%d.%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor,
                           &driverPoint);
            if (n == 5) {
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, driverPoint);
            }
        } else if (vendor == GrGLVendor::kQualcomm) {
            driver = GrGLDriver::kQualcomm;
            int n = sscanf(versionString,
                           "OpenGL ES %d.%d V@%d.%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor);
            if (n == 4) {
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        } else if (vendor == GrGLVendor::kImagination) {
            int revision;
            int n = sscanf(versionString,
                           "OpenGL ES %d.%d build %d.%d@%d",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor,
                           &revision);
            if (n == 5) {
                // Revision is a large number (looks like a source control revision number) that
                // doesn't fit into the 'patch' bits, so omit it until we need it.
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        } else if (vendor == GrGLVendor::kARM) {
            // Example:
            // OpenGL ES 3.2 v1.r26p0-01rel0.217d2597f6bd19b169343737782e56e3
            // It's unclear how to interpret what comes between "p" and "rel". Every string we've
            // seen so far has "0-01" there. We ignore it for now.
            int ignored0;
            int ignored1;
            int n = sscanf(versionString,
                           "OpenGL ES %d.%d v%d.r%dp%d-%drel",
                           &major,
                           &minor,
                           &driverMajor,
                           &driverMinor,
                           &ignored0,
                           &ignored1);
            if (n == 6) {
                driver = GrGLDriver::kARM;
                driverVersion = GR_GL_DRIVER_VER(driverMajor, driverMinor, 0);
            }
        } else {
            static constexpr char kEmulatorPrefix[] = "Android Emulator OpenGL ES Translator";
            if (0 == strncmp(kEmulatorPrefix, rendererString, strlen(kEmulatorPrefix))) {
                driver = GrGLDriver::kAndroidEmulator;
            }
        }
    }
    return {driver, driverVersion};
}

// If this is detected as ANGLE then the ANGLE backend is returned along with rendererString
// stripped of "ANGLE(" and ")" at the start and end, respectively.
static std::tuple<GrGLANGLEBackend, SkString> get_angle_backend(const char* rendererString) {
    // crbug.com/1203705 ANGLE renderer will be "ANGLE (<gl-vendor>, <gl-renderer>, <gl-version>)"
    // on ANGLE's GL backend with related substitutions for the inner strings on other backends.
    static constexpr char kHeader[] = "ANGLE (";
    static constexpr size_t kHeaderLength = SK_ARRAY_COUNT(kHeader) - 1;
    int rendererLength = strlen(rendererString);
    if (!strncmp(rendererString, kHeader, kHeaderLength) &&
        rendererString[rendererLength - 1] == ')') {
        SkString innerString;
        innerString.set(rendererString + kHeaderLength, rendererLength - kHeaderLength - 1);
        if (strstr(rendererString, "Direct3D11")) {
            return {GrGLANGLEBackend::kD3D11, std::move(innerString)};
        } else if (strstr(rendererString, "Direct3D9")) {
            return {GrGLANGLEBackend::kD3D9, std::move(innerString)};
        } else if (strstr(rendererString, "OpenGL")) {
            return {GrGLANGLEBackend::kOpenGL, std::move(innerString)};
        }
    }
    return {GrGLANGLEBackend::kUnknown, {}};
}

static std::tuple<GrGLVendor, GrGLRenderer, GrGLDriver, GrGLDriverVersion>
get_angle_gl_vendor_and_renderer(
        const char* innerString,
        const GrGLExtensions& extensions) {
    SkTArray<SkString> parts;
    SkStrSplit(innerString, ",", &parts);
    // This would need some fixing if we have substrings that contain commas.
    if (parts.size() != 3) {
        return {GrGLVendor::kOther,
                GrGLRenderer::kOther,
                GrGLDriver::kUnknown,
                GR_GL_DRIVER_UNKNOWN_VER};
    }

    const char* angleVendorString   = parts[0].c_str();
    const char* angleRendererString = parts[1].c_str() + 1; // skip initial space
    const char* angleVersionString  = parts[2].c_str() + 1; // skip initial space

    GrGLVendor angleVendor = get_vendor(angleVendorString);

    auto [angleDriver, angleDriverVersion] = get_driver_and_version(kGLES_GrGLStandard,
                                                                    angleVendor,
                                                                    angleVendorString,
                                                                    angleRendererString,
                                                                    angleVersionString);

    auto angleRenderer = get_renderer(angleRendererString, extensions);

    return {angleVendor, angleRenderer, angleDriver, angleDriverVersion};
}

static std::tuple<GrGLVendor, GrGLRenderer, GrGLDriver, GrGLDriverVersion>
get_angle_d3d_vendor_and_renderer(const char* innerString) {
    auto vendor   = GrGLVendor::kOther;
    auto renderer = GrGLRenderer::kOther;

    if (strstr(innerString, "Intel")) {
        vendor = GrGLVendor::kIntel;

        const char* modelStr;
        int modelNumber;
        if ((modelStr = strstr(innerString, "HD Graphics")) &&
            (1 == sscanf(modelStr, "HD Graphics %i", &modelNumber) ||
             1 == sscanf(modelStr, "HD Graphics P%i", &modelNumber))) {
            switch (modelNumber) {
                case 2000:
                case 3000:
                    renderer = GrGLRenderer::kIntelSandyBridge;
                    break;
                case 4000:
                case 2500:
                    renderer = GrGLRenderer::kIntelSandyBridge;
                    break;
                case 510:
                case 515:
                case 520:
                case 530:
                    renderer = GrGLRenderer::kIntelSkyLake;
                    break;
            }
        } else if ((modelStr = strstr(innerString, "Iris")) &&
                   (1 == sscanf(modelStr, "Iris(TM) Graphics %i", &modelNumber) ||
                    1 == sscanf(modelStr, "Iris(TM) Pro Graphics %i", &modelNumber) ||
                    1 == sscanf(modelStr, "Iris(TM) Pro Graphics P%i", &modelNumber))) {
            switch (modelNumber) {
                case 540:
                case 550:
                case 555:
                case 580:
                    renderer = GrGLRenderer::kIntelSkyLake;
                    break;
            }
        }
    } else if (strstr(innerString, "NVIDIA")) {
        vendor = GrGLVendor::kNVIDIA;
    } else if (strstr(innerString, "Radeon")) {
        vendor = GrGLVendor::kATI;
    }
    // We haven't had a need yet to parse the D3D driver string.
    return {vendor, renderer, GrGLDriver::kUnknown, GR_GL_DRIVER_UNKNOWN_VER};
}

GrGLDriverInfo GrGLGetDriverInfo(const GrGLInterface* interface) {
    if (!interface) {
        return {};
    }
    SkASSERT(interface->fStandard != kNone_GrGLStandard);
    GrGLDriverInfo info;
    info.fStandard = interface->fStandard;

    auto getString = [&](GrGLenum s) {
        const GrGLubyte* bytes = interface->fFunctions.fGetString(s);
        if (!bytes) {
            return "";
        }
        return reinterpret_cast<const char*>(bytes);
    };

    const char* const version   = getString(GR_GL_VERSION);
    const char* const slversion = getString(GR_GL_SHADING_LANGUAGE_VERSION);
    const char* const renderer  = getString(GR_GL_RENDERER);
    const char* const vendor    = getString(GR_GL_VENDOR);

    info.fVersion     = GrGLGetVersionFromString(version);
    info.fGLSLVersion = get_glsl_version(slversion);
    info.fVendor      = get_vendor(vendor);
    info.fRenderer    = get_renderer(renderer, interface->fExtensions);

    std::tie(info.fDriver, info.fDriverVersion) = get_driver_and_version(interface->fStandard,
                                                                         info.fVendor,
                                                                         vendor,
                                                                         renderer,
                                                                         version);

    SkString innerAngleRendererString;
    std::tie(info.fANGLEBackend, innerAngleRendererString) = get_angle_backend(renderer);

    if (info.fANGLEBackend == GrGLANGLEBackend::kD3D9 ||
        info.fANGLEBackend == GrGLANGLEBackend::kD3D11) {
        std::tie(info.fANGLEVendor,
                 info.fANGLERenderer,
                 info.fANGLEDriver,
                 info.fANGLEDriverVersion) =
                get_angle_d3d_vendor_and_renderer(innerAngleRendererString.c_str());
    } else if (info.fANGLEBackend == GrGLANGLEBackend::kOpenGL) {
        std::tie(info.fANGLEVendor,
                 info.fANGLERenderer,
                 info.fANGLEDriver,
                 info.fANGLEDriverVersion) =
                get_angle_gl_vendor_and_renderer(innerAngleRendererString.c_str(),
                                                 interface->fExtensions);
    }

    info.fIsOverCommandBuffer = is_commamd_buffer(renderer, version);

    return info;
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
    static_assert(0 == (int)GrStencilTest::kAlways);
    static_assert(1 == (int)GrStencilTest::kNever);
    static_assert(2 == (int)GrStencilTest::kGreater);
    static_assert(3 == (int)GrStencilTest::kGEqual);
    static_assert(4 == (int)GrStencilTest::kLess);
    static_assert(5 == (int)GrStencilTest::kLEqual);
    static_assert(6 == (int)GrStencilTest::kEqual);
    static_assert(7 == (int)GrStencilTest::kNotEqual);
    SkASSERT(test < (GrStencilTest)kGrStencilTestCount);

    return gTable[(int)test];
}

bool GrGLFormatIsCompressed(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8:
        case GrGLFormat::kCOMPRESSED_RGB8_ETC2:
        case GrGLFormat::kCOMPRESSED_RGB8_BC1:
        case GrGLFormat::kCOMPRESSED_RGBA8_BC1:
            return true;

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
        case GrGLFormat::kDEPTH24_STENCIL8:
        case GrGLFormat::kUnknown:
            return false;
    }
    SkUNREACHABLE;
}

