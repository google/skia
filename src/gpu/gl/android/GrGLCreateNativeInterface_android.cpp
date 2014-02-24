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

static GrGLInterface* create_es_interface(GrGLVersion version,
                                          const GrGLExtensions& extensions) {
    if (version < GR_GL_VER(2,0)) {
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface);
    interface->fStandard = kGLES_GrGLStandard;
    GrGLInterface::Functions* functions = &interface->fFunctions;

    functions->fActiveTexture = glActiveTexture;
    functions->fAttachShader = glAttachShader;
    functions->fBindAttribLocation = glBindAttribLocation;
    functions->fBindBuffer = glBindBuffer;
    functions->fBindTexture = glBindTexture;
    functions->fBindVertexArray = glBindVertexArrayOES;
    functions->fBlendColor = glBlendColor;
    functions->fBlendFunc = glBlendFunc;
    functions->fBufferData = glBufferData;
    functions->fBufferSubData = glBufferSubData;
    functions->fClear = glClear;
    functions->fClearColor = glClearColor;
    functions->fClearStencil = glClearStencil;
    functions->fColorMask = glColorMask;
    functions->fCompileShader = glCompileShader;
    functions->fCompressedTexImage2D = glCompressedTexImage2D;
    functions->fCopyTexSubImage2D = glCopyTexSubImage2D;
    functions->fCreateProgram = glCreateProgram;
    functions->fCreateShader = glCreateShader;
    functions->fCullFace = glCullFace;
    functions->fDeleteBuffers = glDeleteBuffers;
    functions->fDeleteProgram = glDeleteProgram;
    functions->fDeleteShader = glDeleteShader;
    functions->fDeleteTextures = glDeleteTextures;
    functions->fDeleteVertexArrays = glDeleteVertexArraysOES;
    functions->fDepthMask = glDepthMask;
    functions->fDisable = glDisable;
    functions->fDisableVertexAttribArray = glDisableVertexAttribArray;
    functions->fDrawArrays = glDrawArrays;
    functions->fDrawElements = glDrawElements;
    functions->fEnable = glEnable;
    functions->fEnableVertexAttribArray = glEnableVertexAttribArray;
    functions->fFinish = glFinish;
    functions->fFlush = glFlush;
    functions->fFrontFace = glFrontFace;
    functions->fGenBuffers = glGenBuffers;
    functions->fGenerateMipmap = glGenerateMipmap;
    functions->fGenTextures = glGenTextures;
    functions->fGenVertexArrays = glGenVertexArraysOES;
    functions->fGetBufferParameteriv = glGetBufferParameteriv;
    functions->fGetError = glGetError;
    functions->fGetIntegerv = glGetIntegerv;
    functions->fGetProgramInfoLog = glGetProgramInfoLog;
    functions->fGetProgramiv = glGetProgramiv;
    functions->fGetShaderInfoLog = glGetShaderInfoLog;
    functions->fGetShaderiv = glGetShaderiv;
    functions->fGetString = glGetString;
#if GL_ES_VERSION_30
    functions->fGetStringi = glGetStringi;
#else
    functions->fGetStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");
#endif
    functions->fGetUniformLocation = glGetUniformLocation;
    functions->fLineWidth = glLineWidth;
    functions->fLinkProgram = glLinkProgram;
    functions->fPixelStorei = glPixelStorei;
    functions->fReadPixels = glReadPixels;
    functions->fScissor = glScissor;
#if GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE
    functions->fShaderSource = (GrGLShaderSourceProc) glShaderSource;
#else
    functions->fShaderSource = glShaderSource;
#endif
    functions->fStencilFunc = glStencilFunc;
    functions->fStencilFuncSeparate = glStencilFuncSeparate;
    functions->fStencilMask = glStencilMask;
    functions->fStencilMaskSeparate = glStencilMaskSeparate;
    functions->fStencilOp = glStencilOp;
    functions->fStencilOpSeparate = glStencilOpSeparate;
    functions->fTexImage2D = glTexImage2D;
    functions->fTexParameteri = glTexParameteri;
    functions->fTexParameteriv = glTexParameteriv;
    functions->fTexSubImage2D = glTexSubImage2D;

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        functions->fTexStorage2D = glTexStorage2D;
#else
        functions->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2D");
#endif
    } else {
#if GL_EXT_texture_storage
        functions->fTexStorage2D = glTexStorage2DEXT;
#else
        functions->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2DEXT");
#endif
    }

