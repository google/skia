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

#include <Windows.h>
#include <GL/GL.h>

/*
 * Windows makes the GL funcs all be __stdcall instead of __cdecl :(
 * This implementation will only work if GR_GL_FUNCTION_TYPE is __stdcall.
 * Otherwise, a springboard would be needed that hides the calling convention.
 */

#define GR_GL_GET_PROC(F) gDefaultInterface.f ## F = (GrGL ## F ## Proc) wglGetProcAddress("gl" #F);
#define GR_GL_GET_PROC_SUFFIX(F, S) gDefaultInterface.f ## F = (GrGL ## F ## Proc) wglGetProcAddress("gl" #F #S);

void GrGLSetDefaultGLInterface() {
    static GrGLInterface gDefaultInterface;
    static bool gDefaultInterfaceInit;
    if (!gDefaultInterfaceInit) {

        // wglGetProcAddress requires a context.
        if (NULL != wglGetCurrentContext()) {
            int major, minor;
            const char* versionString = (const char*) glGetString(GL_VERSION);
            const char* extString = (const char*) glGetString(GL_EXTENSIONS);
            gl_version_from_string(&major, &minor, versionString);

            if (major == 1 && minor < 5) {
                // We must have array and element_array buffer objects.
                return;
            }

            gDefaultInterface.fNPOTRenderTargetSupport = kProbe_GrGLCapability;
            gDefaultInterface.fMinRenderTargetHeight = kProbe_GrGLCapability;
            gDefaultInterface.fMinRenderTargetWidth = kProbe_GrGLCapability;

            // Functions that are part of GL 1.1 will return NULL in
            // wglGetProcAddress
            gDefaultInterface.fBlendFunc = glBlendFunc;
            gDefaultInterface.fClear = glClear;
            gDefaultInterface.fClearColor = glClearColor;
            gDefaultInterface.fClearStencil = glClearStencil;
            gDefaultInterface.fColor4ub = glColor4ub;
            gDefaultInterface.fColorMask = glColorMask;
            gDefaultInterface.fColorPointer = glColorPointer;
            gDefaultInterface.fCullFace = glCullFace;
            gDefaultInterface.fDeleteTextures = glDeleteTextures;
            gDefaultInterface.fDepthMask = glDepthMask;
            gDefaultInterface.fDisable = glDisable;
            gDefaultInterface.fDisableClientState = glDisableClientState;
            gDefaultInterface.fDrawArrays = glDrawArrays;
            gDefaultInterface.fDrawElements = glDrawElements;
            gDefaultInterface.fEnable = glEnable;
            gDefaultInterface.fEnableClientState = glEnableClientState;
            gDefaultInterface.fFrontFace = glFrontFace;
            gDefaultInterface.fGenTextures = glGenTextures;
            gDefaultInterface.fGetError = glGetError;
            gDefaultInterface.fGetIntegerv = glGetIntegerv;
            gDefaultInterface.fGetString = glGetString;
            gDefaultInterface.fLineWidth = glLineWidth;
            gDefaultInterface.fLoadMatrixf = glLoadMatrixf;
            gDefaultInterface.fMatrixMode = glMatrixMode;
            gDefaultInterface.fPixelStorei = glPixelStorei;
            gDefaultInterface.fPointSize = glPointSize;
            gDefaultInterface.fReadPixels = glReadPixels;
            gDefaultInterface.fScissor = glScissor;
            gDefaultInterface.fShadeModel = glShadeModel;
            gDefaultInterface.fStencilFunc = glStencilFunc;
            gDefaultInterface.fStencilMask = glStencilMask;
            gDefaultInterface.fStencilOp = glStencilOp;
            gDefaultInterface.fTexImage2D = glTexImage2D;
            gDefaultInterface.fTexParameteri = glTexParameteri;
            gDefaultInterface.fTexCoordPointer = glTexCoordPointer;
            gDefaultInterface.fTexEnvi = glTexEnvi;
            gDefaultInterface.fTexSubImage2D = glTexSubImage2D;
            gDefaultInterface.fViewport = glViewport;
            gDefaultInterface.fVertexPointer = glVertexPointer;

            GR_GL_GET_PROC(ActiveTexture);
            GR_GL_GET_PROC(AttachShader);
            GR_GL_GET_PROC(BindAttribLocation);
            GR_GL_GET_PROC(BindBuffer);
            GR_GL_GET_PROC(BindTexture);
            GR_GL_GET_PROC(BlendColor);
            GR_GL_GET_PROC(BufferData);
            GR_GL_GET_PROC(BufferSubData);
            GR_GL_GET_PROC(ClientActiveTexture);
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
            GR_GL_GET_PROC(Uniform1f);
            GR_GL_GET_PROC(Uniform1i);
            GR_GL_GET_PROC(Uniform1fv);
            GR_GL_GET_PROC(Uniform1iv);
            GR_GL_GET_PROC(Uniform2f);
            GR_GL_GET_PROC(Uniform2i);
            GR_GL_GET_PROC(Uniform2fv);
            GR_GL_GET_PROC(Uniform2iv);
            GR_GL_GET_PROC(Uniform3f);
            GR_GL_GET_PROC(Uniform3i);
            GR_GL_GET_PROC(Uniform3fv);
            GR_GL_GET_PROC(Uniform3iv);
            GR_GL_GET_PROC(Uniform4f);
            GR_GL_GET_PROC(Uniform4i);
            GR_GL_GET_PROC(Uniform4fv);
            GR_GL_GET_PROC(Uniform4iv);
            GR_GL_GET_PROC(UniformMatrix2fv);
            GR_GL_GET_PROC(UniformMatrix3fv);
            GR_GL_GET_PROC(UniformMatrix4fv);
            GR_GL_GET_PROC(UseProgram);
            GR_GL_GET_PROC(VertexAttrib4fv);
            GR_GL_GET_PROC(VertexAttribPointer);
            GR_GL_GET_PROC(BindFragDataLocationIndexed);

            // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
            // GL_ARB_framebuffer_object doesn't use ARB suffix.)
            if (major >= 3 || has_gl_extension_from_string("GL_ARB_framebuffer_object", extString)) {
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
            } else if (has_gl_extension_from_string("GL_EXT_framebuffer_object", extString)) {
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
                if (has_gl_extension_from_string("GL_EXT_framebuffer_multisample", extString)) {
                    GR_GL_GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
                }
                if (has_gl_extension_from_string("GL_EXT_framebuffer_blit", extString)) {
                    GR_GL_GET_PROC_SUFFIX(BlitFramebuffer, EXT);
                }
            } else {
                // we must have FBOs
                return;
            }
            GR_GL_GET_PROC(MapBuffer);
            GR_GL_GET_PROC(UnmapBuffer);

            gDefaultInterface.fBindingsExported = kDesktop_GrGLBinding;

            gDefaultInterfaceInit = true;
        }
    }
    if (gDefaultInterfaceInit) {
        GrGLSetGLInterface(&gDefaultInterface);
    }
}
