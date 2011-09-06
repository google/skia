// Modified from chromium/src/webkit/glue/gl_bindings_skia_cmd_buffer.cc

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GrGLInterface.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

void GrGLInitializeDefaultGLInterface() {
    GrGLInterface* glInterface = new GrGLInterface;

    glInterface->fBindingsExported = kES2_GrGLBinding;
    glInterface->fActiveTexture = glActiveTexture;
    glInterface->fAttachShader = glAttachShader;
    glInterface->fBindAttribLocation = glBindAttribLocation;
    glInterface->fBindBuffer = glBindBuffer;
    glInterface->fBindTexture = glBindTexture;
    glInterface->fBlendColor = glBlendColor;
    glInterface->fBlendFunc = glBlendFunc;
    glInterface->fBufferData = glBufferData;
    glInterface->fBufferSubData = glBufferSubData;
    glInterface->fClear = glClear;
    glInterface->fClearColor = glClearColor;
    glInterface->fClearStencil = glClearStencil;
    glInterface->fColorMask = glColorMask;
    glInterface->fCompileShader = glCompileShader;
    glInterface->fCompressedTexImage2D = glCompressedTexImage2D;
    glInterface->fCreateProgram = glCreateProgram;
    glInterface->fCreateShader = glCreateShader;
    glInterface->fCullFace = glCullFace;
    glInterface->fDeleteBuffers = glDeleteBuffers;
    glInterface->fDeleteProgram = glDeleteProgram;
    glInterface->fDeleteShader = glDeleteShader;
    glInterface->fDeleteTextures = glDeleteTextures;
    glInterface->fDepthMask = glDepthMask;
    glInterface->fDisable = glDisable;
    glInterface->fDisableVertexAttribArray = glDisableVertexAttribArray;
    glInterface->fDrawArrays = glDrawArrays;
    glInterface->fDrawElements = glDrawElements;
    glInterface->fEnable = glEnable;
    glInterface->fEnableVertexAttribArray = glEnableVertexAttribArray;
    glInterface->fFrontFace = glFrontFace;
    glInterface->fGenBuffers = glGenBuffers;
    glInterface->fGenTextures = glGenTextures;
    glInterface->fGetBufferParameteriv = glGetBufferParameteriv;
    glInterface->fGetError = glGetError;
    glInterface->fGetIntegerv = glGetIntegerv;
    glInterface->fGetProgramInfoLog = glGetProgramInfoLog;
    glInterface->fGetProgramiv = glGetProgramiv;
    glInterface->fGetShaderInfoLog = glGetShaderInfoLog;
    glInterface->fGetShaderiv = glGetShaderiv;
    glInterface->fGetString = glGetString;
    glInterface->fGetUniformLocation = glGetUniformLocation;
    glInterface->fLineWidth = glLineWidth;
    glInterface->fLinkProgram = glLinkProgram;
    glInterface->fPixelStorei = glPixelStorei;
    glInterface->fReadPixels = glReadPixels;
    glInterface->fScissor = glScissor;
    glInterface->fShaderSource = glShaderSource;
    glInterface->fStencilFunc = glStencilFunc;
    glInterface->fStencilFuncSeparate = glStencilFuncSeparate;
    glInterface->fStencilMask = glStencilMask;
    glInterface->fStencilMaskSeparate = glStencilMaskSeparate;
    glInterface->fStencilOp = glStencilOp;
    glInterface->fStencilOpSeparate = glStencilOpSeparate;
    glInterface->fTexImage2D = glTexImage2D;
    glInterface->fTexParameteri = glTexParameteri;
    glInterface->fTexSubImage2D = glTexSubImage2D;
    glInterface->fUniform1f = glUniform1f;
    glInterface->fUniform1i = glUniform1i;
    glInterface->fUniform1fv = glUniform1fv;
    glInterface->fUniform1iv = glUniform1iv;
    glInterface->fUniform2f = glUniform2f;
    glInterface->fUniform2i = glUniform2i;
    glInterface->fUniform2fv = glUniform2fv;
    glInterface->fUniform2iv = glUniform2iv;
    glInterface->fUniform3f = glUniform3f;
    glInterface->fUniform3i = glUniform3i;
    glInterface->fUniform3fv = glUniform3fv;
    glInterface->fUniform3iv = glUniform3iv;
    glInterface->fUniform4f = glUniform4f;
    glInterface->fUniform4i = glUniform4i;
    glInterface->fUniform4fv = glUniform4fv;
    glInterface->fUniform4iv = glUniform4iv;
    glInterface->fUniformMatrix2fv = glUniformMatrix2fv;
    glInterface->fUniformMatrix3fv = glUniformMatrix3fv;
    glInterface->fUniformMatrix4fv = glUniformMatrix4fv;
    glInterface->fUseProgram = glUseProgram;
    glInterface->fVertexAttrib4fv = glVertexAttrib4fv;
    glInterface->fVertexAttribPointer = glVertexAttribPointer;
    glInterface->fViewport = glViewport;
    glInterface->fBindFramebuffer = glBindFramebuffer;
    glInterface->fBindRenderbuffer = glBindRenderbuffer;
    glInterface->fCheckFramebufferStatus = glCheckFramebufferStatus;
    glInterface->fDeleteFramebuffers = glDeleteFramebuffers;
    glInterface->fDeleteRenderbuffers = glDeleteRenderbuffers;
    glInterface->fFramebufferRenderbuffer = glFramebufferRenderbuffer;
    glInterface->fFramebufferTexture2D = glFramebufferTexture2D;
    glInterface->fGenFramebuffers = glGenFramebuffers;
    glInterface->fGenRenderbuffers = glGenRenderbuffers;
    glInterface->fGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameteriv;
    glInterface->fGetRenderbufferParameteriv = glGetRenderbufferParameteriv;
    glInterface->fRenderbufferStorage = glRenderbufferStorage;
#if GL_OES_mapbuffer
    glInterface->fMapBuffer = glMapBufferOES;
    glInterface->fUnmapBuffer = glUnmapBufferOES;
#endif
    GrGLSetDefaultGLInterface(glInterface)->unref();
}
