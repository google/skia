
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLExtensions.h"
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
        GrGLExtensions extensions;
        GrGLGetStringiProc glGetStringi = (GrGLGetStringiProc) GetProcAddress("glGetStringi");
        if (!extensions.init(kDesktop_GrGLBinding, glGetString, glGetStringi, glGetIntegerv)) {
            glInterface.reset(NULL);
            return NULL;
        }
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
            GET_PROC(BindFragDataLocation);
#endif
        }
        interface->fBindTexture = glBindTexture;
        interface->fBlendFunc = glBlendFunc;

        if (ver >= GR_GL_VER(1,4)) {
            interface->fBlendColor = glBlendColor;
        } else if (extensions.has("GL_ARB_imaging") ||
                   extensions.has("GL_EXT_blend_color")) {
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
        interface->fDisableVertexAttribArray = glDisableVertexAttribArray;
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
        interface->fGetStringi = glGetStringi;
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
    // The new OpenGLES2 header has an extra "const" in it.  :(
#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
        interface->fShaderSource = (GrGLShaderSourceProc) glShaderSource;
#else
        interface->fShaderSource = glShaderSource;
#endif
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
        if (ver >= GR_GL_VER(4,2) || extensions.has("GL_ARB_texture_storage")) {
            GET_PROC(TexStorage2D);
        } else if (extensions.has("GL_EXT_texture_storage")) {
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

        if (ver >= GR_GL_VER(3,0) || extensions.has("GL_ARB_vertex_array_object")) {
            // no ARB suffix for GL_ARB_vertex_array_object
#if GL_ARB_vertex_array_object || GL_VERSION_3_0
            interface->fBindVertexArray = glBindVertexArray;
            interface->fDeleteVertexArrays = glDeleteVertexArrays;
            interface->fGenVertexArrays = glGenVertexArrays;
#else
            GET_PROC(BindVertexArray);
            GET_PROC(DeleteVertexArrays);
            GET_PROC(GenVertexArrays);
#endif
        }

        if (ver >= GR_GL_VER(3,3) || extensions.has("GL_ARB_timer_query")) {
            // ARB extension doesn't use the ARB suffix on the function name
#if GL_ARB_timer_query || GL_VERSION_3_3
            interface->fQueryCounter = glQueryCounter;
            interface->fGetQueryObjecti64v = glGetQueryObjecti64v;
            interface->fGetQueryObjectui64v = glGetQueryObjectui64v;
#else
            GET_PROC(QueryCounter);
            GET_PROC(GetQueryObjecti64v);
            GET_PROC(GetQueryObjectui64v);
#endif
        } else if (extensions.has("GL_EXT_timer_query")) {
#if GL_EXT_timer_query
            interface->fGetQueryObjecti64v = glGetQueryObjecti64vEXT;
            interface->fGetQueryObjectui64v = glGetQueryObjectui64vEXT;
#else
            GET_PROC_SUFFIX(GetQueryObjecti64v, EXT);
            GET_PROC_SUFFIX(GetQueryObjectui64v, EXT);
#endif
        }

        if (ver >= GR_GL_VER(3,0) || extensions.has("GL_ARB_framebuffer_object")) {
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
                GET_PROC(GenFramebuffers);
                GET_PROC(GetFramebufferAttachmentParameteriv);
                GET_PROC(GetRenderbufferParameteriv);
                GET_PROC(BindFramebuffer);
                GET_PROC(FramebufferTexture2D);
                GET_PROC(CheckFramebufferStatus);
                GET_PROC(DeleteFramebuffers);
                GET_PROC(RenderbufferStorage);
                GET_PROC(GenRenderbuffers);
                GET_PROC(DeleteRenderbuffers);
                GET_PROC(FramebufferRenderbuffer);
                GET_PROC(BindRenderbuffer);
                GET_PROC(RenderbufferStorageMultisample);
                GET_PROC(BlitFramebuffer);
#endif
        } else {
            if (extensions.has("GL_EXT_framebuffer_object")) {
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
                GET_PROC_SUFFIX(GenFramebuffers, EXT);
                GET_PROC_SUFFIX(GetFramebufferAttachmentParameteriv, EXT);
                GET_PROC_SUFFIX(GetRenderbufferParameteriv, EXT);
                GET_PROC_SUFFIX(BindFramebuffer, EXT);
                GET_PROC_SUFFIX(FramebufferTexture2D, EXT);
                GET_PROC_SUFFIX(CheckFramebufferStatus, EXT);
                GET_PROC_SUFFIX(DeleteFramebuffers, EXT);
                GET_PROC_SUFFIX(RenderbufferStorage, EXT);
                GET_PROC_SUFFIX(GenRenderbuffers, EXT);
                GET_PROC_SUFFIX(DeleteRenderbuffers, EXT);
                GET_PROC_SUFFIX(FramebufferRenderbuffer, EXT);
                GET_PROC_SUFFIX(BindRenderbuffer, EXT);
#endif
            }
            if (extensions.has("GL_EXT_framebuffer_multisample")) {
#if GL_EXT_framebuffer_multisample
                interface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;
#else
                GET_PROC_SUFFIX(RenderbufferStorageMultisample, EXT);
#endif
            }
            if (extensions.has("GL_EXT_framebuffer_blit")) {
#if GL_EXT_framebuffer_blit
                interface->fBlitFramebuffer = glBlitFramebufferEXT;
#else
                GET_PROC_SUFFIX(BlitFramebuffer, EXT);
#endif
            }
        }
        if (ver >= GR_GL_VER(3,3) || extensions.has("GL_ARB_blend_func_extended")) {
            // ARB extension doesn't use the ARB suffix on the function name
#if GL_VERSION_3_3 || GL_ARB_blend_func_extended
            interface->fBindFragDataLocationIndexed = glBindFragDataLocationIndexed;
#else
            GET_PROC(BindFragDataLocationIndexed);
#endif
        }
    }
    glInterface.get()->ref();
    return glInterface.get();
}
