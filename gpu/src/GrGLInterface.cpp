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


#include "GrGLInterface.h"
#include "GrTypes.h"

#include <stdio.h>

#if defined(GR_GL_PROC_ADDRESS_HEADER)
    #include GR_GL_PROC_ADDRESS_HEADER
#endif

#if !defined(GR_GL_PROC_ADDRESS)
    #error "Must define GR_GL_PROC_ADDRESS"
#endif

#define GR_GL_GET_PROC(PROC_NAME) \
    glBindings->f##PROC_NAME = \
                reinterpret_cast<GrGLInterface::GrGL##PROC_NAME##Proc>( \
                            GR_GL_PROC_ADDRESS(gl##PROC_NAME)); \
    GrAssert(NULL != glBindings->f##PROC_NAME && \
             "Missing GL binding: " #PROC_NAME);

#define GR_GL_GET_PROC_SUFFIX(PROC_NAME, SUFFIX) \
    glBindings->f##PROC_NAME = \
                reinterpret_cast<GrGLInterface::GrGL##PROC_NAME##Proc>( \
                        GR_GL_PROC_ADDRESS(gl##PROC_NAME##SUFFIX)); \
    GrAssert(NULL != glBindings->f##PROC_NAME && \
             "Missing GL binding: " #PROC_NAME);

#define GR_GL_GET_PROC_SYMBOL(PROC_NAME) \
    glBindings->f##PROC_NAME = reinterpret_cast<GrGLInterface::GrGL##PROC_NAME##Proc>(gl##PROC_NAME);

namespace {

void gl_version_from_string(int* major, int* minor,
                            const char* versionString) {
    if (NULL == versionString) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
#if GR_SUPPORT_GLDESKTOP
    int n = sscanf(versionString, "%d.%d", major, minor);
    if (n != 2) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
#else
    char profile[2];
    int n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile+1,
                   major, minor);
    bool ok = 4 == n;
    if (!ok) {
        int n = sscanf(versionString, "OpenGL ES %d.%d", major, minor);
        ok = 2 == n;
    }
    if (!ok) {
        GrAssert(0);
        *major = 0;
        *minor = 0;
        return;
    }
#endif
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

GrGLInterface* gGLInterface = NULL;

void InitializeGLInterfaceExtensions(GrGLInterface* glBindings) {
    int major, minor;
    const char* versionString = reinterpret_cast<const char*>(
                glBindings->fGetString(GL_VERSION));
    const char* extensionString = reinterpret_cast<const char*>(
                glBindings->fGetString(GL_EXTENSIONS));
    gl_version_from_string(&major, &minor, versionString);

    bool fboFound = false;
#if GR_SUPPORT_GLDESKTOP
    if (major >= 3 || has_gl_extension_from_string("GL_ARB_framebuffer_object",
                                                   extensionString)) {
        // GL_ARB_framebuffer_object doesn't use ARB suffix.
        GR_GL_GET_PROC(GenFramebuffers);
        GR_GL_GET_PROC(BindFramebuffer);
        GR_GL_GET_PROC(FramebufferTexture2D);
        GR_GL_GET_PROC(CheckFramebufferStatus);
        GR_GL_GET_PROC(DeleteFramebuffers);
        GR_GL_GET_PROC(RenderbufferStorage);
        GR_GL_GET_PROC(GenRenderbuffers);
        GR_GL_GET_PROC(DeleteRenderbuffers);
        GR_GL_GET_PROC(FramebufferRenderbuffer);
        GR_GL_GET_PROC(BindRenderbuffer);
        GR_GL_GET_PROC(RenderbufferStorageMultisample);
        GR_GL_GET_PROC(BlitFramebuffer);
        fboFound = true;
    }

    #if GL_EXT_framebuffer_object && !GR_MAC_BUILD
    if (!fboFound &&
            has_gl_extension_from_string("GL_EXT_framebuffer_object",
                                         extensionString)) {
        GR_GL_GET_PROC_SUFFIX(GenFramebuffers, EXT);
        GR_GL_GET_PROC_SUFFIX(BindFramebuffer, EXT);
        GR_GL_GET_PROC_SUFFIX(FramebufferTexture2D, EXT);
        GR_GL_GET_PROC_SUFFIX(CheckFramebufferStatus, EXT);
        GR_GL_GET_PROC_SUFFIX(DeleteFramebuffers, EXT);
        GR_GL_GET_PROC_SUFFIX(RenderbufferStorage, EXT);
        GR_GL_GET_PROC_SUFFIX(GenRenderbuffers, EXT);
        GR_GL_GET_PROC_SUFFIX(DeleteRenderbuffers, EXT);
        GR_GL_GET_PROC_SUFFIX(FramebufferRenderbuffer, EXT);
        GR_GL_GET_PROC_SUFFIX(BindRenderbuffer, EXT);
        fboFound = true;

        if (has_gl_extension_from_string("GL_EXT_framebuffer_multisample",
                                         extensionString)) {
            GR_GL_GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
        }
        if (has_gl_extension_from_string("GL_EXT_framebuffer_blit",
                                         extensionString)) {
            GR_GL_GET_PROC_SUFFIX(BlitFramebuffer, EXT);
        }
    }
    #endif

    // we assume we have at least GL 1.5 or higher (VBOs introduced in 1.5)
    GrAssert((major == 1 && minor >= 5) || major >=2);
    GR_GL_GET_PROC(MapBuffer);
    GR_GL_GET_PROC(UnmapBuffer);
#else // !GR_SUPPORT_GLDESKTOP
    #if GR_SUPPORT_GLES2
    if (!fboFound && major >= 2) {// ES 2.0 supports FBO
        GR_GL_GET_PROC(GenFramebuffers);
        GR_GL_GET_PROC(BindFramebuffer);
        GR_GL_GET_PROC(FramebufferTexture2D);
        GR_GL_GET_PROC(CheckFramebufferStatus);
        GR_GL_GET_PROC(DeleteFramebuffers);
        GR_GL_GET_PROC(RenderbufferStorage);
        GR_GL_GET_PROC(GenRenderbuffers);
        GR_GL_GET_PROC(DeleteRenderbuffers);
        GR_GL_GET_PROC(FramebufferRenderbuffer);
        GR_GL_GET_PROC(BindRenderbuffer);
        fboFound = true;
    }
    #endif

    #if GL_OES_mapbuffer
    if (!fboFound &&
            has_gl_extension_from_string("GL_OES_framebuffer_object",
                                         extensionString)) {
        GR_GL_GET_PROC_SUFFIX(GenFramebuffers, OES);
        GR_GL_GET_PROC_SUFFIX(BindFramebuffer, OES);
        GR_GL_GET_PROC_SUFFIX(FramebufferTexture2D, OES);
        GR_GL_GET_PROC_SUFFIX(CheckFramebufferStatus, OES);
        GR_GL_GET_PROC_SUFFIX(DeleteFramebuffers, OES);
        GR_GL_GET_PROC_SUFFIX(RenderbufferStorage, OES);
        GR_GL_GET_PROC_SUFFIX(GenRenderbuffers, OES);
        GR_GL_GET_PROC_SUFFIX(DeleteRenderbuffers, OES);
        GR_GL_GET_PROC_SUFFIX(FramebufferRenderbuffer, OES);
        GR_GL_GET_PROC_SUFFIX(BindRenderbuffer, OES);
        fboFound = true;
    }
    #endif

    #if GL_APPLE_framebuffer_multisample
    if (has_gl_extension_from_string("GL_APPLE_framebuffer_multisample",
                                     extensionString)) {
        GR_GL_GET_PROC_SUFFIX(ResolveMultisampleFramebuffer, APPLE);
    }
    #endif

    #if GL_IMG_multisampled_render_to_texture
    if (has_gl_extension_from_string(
                "GL_IMG_multisampled_render_to_texture", extensionString)) {
        GR_GL_GET_PROC_SUFFIX(FramebufferTexture2DMultisample, IMG);
    }
    #endif

    #if GL_OES_mapbuffer
    if (has_gl_extension_from_string("GL_OES_mapbuffer", extensionString)) {
        GR_GL_GET_PROC_SUFFIX(MapBuffer, OES);
        GR_GL_GET_PROC_SUFFIX(UnmapBuffer, OES);
    }
    #endif
#endif  // !GR_SUPPORT_GLDESKTOP

    if (!fboFound) {
        // we require some form of FBO
        GrAssert(!"No FBOs supported?");
    }
}

void GrGLInitializeGLInterface(GrGLInterface* glBindings) {
    Gr_bzero(glBindings, sizeof(GrGLInterface));

#if GR_SUPPORT_GLDESKTOP || GR_SUPPORT_GLES1
    // These entry points only exist on desktop GL implementations.
    GR_GL_GET_PROC_SYMBOL(Color4ub);
    GR_GL_GET_PROC_SYMBOL(ColorPointer);
    GR_GL_GET_PROC_SYMBOL(DisableClientState);
    GR_GL_GET_PROC_SYMBOL(EnableClientState);
    GR_GL_GET_PROC_SYMBOL(LoadMatrixf);
    GR_GL_GET_PROC_SYMBOL(MatrixMode);
    GR_GL_GET_PROC_SYMBOL(PointSize);
    GR_GL_GET_PROC_SYMBOL(ShadeModel);
    GR_GL_GET_PROC_SYMBOL(TexCoordPointer);
    GR_GL_GET_PROC_SYMBOL(TexEnvi);
    GR_GL_GET_PROC_SYMBOL(VertexPointer);
    GR_GL_GET_PROC(ClientActiveTexture);
#endif

    // The following gl entry points are part of GL 1.1, and will always be
    // exported as symbols.
    // Note that on windows, the wglGetProcAddress call will fail to retrieve
    // these entry points.
    GR_GL_GET_PROC_SYMBOL(BlendFunc);
    GR_GL_GET_PROC_SYMBOL(Clear);
    GR_GL_GET_PROC_SYMBOL(ClearColor);
    GR_GL_GET_PROC_SYMBOL(ClearStencil);
    GR_GL_GET_PROC_SYMBOL(ColorMask);
    GR_GL_GET_PROC_SYMBOL(CullFace);
    GR_GL_GET_PROC_SYMBOL(DeleteTextures);
    GR_GL_GET_PROC_SYMBOL(DepthMask);
    GR_GL_GET_PROC_SYMBOL(Disable);
    GR_GL_GET_PROC_SYMBOL(DrawArrays);
    GR_GL_GET_PROC_SYMBOL(DrawElements);
    GR_GL_GET_PROC_SYMBOL(Enable);
    GR_GL_GET_PROC_SYMBOL(FrontFace);
    GR_GL_GET_PROC_SYMBOL(GenTextures);
    GR_GL_GET_PROC_SYMBOL(GetError);
    GR_GL_GET_PROC_SYMBOL(GetIntegerv);
    GR_GL_GET_PROC_SYMBOL(GetString);
    GR_GL_GET_PROC_SYMBOL(LineWidth);
    GR_GL_GET_PROC_SYMBOL(PixelStorei);
    GR_GL_GET_PROC_SYMBOL(ReadPixels);
    GR_GL_GET_PROC_SYMBOL(Scissor);
    GR_GL_GET_PROC_SYMBOL(StencilFunc);
    GR_GL_GET_PROC_SYMBOL(StencilMask);
    GR_GL_GET_PROC_SYMBOL(StencilOp);
    GR_GL_GET_PROC_SYMBOL(TexImage2D);
    GR_GL_GET_PROC_SYMBOL(TexParameteri);
    GR_GL_GET_PROC_SYMBOL(TexSubImage2D);
    GR_GL_GET_PROC_SYMBOL(Viewport);

    // Capture the remaining entry points as gl extensions.
    GR_GL_GET_PROC(ActiveTexture);
    GR_GL_GET_PROC(AttachShader);
    GR_GL_GET_PROC(BindAttribLocation);
    GR_GL_GET_PROC(BindBuffer);
    GR_GL_GET_PROC(BindTexture);
    GR_GL_GET_PROC(BlendColor);
    GR_GL_GET_PROC(BufferData);
    GR_GL_GET_PROC(BufferSubData);
    GR_GL_GET_PROC(CompileShader);
    GR_GL_GET_PROC(CompressedTexImage2D);
    GR_GL_GET_PROC(CreateProgram);
    GR_GL_GET_PROC(CreateShader);
    GR_GL_GET_PROC(DeleteBuffers);
    GR_GL_GET_PROC(DeleteProgram);
    GR_GL_GET_PROC(DeleteShader);
    GR_GL_GET_PROC(DisableVertexAttribArray);
    GR_GL_GET_PROC(EnableVertexAttribArray);
    GR_GL_GET_PROC(GenBuffers);
    GR_GL_GET_PROC(GetBufferParameteriv);
    GR_GL_GET_PROC(GetProgramInfoLog);
    GR_GL_GET_PROC(GetProgramiv);
    GR_GL_GET_PROC(GetShaderInfoLog);
    GR_GL_GET_PROC(GetShaderiv);
    GR_GL_GET_PROC(GetUniformLocation);
    GR_GL_GET_PROC(LinkProgram);
    GR_GL_GET_PROC(ShaderSource);
    GR_GL_GET_PROC(StencilFuncSeparate);
    GR_GL_GET_PROC(StencilMaskSeparate);
    GR_GL_GET_PROC(StencilOpSeparate);
    GR_GL_GET_PROC(Uniform1fv);
    GR_GL_GET_PROC(Uniform1i);
    GR_GL_GET_PROC(Uniform4fv);
    GR_GL_GET_PROC(UniformMatrix3fv);
    GR_GL_GET_PROC(UseProgram);
    GR_GL_GET_PROC(VertexAttrib4fv);
    GR_GL_GET_PROC(VertexAttribPointer);

    InitializeGLInterfaceExtensions(glBindings);
}

}  // unnamed namespace

void GrGLSetGLInterface(GrGLInterface* gl_interface) {
    gGLInterface = gl_interface;
}

GrGLInterface* GrGLGetGLInterface() {
    return gGLInterface;
}

void GrGLSetDefaultGLInterface() {
    static GrGLInterface gDefaultInterface;
    static bool gDefaultInitialized = false;
    GrAssert(!gDefaultInitialized);

    if (!gDefaultInitialized) {
        GrGLInitializeGLInterface(&gDefaultInterface);
        GrGLSetGLInterface(&gDefaultInterface);
    }
}

bool has_gl_extension(const char* ext) {
    const char* glstr = reinterpret_cast<const char*>(
                GrGLGetGLInterface()->fGetString(GL_EXTENSIONS));

    return has_gl_extension_from_string(ext, glstr);
}

void gl_version(int* major, int* minor) {
    const char* v = reinterpret_cast<const char*>(
                GrGLGetGLInterface()->fGetString(GL_VERSION));
    gl_version_from_string(major, minor, v);
}
