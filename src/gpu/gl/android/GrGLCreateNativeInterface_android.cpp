// Modified from chromium/src/webkit/glue/gl_bindings_skia_cmd_buffer.cc

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl/GrGLExtensions.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLUtil.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

static const GrGLInterface* create_es_interface(GrGLVersion version,
                                                const GrGLExtensions& extensions) {
    if (version < GR_GL_VER(2,0)) {
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface);
    interface->fBindingsExported = kES_GrGLBinding;

    interface->fActiveTexture = glActiveTexture;
    interface->fAttachShader = glAttachShader;
    interface->fBindAttribLocation = glBindAttribLocation;
    interface->fBindBuffer = glBindBuffer;
    interface->fBindTexture = glBindTexture;
    interface->fBindVertexArray = glBindVertexArrayOES;
    interface->fBlendColor = glBlendColor;
    interface->fBlendFunc = glBlendFunc;
    interface->fBufferData = glBufferData;
    interface->fBufferSubData = glBufferSubData;
    interface->fClear = glClear;
    interface->fClearColor = glClearColor;
    interface->fClearStencil = glClearStencil;
    interface->fColorMask = glColorMask;
    interface->fCompileShader = glCompileShader;
    interface->fCompressedTexImage2D = glCompressedTexImage2D;
    interface->fCopyTexSubImage2D = glCopyTexSubImage2D;
    interface->fCreateProgram = glCreateProgram;
    interface->fCreateShader = glCreateShader;
    interface->fCullFace = glCullFace;
    interface->fDeleteBuffers = glDeleteBuffers;
    interface->fDeleteProgram = glDeleteProgram;
    interface->fDeleteShader = glDeleteShader;
    interface->fDeleteTextures = glDeleteTextures;
    interface->fDeleteVertexArrays = glDeleteVertexArraysOES;
    interface->fDepthMask = glDepthMask;
    interface->fDisable = glDisable;
    interface->fDisableVertexAttribArray = glDisableVertexAttribArray;
    interface->fDrawArrays = glDrawArrays;
    interface->fDrawElements = glDrawElements;
    interface->fEnable = glEnable;
    interface->fEnableVertexAttribArray = glEnableVertexAttribArray;
    interface->fFinish = glFinish;
    interface->fFlush = glFlush;
    interface->fFrontFace = glFrontFace;
    interface->fGenBuffers = glGenBuffers;
    interface->fGenerateMipmap = glGenerateMipmap;
    interface->fGenTextures = glGenTextures;
    interface->fGenVertexArrays = glGenVertexArraysOES;
    interface->fGetBufferParameteriv = glGetBufferParameteriv;
    interface->fGetError = glGetError;
    interface->fGetIntegerv = glGetIntegerv;
    interface->fGetProgramInfoLog = glGetProgramInfoLog;
    interface->fGetProgramiv = glGetProgramiv;
    interface->fGetShaderInfoLog = glGetShaderInfoLog;
    interface->fGetShaderiv = glGetShaderiv;
    interface->fGetString = glGetString;
#if GL_ES_VERSION_30
    interface->fGetStringi = glGetStringi;
#else
    interface->fGetStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");
#endif
    interface->fGetUniformLocation = glGetUniformLocation;
    interface->fLineWidth = glLineWidth;
    interface->fLinkProgram = glLinkProgram;
    interface->fPixelStorei = glPixelStorei;
    interface->fReadPixels = glReadPixels;
    interface->fScissor = glScissor;
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
    interface->fTexImage2D = glTexImage2D;
    interface->fTexParameteri = glTexParameteri;
    interface->fTexParameteriv = glTexParameteriv;
    interface->fTexSubImage2D = glTexSubImage2D;

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        interface->fTexStorage2D = glTexStorage2D;
#else
        interface->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2D");
#endif
    } else {
#if GL_EXT_texture_storage
        interface->fTexStorage2D = glTexStorage2DEXT;
#else
        interface->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2DEXT");
#endif
    }

#if GL_EXT_discard_framebuffer
    interface->fDiscardFramebuffer = glDiscardFramebufferEXT;
