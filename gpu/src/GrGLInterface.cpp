
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTypes.h"
#include "GrGLInterface.h"
#include "GrGLDefines.h"

#include <stdio.h>

#if GR_GL_PER_GL_FUNC_CALLBACK
namespace {
void GrGLDefaultInterfaceCallback(const GrGLInterface*) {}
}
#endif

static SkAutoTUnref<const GrGLInterface> gDefaultGLInterface;

void gl_version_from_string(int* major, int* minor,
                            const char* versionString) {
    if (NULL == versionString) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }

    int n = sscanf(versionString, "%d.%d", major, minor);
    if (2 == n) {
        return;
    }

    char profile[2];
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile+1,
               major, minor);
    bool ok = 4 == n;
    if (!ok) {
        n = sscanf(versionString, "OpenGL ES %d.%d", major, minor);
        ok = 2 == n;
    }

    if (!ok) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
}

float gl_version_as_float_from_string(const char* versionString) {
    int major, minor;
    gl_version_from_string(&major, &minor, versionString);
    GrAssert(minor >= 0);
    // AFAIK there are only single digit minor numbers
    if (minor < 10) {
        return major + minor / 10.f;
    } else if (minor < 100) {
        return major + minor / 100.f;
    } else if (minor < 1000) {
        return major + minor / 1000.f;
    } else {
        GrAssert(!"Why so many digits in minor revision number?");
        char temp[32];
        sprintf(temp, "%d.%d", major, minor);
        return (float) atof(temp);
    }
}

bool has_gl_extension_from_string(const char* ext,
                                  const char* extensionString) {
    int extLength = strlen(ext);

    while (true) {
        int n = strcspn(extensionString, " ");
        if (n == extLength && 0 == strncmp(ext, extensionString, n)) {
            return true;
        }
        if (0 == extensionString[n]) {
            return false;
        }
        extensionString += n+1;
    }

    return false;
}

GR_API const GrGLInterface* GrGLSetDefaultGLInterface(const GrGLInterface* gl_interface) {
    gl_interface->ref();
    gDefaultGLInterface.reset(gl_interface);
    return gl_interface;
}

GR_API const GrGLInterface* GrGLGetDefaultGLInterface() {
    if (NULL == gDefaultGLInterface.get()) {
        GrGLInitializeDefaultGLInterface();
    }
    return gDefaultGLInterface.get();
}

bool has_gl_extension(const GrGLInterface* gl, const char* ext) {
    const GrGLubyte* glstr;
    GR_GL_CALL_RET(gl, glstr, GetString(GR_GL_EXTENSIONS));
    return has_gl_extension_from_string(ext, (const char*) glstr);
}

void gl_version(const GrGLInterface* gl, int* major, int* minor) {
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_VERSION));
    gl_version_from_string(major, minor, (const char*) v);
}

float gl_version_as_float(const GrGLInterface* gl) {
    const GrGLubyte* v;
    GR_GL_CALL_RET(gl, v, GetString(GR_GL_VERSION));
    return gl_version_as_float_from_string((const char*)v);
}

