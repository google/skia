/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLUtil.h"

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
        GrAssert(!"NULL GL version string.");
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
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile+1,
               &major, &minor);
    if (4 == n) {
        // we no longer support ES1.
        return kNone_GrGLBinding;
    }

    // check for ES2
    n = sscanf(versionString, "OpenGL ES %d.%d", &major, &minor);
    if (2 == n) {
        return kES2_GrGLBinding;
    }
    return kNone_GrGLBinding;
}

GrGLVersion GrGLGetVersionFromString(const char* versionString) {
    if (NULL == versionString) {
        GrAssert(!"NULL GL version string.");
        return 0;
    }

    int major, minor;

    int n = sscanf(versionString, "%d.%d", &major, &minor);
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
        GrAssert(!"NULL GLSL version string.");
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
    }
    return kOther_GrGLVendor;
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