#endif
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
    interface->fUniformMatrix2fv = glUniformMatrix2fv;
    interface->fUniformMatrix3fv = glUniformMatrix3fv;
    interface->fUniformMatrix4fv = glUniformMatrix4fv;
    interface->fUseProgram = glUseProgram;
    interface->fVertexAttrib4fv = glVertexAttrib4fv;
    interface->fVertexAttribPointer = glVertexAttribPointer;
    interface->fViewport = glViewport;
    interface->fBindFramebuffer = glBindFramebuffer;
    interface->fBindRenderbuffer = glBindRenderbuffer;
    interface->fCheckFramebufferStatus = glCheckFramebufferStatus;
    interface->fDeleteFramebuffers = glDeleteFramebuffers;
    interface->fDeleteRenderbuffers = glDeleteRenderbuffers;
    interface->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
    interface->fFramebufferTexture2D = glFramebufferTexture2D;
#if GR_GL_IGNORE_ES3_MSAA

    if (extensions.has("GL_EXT_multisampled_render_to_texture")) {
#if GL_EXT_multisampled_render_to_texture
        interface->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleEXT;
        interface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;
#else
        interface->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
        interface->fRenderbufferStorageMultisample = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
#endif
    } else if (extensions.has("GL_IMG_multisampled_render_to_texture")) {
#if GL_IMG_multisampled_render_to_texture
        interface->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleIMG;
        interface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisampleIMG;
#else
        interface->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleIMG");
        interface->fRenderbufferStorageMultisample = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleIMG");
#endif
    }

#else // GR_GL_IGNORE_ES3_MSAA

        if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
            interface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisample;
            interface->fBlitFramebuffer = glBlitFramebuffer;
#else
            interface->fRenderbufferStorageMultisample = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisample");
            interface->fBlitFramebuffer = (GrGLBlitFramebufferProc) eglGetProcAddress("glBlitFramebuffer");
#endif
        }
        if (extensions.has("GL_EXT_multisampled_render_to_texture")) {
#if GL_EXT_multisampled_render_to_texture
            interface->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleEXT;
            interface->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleEXT;
#else
            interface->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
            interface->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
#endif
        } else if (extensions.has("GL_IMG_multisampled_render_to_texture")) {
#if GL_IMG_multisampled_render_to_texture
            interface->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleIMG;
            interface->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleIMG;
#else
            interface->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleIMG");
            interface->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleIMG");
#endif
        }

#endif // GR_GL_IGNORE_ES3_MSAA
    interface->fGenFramebuffers = glGenFramebuffers;
    interface->fGenRenderbuffers = glGenRenderbuffers;
    interface->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
    interface->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
    interface->fRenderbufferStorage = glRenderbufferStorage;
#if GL_OES_mapbuffer
    interface->fMapBuffer = glMapBufferOES;
    interface->fUnmapBuffer = glUnmapBufferOES;
#else
    interface->fMapBuffer = (GrGLMapBufferProc) eglGetProcAddress("glMapBufferOES");
    interface->fUnmapBuffer = (GrGLUnmapBufferProc) eglGetProcAddress("glUnmapBufferOES");
#endif

    return interface;
}