GrGLInterface::GrGLInterface() {
    fBindingsExported = (GrGLBinding)0;
    fNPOTRenderTargetSupport = kProbe_GrGLCapability;
    fMinRenderTargetHeight = kProbe_GrGLCapability;
    fMinRenderTargetWidth = kProbe_GrGLCapability;

    fActiveTexture = NULL;
    fAttachShader = NULL;
    fBindAttribLocation = NULL;
    fBindBuffer = NULL;
    fBindTexture = NULL;
    fBlendColor = NULL;
    fBlendFunc = NULL;
    fBufferData = NULL;
    fBufferSubData = NULL;
    fClear = NULL;
    fClearColor = NULL;
    fClearStencil = NULL;
    fClientActiveTexture = NULL;
    fColor4ub = NULL;
    fColorMask = NULL;
    fColorPointer = NULL;
    fCompileShader = NULL;
    fCompressedTexImage2D = NULL;
    fCreateProgram = NULL;
    fCreateShader = NULL;
    fCullFace = NULL;
    fDeleteBuffers = NULL;
    fDeleteProgram = NULL;
    fDeleteShader = NULL;
    fDeleteTextures = NULL;
    fDepthMask = NULL;
    fDisable = NULL;
    fDisableClientState = NULL;
    fDisableVertexAttribArray = NULL;
    fDrawArrays = NULL;
    fDrawBuffer = NULL;
    fDrawBuffers = NULL;
    fDrawElements = NULL;
    fEnable = NULL;
    fEnableClientState = NULL;
    fEnableVertexAttribArray = NULL;
    fFrontFace = NULL;
    fGenBuffers = NULL;
    fGenTextures = NULL;
    fGetBufferParameteriv = NULL;
    fGetError = NULL;
    fGetIntegerv = NULL;
    fGetProgramInfoLog = NULL;
    fGetProgramiv = NULL;
    fGetShaderInfoLog = NULL;
    fGetShaderiv = NULL;
    fGetString = NULL;
    fGetTexLevelParameteriv = NULL;
    fGetUniformLocation = NULL;
    fLineWidth = NULL;
    fLinkProgram = NULL;
    fLoadMatrixf = NULL;
    fMatrixMode = NULL;
    fPixelStorei = NULL;
    fPointSize = NULL;
    fReadBuffer = NULL;
    fReadPixels = NULL;
    fScissor = NULL;
    fShadeModel = NULL;
    fShaderSource = NULL;
    fStencilFunc = NULL;
    fStencilFuncSeparate = NULL;
    fStencilMask = NULL;
    fStencilMaskSeparate = NULL;
    fStencilOp = NULL;
    fStencilOpSeparate = NULL;
    fTexCoordPointer = NULL;
    fTexEnvi = NULL;
    fTexImage2D = NULL;
    fTexParameteri = NULL;
    fTexSubImage2D = NULL;
    fUniform1f = NULL;
    fUniform1i = NULL;
    fUniform1fv = NULL;
    fUniform1iv = NULL;
    fUniform2f = NULL;
    fUniform2i = NULL;
    fUniform2fv = NULL;
    fUniform2iv = NULL;
    fUniform3f = NULL;
    fUniform3i = NULL;
    fUniform3fv = NULL;
    fUniform3iv = NULL;
    fUniform4f = NULL;
    fUniform4i = NULL;
    fUniform4fv = NULL;
    fUniform4iv = NULL;
    fUniformMatrix2fv = NULL;
    fUniformMatrix3fv = NULL;
    fUniformMatrix4fv = NULL;
    fUseProgram = NULL;
    fVertexAttrib4fv = NULL;
    fVertexAttribPointer = NULL;
    fVertexPointer = NULL;
    fViewport = NULL;
    fBindFramebuffer = NULL;
    fBindRenderbuffer = NULL;
    fCheckFramebufferStatus = NULL;
    fDeleteFramebuffers = NULL;
    fDeleteRenderbuffers = NULL;
    fFramebufferRenderbuffer = NULL;
    fFramebufferTexture2D = NULL;
    fGenFramebuffers = NULL;
    fGenRenderbuffers = NULL;
    fGetFramebufferAttachmentParameteriv = NULL;
    fGetRenderbufferParameteriv = NULL;
    fRenderbufferStorage = NULL;
    fRenderbufferStorageMultisample = NULL;
    fBlitFramebuffer = NULL;
    fResolveMultisampleFramebuffer = NULL;
    fMapBuffer = NULL;
    fUnmapBuffer = NULL;
    fBindFragDataLocationIndexed = NULL;

#if GR_GL_PER_GL_FUNC_CALLBACK
    fCallback = GrGLDefaultInterfaceCallback;
    fCallbackData = 0;
#endif
}


