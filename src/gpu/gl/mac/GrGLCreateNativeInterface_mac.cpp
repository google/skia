
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "../GrGLUtil.h"

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#include <dlfcn.h>

static void* GetProcAddress(const char* name) {
    return dlsym(RTLD_DEFAULT, name);
}

#define GET_PROC(name) (interface->f ## name = ((GrGL ## name ## Proc) GetProcAddress("gl" #name)))
#define GET_PROC_SUFFIX(name, suffix) (interface->f ## name = ((GrGL ## name ## Proc) GetProcAddress("gl" #name #suffix)))

const GrGLInterface* GrGLCreateNativeInterface() {
    // The gl functions are not context-specific so we create one global
    // interface
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = new GrGLInterface;
        glInterface.reset(interface);
        const char* verStr = (const char*) glGetString(GL_VERSION);
        GrGLVersion ver = GrGLGetVersionFromString(verStr);
        const char* extStr = (const char*) glGetString(GL_EXTENSIONS);

        interface->fBindingsExported = kDesktop_GrGLBinding;
        interface->fActiveTexture = glActiveTexture;
        interface->fAttachShader = glAttachShader;
        interface->fBeginQuery = glBeginQuery;
        interface->fBindAttribLocation = glBindAttribLocation;
        interface->fBindBuffer = glBindBuffer;
        if (ver >= GR_GL_VER(3,0)) {
            #if GL_VERSION_3_0
                interface->fBindFragDataLocation = glBindFragDataLocation;
            #else
                interface->fBindFragDataLocation = GET_PROC(BindFragDataLocation);
            #endif
        }
        interface->fBindTexture = glBindTexture;
        interface->fBlendFunc = glBlendFunc;

        if (ver >= GR_GL_VER(1,4)) {
            interface->fBlendColor = glBlendColor;
        } else if (GrGLHasExtensionFromString("GL_ARB_imaging", extStr) ||
                   GrGLHasExtensionFromString("GL_EXT_blend_color", extStr)) {
            GET_PROC(BlendColor);
        }

        interface->fBufferData = glBufferData;
        interface->fBufferSubData = glBufferSubData;
        interface->fClear = glClear;
        interface->fClearColor = glClearColor;
        interface->fClearStencil = glClearStencil;
        interface->fColorMask = glColorMask;
        interface->fCompileShader = glCompileShader;
        interface->fCompressedTexImage2D = glCompressedTexImage2D;
        interface->fCreateProgram = glCreateProgram;
        interface->fCreateShader = glCreateShader;
        interface->fCullFace = glCullFace;
        interface->fDeleteBuffers = glDeleteBuffers;
        interface->fDeleteProgram = glDeleteProgram;
        interface->fDeleteQueries = glDeleteQueries;
        interface->fDeleteShader = glDeleteShader;
        interface->fDeleteTextures = glDeleteTextures;
        interface->fDepthMask = glDepthMask;
        interface->fDisable = glDisable;
        interface->fDisableVertexAttribArray =
                                            glDisableVertexAttribArray;
        interface->fDrawArrays = glDrawArrays;
        interface->fDrawBuffer = glDrawBuffer;
        interface->fDrawBuffers = glDrawBuffers;
        interface->fDrawElements = glDrawElements;
        interface->fEnable = glEnable;
        interface->fEnableVertexAttribArray = glEnableVertexAttribArray;
        interface->fEndQuery = glEndQuery;
        interface->fFinish = glFinish;
        interface->fFlush = glFlush;
        interface->fFrontFace = glFrontFace;
        interface->fGenBuffers = glGenBuffers;
        interface->fGenQueries = glGenQueries;
        interface->fGetBufferParameteriv = glGetBufferParameteriv;
        interface->fGetError = glGetError;
        interface->fGetIntegerv = glGetIntegerv;
        interface->fGetProgramInfoLog = glGetProgramInfoLog;
        interface->fGetProgramiv = glGetProgramiv;
        interface->fGetQueryiv = glGetQueryiv;
        interface->fGetQueryObjectiv = glGetQueryObjectiv;
        interface->fGetQueryObjectuiv = glGetQueryObjectuiv;
        interface->fGetShaderInfoLog = glGetShaderInfoLog;
        interface->fGetShaderiv = glGetShaderiv;
        interface->fGetString = glGetString;
        interface->fGetTexLevelParameteriv = glGetTexLevelParameteriv;
        interface->fGenTextures = glGenTextures;
        interface->fGetUniformLocation = glGetUniformLocation;
        interface->fLineWidth = glLineWidth;
        interface->fLinkProgram = glLinkProgram;
        interface->fMapBuffer = glMapBuffer;
        interface->fPixelStorei = glPixelStorei;
        interface->fReadBuffer = glReadBuffer;
        interface->fReadPixels = glReadPixels;
        interface->fScissor = glScissor;
        interface->fShaderSource = glShaderSource;
        interface->fStencilFunc = glStencilFunc;
        interface->fStencilFuncSeparate = glStencilFuncSeparate;
        interface->fStencilMask = glStencilMask;
        interface->fStencilMaskSeparate = glStencilMaskSeparate;
        interface->fStencilOp = glStencilOp;
        interface->fStencilOpSeparate = glStencilOpSeparate;
        // mac uses GLenum for internalFormat param (non-standard)
        // amounts to int vs. uint.
        interface->fTexImage2D = (GrGLTexImage2DProc)glTexImage2D;
        interface->fTexParameteri = glTexParameteri;
        interface->fTexParameteriv = glTexParameteriv;
    #if GL_ARB_texture_storage || GL_VERSION_4_2
        interface->fTexStorage2D = glTexStorage2D
    #elif GL_EXT_texture_storage
        interface->fTexStorage2D = glTexStorage2DEXT;
    #else
        if (ver >= GR_GL_VER(4,2) ||
            GrGLHasExtensionFromString("GL_ARB_texture_storage", extStr)) {
            GET_PROC(TexStorage2D);
        } else if (GrGLHasExtensionFromString("GL_EXT_texture_storage", extStr)) {
            GET_PROC_SUFFIX(TexStorage2D, EXT);
        }
    #endif
        interface->fTexSubImage2D = glTexSubImage2D;
        interface->fUniform1f = glUniform1f;
        interface->fUniform1i = glUniform1i;
        interface->fUniform1fv = glUniform1fv;
        interface->fUniform1iv = glUniform1iv;
        interface->fUniform2f = glUniform2f;
        interface->fUniform2i = glUniform2i;
        interface->fUniform2fv = glUniform2fv;
        interface->fUniform2iv = glUniform2iv;
        interface->fUniform3f = glUniform3f;
        interface->fUniform3i = glUniform3i;
        interface->fUniform3fv = glUniform3fv;
        interface->fUniform3iv = glUniform3iv;
        interface->fUniform4f = glUniform4f;
        interface->fUniform4i = glUniform4i;
        interface->fUniform4fv = glUniform4fv;
        interface->fUniform4iv = glUniform4iv;
        interface->fUniform4fv = glUniform4fv;
        interface->fUniformMatrix2fv = glUniformMatrix2fv;
        interface->fUniformMatrix3fv = glUniformMatrix3fv;
        interface->fUniformMatrix4fv = glUniformMatrix4fv;
        interface->fUnmapBuffer = glUnmapBuffer;
        interface->fUseProgram = glUseProgram;
        interface->fVertexAttrib4fv = glVertexAttrib4fv;
        interface->fVertexAttribPointer = glVertexAttribPointer;
        interface->fViewport = glViewport;

        if (ver >= GR_GL_VER(3,3) || GrGLHasExtensionFromString("GL_ARB_timer_query", extStr)) {
            // ARB extension doesn't use the ARB suffix on the function name
            #if GL_ARB_timer_query || GL_VERSION_3_3
                interface->fQueryCounter = glQueryCounter;
                interface->fGetQueryObjecti64v = glGetQueryObjecti64v;
                interface->fGetQueryObjectui64v = glGetQueryObjectui64v;
            #else
                interface->fQueryCounter = GET_PROC(QueryCounter);
                interface->fGetQueryObjecti64v = GET_PROC(GetQueryObjecti64v);
                interface->fGetQueryObjectui64v = GET_PROC(GetQueryObjectui64v);
            #endif
        } else if (GrGLHasExtensionFromString("GL_EXT_timer_query", extStr)) {
            #if GL_EXT_timer_query
                interface->fGetQueryObjecti64v = glGetQueryObjecti64vEXT;
                interface->fGetQueryObjectui64v = glGetQueryObjectui64vEXT;
            #else
                interface->fGetQueryObjecti64v = GET_PROC_SUFFIX(GetQueryObjecti64v, EXT);
                interface->fGetQueryObjectui64v = GET_PROC_SUFFIX(GetQueryObjectui64v, EXT);
            #endif
        }

        if (ver >= GR_GL_VER(3,0) || GrGLHasExtensionFromString("GL_ARB_framebuffer_object", extStr)) {
            // ARB extension doesn't use the ARB suffix on the function names
            #if GL_VERSION_3_0 || GL_ARB_framebuffer_object
                interface->fGenFramebuffers = glGenFramebuffers;
                interface->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
                interface->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
                interface->fBindFramebuffer = glBindFramebuffer;
                interface->fFramebufferTexture2D = glFramebufferTexture2D;
                interface->fCheckFramebufferStatus = glCheckFramebufferStatus;
                interface->fDeleteFramebuffers = glDeleteFramebuffers;
                interface->fRenderbufferStorage = glRenderbufferStorage;
                interface->fGenRenderbuffers = glGenRenderbuffers;
                interface->fDeleteRenderbuffers = glDeleteRenderbuffers;
                interface->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
                interface->fBindRenderbuffer = glBindRenderbuffer;
                interface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisample;
                interface->fBlitFramebuffer = glBlitFramebuffer;
            #else
                interface->fGenFramebuffers = GET_PROC(GenFramebuffers);
                interface->fGetFramebufferAttachmentParameteriv = GET_PROC(GetFramebufferAttachmentParameteriv);
                interface->fGetRenderbufferParameteriv = GET_PROC(GetRenderbufferParameteriv);
                interface->fBindFramebuffer = GET_PROC(BindFramebuffer);
                interface->fFramebufferTexture2D = GET_PROC(FramebufferTexture2D);
                interface->fCheckFramebufferStatus = GET_PROC(CheckFramebufferStatus);
                interface->fDeleteFramebuffers = GET_PROC(DeleteFramebuffers);
                interface->fRenderbufferStorage = GET_PROC(RenderbufferStorage);
                interface->fGenRenderbuffers = GET_PROC(GenRenderbuffers);
                interface->fDeleteRenderbuffers = GET_PROC(DeleteRenderbuffers);
                interface->fFramebufferRenderbuffer = GET_PROC(FramebufferRenderbuffer);
                interface->fBindRenderbuffer = GET_PROC(BindRenderbuffer);
                interface->fRenderbufferStorageMultisample = GET_PROC(RenderbufferStorageMultisample);
                interface->fBlitFramebuffer = GET_PROC(BlitFramebuffer);
            #endif
        } else {
            if (GrGLHasExtensionFromString("GL_EXT_framebuffer_object", extStr)) {
                #if GL_EXT_framebuffer_object
                    interface->fGenFramebuffers = glGenFramebuffersEXT;
                    interface->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameterivEXT;
                    interface->fGetRenderbufferParameteriv = glGetRenderbufferParameterivEXT;
                    interface->fBindFramebuffer = glBindFramebufferEXT;
                    interface->fFramebufferTexture2D = glFramebufferTexture2DEXT;
                    interface->fCheckFramebufferStatus = glCheckFramebufferStatusEXT;
                    interface->fDeleteFramebuffers = glDeleteFramebuffersEXT;
                    interface->fRenderbufferStorage = glRenderbufferStorageEXT;
                    interface->fGenRenderbuffers = glGenRenderbuffersEXT;
                    interface->fDeleteRenderbuffers = glDeleteRenderbuffersEXT;
                    interface->fFramebufferRenderbuffer = glFramebufferRenderbufferEXT;
                    interface->fBindRenderbuffer = glBindRenderbufferEXT;
                #else
                    interface->fGenFramebuffers = GET_PROC_SUFFIX(GenFramebuffers, EXT);
                    interface->fGetFramebufferAttachmentParameteriv = GET_PROC_SUFFIX(GetFramebufferAttachmentParameteriv, EXT);
                    interface->fGetRenderbufferParameteriv = GET_PROC_SUFFIX(GetRenderbufferParameteriv, EXT);
                    interface->fBindFramebuffer = GET_PROC_SUFFIX(BindFramebuffer, EXT);
                    interface->fFramebufferTexture2D = GET_PROC_SUFFIX(FramebufferTexture2D, EXT);
                    interface->fCheckFramebufferStatus = GET_PROC_SUFFIX(CheckFramebufferStatus, EXT);
                    interface->fDeleteFramebuffers = GET_PROC_SUFFIX(DeleteFramebuffers, EXT);
                    interface->fRenderbufferStorage = GET_PROC_SUFFIX(RenderbufferStorage, EXT);
                    interface->fGenRenderbuffers = GET_PROC_SUFFIX(GenRenderbuffers, EXT);
                    interface->fDeleteRenderbuffers = GET_PROC_SUFFIX(DeleteRenderbuffers, EXT);
                    interface->fFramebufferRenderbuffer = GET_PROC_SUFFIX(FramebufferRenderbuffer, EXT);
                    interface->fBindRenderbuffer = GET_PROC_SUFFIX(BindRenderbuffer, EXT);
                #endif
            }
            if (GrGLHasExtensionFromString("GL_EXT_framebuffer_multisample", extStr)) {
                #if GL_EXT_framebuffer_multisample
                    interface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;
                #else
                    interface->fRenderbufferStorageMultisample = GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
                #endif
            }
            if (GrGLHasExtensionFromString("", extStr)) {
                #if GL_EXT_framebuffer_blit
                    interface->fBlitFramebuffer = glBlitFramebufferEXT;
                #else
                    interface->fBlitFramebuffer = GET_PROC_SUFFIX(BlitFramebuffer, EXT);
                #endif
            }
        }
        if (ver >= GR_GL_VER(3,3) || GrGLHasExtensionFromString("GL_ARB_blend_func_extended", extStr)) {
            // ARB extension doesn't use the ARB suffix on the function name
            #if GL_VERSION_3_3 || GL_ARB_blend_func_extended
                interface->fBindFragDataLocationIndexed = glBindFragDataLocationIndexed;
            #else
                interface->fBindFragDataLocationIndexed = GET_PROC(BindFragDataLocationIndexed);
            #endif
        }

        interface->fBindingsExported = kDesktop_GrGLBinding;
    }
    glInterface.get()->ref();
    return glInterface.get();
}
