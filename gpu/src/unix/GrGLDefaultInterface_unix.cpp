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

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#define GR_GL_GET_PROC(F) gDefaultInterface.f ## F = (GrGL ## F ## Proc) \
        glXGetProcAddress(reinterpret_cast<const GLubyte*>("gl" #F));
#define GR_GL_GET_PROC_SUFFIX(F, S) gDefaultInterface.f ## F = (GrGL ## F ## Proc) \
        glXGetProcAddress(reinterpret_cast<const GLubyte*>("gl" #F #S));

void GrGLSetDefaultGLInterface() {
    static GrGLInterface gDefaultInterface;
    static bool gDefaultInterfaceInit;
    if (!gDefaultInterfaceInit && NULL != glXGetCurrentContext()) {
        int major, minor;
        const char* versionString = (const char*) glGetString(GL_VERSION);
        const char* extString = (const char*) glGetString(GL_EXTENSIONS);
        gl_version_from_string(&major, &minor, versionString);

        if (major == 1 && minor < 5) {
            // We must have array and element_array buffer objects.
            return;
        }

        gDefaultInterface.fActiveTexture = glActiveTexture;
        GR_GL_GET_PROC(AttachShader);
        GR_GL_GET_PROC(BindAttribLocation);
        GR_GL_GET_PROC(BindBuffer);
        gDefaultInterface.fBindTexture = glBindTexture;
        gDefaultInterface.fBlendColor = glBlendColor;
        gDefaultInterface.fBlendFunc = glBlendFunc;
        GR_GL_GET_PROC(BufferData);
        GR_GL_GET_PROC(BufferSubData);
        gDefaultInterface.fClear = glClear;
        gDefaultInterface.fClearColor = glClearColor;
        gDefaultInterface.fClearStencil = glClearStencil;
        gDefaultInterface.fClientActiveTexture = glClientActiveTexture;
        gDefaultInterface.fColorMask = glColorMask;
        gDefaultInterface.fColorPointer = glColorPointer;
        gDefaultInterface.fColor4ub = glColor4ub;
        GR_GL_GET_PROC(CompileShader);
        gDefaultInterface.fCompressedTexImage2D = glCompressedTexImage2D;
        GR_GL_GET_PROC(CreateProgram);
        GR_GL_GET_PROC(CreateShader);
        gDefaultInterface.fCullFace = glCullFace;
        GR_GL_GET_PROC(DeleteBuffers);
        GR_GL_GET_PROC(DeleteProgram);
        GR_GL_GET_PROC(DeleteShader);
        gDefaultInterface.fDeleteTextures = glDeleteTextures;
        gDefaultInterface.fDepthMask = glDepthMask;
        gDefaultInterface.fDisable = glDisable;
        gDefaultInterface.fDisableClientState = glDisableClientState;
        GR_GL_GET_PROC(DisableVertexAttribArray);
        gDefaultInterface.fDrawArrays = glDrawArrays;
        gDefaultInterface.fDrawElements = glDrawElements;
        gDefaultInterface.fEnable = glEnable;
        gDefaultInterface.fEnableClientState = glEnableClientState;
        GR_GL_GET_PROC(EnableVertexAttribArray);
        gDefaultInterface.fFrontFace = glFrontFace;
        GR_GL_GET_PROC(GenBuffers);
        GR_GL_GET_PROC(GetBufferParameteriv);
        gDefaultInterface.fGetError = glGetError;
        gDefaultInterface.fGetIntegerv = glGetIntegerv;
        GR_GL_GET_PROC(GetProgramInfoLog);
        GR_GL_GET_PROC(GetProgramiv);
        GR_GL_GET_PROC(GetShaderInfoLog);
        GR_GL_GET_PROC(GetShaderiv);
        gDefaultInterface.fGetString = glGetString;
        gDefaultInterface.fGenTextures = glGenTextures;
        GR_GL_GET_PROC(GetUniformLocation);
        gDefaultInterface.fLineWidth = glLineWidth;
        GR_GL_GET_PROC(LinkProgram);
        gDefaultInterface.fLoadMatrixf = glLoadMatrixf;
        GR_GL_GET_PROC(MapBuffer);
        gDefaultInterface.fMatrixMode = glMatrixMode;
        gDefaultInterface.fPointSize = glPointSize;
        gDefaultInterface.fPixelStorei = glPixelStorei;
        gDefaultInterface.fReadPixels = glReadPixels;
        gDefaultInterface.fScissor = glScissor;
        gDefaultInterface.fShadeModel = glShadeModel;
        GR_GL_GET_PROC(ShaderSource);
        gDefaultInterface.fStencilFunc = glStencilFunc;
        GR_GL_GET_PROC(StencilFuncSeparate);
        gDefaultInterface.fStencilMask = glStencilMask;
        GR_GL_GET_PROC(StencilMaskSeparate);
        gDefaultInterface.fStencilOp = glStencilOp;
        GR_GL_GET_PROC(StencilOpSeparate);
        gDefaultInterface.fTexCoordPointer = glTexCoordPointer;
        gDefaultInterface.fTexEnvi = glTexEnvi;
        gDefaultInterface.fTexImage2D = glTexImage2D;
        gDefaultInterface.fTexParameteri = glTexParameteri;
        gDefaultInterface.fTexSubImage2D = glTexSubImage2D;
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
        GR_GL_GET_PROC(UnmapBuffer);
        GR_GL_GET_PROC(UseProgram);
        GR_GL_GET_PROC(VertexAttrib4fv);
        GR_GL_GET_PROC(VertexAttribPointer);
        gDefaultInterface.fVertexPointer = glVertexPointer;
        gDefaultInterface.fViewport = glViewport;
        GR_GL_GET_PROC(BindFragDataLocationIndexed);

        // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
        // GL_ARB_framebuffer_object doesn't use ARB suffix.)
        if (major >= 3 || has_gl_extension_from_string(
                "GL_ARB_framebuffer_object", extString)) {
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
        } else if (has_gl_extension_from_string("GL_EXT_framebuffer_object",
                                                extString)) {
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
            if (has_gl_extension_from_string("GL_EXT_framebuffer_multisample",
                                             extString)) {
                GR_GL_GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
            }
            if (has_gl_extension_from_string("GL_EXT_framebuffer_blit",
                                             extString)) {
                GR_GL_GET_PROC_SUFFIX(BlitFramebuffer, EXT);
            }
        } else {
            // we must have FBOs
            return;
        }
        gDefaultInterface.fBindingsExported = kDesktop_GrGLBinding;

        gDefaultInterfaceInit = true;
    }
    if (gDefaultInterfaceInit)
        GrGLSetGLInterface(&gDefaultInterface);
}