bool GrGLInterface::validateShaderFunctions() const {
    // required for GrGpuGLShaders
    if (NULL == fAttachShader ||
        NULL == fBindAttribLocation ||
        NULL == fCompileShader ||
        NULL == fCreateProgram ||
        NULL == fCreateShader ||
        NULL == fDeleteProgram ||
        NULL == fDeleteShader ||
        NULL == fDisableVertexAttribArray ||
        NULL == fEnableVertexAttribArray ||
        NULL == fGetProgramInfoLog ||
        NULL == fGetProgramiv ||
        NULL == fGetShaderInfoLog ||
        NULL == fGetShaderiv ||
        NULL == fGetUniformLocation ||
        NULL == fLinkProgram ||
        NULL == fShaderSource ||
        NULL == fUniform1f ||
        NULL == fUniform1i ||
        NULL == fUniform1fv ||
        NULL == fUniform1iv ||
        NULL == fUniform2f ||
        NULL == fUniform2i ||
        NULL == fUniform2fv ||
        NULL == fUniform2iv ||
        NULL == fUniform3f ||
        NULL == fUniform3i ||
        NULL == fUniform3fv ||
        NULL == fUniform3iv ||
        NULL == fUniform4f ||
        NULL == fUniform4i ||
        NULL == fUniform4fv ||
        NULL == fUniform4iv ||
        NULL == fUniformMatrix2fv ||
        NULL == fUniformMatrix3fv ||
        NULL == fUniformMatrix4fv ||
        NULL == fUseProgram ||
        NULL == fVertexAttrib4fv ||
        NULL == fVertexAttribPointer) {
        return false;
    }
    return true;
}

bool GrGLInterface::validateFixedFunctions() const {
    if (NULL == fClientActiveTexture ||
        NULL == fColor4ub ||
        NULL == fColorPointer ||
        NULL == fDisableClientState ||
        NULL == fEnableClientState ||
        NULL == fLoadMatrixf ||
        NULL == fMatrixMode ||
        NULL == fPointSize ||
        NULL == fShadeModel ||
        NULL == fTexCoordPointer ||
        NULL == fTexEnvi ||
        NULL == fVertexPointer) {
        return false;
    }
    return true;
}

