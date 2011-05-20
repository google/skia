/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrTypes.h"
#include "GrGLInterface.h"
#include "GrGLDefines.h"

#include <stdio.h>

GrGLInterface* gGLInterface = NULL;

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


GR_API void GrGLSetGLInterface(GrGLInterface* gl_interface) {
    gGLInterface = gl_interface;
}

GR_API GrGLInterface* GrGLGetGLInterface() {
    return gGLInterface;
}

bool has_gl_extension(const char* ext) {
    const char* glstr = reinterpret_cast<const char*>(
                GrGLGetGLInterface()->fGetString(GR_GL_EXTENSIONS));

    return has_gl_extension_from_string(ext, glstr);
}

void gl_version(int* major, int* minor) {
    const char* v = reinterpret_cast<const char*>(
                GrGLGetGLInterface()->fGetString(GR_GL_VERSION));
    gl_version_from_string(major, minor, v);
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

    bool isDesktop = kDesktop_GrGLBinding == fBindingsExported;

    // ES1 and 2 can be supported in the same interface
    bool isES = ((kES1_GrGLBinding | kES2_GrGLBinding) & fBindingsExported &&
                 !(~(kES1_GrGLBinding | kES2_GrGLBinding) & fBindingsExported));
    
    if (!isDesktop && !isES) {
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

    gl_version(&major, &minor);
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

    // part of desktop GL
    if (kDesktop_GrGLBinding == fBindingsExported &&
        NULL == fLineWidth) {
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