#if GL_EXT_discard_framebuffer
    functions->fDiscardFramebuffer = glDiscardFramebufferEXT;
#endif
    functions->fUniform1f = glUniform1f;
    functions->fUniform1i = glUniform1i;
    functions->fUniform1fv = glUniform1fv;
    functions->fUniform1iv = glUniform1iv;
    functions->fUniform2f = glUniform2f;
    functions->fUniform2i = glUniform2i;
    functions->fUniform2fv = glUniform2fv;
    functions->fUniform2iv = glUniform2iv;
    functions->fUniform3f = glUniform3f;
    functions->fUniform3i = glUniform3i;
    functions->fUniform3fv = glUniform3fv;
    functions->fUniform3iv = glUniform3iv;
    functions->fUniform4f = glUniform4f;
    functions->fUniform4i = glUniform4i;
    functions->fUniform4fv = glUniform4fv;
    functions->fUniform4iv = glUniform4iv;
    functions->fUniformMatrix2fv = glUniformMatrix2fv;
    functions->fUniformMatrix3fv = glUniformMatrix3fv;
    functions->fUniformMatrix4fv = glUniformMatrix4fv;
    functions->fUseProgram = glUseProgram;
    functions->fVertexAttrib4fv = glVertexAttrib4fv;
    functions->fVertexAttribPointer = glVertexAttribPointer;
    functions->fViewport = glViewport;
    functions->fBindFramebuffer = glBindFramebuffer;
    functions->fBindRenderbuffer = glBindRenderbuffer;
    functions->fCheckFramebufferStatus = glCheckFramebufferStatus;
    functions->fDeleteFramebuffers = glDeleteFramebuffers;
    functions->fDeleteRenderbuffers = glDeleteRenderbuffers;
    functions->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
    functions->fFramebufferTexture2D = glFramebufferTexture2D;

    if (version >= GR_GL_VER(3,0)) {
#if GL_ES_VERSION_3_0
        functions->fRenderbufferStorageMultisample = glRenderbufferStorageMultisample;
        functions->fBlitFramebuffer = glBlitFramebuffer;
#else
        functions->fRenderbufferStorageMultisample = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisample");
        functions->fBlitFramebuffer = (GrGLBlitFramebufferProc) eglGetProcAddress("glBlitFramebuffer");
#endif
    }

    if (extensions.has("GL_EXT_multisampled_render_to_texture")) {
#if GL_EXT_multisampled_render_to_texture
        functions->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleEXT;
        functions->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleEXT;
#else
        functions->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
#endif
    } else if (extensions.has("GL_IMG_multisampled_render_to_texture")) {
#if GL_IMG_multisampled_render_to_texture
        functions->fFramebufferTexture2DMultisample = glFramebufferTexture2DMultisampleIMG;
        functions->fRenderbufferStorageMultisampleES2EXT = glRenderbufferStorageMultisampleIMG;
#else
        functions->fFramebufferTexture2DMultisample = (GrGLFramebufferTexture2DMultisampleProc) eglGetProcAddress("glFramebufferTexture2DMultisampleIMG");
        functions->fRenderbufferStorageMultisampleES2EXT = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisampleIMG");
#endif
    }

    functions->fGenFramebuffers = glGenFramebuffers;
    functions->fGenRenderbuffers = glGenRenderbuffers;
    functions->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
    functions->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
    functions->fRenderbufferStorage = glRenderbufferStorage;
#if GL_OES_mapbuffer
    functions->fMapBuffer = glMapBufferOES;
    functions->fUnmapBuffer = glUnmapBufferOES;