bool GrGLInterface::validate(GrEngine engine) const {

    bool isDesktop = this->supportsDesktop();

    bool isES = this->supportsES();
    
    if (isDesktop == isES) {
        // must have one, don't support both in same interface
        return false;
    }

    // functions that are always required
    if (NULL == fActiveTexture ||
        NULL == fBindBuffer ||
        NULL == fBindTexture ||
        NULL == fBlendFunc ||
        NULL == fBufferData ||
        NULL == fBufferSubData ||
        NULL == fClear ||
        NULL == fClearColor ||
        NULL == fClearStencil ||
        NULL == fColorMask ||
        NULL == fCullFace ||
        NULL == fDeleteBuffers ||
        NULL == fDeleteTextures ||
        NULL == fDepthMask ||
        NULL == fDisable ||
        NULL == fDrawArrays ||
        NULL == fDrawElements ||
        NULL == fEnable ||
        NULL == fFrontFace ||
        NULL == fGenBuffers ||
        NULL == fGenTextures ||
        NULL == fGetBufferParameteriv ||
        NULL == fGetError ||
        NULL == fGetIntegerv ||
        NULL == fGetString ||
        NULL == fPixelStorei ||
        NULL == fReadPixels ||
        NULL == fScissor ||
        NULL == fStencilFunc ||
        NULL == fStencilMask ||
        NULL == fStencilOp ||
        NULL == fTexImage2D ||
        NULL == fTexParameteri ||
        NULL == fTexSubImage2D ||
        NULL == fViewport ||
        NULL == fBindFramebuffer ||
        NULL == fBindRenderbuffer ||
        NULL == fCheckFramebufferStatus ||
        NULL == fDeleteFramebuffers ||
        NULL == fDeleteRenderbuffers ||
        NULL == fFramebufferRenderbuffer ||
        NULL == fFramebufferTexture2D ||
        NULL == fGetFramebufferAttachmentParameteriv ||
        NULL == fGetRenderbufferParameteriv ||
        NULL == fGenFramebuffers ||
        NULL == fGenRenderbuffers ||
        NULL == fRenderbufferStorage) {
        return false;
    }

    switch (engine) {
        case kOpenGL_Shaders_GrEngine:
            if (kES1_GrGLBinding == fBindingsExported) {
                return false;
            }
            if (!this->validateShaderFunctions()) {
                return false;
            }
            break;
        case kOpenGL_Fixed_GrEngine:
            if (kES1_GrGLBinding == fBindingsExported) {
                return false;
            }
            if (!this->validateFixedFunctions()) {
                return false;
            }
            break;
        default:
            return false;
    }

    int major, minor;
    const char* ext;

    gl_version(this, &major, &minor);
    ext = (const char*)fGetString(GR_GL_EXTENSIONS);

    // Now check that baseline ES/Desktop fns not covered above are present
    // and that we have fn pointers for any advertised extensions that we will
    // try to use.

    // these functions are part of ES2, we assume they are available
    // On the desktop we assume they are available if the extension
    // is present or GL version is high enough.
    if ((kES2_GrGLBinding & fBindingsExported)) {
        if (NULL == fBlendColor ||
            NULL == fStencilFuncSeparate ||
            NULL == fStencilMaskSeparate ||
            NULL == fStencilOpSeparate) {
            return false;
        }
    } else if (kDesktop_GrGLBinding == fBindingsExported) {
        if (major >= 2) {
            if (NULL == fStencilFuncSeparate ||
                NULL == fStencilMaskSeparate ||
                NULL == fStencilOpSeparate) {
                return false;
            }
        }
        if (major >= 2 ||
            has_gl_extension_from_string("GL_ARB_draw_buffers", ext)) {
            if (NULL == fDrawBuffers) {
                return false;
            }
        }
        if (1 < major || (1 == major && 4 <= minor) ||
            has_gl_extension_from_string("GL_EXT_blend_color", ext)) {
            if (NULL == fBlendColor) {
                return false;
            }
        }
    }

    // optional function on desktop before 1.3
    if (kDesktop_GrGLBinding != fBindingsExported ||
        (1 < major || (1 == major && 3 <= minor)) ||
        has_gl_extension_from_string("GL_ARB_texture_compression", ext)) {
        if (NULL == fCompressedTexImage2D) {
            return false;
        }
    }

    // part of desktop GL, but not ES
    if (kDesktop_GrGLBinding == fBindingsExported &&
        (NULL == fLineWidth ||
         NULL == fGetTexLevelParameteriv ||
         NULL == fDrawBuffer ||
         NULL == fReadBuffer)) {
        return false;
    }

    // FBO MSAA
    if (kDesktop_GrGLBinding == fBindingsExported) {
        // GL 3.0 and the ARB extension have multisample + blit
        if ((major >= 3) || has_gl_extension_from_string("GL_ARB_framebuffer_object", ext)) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fBlitFramebuffer) {
                return false;
            }
        } else {
            if (has_gl_extension_from_string("GL_EXT_framebuffer_blit", ext) &&
                NULL == fBlitFramebuffer) {
                return false;
            }
            if (has_gl_extension_from_string("GL_EXT_framebuffer_multisample", ext) &&
                NULL == fRenderbufferStorageMultisample) {
                return false;
            }
        }
    } else {
        if (has_gl_extension_from_string("GL_CHROMIUM_framebuffer_multisample", ext)) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fBlitFramebuffer) {
                return false;
            }
        }
        if (has_gl_extension_from_string("GL_APPLE_framebuffer_multisample", ext)) {
            if (NULL == fRenderbufferStorageMultisample ||
                NULL == fResolveMultisampleFramebuffer) {
                return false;
            }
        }
    }

    // On ES buffer mapping is an extension. On Desktop
    // buffer mapping was part of original VBO extension
    // which we require.
    if (kDesktop_GrGLBinding == fBindingsExported  || 
        has_gl_extension_from_string("GL_OES_mapbuffer", ext)) {
        if (NULL == fMapBuffer ||
            NULL == fUnmapBuffer) {
            return false;
        }
    }

    // Dual source blending
    if (kDesktop_GrGLBinding == fBindingsExported  &&
        (has_gl_extension_from_string("GL_ARB_blend_func_extended", ext) ||
         (3 < major) || (3 == major && 3 <= minor))) {
        if (NULL == fBindFragDataLocationIndexed) {
            return false;
        }
    }

    return true;
}