static const GrGLInterface* create_desktop_interface(GrGLVersion version,
                                                     const GrGLExtensions& extensions) {
    // Currently this assumes a 4.4 context or later. Supporting lower GL versions would require
    // getting suffixed versions of pointers for supported extensions.
    if (version < GR_GL_VER(4,4)) {
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface);
    interface->fBindingsExported = kDesktop_GrGLBinding;

    interface->fActiveTexture = (GrGLActiveTextureProc) eglGetProcAddress("glActiveTexture");
    interface->fAttachShader = (GrGLAttachShaderProc) eglGetProcAddress("glAttachShader");
    interface->fBeginQuery = (GrGLBeginQueryProc) eglGetProcAddress("glBeginQuery");
    interface->fBindAttribLocation = (GrGLBindAttribLocationProc) eglGetProcAddress("glBindAttribLocation");
    interface->fBindBuffer = (GrGLBindBufferProc) eglGetProcAddress("glBindBuffer");
    interface->fBindFragDataLocation = (GrGLBindFragDataLocationProc) eglGetProcAddress("glBindFragDataLocation");
    interface->fBindFragDataLocationIndexed = (GrGLBindFragDataLocationIndexedProc) eglGetProcAddress("glBindFragDataLocationIndexed");
    interface->fBindFramebuffer = (GrGLBindFramebufferProc) eglGetProcAddress("glBindFramebuffer");
    interface->fBindRenderbuffer = (GrGLBindRenderbufferProc) eglGetProcAddress("glBindRenderbuffer");
    interface->fBindTexture = (GrGLBindTextureProc) eglGetProcAddress("glBindTexture");
    interface->fBindVertexArray = (GrGLBindVertexArrayProc) eglGetProcAddress("glBindVertexArray");
    interface->fBlendColor = (GrGLBlendColorProc) eglGetProcAddress("glBlendColor");
    interface->fBlendFunc = (GrGLBlendFuncProc) eglGetProcAddress("glBlendFunc");
    interface->fBlitFramebuffer = (GrGLBlitFramebufferProc) eglGetProcAddress("glBlitFramebuffer");
    interface->fBufferData = (GrGLBufferDataProc) eglGetProcAddress("glBufferData");
    interface->fBufferSubData = (GrGLBufferSubDataProc) eglGetProcAddress("glBufferSubData");
    interface->fCheckFramebufferStatus = (GrGLCheckFramebufferStatusProc) eglGetProcAddress("glCheckFramebufferStatus");
    interface->fClear = (GrGLClearProc) eglGetProcAddress("glClear");
    interface->fClearColor = (GrGLClearColorProc) eglGetProcAddress("glClearColor");
    interface->fClearStencil = (GrGLClearStencilProc) eglGetProcAddress("glClearStencil");
    interface->fClientActiveTexture = (GrGLClientActiveTextureProc) eglGetProcAddress("glClientActiveTexture");
    interface->fColorMask = (GrGLColorMaskProc) eglGetProcAddress("glColorMask");
    interface->fCompileShader = (GrGLCompileShaderProc) eglGetProcAddress("glCompileShader");
    interface->fCompressedTexImage2D = (GrGLCompressedTexImage2DProc) eglGetProcAddress("glCompressedTexImage2D");
    interface->fCopyTexSubImage2D = (GrGLCopyTexSubImage2DProc) eglGetProcAddress("glCopyTexSubImage2D");
    interface->fCreateProgram = (GrGLCreateProgramProc) eglGetProcAddress("glCreateProgram");
    interface->fCreateShader = (GrGLCreateShaderProc) eglGetProcAddress("glCreateShader");
    interface->fCullFace = (GrGLCullFaceProc) eglGetProcAddress("glCullFace");
    interface->fDeleteBuffers = (GrGLDeleteBuffersProc) eglGetProcAddress("glDeleteBuffers");
    interface->fDeleteFramebuffers = (GrGLDeleteFramebuffersProc) eglGetProcAddress("glDeleteFramebuffers");
    interface->fDeleteProgram = (GrGLDeleteProgramProc) eglGetProcAddress("glDeleteProgram");
    interface->fDeleteQueries = (GrGLDeleteQueriesProc) eglGetProcAddress("glDeleteQueries");
    interface->fDeleteRenderbuffers = (GrGLDeleteRenderbuffersProc) eglGetProcAddress("glDeleteRenderbuffers");
    interface->fDeleteShader = (GrGLDeleteShaderProc) eglGetProcAddress("glDeleteShader");
    interface->fDeleteTextures = (GrGLDeleteTexturesProc) eglGetProcAddress("glDeleteTextures");
    interface->fDeleteVertexArrays = (GrGLDeleteVertexArraysProc) eglGetProcAddress("glDeleteVertexArrays");
    interface->fDepthMask = (GrGLDepthMaskProc) eglGetProcAddress("glDepthMask");
    interface->fDisable = (GrGLDisableProc) eglGetProcAddress("glDisable");
    interface->fDisableClientState = (GrGLDisableClientStateProc) eglGetProcAddress("glDisableClientState");
    interface->fDisableVertexAttribArray = (GrGLDisableVertexAttribArrayProc) eglGetProcAddress("glDisableVertexAttribArray");
    interface->fDrawArrays = (GrGLDrawArraysProc) eglGetProcAddress("glDrawArrays");
    interface->fDrawBuffer = (GrGLDrawBufferProc) eglGetProcAddress("glDrawBuffer");
    interface->fDrawBuffers = (GrGLDrawBuffersProc) eglGetProcAddress("glDrawBuffers");
    interface->fDrawElements = (GrGLDrawElementsProc) eglGetProcAddress("glDrawElements");
    interface->fEnable = (GrGLEnableProc) eglGetProcAddress("glEnable");
    interface->fEnableClientState = (GrGLEnableClientStateProc) eglGetProcAddress("glEnableClientState");
    interface->fEnableVertexAttribArray = (GrGLEnableVertexAttribArrayProc) eglGetProcAddress("glEnableVertexAttribArray");
    interface->fEndQuery = (GrGLEndQueryProc) eglGetProcAddress("glEndQuery");
    interface->fFinish = (GrGLFinishProc) eglGetProcAddress("glFinish");
    interface->fFlush = (GrGLFlushProc) eglGetProcAddress("glFlush");
    interface->fFramebufferRenderbuffer = (GrGLFramebufferRenderbufferProc) eglGetProcAddress("glFramebufferRenderbuffer");
    interface->fFramebufferTexture2D = (GrGLFramebufferTexture2DProc) eglGetProcAddress("glFramebufferTexture2D");
    interface->fFrontFace = (GrGLFrontFaceProc) eglGetProcAddress("glFrontFace");
    interface->fGenBuffers = (GrGLGenBuffersProc) eglGetProcAddress("glGenBuffers");
    interface->fGenFramebuffers = (GrGLGenFramebuffersProc) eglGetProcAddress("glGenFramebuffers");
    interface->fGenerateMipmap = (GrGLGenerateMipmapProc) eglGetProcAddress("glGenerateMipmap");
    interface->fGenQueries = (GrGLGenQueriesProc) eglGetProcAddress("glGenQueries");
    interface->fGenRenderbuffers = (GrGLGenRenderbuffersProc) eglGetProcAddress("glGenRenderbuffers");
    interface->fGenTextures = (GrGLGenTexturesProc) eglGetProcAddress("glGenTextures");
    interface->fGenVertexArrays = (GrGLGenVertexArraysProc) eglGetProcAddress("glGenVertexArrays");
    interface->fGetBufferParameteriv = (GrGLGetBufferParameterivProc) eglGetProcAddress("glGetBufferParameteriv");
    interface->fGetError = (GrGLGetErrorProc) eglGetProcAddress("glGetError");
    interface->fGetFramebufferAttachmentParameteriv = (GrGLGetFramebufferAttachmentParameterivProc) eglGetProcAddress("glGetFramebufferAttachmentParameteriv");
    interface->fGetIntegerv = (GrGLGetIntegervProc) eglGetProcAddress("glGetIntegerv");
    interface->fGetQueryObjecti64v = (GrGLGetQueryObjecti64vProc) eglGetProcAddress("glGetQueryObjecti64v");
    interface->fGetQueryObjectiv = (GrGLGetQueryObjectivProc) eglGetProcAddress("glGetQueryObjectiv");
    interface->fGetQueryObjectui64v = (GrGLGetQueryObjectui64vProc) eglGetProcAddress("glGetQueryObjectui64v");
    interface->fGetQueryObjectuiv = (GrGLGetQueryObjectuivProc) eglGetProcAddress("glGetQueryObjectuiv");
    interface->fGetQueryiv = (GrGLGetQueryivProc) eglGetProcAddress("glGetQueryiv");
    interface->fGetProgramInfoLog = (GrGLGetProgramInfoLogProc) eglGetProcAddress("glGetProgramInfoLog");
    interface->fGetProgramiv = (GrGLGetProgramivProc) eglGetProcAddress("glGetProgramiv");
    interface->fGetRenderbufferParameteriv = (GrGLGetRenderbufferParameterivProc) eglGetProcAddress("glGetRenderbufferParameteriv");
    interface->fGetShaderInfoLog = (GrGLGetShaderInfoLogProc) eglGetProcAddress("glGetShaderInfoLog");
    interface->fGetShaderiv = (GrGLGetShaderivProc) eglGetProcAddress("glGetShaderiv");
    interface->fGetString = (GrGLGetStringProc) eglGetProcAddress("glGetString");
    interface->fGetStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");
    interface->fGetTexLevelParameteriv = (GrGLGetTexLevelParameterivProc) eglGetProcAddress("glGetTexLevelParameteriv");
    interface->fGetUniformLocation = (GrGLGetUniformLocationProc) eglGetProcAddress("glGetUniformLocation");
    interface->fLineWidth = (GrGLLineWidthProc) eglGetProcAddress("glLineWidth");
    interface->fLinkProgram = (GrGLLinkProgramProc) eglGetProcAddress("glLinkProgram");
    interface->fLoadIdentity = (GrGLLoadIdentityProc) eglGetProcAddress("glLoadIdentity");
    interface->fLoadMatrixf = (GrGLLoadMatrixfProc) eglGetProcAddress("glLoadMatrixf");
    interface->fMapBuffer = (GrGLMapBufferProc) eglGetProcAddress("glMapBuffer");
    interface->fMatrixMode = (GrGLMatrixModeProc) eglGetProcAddress("glMatrixMode");
    interface->fPixelStorei = (GrGLPixelStoreiProc) eglGetProcAddress("glPixelStorei");
    interface->fQueryCounter = (GrGLQueryCounterProc) eglGetProcAddress("glQueryCounter");
    interface->fReadBuffer = (GrGLReadBufferProc) eglGetProcAddress("glReadBuffer");
    interface->fReadPixels = (GrGLReadPixelsProc) eglGetProcAddress("glReadPixels");
    interface->fRenderbufferStorage = (GrGLRenderbufferStorageProc) eglGetProcAddress("glRenderbufferStorage");
    interface->fRenderbufferStorageMultisample = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisample");
    interface->fScissor = (GrGLScissorProc) eglGetProcAddress("glScissor");
    interface->fShaderSource = (GrGLShaderSourceProc) eglGetProcAddress("glShaderSource");
    interface->fStencilFunc = (GrGLStencilFuncProc) eglGetProcAddress("glStencilFunc");
    interface->fStencilFuncSeparate = (GrGLStencilFuncSeparateProc) eglGetProcAddress("glStencilFuncSeparate");
    interface->fStencilMask = (GrGLStencilMaskProc) eglGetProcAddress("glStencilMask");
    interface->fStencilMaskSeparate = (GrGLStencilMaskSeparateProc) eglGetProcAddress("glStencilMaskSeparate");
    interface->fStencilOp = (GrGLStencilOpProc) eglGetProcAddress("glStencilOp");
    interface->fStencilOpSeparate = (GrGLStencilOpSeparateProc) eglGetProcAddress("glStencilOpSeparate");
    interface->fTexGenf = (GrGLTexGenfProc) eglGetProcAddress("glTexGenf");
    interface->fTexGenfv = (GrGLTexGenfvProc) eglGetProcAddress("glTexGenfv");
    interface->fTexGeni = (GrGLTexGeniProc) eglGetProcAddress("glTexGeni");
    interface->fTexImage2D = (GrGLTexImage2DProc) eglGetProcAddress("glTexImage2D");
    interface->fTexParameteri = (GrGLTexParameteriProc) eglGetProcAddress("glTexParameteri");
    interface->fTexParameteriv = (GrGLTexParameterivProc) eglGetProcAddress("glTexParameteriv");
    interface->fTexSubImage2D = (GrGLTexSubImage2DProc) eglGetProcAddress("glTexSubImage2D");
    interface->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2D");
    interface->fUniform1f = (GrGLUniform1fProc) eglGetProcAddress("glUniform1f");
    interface->fUniform1i = (GrGLUniform1iProc) eglGetProcAddress("glUniform1i");
    interface->fUniform1fv = (GrGLUniform1fvProc) eglGetProcAddress("glUniform1fv");
    interface->fUniform1iv = (GrGLUniform1ivProc) eglGetProcAddress("glUniform1iv");
    interface->fUniform2f = (GrGLUniform2fProc) eglGetProcAddress("glUniform2f");
    interface->fUniform2i = (GrGLUniform2iProc) eglGetProcAddress("glUniform2i");
    interface->fUniform2fv = (GrGLUniform2fvProc) eglGetProcAddress("glUniform2fv");
    interface->fUniform2iv = (GrGLUniform2ivProc) eglGetProcAddress("glUniform2iv");
    interface->fUniform3f = (GrGLUniform3fProc) eglGetProcAddress("glUniform3f");
    interface->fUniform3i = (GrGLUniform3iProc) eglGetProcAddress("glUniform3i");
    interface->fUniform3fv = (GrGLUniform3fvProc) eglGetProcAddress("glUniform3fv");
    interface->fUniform3iv = (GrGLUniform3ivProc) eglGetProcAddress("glUniform3iv");
    interface->fUniform4f = (GrGLUniform4fProc) eglGetProcAddress("glUniform4f");
    interface->fUniform4i = (GrGLUniform4iProc) eglGetProcAddress("glUniform4i");
    interface->fUniform4fv = (GrGLUniform4fvProc) eglGetProcAddress("glUniform4fv");
    interface->fUniform4iv = (GrGLUniform4ivProc) eglGetProcAddress("glUniform4iv");
    interface->fUniformMatrix2fv = (GrGLUniformMatrix2fvProc) eglGetProcAddress("glUniformMatrix2fv");
    interface->fUniformMatrix3fv = (GrGLUniformMatrix3fvProc) eglGetProcAddress("glUniformMatrix3fv");
    interface->fUniformMatrix4fv = (GrGLUniformMatrix4fvProc) eglGetProcAddress("glUniformMatrix4fv");
    interface->fUnmapBuffer = (GrGLUnmapBufferProc) eglGetProcAddress("glUnmapBuffer");
    interface->fUseProgram = (GrGLUseProgramProc) eglGetProcAddress("glUseProgram");
    interface->fVertexAttrib4fv = (GrGLVertexAttrib4fvProc) eglGetProcAddress("glVertexAttrib4fv");
    interface->fVertexAttribPointer = (GrGLVertexAttribPointerProc) eglGetProcAddress("glVertexAttribPointer");
    interface->fVertexPointer = (GrGLVertexPointerProc) eglGetProcAddress("glVertexPointer");
    interface->fViewport = (GrGLViewportProc) eglGetProcAddress("glViewport");

    if (extensions.has("GL_NV_path_rendering")) {
        interface->fPathCommands = (GrGLPathCommandsProc) eglGetProcAddress("glPathCommandsNV");
        interface->fPathCoords = (GrGLPathCoordsProc) eglGetProcAddress("glPathCoordsNV");
        interface->fPathSubCommands = (GrGLPathSubCommandsProc) eglGetProcAddress("glPathSubCommandsNV");
        interface->fPathSubCoords = (GrGLPathSubCoordsProc) eglGetProcAddress("glPathSubCoordsNV");
        interface->fPathString = (GrGLPathStringProc) eglGetProcAddress("glPathStringNV");
        interface->fPathGlyphs = (GrGLPathGlyphsProc) eglGetProcAddress("glPathGlyphsNV");
        interface->fPathGlyphRange = (GrGLPathGlyphRangeProc) eglGetProcAddress("glPathGlyphRangeNV");
        interface->fWeightPaths = (GrGLWeightPathsProc) eglGetProcAddress("glWeightPathsNV");
        interface->fCopyPath = (GrGLCopyPathProc) eglGetProcAddress("glCopyPathNV");
        interface->fInterpolatePaths = (GrGLInterpolatePathsProc) eglGetProcAddress("glInterpolatePathsNV");
        interface->fTransformPath = (GrGLTransformPathProc) eglGetProcAddress("glTransformPathNV");
        interface->fPathParameteriv = (GrGLPathParameterivProc) eglGetProcAddress("glPathParameterivNV");
        interface->fPathParameteri = (GrGLPathParameteriProc) eglGetProcAddress("glPathParameteriNV");
        interface->fPathParameterfv = (GrGLPathParameterfvProc) eglGetProcAddress("glPathParameterfvNV");
        interface->fPathParameterf = (GrGLPathParameterfProc) eglGetProcAddress("glPathParameterfNV");
        interface->fPathDashArray = (GrGLPathDashArrayProc) eglGetProcAddress("glPathDashArrayNV");
        interface->fGenPaths = (GrGLGenPathsProc) eglGetProcAddress("glGenPathsNV");
        interface->fDeletePaths = (GrGLDeletePathsProc) eglGetProcAddress("glDeletePathsNV");
        interface->fIsPath = (GrGLIsPathProc) eglGetProcAddress("glIsPathNV");
        interface->fPathStencilFunc = (GrGLPathStencilFuncProc) eglGetProcAddress("glPathStencilFuncNV");
        interface->fPathStencilDepthOffset = (GrGLPathStencilDepthOffsetProc) eglGetProcAddress("glPathStencilDepthOffsetNV");
        interface->fStencilFillPath = (GrGLStencilFillPathProc) eglGetProcAddress("glStencilFillPathNV");
        interface->fStencilStrokePath = (GrGLStencilStrokePathProc) eglGetProcAddress("glStencilStrokePathNV");
        interface->fStencilFillPathInstanced = (GrGLStencilFillPathInstancedProc) eglGetProcAddress("glStencilFillPathInstancedNV");
        interface->fStencilStrokePathInstanced = (GrGLStencilStrokePathInstancedProc) eglGetProcAddress("glStencilStrokePathInstancedNV");
        interface->fPathCoverDepthFunc = (GrGLPathCoverDepthFuncProc) eglGetProcAddress("glPathCoverDepthFuncNV");
        interface->fPathColorGen = (GrGLPathColorGenProc) eglGetProcAddress("glPathColorGenNV");
        interface->fPathTexGen = (GrGLPathTexGenProc) eglGetProcAddress("glPathTexGenNV");
        interface->fPathFogGen = (GrGLPathFogGenProc) eglGetProcAddress("glPathFogGenNV");
        interface->fCoverFillPath = (GrGLCoverFillPathProc) eglGetProcAddress("glCoverFillPathNV");
        interface->fCoverStrokePath = (GrGLCoverStrokePathProc) eglGetProcAddress("glCoverStrokePathNV");
        interface->fCoverFillPathInstanced = (GrGLCoverFillPathInstancedProc) eglGetProcAddress("glCoverFillPathInstancedNV");
        interface->fCoverStrokePathInstanced = (GrGLCoverStrokePathInstancedProc) eglGetProcAddress("glCoverStrokePathInstancedNV");
        interface->fGetPathParameteriv = (GrGLGetPathParameterivProc) eglGetProcAddress("glGetPathParameterivNV");
        interface->fGetPathParameterfv = (GrGLGetPathParameterfvProc) eglGetProcAddress("glGetPathParameterfvNV");
        interface->fGetPathCommands = (GrGLGetPathCommandsProc) eglGetProcAddress("glGetPathCommandsNV");
        interface->fGetPathCoords = (GrGLGetPathCoordsProc) eglGetProcAddress("glGetPathCoordsNV");
        interface->fGetPathDashArray = (GrGLGetPathDashArrayProc) eglGetProcAddress("glGetPathDashArrayNV");
        interface->fGetPathMetrics = (GrGLGetPathMetricsProc) eglGetProcAddress("glGetPathMetricsNV");
        interface->fGetPathMetricRange = (GrGLGetPathMetricRangeProc) eglGetProcAddress("glGetPathMetricRangeNV");
        interface->fGetPathSpacing = (GrGLGetPathSpacingProc) eglGetProcAddress("glGetPathSpacingNV");
        interface->fGetPathColorGeniv = (GrGLGetPathColorGenivProc) eglGetProcAddress("glGetPathColorGenivNV");
        interface->fGetPathColorGenfv = (GrGLGetPathColorGenfvProc) eglGetProcAddress("glGetPathColorGenfvNV");
        interface->fGetPathTexGeniv = (GrGLGetPathTexGenivProc) eglGetProcAddress("glGetPathTexGenivNV");
        interface->fGetPathTexGenfv = (GrGLGetPathTexGenfvProc) eglGetProcAddress("glGetPathTexGenfvNV");
        interface->fIsPointInFillPath = (GrGLIsPointInFillPathProc) eglGetProcAddress("glIsPointInFillPathNV");
        interface->fIsPointInStrokePath = (GrGLIsPointInStrokePathProc) eglGetProcAddress("glIsPointInStrokePathNV");
        interface->fGetPathLength = (GrGLGetPathLengthProc) eglGetProcAddress("glGetPathLengthNV");
        interface->fPointAlongPath = (GrGLPointAlongPathProc) eglGetProcAddress("glPointAlongPathNV");
    }

    return interface;
}

const GrGLInterface* GrGLCreateNativeInterface() {

    GrGLGetStringiProc getStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");

    const char* verStr = reinterpret_cast<const char*>(glGetString(GR_GL_VERSION));
    GrGLVersion version = GrGLGetVersionFromString(verStr);
    GrGLBinding binding = GrGLGetBindingInUseFromString(verStr);

    GrGLExtensions extensions;
    if (!extensions.init(binding, glGetString, getStringi, glGetIntegerv)) {
        return NULL;
    }

    if (kES_GrGLBinding == binding) {
        return create_es_interface(version, extensions);
    } else if (kDesktop_GrGLBinding == binding) {
        return create_desktop_interface(version, extensions);
    } else {
        return NULL;
    }
}
