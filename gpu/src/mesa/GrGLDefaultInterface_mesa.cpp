
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLInterface.h"

#include "GL/osmesa.h"
#include <GL/glext.h>
#include <GL/glu.h>

#define GR_GL_GET_PROC(F) defaultInterface->f ## F = (GrGL ## F ## Proc) \
        OSMesaGetProcAddress("gl" #F);
#define GR_GL_GET_PROC_SUFFIX(F, S) defaultInterface->f ## F = (GrGL ## F ## Proc) \
        OSMesaGetProcAddress("gl" #F #S);

void GrGLInitializeDefaultGLInterface() {
    if (NULL != OSMesaGetCurrentContext()) {
        GrGLInterface defaultInterface = new GrGLInterface();
        int major, minor;
        const char* versionString = (const char*) glGetString(GL_VERSION);
        const char* extString = (const char*) glGetString(GL_EXTENSIONS);
        gl_version_from_string(&major, &minor, versionString);

        if (major == 1 && minor < 5) {
            // We must have array and element_array buffer objects.
            return;
        }
        defaultInterface->fNPOTRenderTargetSupport = kProbe_GrGLCapability;
        defaultInterface->fMinRenderTargetHeight = kProbe_GrGLCapability;
        defaultInterface->fMinRenderTargetWidth = kProbe_GrGLCapability;

        defaultInterface->fActiveTexture = glActiveTexture;
        GR_GL_GET_PROC(AttachShader);
        GR_GL_GET_PROC(BindAttribLocation);
        GR_GL_GET_PROC(BindBuffer);
        defaultInterface->fBindTexture = glBindTexture;
        defaultInterface->fBlendColor = glBlendColor;
        defaultInterface->fBlendFunc = glBlendFunc;
        GR_GL_GET_PROC(BufferData);
        GR_GL_GET_PROC(BufferSubData);
        defaultInterface->fClear = glClear;
        defaultInterface->fClearColor = glClearColor;
        defaultInterface->fClearStencil = glClearStencil;
        defaultInterface->fClientActiveTexture = glClientActiveTexture;
        defaultInterface->fColorMask = glColorMask;
        defaultInterface->fColorPointer = glColorPointer;
        defaultInterface->fColor4ub = glColor4ub;
        GR_GL_GET_PROC(CompileShader);
        defaultInterface->fCompressedTexImage2D = glCompressedTexImage2D;
        GR_GL_GET_PROC(CreateProgram);
        GR_GL_GET_PROC(CreateShader);
        defaultInterface->fCullFace = glCullFace;
        GR_GL_GET_PROC(DeleteBuffers);
        GR_GL_GET_PROC(DeleteProgram);
        GR_GL_GET_PROC(DeleteShader);
        defaultInterface->fDeleteTextures = glDeleteTextures;
        defaultInterface->fDepthMask = glDepthMask;
        defaultInterface->fDisable = glDisable;
        defaultInterface->fDisableClientState = glDisableClientState;
        GR_GL_GET_PROC(DisableVertexAttribArray);
        defaultInterface->fDrawArrays = glDrawArrays;
        defaultInterface->fDrawBuffer = glDrawBuffer;
        GR_GL_GET_PROC(DrawBuffers);
        defaultInterface->fDrawElements = glDrawElements;
        defaultInterface->fEnable = glEnable;
        defaultInterface->fEnableClientState = glEnableClientState;
        GR_GL_GET_PROC(EnableVertexAttribArray);
        defaultInterface->fFrontFace = glFrontFace;
        GR_GL_GET_PROC(GenBuffers);
        GR_GL_GET_PROC(GetBufferParameteriv);
        defaultInterface->fGetError = glGetError;
        defaultInterface->fGetIntegerv = glGetIntegerv;
        GR_GL_GET_PROC(GetProgramInfoLog);
        GR_GL_GET_PROC(GetProgramiv);
        GR_GL_GET_PROC(GetShaderInfoLog);
        GR_GL_GET_PROC(GetShaderiv);
        defaultInterface->fGetString = glGetString;
        defaultInterface->fGetTexLevelParameteriv = glGetTexLevelParameteriv;
        defaultInterface->fGenTextures = glGenTextures;
        GR_GL_GET_PROC(GetUniformLocation);
        defaultInterface->fLineWidth = glLineWidth;
        GR_GL_GET_PROC(LinkProgram);
        defaultInterface->fLoadMatrixf = glLoadMatrixf;
        GR_GL_GET_PROC(MapBuffer);
        defaultInterface->fMatrixMode = glMatrixMode;
        defaultInterface->fPointSize = glPointSize;
        defaultInterface->fPixelStorei = glPixelStorei;
        defaultInterface->fReadBuffer = glReadBuffer;
        defaultInterface->fReadPixels = glReadPixels;
        defaultInterface->fScissor = glScissor;
        defaultInterface->fShadeModel = glShadeModel;
        GR_GL_GET_PROC(ShaderSource);
        defaultInterface->fStencilFunc = glStencilFunc;
        GR_GL_GET_PROC(StencilFuncSeparate);
        defaultInterface->fStencilMask = glStencilMask;
        GR_GL_GET_PROC(StencilMaskSeparate);
        defaultInterface->fStencilOp = glStencilOp;
        GR_GL_GET_PROC(StencilOpSeparate);
        defaultInterface->fTexCoordPointer = glTexCoordPointer;
        defaultInterface->fTexEnvi = glTexEnvi;
        //OSMesa on Mac's glTexImage2D takes a GLenum for internalFormat rather than a GLint.
        defaultInterface->fTexImage2D = reinterpret_cast<GrGLTexImage2DProc>(glTexImage2D);
        defaultInterface->fTexParameteri = glTexParameteri;
        defaultInterface->fTexSubImage2D = glTexSubImage2D;
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
        defaultInterface->fVertexPointer = glVertexPointer;
        defaultInterface->fViewport = glViewport;

        // First look for GL3.0 FBO or GL_ARB_framebuffer_object (same since
        // GL_ARB_framebuffer_object doesn't use ARB suffix.)
        if (major >= 3 || has_gl_extension_from_string(
                "GL_ARB_framebuffer_object", extString)) {
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
        } else if (has_gl_extension_from_string("GL_EXT_framebuffer_object",
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
        GR_GL_GET_PROC(BindFragDataLocationIndexed);
        defaultInterface->fBindingsExported = kDesktop_GrGLBinding;
        GrGLSetDefaultGLInterface(defaultInterface)->unref();
    }
}
