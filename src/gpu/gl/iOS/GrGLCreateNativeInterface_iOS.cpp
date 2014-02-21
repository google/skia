
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

const GrGLInterface* GrGLCreateNativeInterface() {
    GrGLInterface* interface = SkNEW(GrGLInterface);

    GrGLInterface::Functions* functions = &interface->fFunctions;

    functions->fActiveTexture = glActiveTexture;
    functions->fAttachShader = glAttachShader;
    functions->fBindAttribLocation = glBindAttribLocation;
    functions->fBindBuffer = glBindBuffer;
    functions->fBindTexture = glBindTexture;
    functions->fBlendColor = glBlendColor;
    functions->fBlendFunc = glBlendFunc;
    functions->fBufferData = (GrGLBufferDataProc)glBufferData;
    functions->fBufferSubData = (GrGLBufferSubDataProc)glBufferSubData;
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
    functions->fDepthMask = glDepthMask;
    functions->fDisable = glDisable;
    functions->fDisableVertexAttribArray = glDisableVertexAttribArray;
    functions->fDrawArrays = glDrawArrays;
    functions->fDrawBuffer = NULL;
    functions->fDrawBuffers = NULL;
    functions->fDrawElements = glDrawElements;
    functions->fEnable = glEnable;
    functions->fEnableVertexAttribArray = glEnableVertexAttribArray;
    functions->fFinish = glFinish;
    functions->fFlush = glFlush;
    functions->fFrontFace = glFrontFace;
    functions->fGenBuffers = glGenBuffers;
    functions->fGenerateMipmap = glGenerateMipmap;
    functions->fGetBufferParameteriv = glGetBufferParameteriv;
    functions->fGetError = glGetError;
    functions->fGetIntegerv = glGetIntegerv;
    functions->fGetProgramInfoLog = glGetProgramInfoLog;
    functions->fGetProgramiv = glGetProgramiv;
    functions->fGetShaderInfoLog = glGetShaderInfoLog;
    functions->fGetShaderiv = glGetShaderiv;
    functions->fGetString = glGetString;
    functions->fGenTextures = glGenTextures;
    functions->fGetUniformLocation = glGetUniformLocation;
    functions->fLineWidth = glLineWidth;
    functions->fLinkProgram = glLinkProgram;
    functions->fPixelStorei = glPixelStorei;
    functions->fReadBuffer = NULL;
    functions->fReadPixels = glReadPixels;
    functions->fScissor = glScissor;
    functions->fShaderSource = glShaderSource;
    functions->fStencilFunc = glStencilFunc;
    functions->fStencilFuncSeparate = glStencilFuncSeparate;
    functions->fStencilMask = glStencilMask;
    functions->fStencilMaskSeparate = glStencilMaskSeparate;
    functions->fStencilOp = glStencilOp;
    functions->fStencilOpSeparate = glStencilOpSeparate;
    // mac uses GLenum for internalFormat param (non-standard)
    // amounts to int vs. uint.
    functions->fTexImage2D = (GrGLTexImage2DProc)glTexImage2D;
#if GL_ARB_texture_storage
    functions->fTexStorage2D = glTexStorage2D;
#elif GL_EXT_texture_storage
    functions->fTexStorage2D = glTexStorage2DEXT;
#endif
#if GL_EXT_discard_framebuffer
    functions->fDiscardFramebuffer = glDiscardFramebufferEXT;
#endif
    functions->fTexParameteri = glTexParameteri;
    functions->fTexParameteriv = glTexParameteriv;
    functions->fTexSubImage2D = glTexSubImage2D;
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
    functions->fUniform4fv = glUniform4fv;
    functions->fUniformMatrix2fv = glUniformMatrix2fv;
    functions->fUniformMatrix3fv = glUniformMatrix3fv;
    functions->fUniformMatrix4fv = glUniformMatrix4fv;
    functions->fUseProgram = glUseProgram;
    functions->fVertexAttrib4fv = glVertexAttrib4fv;
    functions->fVertexAttribPointer = glVertexAttribPointer;
    functions->fViewport = glViewport;
    functions->fGenFramebuffers = glGenFramebuffers;
    functions->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
    functions->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
    functions->fBindFramebuffer = glBindFramebuffer;
    functions->fFramebufferTexture2D = glFramebufferTexture2D;
    functions->fCheckFramebufferStatus = glCheckFramebufferStatus;
    functions->fDeleteFramebuffers = glDeleteFramebuffers;
    functions->fRenderbufferStorage = glRenderbufferStorage;
    functions->fGenRenderbuffers = glGenRenderbuffers;
    functions->fDeleteRenderbuffers = glDeleteRenderbuffers;
    functions->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
    functions->fBindRenderbuffer = glBindRenderbuffer;

#if GL_OES_mapbuffer
    functions->fMapBuffer = glMapBufferOES;
    functions->fUnmapBuffer = glUnmapBufferOES;
#endif

#if GL_APPLE_framebuffer_multisample
    functions->fRenderbufferStorageMultisample = glRenderbufferStorageMultisampleAPPLE;
    functions->fResolveMultisampleFramebuffer = glResolveMultisampleFramebufferAPPLE;
#endif

#if GL_OES_vertex_array_object
    functions->fBindVertexArray = glBindVertexArrayOES;
    functions->fDeleteVertexArrays = glDeleteVertexArraysOES;
    functions->fGenVertexArrays = glGenVertexArraysOES;
#endif

#if GL_EXT_debug_marker
    functions->fInsertEventMarker = glInsertEventMarkerEXT;
    functions->fPushGroupMarker = glPushGroupMarkerEXT;
    functions->fPopGroupMarker = glPopGroupMarkerEXT;
#endif

    interface->fStandard = kGLES_GrGLStandard;
    interface->fExtensions.init(kGLES_GrGLStandard, glGetString, NULL, glGetIntegerv);

    return interface;
}
