
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLInterface.h"

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

void GrGLInitializeDefaultGLInterface() {
    GrGLInterface* defaultInterface = new GrGLInterface();

    defaultInterface->fNPOTRenderTargetSupport = kProbe_GrGLCapability;
    defaultInterface->fMinRenderTargetHeight = kProbe_GrGLCapability;
    defaultInterface->fMinRenderTargetWidth = kProbe_GrGLCapability;
    defaultInterface->fActiveTexture = glActiveTexture;
    defaultInterface->fAttachShader = glAttachShader;
    defaultInterface->fBindAttribLocation = glBindAttribLocation;
    defaultInterface->fBindBuffer = glBindBuffer;
    defaultInterface->fBindTexture = glBindTexture;
    defaultInterface->fBlendColor = glBlendColor;
    defaultInterface->fBlendFunc = glBlendFunc;
    defaultInterface->fBufferData = (GrGLBufferDataProc)glBufferData;
    defaultInterface->fBufferSubData = (GrGLBufferSubDataProc)glBufferSubData;
    defaultInterface->fClear = glClear;
    defaultInterface->fClearColor = glClearColor;
    defaultInterface->fClearStencil = glClearStencil;
    defaultInterface->fClientActiveTexture = glClientActiveTexture;
    defaultInterface->fColorMask = glColorMask;
    defaultInterface->fColorPointer = glColorPointer;
    defaultInterface->fColor4ub = glColor4ub;
    defaultInterface->fCompileShader = glCompileShader;
    defaultInterface->fCompressedTexImage2D = glCompressedTexImage2D;
    defaultInterface->fCreateProgram = glCreateProgram;
    defaultInterface->fCreateShader = glCreateShader;
    defaultInterface->fCullFace = glCullFace;
    defaultInterface->fDeleteBuffers = glDeleteBuffers;
    defaultInterface->fDeleteProgram = glDeleteProgram;
    defaultInterface->fDeleteShader = glDeleteShader;
    defaultInterface->fDeleteTextures = glDeleteTextures;
    defaultInterface->fDepthMask = glDepthMask;
    defaultInterface->fDisable = glDisable;
    defaultInterface->fDisableClientState = glDisableClientState;
    defaultInterface->fDisableVertexAttribArray =
    glDisableVertexAttribArray;
    defaultInterface->fDrawArrays = glDrawArrays;
    defaultInterface->fDrawBuffer = NULL;
    defaultInterface->fDrawBuffers = NULL;
    defaultInterface->fDrawElements = glDrawElements;
    defaultInterface->fEnable = glEnable;
    defaultInterface->fEnableClientState = glEnableClientState;
    defaultInterface->fEnableVertexAttribArray = glEnableVertexAttribArray;
    defaultInterface->fFrontFace = glFrontFace;
    defaultInterface->fGenBuffers = glGenBuffers;
    defaultInterface->fGetBufferParameteriv = glGetBufferParameteriv;
    defaultInterface->fGetError = glGetError;
    defaultInterface->fGetIntegerv = glGetIntegerv;
    defaultInterface->fGetProgramInfoLog = glGetProgramInfoLog;
    defaultInterface->fGetProgramiv = glGetProgramiv;
    defaultInterface->fGetShaderInfoLog = glGetShaderInfoLog;
    defaultInterface->fGetShaderiv = glGetShaderiv;
    defaultInterface->fGetString = glGetString;
    defaultInterface->fGenTextures = glGenTextures;
    defaultInterface->fGetUniformLocation = glGetUniformLocation;
    defaultInterface->fLineWidth = glLineWidth;
    defaultInterface->fLinkProgram = glLinkProgram;
    defaultInterface->fLoadMatrixf = glLoadMatrixf;
    defaultInterface->fMatrixMode = glMatrixMode;
    defaultInterface->fPointSize = glPointSize;
    defaultInterface->fPixelStorei = glPixelStorei;
    defaultInterface->fReadBuffer = NULL;
    defaultInterface->fReadPixels = glReadPixels;
    defaultInterface->fScissor = glScissor;
    defaultInterface->fShadeModel = glShadeModel;
    defaultInterface->fShaderSource = glShaderSource;
    defaultInterface->fStencilFunc = glStencilFunc;
    defaultInterface->fStencilFuncSeparate = glStencilFuncSeparate;
    defaultInterface->fStencilMask = glStencilMask;
    defaultInterface->fStencilMaskSeparate = glStencilMaskSeparate;
    defaultInterface->fStencilOp = glStencilOp;
    defaultInterface->fStencilOpSeparate = glStencilOpSeparate;
    defaultInterface->fTexCoordPointer = glTexCoordPointer;
    defaultInterface->fTexEnvi = glTexEnvi;
    // mac uses GLenum for internalFormat param (non-standard)
    // amounts to int vs. uint.
    defaultInterface->fTexImage2D = (GrGLTexImage2DProc)glTexImage2D;
    defaultInterface->fTexParameteri = glTexParameteri;
    defaultInterface->fTexSubImage2D = glTexSubImage2D;
    defaultInterface->fUniform1f = glUniform1f;
    defaultInterface->fUniform1i = glUniform1i;
    defaultInterface->fUniform1fv = glUniform1fv;
    defaultInterface->fUniform1iv = glUniform1iv;
    defaultInterface->fUniform2f = glUniform2f;
    defaultInterface->fUniform2i = glUniform2i;
    defaultInterface->fUniform2fv = glUniform2fv;
    defaultInterface->fUniform2iv = glUniform2iv;
    defaultInterface->fUniform3f = glUniform3f;
    defaultInterface->fUniform3i = glUniform3i;
    defaultInterface->fUniform3fv = glUniform3fv;
    defaultInterface->fUniform3iv = glUniform3iv;
    defaultInterface->fUniform4f = glUniform4f;
    defaultInterface->fUniform4i = glUniform4i;
    defaultInterface->fUniform4fv = glUniform4fv;
    defaultInterface->fUniform4iv = glUniform4iv;
    defaultInterface->fUniform4fv = glUniform4fv;
    defaultInterface->fUniformMatrix2fv = glUniformMatrix2fv;
    defaultInterface->fUniformMatrix3fv = glUniformMatrix3fv;
    defaultInterface->fUniformMatrix4fv = glUniformMatrix4fv;
    defaultInterface->fUseProgram = glUseProgram;
    defaultInterface->fVertexAttrib4fv = glVertexAttrib4fv;
    defaultInterface->fVertexAttribPointer = glVertexAttribPointer;
    defaultInterface->fVertexPointer = glVertexPointer;
    defaultInterface->fViewport = glViewport;
    defaultInterface->fGenFramebuffers = glGenFramebuffers;
    defaultInterface->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
    defaultInterface->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
    defaultInterface->fBindFramebuffer = glBindFramebuffer;
    defaultInterface->fFramebufferTexture2D = glFramebufferTexture2D;
    defaultInterface->fCheckFramebufferStatus = glCheckFramebufferStatus;
    defaultInterface->fDeleteFramebuffers = glDeleteFramebuffers;
    defaultInterface->fRenderbufferStorage = glRenderbufferStorage;
    defaultInterface->fGenRenderbuffers = glGenRenderbuffers;
    defaultInterface->fDeleteRenderbuffers = glDeleteRenderbuffers;
    defaultInterface->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
    defaultInterface->fBindRenderbuffer = glBindRenderbuffer;
       
#if GL_OES_mapbuffer
    defaultInterface->fMapBuffer = glMapBufferOES;
    defaultInterface->fUnmapBuffer = glUnmapBufferOES;
#endif
        
#if GL_APPLE_framebuffer_multisample
    defaultInterface->fRenderbufferStorageMultisample = glRenderbufferStorageMultisampleAPPLE;
    defaultInterface->fResolveMultisampleFramebuffer = glResolveMultisampleFramebufferAPPLE;
#endif
    defaultInterface->fBindFragDataLocationIndexed = NULL;
        
    defaultInterface->fBindingsExported = (GrGLBinding)(kES2_GrGLBinding | kES1_GrGLBinding);

    GrGLSetDefaultGLInterface(defaultInterface)->unref();
}