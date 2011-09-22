
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLInterface.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#define GR_GL_GET_PROC(F) interface->f ## F = (GrGL ## F ## Proc) \
        glXGetProcAddress(reinterpret_cast<const GLubyte*>("gl" #F));
#define GR_GL_GET_PROC_SUFFIX(F, S) interface->f ## F = (GrGL ## F ## Proc) \
        glXGetProcAddress(reinterpret_cast<const GLubyte*>("gl" #F #S));

const GrGLInterface* GrGLDefaultInterface() {
    if (NULL != glXGetCurrentContext()) {
        const char* versionString = (const char*) glGetString(GL_VERSION);
        const char* extString = (const char*) glGetString(GL_EXTENSIONS);
        GrGLVersion glVer = GrGLGetVersionFromString(versionString);

        if (glVer < GR_GL_VER(1,5)) {
            // We must have array and element_array buffer objects.
            return NULL;
        }

        GrGLInterface* interface = new GrGLInterface();

        interface->fNPOTRenderTargetSupport = kProbe_GrGLCapability;
        interface->fMinRenderTargetHeight = kProbe_GrGLCapability;
        interface->fMinRenderTargetWidth = kProbe_GrGLCapability;

        interface->fActiveTexture = glActiveTexture;
        GR_GL_GET_PROC(AttachShader);
        GR_GL_GET_PROC(BindAttribLocation);
        GR_GL_GET_PROC(BindBuffer);
        GR_GL_GET_PROC(BindFragDataLocation);
        interface->fBindTexture = glBindTexture;
        interface->fBlendColor = glBlendColor;
        interface->fBlendFunc = glBlendFunc;
        GR_GL_GET_PROC(BufferData);
        GR_GL_GET_PROC(BufferSubData);
        interface->fClear = glClear;
        interface->fClearColor = glClearColor;
        interface->fClearStencil = glClearStencil;
        interface->fClientActiveTexture = glClientActiveTexture;
        interface->fColorMask = glColorMask;
        interface->fColorPointer = glColorPointer;
        interface->fColor4ub = glColor4ub;
        GR_GL_GET_PROC(CompileShader);
        interface->fCompressedTexImage2D = glCompressedTexImage2D;
        GR_GL_GET_PROC(CreateProgram);
        GR_GL_GET_PROC(CreateShader);
        interface->fCullFace = glCullFace;
        GR_GL_GET_PROC(DeleteBuffers);
        GR_GL_GET_PROC(DeleteProgram);
        GR_GL_GET_PROC(DeleteShader);
        interface->fDeleteTextures = glDeleteTextures;
        interface->fDepthMask = glDepthMask;
        interface->fDisable = glDisable;
        interface->fDisableClientState = glDisableClientState;
        GR_GL_GET_PROC(DisableVertexAttribArray);
        interface->fDrawArrays = glDrawArrays;
        interface->fDrawBuffer = glDrawBuffer;
        GR_GL_GET_PROC(DrawBuffers);
        interface->fDrawElements = glDrawElements;
        interface->fEnable = glEnable;
        interface->fEnableClientState = glEnableClientState;
        GR_GL_GET_PROC(EnableVertexAttribArray);
        interface->fFrontFace = glFrontFace;
        GR_GL_GET_PROC(GenBuffers);
        GR_GL_GET_PROC(GetBufferParameteriv);
        interface->fGetError = glGetError;
        interface->fGetIntegerv = glGetIntegerv;
        GR_GL_GET_PROC(GetProgramInfoLog);
        GR_GL_GET_PROC(GetProgramiv);
        GR_GL_GET_PROC(GetShaderInfoLog);
        GR_GL_GET_PROC(GetShaderiv);
        interface->fGetString = glGetString;
        interface->fGetTexLevelParameteriv = glGetTexLevelParameteriv;
        interface->fGenTextures = glGenTextures;
        GR_GL_GET_PROC(GetUniformLocation);
        interface->fLineWidth = glLineWidth;
        GR_GL_GET_PROC(LinkProgram);
        interface->fLoadMatrixf = glLoadMatrixf;
        GR_GL_GET_PROC(MapBuffer);
        interface->fMatrixMode = glMatrixMode;
        interface->fPointSize = glPointSize;
        interface->fPixelStorei = glPixelStorei;
        interface->fReadBuffer = glReadBuffer;
        interface->fReadPixels = glReadPixels;
        interface->fScissor = glScissor;
        interface->fShadeModel = glShadeModel;
        GR_GL_GET_PROC(ShaderSource);
        interface->fStencilFunc = glStencilFunc;
        GR_GL_GET_PROC(StencilFuncSeparate);
        interface->fStencilMask = glStencilMask;
        GR_GL_GET_PROC(StencilMaskSeparate);
        interface->fStencilOp = glStencilOp;
        GR_GL_GET_PROC(StencilOpSeparate);
        interface->fTexCoordPointer = glTexCoordPointer;
        interface->fTexEnvi = glTexEnvi;
        interface->fTexImage2D = glTexImage2D;
        interface->fTexParameteri = glTexParameteri;
        interface->fTexSubImage2D = glTexSubImage2D;
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
        interface->fVertexPointer = glVertexPointer;
        interface->fViewport = glViewport;
        GR_GL_GET_PROC(BindFragDataLocationIndexed);

        // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
        // GL_ARB_framebuffer_object doesn't use ARB suffix.)
        if (glVer >= GR_GL_VER(3,0) ||
            GrGLHasExtensionFromString("GL_ARB_framebuffer_object",
                                       extString)) {
            GR_GL_GET_PROC(GenFramebuffers);
            GR_GL_GET_PROC(GetFramebufferAttachmentParameteriv);
            GR_GL_GET_PROC(GetRenderbufferParameteriv);
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
        } else if (GrGLHasExtensionFromString("GL_EXT_framebuffer_object",
                                              extString)) {
            GR_GL_GET_PROC_SUFFIX(GenFramebuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(GetFramebufferAttachmentParameteriv, EXT);
            GR_GL_GET_PROC_SUFFIX(GetRenderbufferParameteriv, EXT);
            GR_GL_GET_PROC_SUFFIX(BindFramebuffer, EXT);
            GR_GL_GET_PROC_SUFFIX(FramebufferTexture2D, EXT);
            GR_GL_GET_PROC_SUFFIX(CheckFramebufferStatus, EXT);
            GR_GL_GET_PROC_SUFFIX(DeleteFramebuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(RenderbufferStorage, EXT);
            GR_GL_GET_PROC_SUFFIX(GenRenderbuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(DeleteRenderbuffers, EXT);
            GR_GL_GET_PROC_SUFFIX(FramebufferRenderbuffer, EXT);
            GR_GL_GET_PROC_SUFFIX(BindRenderbuffer, EXT);
            if (GrGLHasExtensionFromString("GL_EXT_framebuffer_multisample",
                                             extString)) {
                GR_GL_GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
            }
            if (GrGLHasExtensionFromString("GL_EXT_framebuffer_blit",
                                             extString)) {
                GR_GL_GET_PROC_SUFFIX(BlitFramebuffer, EXT);
            }
        } else {
            // we must have FBOs
            delete interface;
            return NULL;
        }
        interface->fBindingsExported = kDesktop_GrGLBinding;

        return interface;
    } else {
        return NULL;
    }
}
