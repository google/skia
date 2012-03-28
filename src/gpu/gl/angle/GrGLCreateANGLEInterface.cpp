
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "EGL/egl.h"

const GrGLInterface* GrGLCreateANGLEInterface() {
    static SkAutoTUnref<GrGLInterface> glInterface;
    if (!glInterface.get()) {
        GrGLInterface* interface = new GrGLInterface;
        glInterface.reset(interface);
        interface->fBindingsExported = kES2_GrGLBinding;
        interface->fActiveTexture = angle::glActiveTexture;
        interface->fAttachShader = angle::glAttachShader;
        interface->fBindAttribLocation = angle::glBindAttribLocation;
        interface->fBindBuffer = angle::glBindBuffer;
        interface->fBindTexture = angle::glBindTexture;
        interface->fBlendColor = angle::glBlendColor;
        interface->fBlendFunc = angle::glBlendFunc;
        interface->fBufferData = angle::glBufferData;
        interface->fBufferSubData = angle::glBufferSubData;
        interface->fClear = angle::glClear;
        interface->fClearColor = angle::glClearColor;
        interface->fClearStencil = angle::glClearStencil;
        interface->fColorMask = angle::glColorMask;
        interface->fCompileShader = angle::glCompileShader;
        interface->fCompressedTexImage2D = angle::glCompressedTexImage2D;
        interface->fCreateProgram = angle::glCreateProgram;
        interface->fCreateShader = angle::glCreateShader;
        interface->fCullFace = angle::glCullFace;
        interface->fDeleteBuffers = angle::glDeleteBuffers;
        interface->fDeleteProgram = angle::glDeleteProgram;
        interface->fDeleteShader = angle::glDeleteShader;
        interface->fDeleteTextures = angle::glDeleteTextures;
        interface->fDepthMask = angle::glDepthMask;
        interface->fDisable = angle::glDisable;
        interface->fDisableVertexAttribArray = angle::glDisableVertexAttribArray;
        interface->fDrawArrays = angle::glDrawArrays;
        interface->fDrawElements = angle::glDrawElements;
        interface->fEnable = angle::glEnable;
        interface->fEnableVertexAttribArray = angle::glEnableVertexAttribArray;
        interface->fFinish = angle::glFinish;
        interface->fFlush = angle::glFlush;
        interface->fFrontFace = angle::glFrontFace;
        interface->fGenBuffers = angle::glGenBuffers;
        interface->fGenTextures = angle::glGenTextures;
        interface->fGetBufferParameteriv = angle::glGetBufferParameteriv;
        interface->fGetError = angle::glGetError;
        interface->fGetIntegerv = angle::glGetIntegerv;
        interface->fGetProgramInfoLog = angle::glGetProgramInfoLog;
        interface->fGetProgramiv = angle::glGetProgramiv;
        interface->fGetShaderInfoLog = angle::glGetShaderInfoLog;
        interface->fGetShaderiv = angle::glGetShaderiv;
        interface->fGetString = angle::glGetString;
        interface->fGetUniformLocation = angle::glGetUniformLocation;
        interface->fLineWidth = angle::glLineWidth;
        interface->fLinkProgram = angle::glLinkProgram;
        interface->fPixelStorei = angle::glPixelStorei;
        interface->fReadPixels = angle::glReadPixels;
        interface->fScissor = angle::glScissor;
        interface->fShaderSource = angle::glShaderSource;
        interface->fStencilFunc = angle::glStencilFunc;
        interface->fStencilFuncSeparate = angle::glStencilFuncSeparate;
        interface->fStencilMask = angle::glStencilMask;
        interface->fStencilMaskSeparate = angle::glStencilMaskSeparate;
        interface->fStencilOp = angle::glStencilOp;
        interface->fStencilOpSeparate = angle::glStencilOpSeparate;
        interface->fTexImage2D = angle::glTexImage2D;
        interface->fTexParameteri = angle::glTexParameteri;
        interface->fTexSubImage2D = angle::glTexSubImage2D;
#if GL_ARB_texture_storage
        interface->fTexStorage2D = angle::glTexStorage2D;
#elif GL_EXT_texture_storage
        interface->fTexStorage2D = angle::glTexStorage2DEXT;
#endif
        interface->fUniform1f = angle::glUniform1f;
        interface->fUniform1i = angle::glUniform1i;
        interface->fUniform1fv = angle::glUniform1fv;
        interface->fUniform1iv = angle::glUniform1iv;
        interface->fUniform2f = angle::glUniform2f;
        interface->fUniform2i = angle::glUniform2i;
        interface->fUniform2fv = angle::glUniform2fv;
        interface->fUniform2iv = angle::glUniform2iv;
        interface->fUniform3f = angle::glUniform3f;
        interface->fUniform3i = angle::glUniform3i;
        interface->fUniform3fv = angle::glUniform3fv;
        interface->fUniform3iv = angle::glUniform3iv;
        interface->fUniform4f = angle::glUniform4f;
        interface->fUniform4i = angle::glUniform4i;
        interface->fUniform4fv = angle::glUniform4fv;
        interface->fUniform4iv = angle::glUniform4iv;
        interface->fUniformMatrix2fv = angle::glUniformMatrix2fv;
        interface->fUniformMatrix3fv = angle::glUniformMatrix3fv;
        interface->fUniformMatrix4fv = angle::glUniformMatrix4fv;
        interface->fUseProgram = angle::glUseProgram;
        interface->fVertexAttrib4fv = angle::glVertexAttrib4fv;
        interface->fVertexAttribPointer = angle::glVertexAttribPointer;
        interface->fViewport = angle::glViewport;
        interface->fBindFramebuffer = angle::glBindFramebuffer;
        interface->fBindRenderbuffer = angle::glBindRenderbuffer;
        interface->fCheckFramebufferStatus = angle::glCheckFramebufferStatus;
        interface->fDeleteFramebuffers = angle::glDeleteFramebuffers;
        interface->fDeleteRenderbuffers = angle::glDeleteRenderbuffers;
        interface->fFramebufferRenderbuffer = angle::glFramebufferRenderbuffer;
        interface->fFramebufferTexture2D = angle::glFramebufferTexture2D;
        interface->fGenFramebuffers = angle::glGenFramebuffers;
        interface->fGenRenderbuffers = angle::glGenRenderbuffers;
        interface->fGetFramebufferAttachmentParameteriv = angle::glGetFramebufferAttachmentParameteriv;
        interface->fGetRenderbufferParameteriv = angle::glGetRenderbufferParameteriv;
        interface->fRenderbufferStorage = angle::glRenderbufferStorage;

        interface->fMapBuffer = (angle::PFNGLMAPBUFFEROESPROC) angle::eglGetProcAddress("glMapBufferOES");
        interface->fUnmapBuffer = (angle::PFNGLUNMAPBUFFEROESPROC) angle::eglGetProcAddress("glUnmapBufferOES");
    }
    glInterface.get()->ref();
    return glInterface.get();
}