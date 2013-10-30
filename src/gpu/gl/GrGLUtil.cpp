/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLUtil.h"
#include "SkMatrix.h"
#include <stdio.h>

void GrGLClearErr(const GrGLInterface* gl) {
    while (GR_GL_NO_ERROR != gl->fGetError()) {}
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
        GrPrintf("---- glGetError 0x%x(%s)", err, get_error_string(err));
        if (NULL != location) {
            GrPrintf(" at\n\t%s", location);
        }
        if (NULL != call) {
            GrPrintf("\n\t\t%s", call);
        }
        GrPrintf("\n");
    }
}

namespace {
// Mesa uses a non-standard version string of format: 1.4 Mesa <mesa_major>.<mesa_minor>.
// The mapping of from mesa version to GL version came from here: http://www.mesa3d.org/intro.html
bool get_gl_version_for_mesa(int mesaMajorVersion, int* major, int* minor) {
    switch (mesaMajorVersion) {
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            *major = 1;
            *minor = mesaMajorVersion - 1;
            return true;
        case 7:
            *major = 2;
            *minor = 1;
            return true;
        case 8:
            *major = 3;
            *minor = 0;
            return true;
        case 9:
            *major = 3;
            *minor = 1;
            return true;
        default:
            return false;
    }
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

GrGLBinding GrGLGetBindingInUseFromString(const char* versionString) {
    if (NULL == versionString) {
        SkDEBUGFAIL("NULL GL version string.");
        return kNone_GrGLBinding;
    }

    int major, minor;

    // check for desktop
    int n = sscanf(versionString, "%d.%d", &major, &minor);
    if (2 == n) {
        return kDesktop_GrGLBinding;
    }

    // check for ES 1
    char profile[2];
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile+1, &major, &minor);
    if (4 == n) {
        // we no longer support ES1.
        return kNone_GrGLBinding;
    }

    // check for ES2
    n = sscanf(versionString, "OpenGL ES %d.%d", &major, &minor);
    if (2 == n) {
        return kES_GrGLBinding;
    }
    return kNone_GrGLBinding;
}

bool GrGLIsMesaFromVersionString(const char* versionString) {
    int major, minor, mesaMajor, mesaMinor;
    int n = sscanf(versionString, "%d.%d Mesa %d.%d", &major, &minor, &mesaMajor, &mesaMinor);
    return 4 == n;
}

bool GrGLIsChromiumFromRendererString(const char* rendererString) {
    return 0 == strcmp(rendererString, "Chromium");
}

GrGLVersion GrGLGetVersionFromString(const char* versionString) {
    if (NULL == versionString) {
        SkDEBUGFAIL("NULL GL version string.");
        return 0;
    }

    int major, minor;

    // check for mesa
    int mesaMajor, mesaMinor;
    int n = sscanf(versionString, "%d.%d Mesa %d.%d", &major, &minor, &mesaMajor, &mesaMinor);
    if (4 == n) {
        if (get_gl_version_for_mesa(mesaMajor, &major, &minor)) {
            return GR_GL_VER(major, minor);
        } else {
            return 0;
        }
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

    return 0;
}

GrGLSLVersion GrGLGetGLSLVersionFromString(const char* versionString) {
    if (NULL == versionString) {
        SkDEBUGFAIL("NULL GLSL version string.");
        return 0;
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

    return 0;
}

GrGLVendor GrGLGetVendorFromString(const char* vendorString) {
    if (NULL != vendorString) {
        if (0 == strcmp(vendorString, "ARM")) {
            return kARM_GrGLVendor;
        }
        if (0 == strcmp(vendorString, "Imagination Technologies")) {
            return kImagination_GrGLVendor;
        }
        if (0 == strcmp(vendorString, "Intel")) {
            return kIntel_GrGLVendor;
        }
        if (0 == strcmp(vendorString, "Qualcomm")) {
            return kQualcomm_GrGLVendor;
        }
    }
    return kOther_GrGLVendor;
}

GrGLRenderer GrGLGetRendererFromString(const char* rendererString) {
    if (NULL != rendererString) {
        if (0 == strcmp(rendererString, "NVIDIA Tegra 3")) {
            return kTegra3_GrGLRenderer;
        }
    }
    return kOther_GrGLRenderer;
}

GrGLBinding GrGLGetBindingInUse(const GrGLInterface* gl) {
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_VERSION));
    return GrGLGetBindingInUseFromString((const char*) v);
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

template<> void GrGLGetMatrix<3>(GrGLfloat* dest, const SkMatrix& src) {
    // Col 0
    dest[0] = SkScalarToFloat(src[SkMatrix::kMScaleX]);
    dest[1] = SkScalarToFloat(src[SkMatrix::kMSkewY]);
    dest[2] = SkScalarToFloat(src[SkMatrix::kMPersp0]);

    // Col 1
    dest[3] = SkScalarToFloat(src[SkMatrix::kMSkewX]);
    dest[4] = SkScalarToFloat(src[SkMatrix::kMScaleY]);
    dest[5] = SkScalarToFloat(src[SkMatrix::kMPersp1]);

    // Col 2
    dest[6] = SkScalarToFloat(src[SkMatrix::kMTransX]);
    dest[7] = SkScalarToFloat(src[SkMatrix::kMTransY]);
    dest[8] = SkScalarToFloat(src[SkMatrix::kMPersp2]);
}

template<> void GrGLGetMatrix<4>(GrGLfloat* dest, const SkMatrix& src) {
    // Col 0
    dest[0]  = SkScalarToFloat(src[SkMatrix::kMScaleX]);
    dest[1]  = SkScalarToFloat(src[SkMatrix::kMSkewY]);
    dest[2]  = 0;
    dest[3]  = SkScalarToFloat(src[SkMatrix::kMPersp0]);

    // Col 1
    dest[4]  = SkScalarToFloat(src[SkMatrix::kMSkewX]);
    dest[5]  = SkScalarToFloat(src[SkMatrix::kMScaleY]);
    dest[6]  = 0;
    dest[7]  = SkScalarToFloat(src[SkMatrix::kMPersp1]);

    // Col 2
    dest[8]  = 0;
    dest[9]  = 0;
    dest[10] = 1;
    dest[11] = 0;

    // Col 3
    dest[12] = SkScalarToFloat(src[SkMatrix::kMTransX]);
    dest[13] = SkScalarToFloat(src[SkMatrix::kMTransY]);
    dest[14] = 0;
    dest[15] = SkScalarToFloat(src[SkMatrix::kMPersp2]);
}