#else
    functions->fMapBuffer = (GrGLMapBufferProc) eglGetProcAddress("glMapBufferOES");
    functions->fUnmapBuffer = (GrGLUnmapBufferProc) eglGetProcAddress("glUnmapBufferOES");
#endif

    if (extensions.has("GL_EXT_debug_marker")) {
        functions->fInsertEventMarker = (GrGLInsertEventMarkerProc) eglGetProcAddress("glInsertEventMarkerEXT");
        functions->fPushGroupMarker = (GrGLInsertEventMarkerProc) eglGetProcAddress("glPushGroupMarkerEXT");
        functions->fPopGroupMarker = (GrGLPopGroupMarkerProc) eglGetProcAddress("glPopGropuMarkerEXT");
    }

    return interface;
}

static GrGLInterface* create_desktop_interface(GrGLVersion version,
                                               const GrGLExtensions& extensions) {
    // Currently this assumes a 4.4 context or later. Supporting lower GL versions would require
    // getting suffixed versions of pointers for supported extensions.
    if (version < GR_GL_VER(4,4)) {
        return NULL;
    }

    GrGLInterface* interface = SkNEW(GrGLInterface);
    interface->fStandard = kGL_GrGLStandard;
    GrGLInterface::Functions* functions = &interface->fFunctions;

    functions->fActiveTexture = (GrGLActiveTextureProc) eglGetProcAddress("glActiveTexture");
    functions->fAttachShader = (GrGLAttachShaderProc) eglGetProcAddress("glAttachShader");
    functions->fBeginQuery = (GrGLBeginQueryProc) eglGetProcAddress("glBeginQuery");
    functions->fBindAttribLocation = (GrGLBindAttribLocationProc) eglGetProcAddress("glBindAttribLocation");
    functions->fBindBuffer = (GrGLBindBufferProc) eglGetProcAddress("glBindBuffer");
    functions->fBindFragDataLocation = (GrGLBindFragDataLocationProc) eglGetProcAddress("glBindFragDataLocation");
    functions->fBindFragDataLocationIndexed = (GrGLBindFragDataLocationIndexedProc) eglGetProcAddress("glBindFragDataLocationIndexed");
    functions->fBindFramebuffer = (GrGLBindFramebufferProc) eglGetProcAddress("glBindFramebuffer");
    functions->fBindRenderbuffer = (GrGLBindRenderbufferProc) eglGetProcAddress("glBindRenderbuffer");
    functions->fBindTexture = (GrGLBindTextureProc) eglGetProcAddress("glBindTexture");
    functions->fBindVertexArray = (GrGLBindVertexArrayProc) eglGetProcAddress("glBindVertexArray");
    functions->fBlendColor = (GrGLBlendColorProc) eglGetProcAddress("glBlendColor");
    functions->fBlendFunc = (GrGLBlendFuncProc) eglGetProcAddress("glBlendFunc");
    functions->fBlitFramebuffer = (GrGLBlitFramebufferProc) eglGetProcAddress("glBlitFramebuffer");
    functions->fBufferData = (GrGLBufferDataProc) eglGetProcAddress("glBufferData");
    functions->fBufferSubData = (GrGLBufferSubDataProc) eglGetProcAddress("glBufferSubData");
    functions->fCheckFramebufferStatus = (GrGLCheckFramebufferStatusProc) eglGetProcAddress("glCheckFramebufferStatus");
    functions->fClear = (GrGLClearProc) eglGetProcAddress("glClear");
    functions->fClearColor = (GrGLClearColorProc) eglGetProcAddress("glClearColor");
    functions->fClearStencil = (GrGLClearStencilProc) eglGetProcAddress("glClearStencil");
    functions->fColorMask = (GrGLColorMaskProc) eglGetProcAddress("glColorMask");
    functions->fCompileShader = (GrGLCompileShaderProc) eglGetProcAddress("glCompileShader");
    functions->fCompressedTexImage2D = (GrGLCompressedTexImage2DProc) eglGetProcAddress("glCompressedTexImage2D");
    functions->fCopyTexSubImage2D = (GrGLCopyTexSubImage2DProc) eglGetProcAddress("glCopyTexSubImage2D");
    functions->fCreateProgram = (GrGLCreateProgramProc) eglGetProcAddress("glCreateProgram");
    functions->fCreateShader = (GrGLCreateShaderProc) eglGetProcAddress("glCreateShader");
    functions->fCullFace = (GrGLCullFaceProc) eglGetProcAddress("glCullFace");
    functions->fDeleteBuffers = (GrGLDeleteBuffersProc) eglGetProcAddress("glDeleteBuffers");
    functions->fDeleteFramebuffers = (GrGLDeleteFramebuffersProc) eglGetProcAddress("glDeleteFramebuffers");
    functions->fDeleteProgram = (GrGLDeleteProgramProc) eglGetProcAddress("glDeleteProgram");
    functions->fDeleteQueries = (GrGLDeleteQueriesProc) eglGetProcAddress("glDeleteQueries");
    functions->fDeleteRenderbuffers = (GrGLDeleteRenderbuffersProc) eglGetProcAddress("glDeleteRenderbuffers");
    functions->fDeleteShader = (GrGLDeleteShaderProc) eglGetProcAddress("glDeleteShader");
    functions->fDeleteTextures = (GrGLDeleteTexturesProc) eglGetProcAddress("glDeleteTextures");
    functions->fDeleteVertexArrays = (GrGLDeleteVertexArraysProc) eglGetProcAddress("glDeleteVertexArrays");
    functions->fDepthMask = (GrGLDepthMaskProc) eglGetProcAddress("glDepthMask");
    functions->fDisable = (GrGLDisableProc) eglGetProcAddress("glDisable");
    functions->fDisableVertexAttribArray = (GrGLDisableVertexAttribArrayProc) eglGetProcAddress("glDisableVertexAttribArray");
    functions->fDrawArrays = (GrGLDrawArraysProc) eglGetProcAddress("glDrawArrays");
    functions->fDrawBuffer = (GrGLDrawBufferProc) eglGetProcAddress("glDrawBuffer");
    functions->fDrawBuffers = (GrGLDrawBuffersProc) eglGetProcAddress("glDrawBuffers");
    functions->fDrawElements = (GrGLDrawElementsProc) eglGetProcAddress("glDrawElements");
    functions->fEnable = (GrGLEnableProc) eglGetProcAddress("glEnable");
    functions->fEnableVertexAttribArray = (GrGLEnableVertexAttribArrayProc) eglGetProcAddress("glEnableVertexAttribArray");
    functions->fEndQuery = (GrGLEndQueryProc) eglGetProcAddress("glEndQuery");
    functions->fFinish = (GrGLFinishProc) eglGetProcAddress("glFinish");
    functions->fFlush = (GrGLFlushProc) eglGetProcAddress("glFlush");
    functions->fFramebufferRenderbuffer = (GrGLFramebufferRenderbufferProc) eglGetProcAddress("glFramebufferRenderbuffer");
    functions->fFramebufferTexture2D = (GrGLFramebufferTexture2DProc) eglGetProcAddress("glFramebufferTexture2D");
    functions->fFrontFace = (GrGLFrontFaceProc) eglGetProcAddress("glFrontFace");
    functions->fGenBuffers = (GrGLGenBuffersProc) eglGetProcAddress("glGenBuffers");
    functions->fGenFramebuffers = (GrGLGenFramebuffersProc) eglGetProcAddress("glGenFramebuffers");
    functions->fGenerateMipmap = (GrGLGenerateMipmapProc) eglGetProcAddress("glGenerateMipmap");
    functions->fGenQueries = (GrGLGenQueriesProc) eglGetProcAddress("glGenQueries");
    functions->fGenRenderbuffers = (GrGLGenRenderbuffersProc) eglGetProcAddress("glGenRenderbuffers");
    functions->fGenTextures = (GrGLGenTexturesProc) eglGetProcAddress("glGenTextures");
    functions->fGenVertexArrays = (GrGLGenVertexArraysProc) eglGetProcAddress("glGenVertexArrays");
    functions->fGetBufferParameteriv = (GrGLGetBufferParameterivProc) eglGetProcAddress("glGetBufferParameteriv");
    functions->fGetError = (GrGLGetErrorProc) eglGetProcAddress("glGetError");
    functions->fGetFramebufferAttachmentParameteriv = (GrGLGetFramebufferAttachmentParameterivProc) eglGetProcAddress("glGetFramebufferAttachmentParameteriv");
    functions->fGetIntegerv = (GrGLGetIntegervProc) eglGetProcAddress("glGetIntegerv");
    functions->fGetQueryObjecti64v = (GrGLGetQueryObjecti64vProc) eglGetProcAddress("glGetQueryObjecti64v");
    functions->fGetQueryObjectiv = (GrGLGetQueryObjectivProc) eglGetProcAddress("glGetQueryObjectiv");
    functions->fGetQueryObjectui64v = (GrGLGetQueryObjectui64vProc) eglGetProcAddress("glGetQueryObjectui64v");
    functions->fGetQueryObjectuiv = (GrGLGetQueryObjectuivProc) eglGetProcAddress("glGetQueryObjectuiv");
    functions->fGetQueryiv = (GrGLGetQueryivProc) eglGetProcAddress("glGetQueryiv");
    functions->fGetProgramInfoLog = (GrGLGetProgramInfoLogProc) eglGetProcAddress("glGetProgramInfoLog");
    functions->fGetProgramiv = (GrGLGetProgramivProc) eglGetProcAddress("glGetProgramiv");
    functions->fGetRenderbufferParameteriv = (GrGLGetRenderbufferParameterivProc) eglGetProcAddress("glGetRenderbufferParameteriv");
    functions->fGetShaderInfoLog = (GrGLGetShaderInfoLogProc) eglGetProcAddress("glGetShaderInfoLog");
    functions->fGetShaderiv = (GrGLGetShaderivProc) eglGetProcAddress("glGetShaderiv");
    functions->fGetString = (GrGLGetStringProc) eglGetProcAddress("glGetString");
    functions->fGetStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");
    functions->fGetTexLevelParameteriv = (GrGLGetTexLevelParameterivProc) eglGetProcAddress("glGetTexLevelParameteriv");
    functions->fGetUniformLocation = (GrGLGetUniformLocationProc) eglGetProcAddress("glGetUniformLocation");
    functions->fLineWidth = (GrGLLineWidthProc) eglGetProcAddress("glLineWidth");
    functions->fLinkProgram = (GrGLLinkProgramProc) eglGetProcAddress("glLinkProgram");
    functions->fLoadIdentity = (GrGLLoadIdentityProc) eglGetProcAddress("glLoadIdentity");
    functions->fLoadMatrixf = (GrGLLoadMatrixfProc) eglGetProcAddress("glLoadMatrixf");
    functions->fMapBuffer = (GrGLMapBufferProc) eglGetProcAddress("glMapBuffer");
    functions->fMatrixMode = (GrGLMatrixModeProc) eglGetProcAddress("glMatrixMode");
    functions->fPixelStorei = (GrGLPixelStoreiProc) eglGetProcAddress("glPixelStorei");
    functions->fQueryCounter = (GrGLQueryCounterProc) eglGetProcAddress("glQueryCounter");
    functions->fReadBuffer = (GrGLReadBufferProc) eglGetProcAddress("glReadBuffer");
    functions->fReadPixels = (GrGLReadPixelsProc) eglGetProcAddress("glReadPixels");
    functions->fRenderbufferStorage = (GrGLRenderbufferStorageProc) eglGetProcAddress("glRenderbufferStorage");
    functions->fRenderbufferStorageMultisample = (GrGLRenderbufferStorageMultisampleProc) eglGetProcAddress("glRenderbufferStorageMultisample");
    functions->fScissor = (GrGLScissorProc) eglGetProcAddress("glScissor");
    functions->fShaderSource = (GrGLShaderSourceProc) eglGetProcAddress("glShaderSource");
    functions->fStencilFunc = (GrGLStencilFuncProc) eglGetProcAddress("glStencilFunc");
    functions->fStencilFuncSeparate = (GrGLStencilFuncSeparateProc) eglGetProcAddress("glStencilFuncSeparate");
    functions->fStencilMask = (GrGLStencilMaskProc) eglGetProcAddress("glStencilMask");
    functions->fStencilMaskSeparate = (GrGLStencilMaskSeparateProc) eglGetProcAddress("glStencilMaskSeparate");
    functions->fStencilOp = (GrGLStencilOpProc) eglGetProcAddress("glStencilOp");
    functions->fStencilOpSeparate = (GrGLStencilOpSeparateProc) eglGetProcAddress("glStencilOpSeparate");
    functions->fTexGenfv = (GrGLTexGenfvProc) eglGetProcAddress("glTexGenfv");
    functions->fTexGeni = (GrGLTexGeniProc) eglGetProcAddress("glTexGeni");
    functions->fTexImage2D = (GrGLTexImage2DProc) eglGetProcAddress("glTexImage2D");
    functions->fTexParameteri = (GrGLTexParameteriProc) eglGetProcAddress("glTexParameteri");
    functions->fTexParameteriv = (GrGLTexParameterivProc) eglGetProcAddress("glTexParameteriv");
    functions->fTexSubImage2D = (GrGLTexSubImage2DProc) eglGetProcAddress("glTexSubImage2D");
    functions->fTexStorage2D = (GrGLTexStorage2DProc) eglGetProcAddress("glTexStorage2D");
    functions->fUniform1f = (GrGLUniform1fProc) eglGetProcAddress("glUniform1f");
    functions->fUniform1i = (GrGLUniform1iProc) eglGetProcAddress("glUniform1i");
    functions->fUniform1fv = (GrGLUniform1fvProc) eglGetProcAddress("glUniform1fv");
    functions->fUniform1iv = (GrGLUniform1ivProc) eglGetProcAddress("glUniform1iv");
    functions->fUniform2f = (GrGLUniform2fProc) eglGetProcAddress("glUniform2f");
    functions->fUniform2i = (GrGLUniform2iProc) eglGetProcAddress("glUniform2i");
    functions->fUniform2fv = (GrGLUniform2fvProc) eglGetProcAddress("glUniform2fv");
    functions->fUniform2iv = (GrGLUniform2ivProc) eglGetProcAddress("glUniform2iv");
    functions->fUniform3f = (GrGLUniform3fProc) eglGetProcAddress("glUniform3f");
    functions->fUniform3i = (GrGLUniform3iProc) eglGetProcAddress("glUniform3i");
    functions->fUniform3fv = (GrGLUniform3fvProc) eglGetProcAddress("glUniform3fv");
    functions->fUniform3iv = (GrGLUniform3ivProc) eglGetProcAddress("glUniform3iv");
    functions->fUniform4f = (GrGLUniform4fProc) eglGetProcAddress("glUniform4f");
    functions->fUniform4i = (GrGLUniform4iProc) eglGetProcAddress("glUniform4i");
    functions->fUniform4fv = (GrGLUniform4fvProc) eglGetProcAddress("glUniform4fv");
    functions->fUniform4iv = (GrGLUniform4ivProc) eglGetProcAddress("glUniform4iv");
    functions->fUniformMatrix2fv = (GrGLUniformMatrix2fvProc) eglGetProcAddress("glUniformMatrix2fv");
    functions->fUniformMatrix3fv = (GrGLUniformMatrix3fvProc) eglGetProcAddress("glUniformMatrix3fv");
    functions->fUniformMatrix4fv = (GrGLUniformMatrix4fvProc) eglGetProcAddress("glUniformMatrix4fv");
    functions->fUnmapBuffer = (GrGLUnmapBufferProc) eglGetProcAddress("glUnmapBuffer");
    functions->fUseProgram = (GrGLUseProgramProc) eglGetProcAddress("glUseProgram");
    functions->fVertexAttrib4fv = (GrGLVertexAttrib4fvProc) eglGetProcAddress("glVertexAttrib4fv");
    functions->fVertexAttribPointer = (GrGLVertexAttribPointerProc) eglGetProcAddress("glVertexAttribPointer");
    functions->fViewport = (GrGLViewportProc) eglGetProcAddress("glViewport");

    if (extensions.has("GL_NV_path_rendering")) {
        functions->fPathCommands = (GrGLPathCommandsProc) eglGetProcAddress("glPathCommandsNV");
        functions->fPathCoords = (GrGLPathCoordsProc) eglGetProcAddress("glPathCoordsNV");
        functions->fPathSubCommands = (GrGLPathSubCommandsProc) eglGetProcAddress("glPathSubCommandsNV");
        functions->fPathSubCoords = (GrGLPathSubCoordsProc) eglGetProcAddress("glPathSubCoordsNV");
        functions->fPathString = (GrGLPathStringProc) eglGetProcAddress("glPathStringNV");
        functions->fPathGlyphs = (GrGLPathGlyphsProc) eglGetProcAddress("glPathGlyphsNV");
        functions->fPathGlyphRange = (GrGLPathGlyphRangeProc) eglGetProcAddress("glPathGlyphRangeNV");
        functions->fWeightPaths = (GrGLWeightPathsProc) eglGetProcAddress("glWeightPathsNV");
        functions->fCopyPath = (GrGLCopyPathProc) eglGetProcAddress("glCopyPathNV");
        functions->fInterpolatePaths = (GrGLInterpolatePathsProc) eglGetProcAddress("glInterpolatePathsNV");
        functions->fTransformPath = (GrGLTransformPathProc) eglGetProcAddress("glTransformPathNV");
        functions->fPathParameteriv = (GrGLPathParameterivProc) eglGetProcAddress("glPathParameterivNV");
        functions->fPathParameteri = (GrGLPathParameteriProc) eglGetProcAddress("glPathParameteriNV");
        functions->fPathParameterfv = (GrGLPathParameterfvProc) eglGetProcAddress("glPathParameterfvNV");
        functions->fPathParameterf = (GrGLPathParameterfProc) eglGetProcAddress("glPathParameterfNV");
        functions->fPathDashArray = (GrGLPathDashArrayProc) eglGetProcAddress("glPathDashArrayNV");
        functions->fGenPaths = (GrGLGenPathsProc) eglGetProcAddress("glGenPathsNV");
        functions->fDeletePaths = (GrGLDeletePathsProc) eglGetProcAddress("glDeletePathsNV");
        functions->fIsPath = (GrGLIsPathProc) eglGetProcAddress("glIsPathNV");
        functions->fPathStencilFunc = (GrGLPathStencilFuncProc) eglGetProcAddress("glPathStencilFuncNV");
        functions->fPathStencilDepthOffset = (GrGLPathStencilDepthOffsetProc) eglGetProcAddress("glPathStencilDepthOffsetNV");
        functions->fStencilFillPath = (GrGLStencilFillPathProc) eglGetProcAddress("glStencilFillPathNV");
        functions->fStencilStrokePath = (GrGLStencilStrokePathProc) eglGetProcAddress("glStencilStrokePathNV");
        functions->fStencilFillPathInstanced = (GrGLStencilFillPathInstancedProc) eglGetProcAddress("glStencilFillPathInstancedNV");
        functions->fStencilStrokePathInstanced = (GrGLStencilStrokePathInstancedProc) eglGetProcAddress("glStencilStrokePathInstancedNV");
        functions->fPathCoverDepthFunc = (GrGLPathCoverDepthFuncProc) eglGetProcAddress("glPathCoverDepthFuncNV");
        functions->fPathColorGen = (GrGLPathColorGenProc) eglGetProcAddress("glPathColorGenNV");
        functions->fPathTexGen = (GrGLPathTexGenProc) eglGetProcAddress("glPathTexGenNV");
        functions->fPathFogGen = (GrGLPathFogGenProc) eglGetProcAddress("glPathFogGenNV");
        functions->fCoverFillPath = (GrGLCoverFillPathProc) eglGetProcAddress("glCoverFillPathNV");
        functions->fCoverStrokePath = (GrGLCoverStrokePathProc) eglGetProcAddress("glCoverStrokePathNV");
        functions->fCoverFillPathInstanced = (GrGLCoverFillPathInstancedProc) eglGetProcAddress("glCoverFillPathInstancedNV");
        functions->fCoverStrokePathInstanced = (GrGLCoverStrokePathInstancedProc) eglGetProcAddress("glCoverStrokePathInstancedNV");
        functions->fGetPathParameteriv = (GrGLGetPathParameterivProc) eglGetProcAddress("glGetPathParameterivNV");
        functions->fGetPathParameterfv = (GrGLGetPathParameterfvProc) eglGetProcAddress("glGetPathParameterfvNV");
        functions->fGetPathCommands = (GrGLGetPathCommandsProc) eglGetProcAddress("glGetPathCommandsNV");
        functions->fGetPathCoords = (GrGLGetPathCoordsProc) eglGetProcAddress("glGetPathCoordsNV");
        functions->fGetPathDashArray = (GrGLGetPathDashArrayProc) eglGetProcAddress("glGetPathDashArrayNV");
        functions->fGetPathMetrics = (GrGLGetPathMetricsProc) eglGetProcAddress("glGetPathMetricsNV");
        functions->fGetPathMetricRange = (GrGLGetPathMetricRangeProc) eglGetProcAddress("glGetPathMetricRangeNV");
        functions->fGetPathSpacing = (GrGLGetPathSpacingProc) eglGetProcAddress("glGetPathSpacingNV");
        functions->fGetPathColorGeniv = (GrGLGetPathColorGenivProc) eglGetProcAddress("glGetPathColorGenivNV");
        functions->fGetPathColorGenfv = (GrGLGetPathColorGenfvProc) eglGetProcAddress("glGetPathColorGenfvNV");
        functions->fGetPathTexGeniv = (GrGLGetPathTexGenivProc) eglGetProcAddress("glGetPathTexGenivNV");
        functions->fGetPathTexGenfv = (GrGLGetPathTexGenfvProc) eglGetProcAddress("glGetPathTexGenfvNV");
        functions->fIsPointInFillPath = (GrGLIsPointInFillPathProc) eglGetProcAddress("glIsPointInFillPathNV");
        functions->fIsPointInStrokePath = (GrGLIsPointInStrokePathProc) eglGetProcAddress("glIsPointInStrokePathNV");
        functions->fGetPathLength = (GrGLGetPathLengthProc) eglGetProcAddress("glGetPathLengthNV");
        functions->fPointAlongPath = (GrGLPointAlongPathProc) eglGetProcAddress("glPointAlongPathNV");
    }

    if (extensions.has("GL_EXT_debug_marker")) {
        functions->fInsertEventMarker = (GrGLInsertEventMarkerProc) eglGetProcAddress("glInsertEventMarkerEXT");
        functions->fPushGroupMarker = (GrGLInsertEventMarkerProc) eglGetProcAddress("glPushGroupMarkerEXT");
        functions->fPopGroupMarker = (GrGLPopGroupMarkerProc) eglGetProcAddress("glPopGropuMarkerEXT");
    }

    return interface;
}

const GrGLInterface* GrGLCreateNativeInterface() {

    GrGLGetStringiProc getStringi = (GrGLGetStringiProc) eglGetProcAddress("glGetStringi");

    const char* verStr = reinterpret_cast<const char*>(glGetString(GR_GL_VERSION));
    GrGLVersion version = GrGLGetVersionFromString(verStr);
    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    GrGLExtensions extensions;
    if (!extensions.init(standard, glGetString, getStringi, glGetIntegerv)) {
        return NULL;
    }

    GrGLInterface* interface = NULL;
    if (kGLES_GrGLStandard == standard) {
        interface = create_es_interface(version, extensions);
    } else if (kGL_GrGLStandard == standard) {
        interface = create_desktop_interface(version, extensions);
    }

    if (NULL != interface) {
        interface->fExtensions.swap(&extensions);
    }

    return interface;
}
